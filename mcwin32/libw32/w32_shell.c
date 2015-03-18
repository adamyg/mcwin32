/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 shell and sub-process support
 *
 * Copyright (c) 2007, 2012 - 2015 Adam Young.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * The Midnight Commander is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained 
 * online at http://www.opengroup.org/unix/online.html.
 * ==end==
 */

#include "win32_internal.h"
#include "win32_child.h"

#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>

struct procdata {
    int                 type;
    DWORD               dwProcessId;
    HANDLE              hInput;
    HANDLE              hOutput;
    HANDLE              hError;
};

typedef struct {
    const char*         what;
    HANDLE              hPipe;
    HANDLE              hDupPipe;
    int                 fd;
} Redirect_t;


static const char *     OutDirect(const char *path, int *append);
static void             ShellCleanup(void *p);

static int              Dup(HANDLE old, HANDLE *dup, BOOL inherit);
static int              Pipe(HANDLE *read, HANDLE *write);
static void             Close(HANDLE handle);
static void             Close2(HANDLE handle, const char *desc);

static int              StartRedirectThread(const char *what, HANDLE hPipe, int fd, HANDLE hDupPipe);
static DWORD WINAPI     RedirectThread(LPVOID p);

static void             DisplayError(HANDLE hOutput, const char *pszAPI, const char *args);
static void             InternalError(const char *pszAPI);


/*
 *  w32_shell ---
 *      System specfic shell interface.
 */
