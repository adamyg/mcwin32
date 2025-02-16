#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_shell_c,"$Id: w32_shell.c,v 1.22 2025/02/16 12:04:05 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 shell and sub-process support
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 * or (at your option) any later version.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#include "win32_internal.h"
#include "win32_child.h"
#include "win32_misc.h"

#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>

#if defined(_MSC_VER)
#pragma warning(disable : 4244) // function : conversion from 'xxx' to 'xxx', possible loss of data
#pragma warning(disable : 4311) // type cast : pointer truncation from 'HANDLE' to 'int'
#pragma warning(disable : 4312) // type cast : conversion from 'xxx' to 'xxx' of greater size
#endif

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

static int              ShellA(const char *shell, const char *cmd, const char *fstdin, const char *fstdout, const char *fstderr);
static int              ShellW(const wchar_t *shell, const wchar_t *cmd, const wchar_t *fstdin, const wchar_t *fstdout, const wchar_t *fstderr);
static const char *     OutDirectA(const char *path, int *append);
static const wchar_t *  OutDirectW(const wchar_t *path, int *append);
static void             ShellCleanup(void *p);

static int              IsAbsPathA(const char *path);
static int              IsAbsPathW(const wchar_t *path);

static const wchar_t *  ImportArgv(const char **argv);
static const wchar_t ** ImportEnvv(const char **envv);

static int              Dup(HANDLE old, HANDLE *dup, BOOL inherit);
static int              Pipe(HANDLE *read, HANDLE *write);
static void             Close(HANDLE handle);
static void             Close2(HANDLE handle, const char *desc);

static int              StartRedirectThread(const char *what, HANDLE hPipe, int fd, HANDLE hDupPipe);
static DWORD WINAPI     RedirectThread(LPVOID p);

static void             DisplayErrorA(HANDLE hOutput, const char *pszAPI, const char *args);
static void             DisplayErrorW(HANDLE hOutput, const wchar_t *pszAPI, const wchar_t *args);
static void             InternalError(const char *pszAPI);


/*
 *  w32_shell ---
 *      System specfic shell interface.
 */
int
w32_shell(const char *shell, const char *cmd,
    const char *fstdin, const char *fstdout, const char *fstderr)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t *wshell = NULL, *wcmd = NULL,
            *wfstdin = NULL, *wfstdout = NULL, *wfstderr = NULL;
        int ret = -1;

        if (NULL == shell || NULL != (wshell = w32_utf2wca(shell, NULL))) {
            if (NULL == cmd || NULL != (wcmd = w32_utf2wca(cmd, NULL))) {
                if (NULL == fstdin || NULL != (wfstdin = w32_utf2wca(fstdin, NULL))) {
                    if (NULL == fstdout || NULL != (wfstdout = w32_utf2wca(fstdout, NULL))) {
                        if (NULL == fstderr || NULL != (wfstderr = w32_utf2wca(fstderr, NULL))) {
                            ret = ShellW(wshell, wcmd, wfstdin, wfstdout, wfstderr);
                            free((void *)wfstderr);
                        }
                        free((void *)wfstdout);
                    }
                    free((void *)wfstdin);
                }
                free((void *)wcmd);
            }
            free((void *)wshell);
        }
        return ret;
    }
#endif  //UTF8FILENAMES

    return ShellA(shell, cmd, fstdin, fstdout, fstderr);
}


int
w32_shellA(const char *shell, const char *cmd,
    const char *fstdin, const char *fstdout, const char *fstderr)
{
    return ShellA(shell, cmd, fstdin, fstdout, fstderr);
}


int
w32_shellW(const wchar_t *shell, const wchar_t *cmd,
    const wchar_t *fstdin, const wchar_t *fstdout, const wchar_t *fstderr)
{
    return ShellW(shell, cmd, fstdin, fstdout, fstderr);
}


