#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_popen_c,"$Id: w32_popen.c,v 1.6 2018/10/12 00:52:04 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 popen implementation
 *
 * Copyright (c) 2007, 2012 - 2018 Adam Young.
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

/*#define USE_NATIVE_POPEN*/                    /* test only */

#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>

struct pipe {
#define PIPE_MAGIC          (0x50695065)        /* PiPe */
    DWORD               magic;
    char                readOrWrite;
    char                textOrBinary;
    FILE *              file;
    HANDLE              handle;
    DWORD               pid;
    HANDLE              hIn;
    HANDLE              hOut;
    HANDLE              hErr;
    struct pipe *       next;
};

static int              Dup(HANDLE old, HANDLE *dup, BOOL inherit);
static int              Pipe2(HANDLE *read, HANDLE *write, int inherit);
static void             Close(HANDLE handle);
static void             Close2(HANDLE handle, const char *desc);
static void             InternalError(const char *pszAPI);

static CRITICAL_SECTION pipe_guard;
static struct pipe *    pipe_queue = (void *)-1;


/*
//  NAME
//
//      popen - initiate pipe streams to or from a process
//
//  SYNOPSIS
//
//      #include <stdio.h>
//
//      FILE *popen(const char *command, const char *mode); [Option End]
//
//  DESCRIPTION
//
//      The popen() function shall execute the command specified by the string command. It
//      shall create a pipe between the calling program and the executed command, and shall
//      return a pointer to a stream that can be used to either read from or write to the
//      pipe.
//
//      The environment of the executed command shall be as if a child process were created
//      within the popen() call using the fork() function, and the child invoked the sh
//      utility using the call:
//
//          execl(shell path, "sh", "-c", command, (char *)0);
//
//      where shell path is an unspecified pathname for the sh utility.
//
//      The popen() function shall ensure that any streams from previous popen() calls that
//      remain open in the parent process are closed in the new child process.
//
//      The mode argument to popen() is a string that specifies I/O mode:
//
//          If mode is 'r', when the child process is started, its file descriptor
//          STDOUT_FILENO shall be the writable end of the pipe, and the file descriptor
//          fileno(stream) in the calling process, where stream is the stream pointer
//          returned by popen(), shall be the readable end of the pipe.
//
//          If mode is 'w', when the child process is started its file descriptor
//          STDIN_FILENO shall be the readable end of the pipe, and the file descriptor
//          fileno(stream) in the calling process, where stream is the stream pointer
//          returned by popen(), shall be the writable end of the pipe.
//
//          If mode is any other value, the result is undefined.
//
//      After popen(), both the parent and the child process shall be capable of executing
//      independently before either terminates.
//
//      Pipe streams are byte-oriented.
//
//  RETURN VALUE
//
//      Upon successful completion, popen() shall return a pointer to an open stream that
//      can be used to read or write to the pipe. Otherwise, it shall return a null pointer
//      and may set errno to indicate the error.
//
//  ERRORS
//
//      The popen() function may fail if:
//
//      [EMFILE]
//          {FOPEN_MAX} or {STREAM_MAX} streams are currently open in the calling process.
//
//      [EINVAL]
//          The mode argument is invalid.
//
//      The popen() function may also set errno values as described by fork() or pipe().
//
//  NOTE
//
//      if cmd contains '2>&1', we connect the standard error file handle to the standard
//      output file handle, otherwise create a STDERR stream.
*/
LIBW32_API FILE *
w32_popen(const char *cmd, const char *mode)
{
#if (defined(_MSVC_VER) || defined(__WATCOMC__)) && \
        defined(USE_NATIVE_POPEN)
    return _popen(cmd, mode);

#else
    int redirect_error = FALSE;
    const char *shell = w32_getshell();
    win32_spawn_t args = {0};
    char readOrWrite, textOrBinary;
    HANDLE in_read = INVALID_HANDLE_VALUE, in_write = INVALID_HANDLE_VALUE,
        out_read = INVALID_HANDLE_VALUE, out_write = INVALID_HANDLE_VALUE,
        err_read = INVALID_HANDLE_VALUE, err_write = INVALID_HANDLE_VALUE;
    const char *argv[4] = {0};
    struct pipe *p = NULL;
    char *cmd2 = NULL;

    if (NULL == cmd || NULL == mode ) {
        errno = EINVAL;
        return NULL;
    }

    // new pipe node
    switch (mode[0]) {
    case 'r': readOrWrite = 'r'; break;
    case 'w': readOrWrite = 'w'; break;
    default:
        return NULL;            // either r or w
    }
    switch (mode[1]) {
    case 't': textOrBinary = 't'; break;
    case 'b': textOrBinary = 'b'; break;
    case 0:
        textOrBinary = 'b';     // optional
        break;
    default:
        return NULL;
    }

    if (NULL == (p = calloc(1, sizeof(*p)))) {
        return NULL;
    }
    p->magic = PIPE_MAGIC;
    p->readOrWrite = readOrWrite;
    p->textOrBinary = textOrBinary;

    // detect the type of shell
    argv[0] = shell;
    if (w32_iscommand(shell)) {
        argv[1] = "/C";
        if (NULL == strstr("2>&1", cmd)) {      // redirect stderr to stdout ? */
            argv[2] = cmd;
        } else {
            argv[2] = cmd2 = WIN32_STRDUP(cmd);
            strncpy(strstr("2>&1", cmd2), "    ", 4);
            redirect_error = TRUE;
        }
        argv[3] = NULL;
    } else {
        argv[1] = "-i";
        argv[2] = cmd;
        argv[3] = NULL;
    }

    // create the Pipes...
    if (! Pipe2(&in_read, &in_write, 1) || ! Pipe2(&out_read, &out_write, 2)) {
        goto pipe_error;
    }

    if (redirect_error) {                       // 2>&1
        if (! Dup(out_write, &err_write, TRUE)) {
            goto pipe_error;
        }
    } else {                                    // .. otherwise seperate pipe
        if (! Pipe2(&err_read, &err_write, 2)) {
            goto pipe_error;
        }
    }

    if ('r' == p->readOrWrite) {
        if (NULL == (p->file = _fdopen(         // readable end of the pipe
                _open_osfhandle((long)out_read,
                    _O_NOINHERIT | ('b' == textOrBinary ? _O_BINARY : _O_TEXT)),
                    'b' == textOrBinary ? "rb" : "rt"))) {
            goto pipe_error;
        }
        out_read = INVALID_HANDLE_VALUE;

    } else {
        if (NULL == (p->file = _fdopen(         // writeable end of the pipe
                _open_osfhandle((long)in_write,
                    _O_NOINHERIT | ('b' == textOrBinary ? _O_BINARY : _O_TEXT)),
                    'b' == textOrBinary ? "wb" : "wt"))) {
            goto pipe_error;
        }
        in_write = INVALID_HANDLE_VALUE;
    }
    setvbuf(p->file, NULL, _IONBF, 0);          // non-buffered

    // create the child process,
    //      on success return pipe
    //
    args.argv = argv;                           // argument vector
    args._dwFlags =                             // creation flags
        CREATE_DEFAULT_ERROR_MODE|CREATE_NO_WINDOW;

    if ((void *)-1 == pipe_queue) {
        InitializeCriticalSection(&pipe_guard);
        pipe_queue = NULL;
    }

    if (0 != (p->handle =
            w32_child_exec(&args, in_read, out_write, err_write))) {

        Close(in_read); Close(out_write); Close(err_write);
        free(cmd2);

        p->hIn  = in_write;
        p->hOut = out_read;
        p->hErr = err_read;
        p->pid  = args._dwProcessId;            // process identifier

        EnterCriticalSection(&pipe_guard);
        p->next = pipe_queue;
        pipe_queue = p;
        LeaveCriticalSection(&pipe_guard);
        return p->file;
    }

    // on error, release pipe resources.
pipe_error:
    Close(in_read); Close(out_write); Close(err_write);
    Close(in_write); Close(out_read); Close(err_read);
    free(cmd2);
    free(p);
    return NULL;
#endif  /*USE_NATIVE_POPEN*/
}