int
w32_shell(const char *shell, const char *cmd,
    const char *fstdin, const char *fstdout, const char *fstderr)
{
    static const char * sharg[] = {             // shell arguments
            "/C",       // command
            "/k"        // interactive
            };
    char *slash, *shname = WIN32_STRDUP(shell ? shell : w32_getshell());
    int xstdout = FALSE, xstderr = FALSE;       // mode (TRUE == append)
    SECURITY_ATTRIBUTES sa;
    HANDLE hInFile, hOutFile, hErrFile;
    struct procdata pd = {0};
    win32_spawn_t args = {0};
    const char *argv[4] = {0};
    HANDLE hProc = 0;
    int interactive = 0;
    int status = 0;

    // sync or async
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;                   // inherited

    fstdout = OutDirect(fstdout, &xstdout);
    fstderr = OutDirect(fstderr, &xstderr);

    // redirection
    hInFile = hOutFile = hErrFile = INVALID_HANDLE_VALUE;

    if (fstdin) {                               // O_RDONLY
        hInFile = CreateFile(fstdin, GENERIC_READ,
                        0, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    if (fstdout) {
        if (! xstdout)  {                       // O_RDWR|O_CREAT|O_TRUNC
            hOutFile = CreateFile(fstdout, GENERIC_READ | GENERIC_WRITE,
                            0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        } else {                                // O_RDWR|O_CREAT|O_APPEND
            hOutFile = CreateFile(fstdout, GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
                            0, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    }

    if (fstderr) {
        if (! xstderr)  {                       // O_RDWR|O_CREAT|O_TRUNC
            hErrFile = CreateFile(fstderr, GENERIC_READ | GENERIC_WRITE,
                            0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        } else {                                // O_RDWR|O_CREAT|O_APPEND
            hErrFile = CreateFile(fstderr, GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
                            0, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }

    } else if (fstdout) {
        if (! xstdout)  {                       // O_RDWR|O_CREAT|O_TRUNC
            hErrFile = CreateFile(fstdout, GENERIC_READ | GENERIC_WRITE,
                            0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        } else {                                // O_RDWR|O_CREAT|O_APPEND
            hErrFile = CreateFile(fstdout, GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
                            0, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    }

                                                // stdin
    if ((pd.hInput = hInFile) == INVALID_HANDLE_VALUE) {
        if (! Dup(GetStdHandle(STD_INPUT_HANDLE), &pd.hInput, TRUE)) {
            InternalError("shell: dup (stdin)");
        }
    }

                                                // stdout
    if ((pd.hOutput = hOutFile) == INVALID_HANDLE_VALUE) {
        if (! Dup(GetStdHandle(STD_OUTPUT_HANDLE), &pd.hOutput, TRUE)) {
            InternalError("shell: dup (stdout)");
        }
    }

                                                // stderr
    if ((pd.hError = hErrFile) == INVALID_HANDLE_VALUE) {
        if (! Dup(GetStdHandle(STD_ERROR_HANDLE), &pd.hError, TRUE)) {
            InternalError("shell: dup (stderr)");
        }

    } else {
        // Create a duplicate of the output (file) handle for the
        // std error write handle. This is necessary in case the
        // child application closes one of its std output handles.
        //
        if (! Dup(hOutFile, &pd.hError, TRUE)) {
            InternalError("shell: dup (fileout)");
        }
    }

    // command or interactive
    if (w32_iscommand(shname)) {
        slash = shname - 1;
        while ((slash = strchr(slash + 1, XSLASHCHAR)) != NULL) {
            *slash = SLASHCHAR;                 // convert slashes
        }
    }

    if (NULL == cmd || !*cmd) {
        ++interactive;                          // interactive if no cmd
    }

    // create child process
    (void)memset(&args, 0, sizeof(args));
    argv[0] = shname;
    argv[1] = sharg[ interactive ];
    argv[2] = cmd;
    argv[3] = NULL;
    args.argv = argv;
    args._dwFlags = 0;

    if (0 == (hProc = w32_child_exec(&args, pd.hInput, pd.hOutput, pd.hError))) {
        ShellCleanup((void *)&pd);
        status = -1;

    } else {
        ShellCleanup((void *)&pd);
        (void) w32_waitpid((int) hProc, &status, 0);
    }

    free(shname);
    return status;
}


static const char *
OutDirect(const char *path, int *append)
{
    *append = FALSE;
    if (path) {
        if ('>' == *path) {                     // ">name"
            ++path;
            if ('>' == *path) {                 // ">>name"
                *append = TRUE;
                ++path;
            }
        }
    }
    return path;
}


static void
ShellCleanup(void *p)
{
    struct procdata *pd = (struct procdata *)p;

    Close(pd->hInput);
    Close(pd->hOutput);
    Close(pd->hError);
}


/*
 *  Spawn -- spawn a child process
 *
 *  Parameters:
 *      Stdout -        [in]  Process 'stdout' file descriptor.
 *      Stderr -        [in]  Optional, process 'stderr' file descritor.
 *      *Stdin -        [out] Storage to processes 'stdin' pipe.
 */
int
w32_spawn(
    win32_spawn_t *args, int Stdout, int Stderr, int *Stdin)
{
    HANDLE hOutputReadTmp = 0, hOutputRead, hOutputWrite;
    HANDLE hErrorReadTmp = 0, hErrorRead, hErrorWrite;
    HANDLE hInputWriteTmp = 0, hInputRead, hInputWrite;
    HANDLE hProc = 0;

    //  Clear result
    //
    *Stdin = -1;                                // file descriptors

    //  Create the child output pipe.
    //
    if (! Pipe(&hOutputReadTmp, &hOutputWrite)) {
         InternalError("createPipe (stdout)");
    }

    //  Either,
    //      Create a duplicate of the output write handle for the std error write handle. This is
    //      necessary in case the child application closes one of its std output handles.
    //  or,
    //      Create stderr pipe, if stderr redirection is required.
    //
    if (Stderr == -1) {                         // no 'stderr' redirection
        if (! Dup(hOutputWrite, &hErrorWrite, TRUE)) {
            InternalError("DuplicateHandle (stdout)");
        }

    } else {                                    // stderr redirection
        if (! Pipe(&hErrorReadTmp, &hErrorWrite)) {
            InternalError("createPipe (stderr)");
        }
    }

    //  Create the child input pipe.
    //
    if (! Pipe(&hInputRead, &hInputWriteTmp)) {
        InternalError("Pipe");
    }

    //  Create new output read handle and the input write handles. Set the Properties to FALSE.
    //  Otherwise, the child inherits the properties and, as a result, non-closeable handles to
    //  the pipes are created.
    //
                                                // stdout
    if (! Dup(hOutputReadTmp, &hOutputRead, FALSE)) {
        InternalError("DupliateHandle (stdout)");  
    }

    if (Stderr >= 0) {                          // stderr
        if (! Dup(hErrorReadTmp, &hErrorRead, FALSE)) {
            InternalError("DupliateHandle (stderr)");
        }
    } else {
        hErrorRead = 0;
    }

                                                // stdin
    if (! Dup(hInputWriteTmp, &hInputWrite, FALSE)) {
        InternalError("DupliateHandle (stdin)");
    } else {
        *Stdin = _open_osfhandle((intptr_t) hInputWrite, _O_NOINHERIT);
    }


    //  Close inheritable copies of the handles you do not want to be inherited.
    //
    if (! CloseHandle(hOutputReadTmp)) {
        InternalError("closeHandle (stdput1)");
    }
    if (Stderr >= 0) {
        if (!CloseHandle(hErrorReadTmp)) {
            InternalError("closeHandle (stderr1)");
        }
    }
    if (!CloseHandle(hInputWriteTmp)) {
        InternalError("closeHandle (stdin1)");
    }

    //  Create child process.
    //
    hProc = w32_child_exec(args, hInputRead, hOutputWrite, hErrorWrite);

    //  Close pipe handles (do not continue to modify the parent).
    //  You need to make sure that no handles to the write end of the
    //  output pipe are maintained in this process or else the pipe will
    //  not close when the child process exits and the ReadFile will hang.
    //
    if (! CloseHandle(hOutputWrite)) InternalError("closeHandle (stdout2)");
    if (! CloseHandle(hErrorWrite)) InternalError("closeHandle (stderr2)");
    if (! CloseHandle(hInputRead)) InternalError("closeHandle (stdin2)");

    //  Launch the thread that gets the input and sends it to the child.
    //
    if (hProc) {
        StartRedirectThread("stdout", hOutputRead, Stdout, (Stderr >= 0 ? INVALID_HANDLE_VALUE : hErrorRead));
        if (Stderr >= 0) {
            StartRedirectThread("stderr", hErrorRead, Stderr, INVALID_HANDLE_VALUE);
        }

    } else {
        if (! CloseHandle(hOutputRead)) {
            InternalError("closeHandle (stdout3)");
        }
        if (Stderr >= 0) {
            if (! CloseHandle(hErrorRead)) {
                InternalError("closeHandle (stderr3)");
            }
        }
        _close(*Stdin);
        *Stdin = -1;
    }
    return (int)(hProc);
}


/*
 *  Spawn -- spawn a child process
 *
 *  Parameters:
 *    *Stdin            [in/out] Storage to processes 'stdin' pipe.
 *    *Stdout           [in/out] Storage to processes 'stdout' pipe.
 *    *Stderr           [in/out] Optional, storage to processes 'stderr' pipe.
 */
int
w32_spawn2(
    win32_spawn_t *args, int *Stdin, int *Stdout, int *Stderr)
{
    HANDLE hInputWriteTmp, hInputRead, hInputWrite;
    HANDLE hErrorReadTmp,  hErrorRead, hErrorWrite;
    HANDLE hOutputReadTmp, hOutputRead, hOutputWrite;
    HANDLE hProc;
    int in, out, err;

    // Clear result
    //
    in = out = err = -1;                        // file descriptors

    if (NULL == Stdin || NULL == Stdout) {      // must be supplied
        return -1;
    }

    // Create the child input pipe.
    //
    if (! Pipe(&hInputRead, &hInputWriteTmp)) {
        InternalError("pipe (stdin)");
    }

    // Create the child output pipe.
    //
    if (! Pipe(&hOutputReadTmp, &hOutputWrite )) {
        InternalError("pipe (stdout)");
    }

    // Create the child error pipe.
    //
    //  Either,
    //      Create a duplicate of the output write handle for the
    //      std error write handle. This is necessary in case the
    //      child application closes one of its std output handles.
    //  or
    //      Create stderr pipe, if stderr redirection is required.
    //
    if (! Stderr) {                             // no 'stderr' redirection
        if (! Dup(hOutputWrite, &hErrorWrite, TRUE)) {
            InternalError("dup (stdout)");
        }
    } else {                                    // stderr redirection
        if (! Pipe(&hErrorReadTmp, &hErrorWrite)) {
            InternalError("pipe (stderr)");
        }
    }

    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    //
    if (! Dup(hInputWriteTmp, &hInputWrite, FALSE)) InternalError("dup (stdin)");
    if (! Dup(hOutputReadTmp, &hOutputRead, FALSE)) InternalError("dup (stdout)");
    if (Stderr) {
        if (! Dup(hErrorReadTmp, &hErrorRead, FALSE)) {
            InternalError("dup (stderr)");
        }
    } else {
        hErrorRead = 0;
    }

    // Close inheritable copies of the handles you do
    // not want to be inherited.
    //
    if (! CloseHandle(hOutputReadTmp)) InternalError("close (stdput1)");
    if (Stderr) {
        if (! CloseHandle(hErrorReadTmp)) InternalError("close (stderr1)");
    }
    if (!CloseHandle(hInputWriteTmp)) InternalError("close (stdin1)");

    // Open LIBC compatible handles (if required) and launch child process
    //
    if ((*Stdin >= 0 ||
            (in  = _open_osfhandle((long)hInputWrite, _O_NOINHERIT)) >= 0) &&
        (*Stdout >= 0 ||
            (out = _open_osfhandle((long)hOutputRead, _O_NOINHERIT)) >= 0) &&
        (Stderr == NULL || *Stderr >= 0 ||
            (err = _open_osfhandle((long)hErrorRead, _O_NOINHERIT))  >= 0)) {
        hProc = w32_child_exec(args, hInputRead, hOutputWrite, hErrorWrite);
    }

    // Close pipe handles (do not continue to modify the parent).
    // You need to make sure that no handles to the write end of the
    // output pipe are maintained in this process or else the pipe will
    // not close when the child process exits and the ReadFile will hang.
    //
    if (!CloseHandle(hInputRead)) InternalError("close (stdin2)");
    if (!CloseHandle(hOutputWrite)) InternalError("close (stdout2)");
    if (!CloseHandle(hErrorWrite)) InternalError("close (stderr2)");

    // Launch the thread(s) that gets the input and sends it to the child.
    //
    //      Theses are only required if the caller supplied the out/err fds.
    //
    if (hProc) {
        if (*Stdin >= 0) {
            assert(in == -1);
        } else {
            assert(in >= 0);
            *Stdin = in;
        }

        if (*Stdout >= 0) {
            assert(out == -1);
            StartRedirectThread("stdout", hOutputRead, *Stdout,
                (Stderr ? INVALID_HANDLE_VALUE : hErrorRead) );
        } else {
            assert( out >= 0);
            *Stdout = out;
        }

        if (Stderr) {
            if (*Stderr >= 0) {
                assert( err == -1);
                StartRedirectThread("stderr",
                    hErrorRead, *Stderr, INVALID_HANDLE_VALUE);
            } else {
                assert(err >= 0);
                *Stderr = err;
            }
        } else {
            assert(err == -1);
        }

    } else {
        if (!CloseHandle(hInputWrite)) InternalError("close (stdin3)");
        if (!CloseHandle(hOutputRead)) InternalError("close (stdout3)");
        if (Stderr) {
            if (!CloseHandle(hErrorRead)) InternalError("close (stderr3)");
        }
        if (in >= 0) WIN32_CLOSE(in);
        if (out >= 0) WIN32_CLOSE(out);
        if (err >= 0) WIN32_CLOSE(err);
    }
    return (int)(hProc);
}


/*
 *  Dup ---
 *      Dup a file handle.
 */
static int
Dup(HANDLE old, HANDLE *dup, BOOL inherit)
{
    HANDLE self = GetCurrentProcess();

    if (dup == NULL || old == INVALID_HANDLE_VALUE ||
            !DuplicateHandle(self, old, self, dup, 0, inherit, DUPLICATE_SAME_ACCESS)) {
        *dup = INVALID_HANDLE_VALUE;
        return (FALSE);
    }
    return (TRUE);
}


/*
 *  Pipe ---
 *      Create a pipe.
 */
static int
Pipe(HANDLE *read, HANDLE *write)
{
    SECURITY_ATTRIBUTES sa = {0};

    sa.nLength = sizeof(sa);                    // length in bytes
    sa.bInheritHandle = TRUE;                   // the child must inherit these handles
    sa.lpSecurityDescriptor = NULL;
    if (CreatePipe(read, write, &sa, 0)) {
        return TRUE;
    }
    errno = EMFILE;                             // popen()
    return FALSE;
}


/*
 *  Close ---
 *      Close a handle.
 */
static void
Close(HANDLE handle)
{
    if (handle != INVALID_HANDLE_VALUE) {
        if (! CloseHandle(handle)) {
            InternalError("Close()");
        }
    }
}


/*
 *  Close ---
 *      Close a handle.
 */
static void
Close2(HANDLE handle, const char *desc)
{
    if (handle != INVALID_HANDLE_VALUE) {
        if (! CloseHandle(handle)) {
            InternalError(desc);
        }
    }
}


/*
 *  ReadAndHandleOutput ---
 *      Monitors handle for input.  Exits when child exits or pipe breaks.
 */
static int
StartRedirectThread(
    const char *what, HANDLE hPipe, int fd, HANDLE hDupPipe)
{
    HANDLE hThread;
    DWORD tid;
    Redirect_t *p;

    if ((p = malloc(sizeof(*p))) == NULL) {
        InternalError("malloc");
    }
    p->what = what;
    p->hPipe = hPipe;
    p->hDupPipe = hDupPipe;
    p->fd = fd;
    if ((hThread = CreateThread(NULL, 0, RedirectThread, (LPVOID)p, 0, &tid)) == 0) {
        InternalError("CreateThread");
    }
    if (!CloseHandle(hThread)) {
        InternalError("closeHandle (hThread)");
    }
    return 0;
}


static DWORD WINAPI
RedirectThread(LPVOID p)
{
    HANDLE  hPipe = ((Redirect_t *)p)->hPipe;
    int     fd = ((Redirect_t *)p)->fd;
    CHAR    buffer[ BUFSIZ ];
    DWORD   cnt;

    while (TRUE) {                              // read pipe
        if (! ReadFile(hPipe, buffer, sizeof(buffer), &cnt, NULL) || !cnt) {
            if (GetLastError() == ERROR_BROKEN_PIPE) {
                break;                          // pipe done
            }
            InternalError("ReadFile");             // .. something bad
        }

                                                // redirect write loop
        if (_write(fd, buffer, cnt) != (int)cnt) {
            break;
        }
    }

    (void) CloseHandle(hPipe);                  // close pipe handle(s)
    if ((hPipe = ((Redirect_t *)p)->hDupPipe) != INVALID_HANDLE_VALUE) {
        (void) CloseHandle(hPipe);
    }
    WIN32_CLOSE(fd);

    free((void *)p);                            // release resources
    return (0);
}


/*
 *  WaiInternalError ---
 *      Displays the error number and corresponding message.
 */
static void
DisplayError(
    HANDLE hOutput, const char *pszAPI, const char *args )
{
    DWORD   rc = GetLastError();
    LPVOID  lpvMessageBuffer;
    CHAR    szPrintBuffer[512];
    DWORD   nCharsWritten;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, rc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpvMessageBuffer, 0, NULL);

    _snprintf( szPrintBuffer, sizeof(szPrintBuffer),
        "Interna Error: %s = %d (%s).\n%s%s", pszAPI, rc, (char *)lpvMessageBuffer,
        args ? args : "", args ? "\n" : "");

    WriteConsole(hOutput, szPrintBuffer,
        lstrlen(szPrintBuffer), &nCharsWritten, NULL);

    LocalFree(lpvMessageBuffer);
}


static void
InternalError(
    const char *pszAPI)
{
    DisplayError(GetStdHandle(STD_OUTPUT_HANDLE), pszAPI, NULL);
    ExitProcess(GetLastError());
}