static int
ShellA(const char *shell, const char *cmd,
    const char *fstdin, const char *fstdout, const char *fstderr)
{
    static const char * sharg[] = {             // shell arguments
            "/C",       // command
            "/k"        // interactive
            };
    const int interactive = ((NULL == cmd || !*cmd) ? 1 : 0);
    char *slash, *shname = WIN32_STRDUP(shell && *shell ? shell : w32_getshell());
    int xstdout = FALSE, xstderr = FALSE;       // mode (TRUE == append)
    SECURITY_ATTRIBUTES sa;
    HANDLE hInFile, hOutFile, hErrFile;
    struct procdata pd = {0};
    win32_spawn_t args = {0};
    const char *argv[4] = {0};
    HANDLE hProc = 0;
    int status = 0;

    // sync or async
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;                   // inherited

    fstdout = OutDirectA(fstdout, &xstdout);
    fstderr = OutDirectA(fstderr, &xstderr);

    // redirection
    hInFile = hOutFile = hErrFile = INVALID_HANDLE_VALUE;

    if (fstdin) {                               // O_RDONLY
        hInFile = CreateFileA(fstdin, GENERIC_READ,
                        0, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    if (fstdout) {
        if (! xstdout)  {                       // O_RDWR|O_CREAT|O_TRUNC
            hOutFile = CreateFileA(fstdout, GENERIC_READ | GENERIC_WRITE,
                            0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        } else {                                // O_RDWR|O_CREAT|O_APPEND
            hOutFile = CreateFileA(fstdout, GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
                            0, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    }

    if (fstderr) {
        if (! xstderr)  {                       // O_RDWR|O_CREAT|O_TRUNC
            hErrFile = CreateFileA(fstderr, GENERIC_READ | GENERIC_WRITE,
                            0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        } else {                                // O_RDWR|O_CREAT|O_APPEND
            hErrFile = CreateFileA(fstderr, GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
                            0, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }

    } else if (fstdout) {
        if (! xstdout)  {                       // O_RDWR|O_CREAT|O_TRUNC
            hErrFile = CreateFileA(fstdout, GENERIC_READ | GENERIC_WRITE,
                            0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        } else {                                // O_RDWR|O_CREAT|O_APPEND
            hErrFile = CreateFileA(fstdout, GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
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
        // Create a duplicate of the output (file) handle for thestd error write handle.
        // This is necessary in case the child application closes one of its std output handles.
        //
        if (! Dup(hOutFile, &pd.hError, TRUE)) {
            InternalError("shell: dup (fileout)");
        }
    }

    // command or interactive
    (void)memset(&args, 0, sizeof(args));

    if (IsAbsPathA(shname))                     // abs-path
        args.arg0 = shname;

    if (w32_iscommandA(shname)) {
        slash = shname - 1;
        while ((slash = strchr(slash + 1, XSLASHCHAR)) != NULL) {
            *slash = SLASHCHAR;                 // convert slashes
        }

        if (!interactive &&                     // /C embedded
                cmd[0] == sharg[0][0] && cmd[1] == sharg[0][1]) {
            argv[0] = shname;
            argv[1] = cmd;
            argv[2] = NULL;

        } else {
            argv[0] = shname;
            argv[1] = sharg[ interactive ];     // /C or /K
            argv[2] = cmd;
            argv[3] = NULL;
        }

    } else {
        argv[0] = shname;
        argv[1] = cmd;
        argv[2] = NULL;
    }

    // create child process
    args.argv = argv;
    args._dwFlags = 0;

    if (0 == (hProc = w32_child_execA(&args, pd.hInput, pd.hOutput, pd.hError))) {
        ShellCleanup((void *)&pd);
        status = -1;

    } else {
        ShellCleanup((void *)&pd);
        (void) w32_waitpid(w32_HTOI(hProc), &status, 0);
    }

    free(shname);
    return status;
}


static int
ShellW(const wchar_t *shell, const wchar_t  *cmd,
    const wchar_t *fstdin, const wchar_t *fstdout, const wchar_t *fstderr)
{
    static const wchar_t *sharg[] = {           // shell arguments
            L"/C",      // command
            L"/k"       // interactive
            };
    const int interactive = ((NULL == cmd || !*cmd) ? 1 : 0);
    wchar_t *slash, *shname = WIN32_STRDUPW(shell && *shell ? shell : w32_getshellW());
    int xstdout = FALSE, xstderr = FALSE;       // mode (TRUE == append)
    SECURITY_ATTRIBUTES sa;
    HANDLE hInFile, hOutFile, hErrFile;
    struct procdata pd = {0};
    win32_spawnw_t args = {0};
    const wchar_t *argv[4] = {NULL};
    HANDLE hProc = 0;
    int status = 0;

    // sync or async
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;                   // inherited

    fstdout = OutDirectW(fstdout, &xstdout);
    fstderr = OutDirectW(fstderr, &xstderr);

    // redirection
    hInFile = hOutFile = hErrFile = INVALID_HANDLE_VALUE;

    if (fstdin) {                               // O_RDONLY
        hInFile = CreateFileW(fstdin, GENERIC_READ,
                        0, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    if (fstdout) {
        if (! xstdout)  {                       // O_RDWR|O_CREAT|O_TRUNC
            hOutFile = CreateFileW(fstdout, GENERIC_READ | GENERIC_WRITE,
                            0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        } else {                                // O_RDWR|O_CREAT|O_APPEND
            hOutFile = CreateFileW(fstdout, GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
                            0, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    }

    if (fstderr) {
        if (! xstderr)  {                       // O_RDWR|O_CREAT|O_TRUNC
            hErrFile = CreateFileW(fstderr, GENERIC_READ | GENERIC_WRITE,
                            0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        } else {                                // O_RDWR|O_CREAT|O_APPEND
            hErrFile = CreateFileW(fstderr, GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
                            0, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }

    } else if (fstdout) {
        if (! xstdout)  {                       // O_RDWR|O_CREAT|O_TRUNC
            hErrFile = CreateFileW(fstdout, GENERIC_READ | GENERIC_WRITE,
                            0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        } else {                                // O_RDWR|O_CREAT|O_APPEND
            hErrFile = CreateFileW(fstdout, GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
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
        // Create a duplicate of the output (file) handle for thestd error write handle.
        // This is necessary in case the child application closes one of its std output handles.
        //
        if (! Dup(hOutFile, &pd.hError, TRUE)) {
            InternalError("shell: dup (fileout)");
        }
    }

    // command or interactive
    (void)memset(&args, 0, sizeof(args));

     if (IsAbsPathW(shname))                     // abs-path
        args.arg0 = shname;

    if (w32_iscommandW(shname)) {
        slash = shname - 1;
        while ((slash =  wcschr(slash + 1, XSLASHCHAR)) != NULL) {
            *slash = SLASHCHAR;                 // convert slashes
        }

        if (!interactive &&                     // /C embedded
                cmd[0] == sharg[0][0] && cmd[1] == sharg[0][1]) {
            argv[0] = shname;
            argv[1] = cmd;
            argv[2] = NULL;

        } else {
            argv[0] = shname;
            argv[1] = sharg[ interactive ];     // /C or /K
            argv[2] = cmd;
            argv[3] = NULL;
        }

    } else {
        argv[0] = shname;
        argv[1] = cmd;
        argv[2] = NULL;
    }

    // create child process   
    args.argv = argv;
    args._dwFlags = 0;

    if (0 == (hProc = w32_child_execW(&args, pd.hInput, pd.hOutput, pd.hError))) {
        ShellCleanup((void *)&pd);
        status = -1;

    } else {
        ShellCleanup((void *)&pd);
        (void) w32_waitpid(w32_HTOI(hProc), &status, 0);
    }

    free(shname);
    return status;
}


static const char *
OutDirectA(const char *path, int *append)
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


static const wchar_t *
OutDirectW(const wchar_t *path, int *append)
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
 *  Spawn -- spawn a child process, using the specified libc handles with
 *      redirection threads piping the underlying output to them.
 *
 *  Parameters:
 *      Stdout -    [in]  Process 'stdout' file descriptor.
 *      Stderr -    [in]  Optional, process 'stderr' file descritor.
 *      Stdin -     [out] Storage to the processes 'stdin' pipe.
 *
 *  Returns:
 *      Non-zero on success, otherwise 0 on error.
 */
int
w32_spawnA(
    win32_spawn_t *args, int Stdout, int Stderr, int *Stdin)
{
    assert(Stdout >=  0);                       // non-optional
    assert(Stderr >= -1);                       // optional
    assert(Stdin);                              // output, non-optional

    if (Stdout < 0 || Stderr < -1 || NULL == Stdin) {
        return 0;
    }

    *Stdin = -1;                                // file descriptors

    return w32_spawnA2(args, Stdin, &Stdout, (Stderr >= 0 ? &Stderr : NULL));
}


int
w32_spawnW(
    win32_spawnw_t *args, int Stdout, int Stderr, int *Stdin)
{
    assert(Stdout >=  0);                       // non-optional
    assert(Stderr >= -1);                       // optional
    assert(Stdin);                              // output, non-optional

    if (Stdout < 0 || Stderr < -1 || NULL == Stdin) {
        return 0;
    }

    *Stdin = -1;                                // file descriptors

    return w32_spawnW2(args, Stdin, &Stdout, (Stderr >= 0 ? &Stderr : NULL));
}


/*
 *  Spawn -- spawn a child process, returning libc handles with
 *      redirection threads piping the underlying output to them.
 *
 *  Parameters:
 *      Stdin -     [in/out] Storage to the processes 'stdin' pipe.
 *      Stdout -    [in/out] Storage to the processes 'stdout' pipe.
 *      Stderr -    [in/out] Optional, storage to processes the 'stderr' pipe.
 *
 *  Returns:
 *      Non-zero process handle on success, otherwise 0 on error.
 */
int
w32_spawnA2(
    win32_spawn_t *args, int *Stdin, int *Stdout, int *Stderr)
{
    int in = -1, out = -1, err = -1;
    HANDLE hInputWriteTmp = 0, hInputRead = 0, hInputWrite = 0,
        hErrorReadTmp = 0, hErrorRead = 0, hErrorWrite = 0,
        hOutputReadTmp = 0, hOutputRead = 0, hOutputWrite = 0;
    HANDLE hProc = 0;

    if (NULL == Stdin || NULL == Stdout) {      // must be supplied
        return 0;
    }

    //  Create the child input pipe.
    //
    if (! Pipe(&hInputRead, &hInputWriteTmp)) {
        InternalError("pipe (stdin)");
    }

    //  Create the child output pipe.
    //
    if (! Pipe(&hOutputReadTmp, &hOutputWrite)) {
        InternalError("spawn: pipe (stdout)");
    }

    //  Create the child error pipe.
    //
    //      Either,
    //          Create a duplicate of the output write handle for the
    //          std error write handle. This is necessary in case the
    //          child application closes one of its std output handles.
    //      or
    //          Create stderr pipe, if stderr redirection is required.
    //
    if (! Stderr) {                             // no 'stderr' redirection
        if (! Dup(hOutputWrite, &hErrorWrite, TRUE)) {
            InternalError("spawn: dup (stdout)");
        }
    } else {                                    // stderr redirection
        if (! Pipe(&hErrorReadTmp, &hErrorWrite)) {
            InternalError("spawn: pipe (stderr)");
        }
    }

    //  Create new output read handle and the input write handles. Set
    //  the Properties to FALSE. Otherwise, the child inherits the
    //  properties and, as a result, non-closeable handles to the pipes
    //  are created.
    //
    if (! Dup(hInputWriteTmp, &hInputWrite, FALSE)) InternalError("spawn: dup (stdin)");
    if (! Dup(hOutputReadTmp, &hOutputRead, FALSE)) InternalError("spawn: dup (stdout)");
    if (Stderr) {
        if (! Dup(hErrorReadTmp, &hErrorRead, FALSE)) {
            InternalError("spawn: dup (stderr)");
        }
    } else {
        hErrorRead = 0;
    }

    //  Close inheritable copies of the handles you do
    //  not want to be inherited.
    //
    Close2(hOutputReadTmp, "spawn (stdput1)");
    Close2(hInputWriteTmp, "spawn (stdin1)");
    if (Stderr) {
        Close2(hErrorReadTmp, "spawn (stderr1)");
    }

    //  Open LIBC compatible handles (if required) and launch child process
    //
    if ((*Stdin  >= 0 ||
            (in  = _open_osfhandle((OSFHANDLE)hInputWrite, _O_NOINHERIT)) >= 0) &&
        (*Stdout >= 0 ||
            (out = _open_osfhandle((OSFHANDLE)hOutputRead, _O_NOINHERIT)) >= 0) &&
        (Stderr == NULL || *Stderr >= 0 ||
            (err = _open_osfhandle((OSFHANDLE)hErrorRead, _O_NOINHERIT)) >= 0)) {
        hProc = w32_child_execA(args, hInputRead, hOutputWrite, hErrorWrite);
    }

    //  Close pipe handles (do not continue to modify the parent).
    //  You need to make sure that no handles to the write end of the
    //  output pipe are maintained in this process or else the pipe will
    //  not close when the child process exits and the ReadFile will hang.
    //
    Close(hInputRead); Close(hOutputWrite); Close(hErrorWrite);

    //  Launch the thread(s) that gets the input and sends it to the child.
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
            assert(out >= 0);
            *Stdout = out;
        }

        if (Stderr) {
            if (*Stderr >= 0) {
                assert(err == -1);
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
        Close(hInputWrite);
        Close(hOutputRead);
        if (Stderr) {
            Close(hErrorRead);
        }
    }
    return w32_HTOI(hProc);
}


int
w32_spawnW2(
    win32_spawnw_t *args, int *Stdin, int *Stdout, int *Stderr)
{
    int in = -1, out = -1, err = -1;
    HANDLE hInputWriteTmp = 0, hInputRead = 0, hInputWrite = 0,
        hErrorReadTmp = 0, hErrorRead = 0, hErrorWrite = 0,
        hOutputReadTmp = 0, hOutputRead = 0, hOutputWrite = 0;
    HANDLE hProc = 0;

    if (NULL == Stdin || NULL == Stdout) {      // must be supplied
        errno = EINVAL;
        return 0;
    }

    //  Create the child input pipe.
    //
    if (! Pipe(&hInputRead, &hInputWriteTmp)) {
        InternalError("pipe (stdin)");
        errno = EINVAL;
        return 0;
    }

    //  Create the child output pipe.
    //
    if (! Pipe(&hOutputReadTmp, &hOutputWrite)) {
        InternalError("spawn: pipe (stdout)");
        errno = EINVAL;
        return 0;
    }

    //  Create the child error pipe.
    //
    //      Either,
    //          Create a duplicate of the output write handle for the
    //          std error write handle. This is necessary in case the
    //          child application closes one of its std output handles.
    //      or
    //          Create stderr pipe, if stderr redirection is required.
    //
    if (! Stderr) {                             // no 'stderr' redirection
        if (! Dup(hOutputWrite, &hErrorWrite, TRUE)) {
            InternalError("spawn: dup (stdout)");
            errno = EINVAL;
            return 0;
        }
    } else {                                    // stderr redirection
        if (! Pipe(&hErrorReadTmp, &hErrorWrite)) {
            InternalError("spawn: pipe (stderr)");
            errno = EINVAL;
            return 0;
        }
    }

    //  Create new output read handle and the input write handles. Set
    //  the Properties to FALSE. Otherwise, the child inherits the
    //  properties and, as a result, non-closeable handles to the pipes
    //  are created.
    //
    if (! Dup(hInputWriteTmp, &hInputWrite, FALSE)) InternalError("spawn: dup (stdin)");
    if (! Dup(hOutputReadTmp, &hOutputRead, FALSE)) InternalError("spawn: dup (stdout)");
    if (Stderr) {
        if (! Dup(hErrorReadTmp, &hErrorRead, FALSE)) {
            InternalError("spawn: dup (stderr)");
            errno = EINVAL;
            return 0;
        }
    } else {
        hErrorRead = 0;
    }

    //  Close inheritable copies of the handles you do
    //  not want to be inherited.
    //
    Close2(hOutputReadTmp, "spawn (stdput1)");
    Close2(hInputWriteTmp, "spawn (stdin1)");
    if (Stderr) {
        Close2(hErrorReadTmp, "spawn (stderr1)");
    }

    //  Open LIBC compatible handles (if required) and launch child process
    //
    if ((*Stdin  >= 0 ||
            (in  = _open_osfhandle((OSFHANDLE)hInputWrite, _O_NOINHERIT)) >= 0) &&
        (*Stdout >= 0 ||
            (out = _open_osfhandle((OSFHANDLE)hOutputRead, _O_NOINHERIT)) >= 0) &&
        (Stderr == NULL || *Stderr >= 0 ||
            (err = _open_osfhandle((OSFHANDLE)hErrorRead, _O_NOINHERIT)) >= 0)) {
        hProc = w32_child_execW(args, hInputRead, hOutputWrite, hErrorWrite);
    }

    //  Close pipe handles (do not continue to modify the parent).
    //  You need to make sure that no handles to the write end of the
    //  output pipe are maintained in this process or else the pipe will
    //  not close when the child process exits and the ReadFile will hang.
    //
    Close(hInputRead); Close(hOutputWrite); Close(hErrorWrite);

    //  Launch the thread(s) that gets the input and sends it to the child.
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
            assert(out >= 0);
            *Stdout = out;
        }

        if (Stderr) {
            if (*Stderr >= 0) {
                assert(err == -1);
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
        Close(hInputWrite);
        Close(hOutputRead);
        if (Stderr) {
            Close(hErrorRead);
        }
    }
    return w32_HTOI(hProc);
}


/*
 *  Exec -- exec a child process, returning native win32 handles.
 *
 *  Parameters:
 *      args -      [in/out] Spawn arguments.
 *
 *  Returns:
 *      Non-zero on success, otherwise 0 on error.
 */

int
w32_exec(win32_exec_t *args)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (args->spawn.cmd || args->spawn.argv) {
            win32_execw_t wargs = { 0 };
            const wchar_t *wcmd = NULL;
            int ret = 0;

            if ((args->spawn.cmd &&
                    NULL != (wcmd = w32_utf2wca(args->spawn.cmd, NULL))) ||
                (args->spawn.argv &&
                    NULL != (wcmd = ImportArgv(args->spawn.argv)))) {

                wargs.spawn.cmd   = wcmd;
                wargs.spawn.envv  = (args->spawn.envv ? ImportEnvv(args->spawn.envv) : NULL);
                wargs.spawn.dir   = (args->spawn.dir  ? w32_utf2wca(args->spawn.dir, NULL) : NULL);
                wargs.spawn.flags = args->spawn.flags;

                ret = w32_execW(&wargs);

                free((void *)wargs.spawn.dir);
                free((void *)wargs.spawn.envv);
                free((void *)wcmd);
            }

            args->hError  = wargs.hError;
            args->hInput  = wargs.hInput;
            args->hOutput = wargs.hOutput;
            args->hProc   = wargs.hProc;
            return ret;
        }
    }
#endif  //UTF8FILENAMES

    return w32_execA(args);
}


static int
IsAbsPathA(const char *path)
{
    if (path && *path) {
        return (ISSLASH(path[0]) || path[1] == ':');
    }
    return 0;
}


static int
IsAbsPathW(const wchar_t *path)
{
    if (path && *path) {
        return (ISSLASH(path[0]) || path[1] == ':');
    }
    return 0;
}


static const wchar_t *
ImportArgv(const char **argv)
{
    const char * const *vp;
    size_t bytes;
    char *buffer;
    int cnt, len;

    /*
     *  Allocate space for environment strings, count the number of bytes
     *  in the environment strings including nulls between strings
     */
    if (NULL == argv) {
        return NULL;
    }

    for (vp = argv, cnt = 0, len = 2 /*quotes*/ + 2 /*delim*/; *vp; ++vp) {
        const char *arg = *vp;
        if (*arg) {                             // non-empty.
            const int wlen = w32_utf2wcl(arg);
            if (wlen < 0) {
                return NULL;                    // conversion error.
            }
            len += wlen;                        // inc's nul
            ++cnt;
        }
    }

    bytes = len * sizeof(wchar_t);
    if (NULL == (buffer = (char *)calloc(bytes, 1))) {
        return NULL;
    }

    /*
     *  Build the command line by concatenating the argument strings
     *  with spaces between, and two null bytes at the end.
     */
    {   wchar_t *cursor = (wchar_t *)(buffer),
            *end = (wchar_t *)(buffer + bytes);
        int argc, wlen;

        vp = argv;
        for (argc = 0; *vp; ++argc) {
            const char *arg = *vp++;
            int quote = FALSE;

            if (*arg) {                         // non-empty.
                if (0 == argc && *arg != '"' && strchr(arg, ' ')) {
                    quote = TRUE;               // quote, contains space.
                }

                if (quote) *cursor++ = '"';
                if ((wlen = w32_utf2wc(arg, cursor, end - cursor)) <= 0) {
                    free((void *)buffer);
                    assert(FALSE);
                    return NULL;
                }
                if (0 == argc) {
                    while (*cursor) {           // convert slashs within arg0.
                        if ('/' == *cursor) *cursor = '\\';
                        ++cursor;
                    }
                } else {
                    cursor += wlen - 1 /*nul*/;
                }
                if (quote) *cursor++ = '"';

                *cursor++ = ' ';                // space delimiter.
            }
        }

        assert(cnt == argc);
        assert(cursor <= end);
        cursor[-1] = '\0';                      // remove extra delimiter.
        *cursor = '\0';                         // terminator.
    }

    return (wchar_t *) buffer;                  // result.
}


static const wchar_t **
ImportEnvv(const char **envv)
{
    const char * const *vp;
    const wchar_t **ret;
    size_t bytes;
    char *buffer;
    int cnt, len;

    /*
     *  Allocate space for environment strings, count the number of bytes
     *  in the environment strings including nulls between strings
     */
    if (NULL == envv) {
        return NULL;
    }

    for (vp = envv, cnt = 0, len = 0; *vp; ++vp) {
        const char *arg = *vp;
        if (*arg) {                             // non-empty.
            const int wlen = w32_utf2wcl(arg);
            if (wlen < 0) {
                return NULL;                    // conversion error.
            }
            len += wlen;                        // inc's nul
            ++cnt;
        }
    }

    bytes = ((cnt + 1) * sizeof(wchar_t *)) + (len * sizeof(wchar_t));
    if (NULL == (buffer = (char *)calloc(bytes, 1))) {
        return NULL;
    }

    /*
     *  Build the environment vector by importing the env collection.
     */
    ret = (const wchar_t **)buffer;

    {   wchar_t *cursor = (wchar_t *)(buffer + ((cnt + 1) * sizeof(void *))),
            *end = (wchar_t *)(buffer + bytes);
        int envc, wlen;

        vp = envv;
        assert((void *)(&ret[cnt+1]) == (void *)cursor);
        for (envc = 0; *vp; ++vp) {
            const char *arg = *vp;
            if (*arg) {                         // non-empty.
                if ((wlen = w32_utf2wc(arg, cursor, end - cursor)) <= 0) {
                    free((void *)buffer);
                    assert(FALSE);
                    return NULL;
                }
                ret[envc++] = cursor;
                cursor += wlen;
            }
        }

        assert(cnt == envc);
        assert(cursor == end);
        ret[envc] = NULL;
    }

    return ret;                                 // result.
}


/*
 *  Exec -- exec a child process, returning native win32 handles.
 *
 *  Parameters:
 *      args -      [in/out] Spawn arguments.
 *
 *  Returns:
 *      Non-zero on success, otherwise 0 on error.
 */

int
w32_execA(win32_exec_t *args)
{
    HANDLE hInputWriteTmp = 0, hInputRead = 0, hInputWrite = 0,
        hErrorReadTmp = 0, hErrorRead = 0, hErrorWrite = 0,
        hOutputReadTmp = 0, hOutputRead = 0, hOutputWrite = 0;

    // Non-optional arguments.
    if (NULL == args ||
            (NULL == args->spawn.cmd && NULL == args->spawn.argv)) {
        errno = EINVAL;
        return -1;
    }

    // Create the child input/out/err pipes.
    if (! Pipe(&hInputRead, &hInputWriteTmp) ||
            ! Pipe(&hOutputReadTmp, &hOutputWrite) ||
            ! Pipe(&hErrorReadTmp, &hErrorWrite)) {
        InternalError("exec: pipe");
        goto einval;
    }

    // Create new output read handle and the input write handles, set as non inheritable.
    // Otherwise, the child inherits the properties and, as a result, non-closeable handles to the pipes are created.
    if (! Dup(hInputWriteTmp, &hInputWrite, FALSE) ||
            ! Dup(hOutputReadTmp, &hOutputRead, FALSE) ||
            ! Dup(hErrorReadTmp, &hErrorRead, FALSE)) {
        InternalError("exec: dup");
        goto einval;
    }
    Close(hOutputReadTmp); Close(hInputWriteTmp); Close(hErrorReadTmp);

    // Execute child.
    args->hProc = w32_child_execA(&args->spawn, hInputRead, hOutputWrite, hErrorWrite);

    // Close pipe handles.
    Close(hInputRead); Close(hOutputWrite); Close(hErrorWrite);

    // Completion.
    if (0 == args->hProc) {
        Close(hInputWrite); Close(hOutputRead); Close(hErrorRead);
        return 0;
    }

    args->hInput  = hInputWrite;
    args->hOutput = hOutputRead;
    args->hError  = hErrorRead;
    return w32_HTOI(args->hProc);

einval:;
    Close(hOutputReadTmp); Close(hInputWriteTmp); Close(hErrorReadTmp);
    Close(hInputRead); Close(hOutputWrite); Close(hErrorWrite);
    Close(hInputWrite); Close(hOutputRead); Close(hErrorRead);
    errno = EINVAL;
    return -1;
}


int
w32_execW(win32_execw_t *args)
{
    HANDLE hInputWriteTmp = 0, hInputRead = 0, hInputWrite = 0,
        hErrorReadTmp = 0, hErrorRead = 0, hErrorWrite = 0,
        hOutputReadTmp = 0, hOutputRead = 0, hOutputWrite = 0;

    // Non-optional arguments.
    if (NULL == args ||
            (NULL == args->spawn.cmd && NULL == args->spawn.argv)) {
        errno = EINVAL;
        return -1;
    }

    // Create the child input/out/err pipes.
    if (! Pipe(&hInputRead, &hInputWriteTmp) ||
            ! Pipe(&hOutputReadTmp, &hOutputWrite) ||
            ! Pipe(&hErrorReadTmp, &hErrorWrite)) {
        InternalError("exec: pipe");
        goto einval;
    }

    // Create new output read handle and the input write handles, set as non inheritable.
    // Otherwise, the child inherits the properties and, as a result, non-closeable handles to the pipes are created.
    if (! Dup(hInputWriteTmp, &hInputWrite, FALSE) ||
            ! Dup(hOutputReadTmp, &hOutputRead, FALSE) ||
            ! Dup(hErrorReadTmp, &hErrorRead, FALSE)) {
        InternalError("exec: dup");
        goto einval;
    }
    Close(hOutputReadTmp); Close(hInputWriteTmp); Close(hErrorReadTmp);

    // Execute child.
    args->hProc = w32_child_execW(&args->spawn, hInputRead, hOutputWrite, hErrorWrite);

    // Close pipe handles.
    Close(hInputRead); Close(hOutputWrite); Close(hErrorWrite);

    // Completion.
    if (0 == args->hProc) {
        Close(hInputWrite); Close(hOutputRead); Close(hErrorRead);
        return 0;
    }

    args->hInput  = hInputWrite;
    args->hOutput = hOutputRead;
    args->hError  = hErrorRead;
    return w32_HTOI(args->hProc);

einval:;
    Close(hOutputReadTmp); Close(hInputWriteTmp); Close(hErrorReadTmp);
    Close(hInputRead); Close(hOutputWrite); Close(hErrorWrite);
    Close(hInputWrite); Close(hOutputRead); Close(hErrorRead);
    errno = EINVAL;
    return -1;
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
        if (dup) *dup = INVALID_HANDLE_VALUE;
        return FALSE;
    }
    return TRUE;
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
    if (handle && handle != INVALID_HANDLE_VALUE) {
        if (! CloseHandle(handle)) {
            InternalError("closehandle()");
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
    if (handle && handle != INVALID_HANDLE_VALUE) {
        if (! CloseHandle(handle)) {
            char buffer[512];
            _snprintf(buffer, sizeof(buffer), "closehandle(%s)", desc);
            buffer[sizeof(buffer)-1]=0;
            InternalError(buffer);
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

    if (NULL == (p = malloc(sizeof(*p)))) {
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
            InternalError("ReadFile");          // .. something bad
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
DisplayErrorA(
    HANDLE hOutput, const char *msg, const char *cmd)
{
    const DWORD rc = GetLastError();
    char t_rcbuffer[512], buffer[512];
    const char *rcmsg = w32_vsyserrorA(rc, t_rcbuffer, sizeof(t_rcbuffer), cmd, NULL);
    int len;

    len = _snprintf(buffer, sizeof(buffer),
            "Internal Error: %s = %u (%s).\n", msg, (unsigned)rc, rcmsg);
    WriteConsoleA(hOutput, buffer, len, NULL, NULL);
}


static void
DisplayErrorW(
    HANDLE hOutput, const wchar_t *msg, const wchar_t *cmd)
{
    const DWORD rc = GetLastError();
    wchar_t t_rcbuffer[512], buffer[512];
    const wchar_t *rcmsg = w32_vsyserrorW(rc, t_rcbuffer, _countof(t_rcbuffer), cmd, NULL);
    int len;

    len = _snwprintf(buffer, _countof(buffer),
            L"Internal Error: %s = %u (%s).\n", msg, (unsigned)rc, rcmsg);
    WriteConsoleW(hOutput, buffer, len, NULL, NULL);
}


static void
InternalError(const char *pszAPI)
{
    DisplayErrorA(GetStdHandle(STD_OUTPUT_HANDLE), pszAPI, NULL);
    ExitProcess(GetLastError());
}

/*end*/