/*
 *  w32_pread_err ---
 *      read from the pipe error stream.
 *
 *  Note:
 *      WIN32 pipes are blocking.
 */
LIBW32_API int
w32_pread_err(FILE *file, char *buf, int length)
{
    if (file) {
        HANDLE handle = 0;

        if ((void *)-1 != pipe_queue) {
            struct pipe **p2;                   // list pointers
            
            EnterCriticalSection(&pipe_guard);
            for (p2 = &pipe_queue; *p2; p2 = &(*p2)->next) {
                struct pipe *p = *p2;
            
                assert(p->magic == PIPE_MAGIC);
                if (p->file == file) {
                    handle = p->hErr;
                    break;
                }
            }
            LeaveCriticalSection(&pipe_guard);
        }

        if (handle) {
            DWORD result;
            if (ReadFile(handle, buf, length, &result, NULL)) {
                return (int)result;
            }   
        }
    }
    return -1;                                  // done
}


/*
//  NAME
//
//      pclose - close a pipe stream to or from a process
//
//  SYNOPSIS
//
//      #include <stdio.h>
//
//      int pclose(FILE *stream); [Option End]
//
//  DESCRIPTION
//
//      The pclose() function shall close a stream that was opened by popen(), wait for the
//      command to terminate, and return the termination status of the process that was
//      running the command language interpreter. However, if a call caused the termination
//      status to be unavailable to pclose(), then pclose() shall return -1 with errno set
//      to [ECHILD] to report this situation. This can happen if the application calls one
//      of the following functions:
//
//          wait()
//
//          waitpid() with a pid argument less than or equal to 0 or equal to the process
//          ID of the command line interpreter
//
//          Any other function not defined in this volume of IEEE Std 1003.1-2001 that
//          could do one of the above
//
//      In any case, pclose() shall not return before the child process created by popen()
//      has terminated.
//
//      If the command language interpreter cannot be executed, the child termination
//      status returned by pclose() shall be as if the command language interpreter
//      terminated using exit(127) or _exit(127).
//
//      The pclose() function shall not affect the termination status of any child of the
//      calling process other than the one created by popen() for the associated stream.
//
//      If the argument stream to pclose() is not a pointer to a stream created by popen(),
//      the result of pclose() is undefined.
//
//  RETURN VALUE
//
//      Upon successful return, pclose() shall return the termination status of the command
//      language interpreter. Otherwise, pclose() shall return -1 and set errno to indicate
//      the error.
//
//  ERRORS
//
//      The pclose() function shall fail if:
//
//      [ECHILD]
//          The status of the child process could not be obtained, as described above.
*/
LIBW32_API int
w32_pclose(FILE *file)
{
#if (defined(_MSVC_VER) || defined(__WATCOMC__)) && \
            defined(USE_NATIVE_POPEN)
    return _pclose(file);

#else
    if (file) {
        struct pipe *pipe = NULL;

        if ((void *)-1 != pipe_queue) {
            struct pipe **p2;                       // list pointers
            
            EnterCriticalSection(&pipe_guard);
            for (p2 = &pipe_queue; *p2; p2 = &(*p2)->next) {
                struct pipe *t_p = *p2;

                assert(t_p->magic == PIPE_MAGIC);
                if (t_p->file == file) {
                    *p2 = t_p->next;                // remove from chain
                    pipe = t_p;
                    break;
                }
            }
            LeaveCriticalSection(&pipe_guard);
        }

        if (pipe) {
            int status = 0, ret = 0;

            if ('w' == pipe->readOrWrite) fclose(file); Close2(pipe->hIn, "pclose/stdin");
            if ('r' == pipe->readOrWrite) fclose(file); Close2(pipe->hOut, "pclose/stdout");
            Close2(pipe->hErr, "pclose/stderr");
            if (! w32_child_wait(pipe->handle, &status, FALSE)) {
                ret = -1;
            }
            free(pipe);
            return (0 == ret ? status : -1);
        }
    }
    return -1;
#endif  /*USE_NATIVE_POPEN*/
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
 *  Pipe2 ---
 *      Create a pipe, yet *only* inherit either 0=both, 1=read or 2=write.
 */
static int
Pipe2(HANDLE *read, HANDLE *write, int inherit)
{
    SECURITY_ATTRIBUTES sa = {0};

    sa.nLength = sizeof(sa);                    // length in bytes
    sa.bInheritHandle = TRUE;                   // the child must inherit these handles
    sa.lpSecurityDescriptor = NULL;
    if (CreatePipe(read, write, &sa, 0)) {
        HANDLE tmp;

        if (1 == inherit) {                     // only read, dup and close write
            if (Dup(*write, &tmp, FALSE)) {
                CloseHandle(*write);
                *write = tmp;
                return TRUE;
            }
            CloseHandle(*read), CloseHandle(*write);
            return FALSE;

        } else if (2 == inherit) {              // only write, dup and close read
            if (Dup(*read, &tmp, FALSE)) {
                CloseHandle(*read);
                *read = tmp;
                return TRUE;
            }
            CloseHandle(*read), CloseHandle(*write);
            return FALSE;
        }
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
            InternalError("CloseHandle(popen)");
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
 *  InternalError ---
 *      Displays the error number and corresponding message.
 */
static void
DisplayError(
    HANDLE hOutput, const char *pszAPI, const char *args)
{
    DWORD   rc = GetLastError();
    LPVOID  lpvMessageBuffer;
    char    szPrintBuffer[512];
    DWORD   nCharsWritten;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, rc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpvMessageBuffer, 0, NULL);

    _snprintf(szPrintBuffer, sizeof(szPrintBuffer),
        "Internal Error: %s = %d (%s).\n%s%s", pszAPI, rc, (char *)lpvMessageBuffer,
            args ? args : "", args ? "\n" : "" );

    if (hOutput == INVALID_HANDLE_VALUE) {
        hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    WriteConsoleA(hOutput, szPrintBuffer, lstrlenA(szPrintBuffer), &nCharsWritten, NULL);
    LocalFree(lpvMessageBuffer);
}


static void
InternalError(const char *pszAPI)
{
    DisplayError(INVALID_HANDLE_VALUE, pszAPI, NULL);
    ExitProcess(GetLastError());
}

/*end*/

