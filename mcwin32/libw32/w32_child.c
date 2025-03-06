#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_child_c,"$Id: w32_child.c,v 1.25 2025/03/06 16:59:46 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 sub-process support
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
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

#include <sys/cdefs.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>

#pragma comment(lib, "user32.lib")

#if defined(_MSC_VER)
#pragma warning(disable : 4244) // conversion from 'xxx' to 'xxx', possible loss of data
#pragma warning(disable : 4312) // type cast' : conversion from 'xxx' to 'xxx' of greater size
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

static int              cmdisA(const char *shell, int slen, const char *cmd);
static int              cmdisW(const wchar_t *shell, int slen, const wchar_t *cmd);
static int              TOLOWER(wchar_t ch);

static BOOL             SendCloseMessage(HANDLE hProc);
static BOOL             WASNOT_ENOENT(void);

static int              BuildVectorsA(win32_spawn_t *args, char **argblk, char **envblk);
static char *           BuildArgA(const char *cmd, const char **argv);
static char *           BuildEnvA(const char **envv);
static int              BuildVectorsW(win32_spawnw_t *args, wchar_t **argblk, wchar_t **envblk);
static wchar_t *        BuildArgW(const wchar_t *cmd, const wchar_t **argv);
static wchar_t *        BuildEnvW(const wchar_t **envv);

static const char *     GetpathA(const char *src, char *dst, unsigned maxlen);
static const wchar_t *  GetpathW(const wchar_t *src, wchar_t *dst, unsigned maxlen);

static const char *     GetenvA(const char *const *envp, const char *val);
static const wchar_t *  GetenvW(const wchar_t *const *envp, const wchar_t *val);

static HANDLE           ExecChildA(win32_spawn_t *args,
                            const char *arg0, char *argv, char *envp, STARTUPINFOA *si, PROCESS_INFORMATION *pi);
static HANDLE           ExecChildW(win32_spawnw_t *args,
                            const wchar_t *arg0, wchar_t *argv, wchar_t *envp, STARTUPINFOW *si, PROCESS_INFORMATION *pi);

static void             DisplayErrorA(HANDLE hOutput, const char *pszAPI, const char *args);
static void             DisplayErrorW(HANDLE hOutput, const wchar_t *pszAPI, const wchar_t *args);
static void             InternalError(const char *pszAPI);


/*
//  NAME
//      wait, waitpid - wait for a child process to stop or terminate
//
//  SYNOPSIS
//
//      #include <sys/wait.h>
//
//      pid_t wait(int *stat_loc);
//      pid_t waitpid(pid_t pid, int *stat_loc, int options);
//
//  DESCRIPTION
//
//      The wait() and waitpid() functions shall obtain status information pertaining to
//      one of the caller's child processes. Various options permit status information to
//      be obtained for child processes that have terminated or stopped. If status
//      information is available for two or more child processes, the order in which their
//      status is reported is unspecified.
//
//      The wait() function shall suspend execution of the calling thread until status
//      information for one of the terminated child processes of the calling process is
//      available, or until delivery of a signal whose action is either to execute a
//      signal-catching function or to terminate the process. If more than one thread is
//      suspended in wait() or waitpid() awaiting termination of the same process, exactly
//      one thread shall return the process status at the time of the target process
//      termination. If status information is available prior to the call to wait(), return
//      shall be immediate.
//
//      The waitpid() function shall be equivalent to wait() if the pid argument is
//      (pid_t)-1 and the options argument is 0. Otherwise, its behavior shall be modified
//      by the values of the pid and options arguments.
//
//      The pid argument specifies a set of child processes for which status is requested.
//      The waitpid() function shall only return the status of a child process from this set:
//
//          If pid is equal to (pid_t)-1, status is requested for any child process. In
//          this respect, waitpid() is then equivalent to wait().
//
//          If pid is greater than 0, it specifies the process ID of a single child process
//          for which status is requested.
//
//          If pid is 0, status is requested for any child process whose process group ID
//          is equal to that of the calling process.
//
//          If pid is less than (pid_t)-1, status is requested for any child process whose
//          process group ID is equal to the absolute value of pid.
//
//      The options argument is constructed from the bitwise-inclusive OR of zero or more
//      of the following flags, defined in the <sys/wait.h> header:
//
//      WCONTINUED
//          The waitpid() function shall report the status of any continued child process
//          specified by pid whose status has not been reported since it continued from a
//          job control stop. [Option End]
//
//      WNOHANG
//          The waitpid() function shall not suspend execution of the calling thread if
//          status is not immediately available for one of the child processes specified by
//          pid.
//
//      WUNTRACED
//          The status of any child processes specified by pid that are stopped, and whose
//          status has not yet been reported since they stopped, shall also be reported to
//          the requesting process.
//
//      If the calling process has SA_NOCLDWAIT set or has SIGCHLD set to SIG_IGN, and the
//      process has no unwaited-for children that were transformed into zombie processes,
//      the calling thread shall block until all of the children of the process containing
//      the calling thread terminate, and wait() and waitpid() shall fail and set errno to
//      [ECHILD]. [Option End]
//
//      If wait() or waitpid() return because the status of a child process is available,
//      these functions shall return a value equal to the process ID of the child process.
//      In this case, if the value of the argument stat_loc is not a null pointer,
//      information shall be stored in the location pointed to by stat_loc. The value
//      stored at the location pointed to by stat_loc shall be 0 if and only if the status
//      returned is from a terminated child process that terminated by one of the following
//      means:
//
//          The process returned 0 from main().
//
//          The process called _exit() or exit() with a status argument of 0.
//
//          The process was terminated because the last thread in the process terminated.
//
//      Regardless of its value, this information may be interpreted using the following
//      macros, which are defined in <sys/wait.h> and evaluate to integral expressions; the
//      stat_val argument is the integer value pointed to by stat_loc.
//
//          WIFEXITED(stat_val)
//              Evaluates to a non-zero value if status was returned for a child process
//              that terminated normally.
//
//          WEXITSTATUS(stat_val)
//              If the value of WIFEXITED(stat_val) is non-zero, this macro evaluates to the
//              low-order 8 bits of the status argument that the child process passed to
//              _exit() or exit(), or the value the child process returned from main().
//
//          WIFSIGNALED(stat_val)
//              Evaluates to a non-zero value if status was returned for a child process that
//              terminated due to the receipt of a signal that was not caught (see <signal.h>).
//
//          WTERMSIG(stat_val)
//              If the value of WIFSIGNALED(stat_val) is non-zero, this macro evaluates to the
//              number of the signal that caused the termination of the child process.
//
//          WIFSTOPPED(stat_val)
//              Evaluates to a non-zero value if status was returned for a child process that
//              is currently stopped.
//
//          WSTOPSIG(stat_val)
//              If the value of WIFSTOPPED(stat_val) is non-zero, this macro evaluates to the
//              number of the signal that caused the child process to stop.
//
//          WIFCONTINUED(stat_val)
//              Evaluates to a non-zero value if status was returned for a child process that
//              has continued from a job control stop. [Option End]
//
//      It is unspecified whether the status value returned by calls to wait() or waitpid()
//      for processes created by posix_spawn() or posix_spawnp() can indicate a
//      WIFSTOPPED(stat_val) before subsequent calls to wait() or waitpid() indicate
//      WIFEXITED(stat_val) as the result of an error detected before the new process image
//      starts executing.
//
//      It is unspecified whether the status value returned by calls to wait() or waitpid()
//      for processes created by posix_spawn() or posix_spawnp() can indicate a
//      WIFSIGNALED(stat_val) if a signal is sent to the parent's process group after
//      posix_spawn() or posix_spawnp() is called. [Option End]
//
//      If the information pointed to by stat_loc was stored by a call to waitpid() that
//      specified the WUNTRACED flag [XSI] [Option Start] and did not specify the
//      WCONTINUED flag, [Option End] exactly one of the macros WIFEXITED(*stat_loc),
//      WIFSIGNALED(*stat_loc), and WIFSTOPPED(*stat_loc) shall evaluate to a non-zero value.
//
//      If the information pointed to by stat_loc was stored by a call to waitpid() that
//      specified the WUNTRACED [XSI] [Option Start] and WCONTINUED [Option End] flags,
//      exactly one of the macros WIFEXITED(*stat_loc), WIFSIGNALED(*stat_loc),
//      WIFSTOPPED(*stat_loc), [XSI] [Option Start] and WIFCONTINUED(*stat_loc) [Option
//      End] shall evaluate to a non-zero value.
//
//      If the information pointed to by stat_loc was stored by a call to waitpid() that
//      did not specify the WUNTRACED [XSI] [Option Start] or WCONTINUED [Option End] flags,
//      or by a call to the wait() function, exactly one of the macros WIFEXITED(*stat_loc)
//      and WIFSIGNALED(*stat_loc) shall evaluate to a non-zero value.
//
//      If the information pointed to by stat_loc was stored by a call to waitpid() that
//      did not specify the WUNTRACED flag [XSI] [Option Start] and specified the
//      WCONTINUED flag, [Option End] or by a call to the wait() function, exactly one of
//      the macros WIFEXITED(*stat_loc), WIFSIGNALED(*stat_loc), [XSI] [Option Start] and
//      WIFCONTINUED(*stat_loc) [Option End] shall evaluate to a non-zero value.
//
//      If _POSIX_REALTIME_SIGNALS is defined, and the implementation queues the SIGCHLD
//      signal, then if wait() or waitpid() returns because the status of a child process
//      is available, any pending SIGCHLD signal associated with the process ID of the
//      child process shall be discarded. Any other pending SIGCHLD signals shall remain
//      pending.
//
//      Otherwise, if SIGCHLD is blocked, if wait() or waitpid() return because the status
//      of a child process is available, any pending SIGCHLD signal shall be cleared unless
//      the status of another child process is available.
//
//      For all other conditions, it is unspecified whether child status will be available
//      when a SIGCHLD signal is delivered.
//
//      There may be additional implementation-defined circumstances under which wait() or
//      waitpid() report status. This shall not occur unless the calling process or one of
//      its child processes explicitly makes use of a non-standard extension. In these
//      cases the interpretation of the reported status is implementation-defined.
//
//      If a parent process terminates without waiting for all of its child processes to
//      terminate, the remaining child processes shall be assigned a new parent process ID
//      corresponding to an implementation-defined system process. [Option End]
//
//  RETURN VALUE
//
//      If wait() or waitpid() returns because the status of a child process is available,
//      these functions shall return a value equal to the process ID of the child process
//      for which status is reported. If wait() or waitpid() returns due to the delivery of
//      a signal to the calling process, -1 shall be returned and errno set to [EINTR]. If
//      waitpid() was invoked with WNOHANG set in options, it has at least one child
//      process specified by pid for which status is not available, and status is not
//      available for any process specified by pid, 0 is returned. Otherwise, (pid_t)-1
//      shall be returned, and errno set to indicate the error.
//
//  ERRORS
//
//      The wait() function shall fail if:
//
//      [ECHILD]
//          The calling process has no existing unwaited-for child processes.
//
//      [EINTR]
//          The function was interrupted by a signal. The value of the location pointed to
//          by stat_loc is undefined.
//
//      The waitpid() function shall fail if:
//
//      [ECHILD]
//          The process specified by pid does not exist or is not a child of the calling
//          process, or the process group specified by pid does not exist or does not have
//          any member process that is a child of the calling process.
//
//      [EINTR]
//          The function was interrupted by a signal. The value of the location pointed to
//          by stat_loc is undefined.
//
//      [EINVAL]
//          The options argument is not valid.
*/
LIBW32_API int
w32_waitpid(int pid, int *status, int options)
{
    int ret = -1;

    if (pid == -1) {
        /*
         *  wait for any child process ...
         */
        errno = EINVAL;

    } else if (pid > 0) {
        /*
         *  wait for the child whose process ID is equal to the value of pid.
         */
        if (w32_child_wait(w32_ITOH(pid), status, options & WNOHANG)) {
            ret = pid;
        }
    }
    return ret;
}


LIBW32_API int
WEXITSTATUS(int status)
{
    return ((status) >> 8) & 0xff;
}


LIBW32_API int
WCOREDUMP(int status)
{
    __CUNUSED(status)
    return 0;
}


LIBW32_API int
WTERMSIG(int status)
{
    return (status & 0xff);
}


LIBW32_API int
WIFSIGNALED(int status)
{
    return (WTERMSIG (status) != 0);
}


LIBW32_API int
WIFEXITED(int status)
{
    return (WTERMSIG (status) == 0);
}


LIBW32_API int
WIFSTOPPED(int status)
{
    __CUNUSED(status)
    return 0;
}


/*
//  NAME
//
//      kill - send a signal to a process or a group of processes
//
//  SYNOPSIS
//
//      #include <signal.h>
//
//      int kill(pid_t pid, int sig);
//
//  DESCRIPTION
//
//      The kill() function shall send a signal to a process or a group of processes
//      specified by pid. The signal to be sent is specified by sig and is either one from
//      the list given in <signal.h> or 0. If sig is 0 (the null signal), error checking is
//      performed but no signal is actually sent. The null signal can be used to check the
//      validity of pid.
//
//      For a process to have permission to send a signal to a process designated by pid,
//      unless the sending process has appropriate privileges, the real or effective user
//      ID of the sending process shall match the real or saved set-user-ID of the
//      receiving process.
//
//      If pid is greater than 0, sig shall be sent to the process whose process ID is
//      equal to pid.
//
//      If pid is 0, sig shall be sent to all processes (excluding an unspecified set of
//      system processes) whose process group ID is equal to the process group ID of the
//      sender, and for which the process has permission to send a signal.
//
//      If pid is -1, sig shall be sent to all processes (excluding an unspecified set of
//      system processes) for which the process has permission to send that signal.
//
//      If pid is negative, but not -1, sig shall be sent to all processes (excluding an
//      unspecified set of system processes) whose process group ID is equal to the
//      absolute value of pid, and for which the process has permission to send a signal.
//
//      If the value of pid causes sig to be generated for the sending process, and if sig
//      is not blocked for the calling thread and if no other thread has sig unblocked or
//      is waiting in a sigwait() function for sig, either sig or at least one pending
//      unblocked signal shall be delivered to the sending thread before kill() returns.
//
//      The user ID tests described above shall not be applied when sending SIGCONT to a
//      process that is a member of the same session as the sending process.
//
//      An implementation that provides extended security controls may impose further
//      implementation-defined restrictions on the sending of signals, including the null
//      signal. In particular, the system may deny the existence of some or all of the
//      processes specified by pid.
//
//      The kill() function is successful if the process has permission to send sig to any
//      of the processes specified by pid. If kill() fails, no signal shall be sent.
//
//  RETURN VALUE
//
//      Upon successful completion, 0 shall be returned. Otherwise, -1 shall be returned
//      and errno set to indicate the error.
//
//  ERRORS
//
//      The kill() function shall fail if:
//
//      [EINVAL]
//          The value of the sig argument is an invalid or unsupported signal number.
//
//      [EPERM]
//          The process does not have permission to send the signal to any receiving process.
//
//      [ESRCH]
//          No process or process group can be found corresponding to that specified by pid.
*/
LIBW32_API int
w32_kill(int pid, int value)
{
    if (pid > 0) {
        HANDLE hProc = w32_ITOH(pid);

        /* Still running ?? */
        if (WaitForSingleObject(hProc, 0) != WAIT_TIMEOUT) {
            errno = ESRCH;
            return -1;
        }

        /* send signal */
        switch (value) {
        case 0:
            return 0;
        case SIGINT:
            /*
             *  CTRL_C_EVENT
             *      Generates a CTRL+C signal. This signal cannot be generated
             *      for process groups. If dwProcessGroupId is nonzero, this
             *      function will succeed, but the CTRL+C signal will not be
             *      received by processes within the specified process group.
             */
            GenerateConsoleCtrlEvent(CTRL_C_EVENT, /*LOCAL*/0);
            return 0;
        case SIGTERM:
            SendCloseMessage(hProc);
            return 0;
        }
    }
    errno = EINVAL;
    return -1;
}


/*
 *  SendCloseMessage ---
 *      Try to kill a process politely by sending a WM_CLOSE message.
 */
struct _enum_win_info {
    HANDLE          hProcess;
    DWORD           dwProcessId;
};


static BOOL CALLBACK
EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    /*
     *  Callback to EnumWindows..
     *
     *  Parameters
     *      hwnd    [in] Handle to a top-level window.
     *      lParam  [in] Specifies the application-defined value given in
     *              EnumWindows or EnumDesktopWindows.
     *
     *  Return Value:
     *      To continue enumeration, the callback function must
     *      return TRUE; to stop enumeration, it must return FALSE.
     */
    struct _enum_win_info *info = (struct _enum_win_info *)lParam;
    DWORD pid, status;

    (void) GetWindowThreadProcessId(hwnd, &pid);
    return pid != info->dwProcessId
            || (GetExitCodeProcess(info->hProcess, &status)
                 && status == STILL_ACTIVE       // value = 259
                 && PostMessage(hwnd, WM_CLOSE, 0, 0));
}


static BOOL
SendCloseMessage(HANDLE hProc)
{
    struct _enum_win_info info;

    info.hProcess = hProc;
    info.dwProcessId = 0;
    return EnumWindows(EnumWindowsProc, (LPARAM)&info);
}


/*
 *  w32_iscommand ---
 *      Determine the given shell is a DOS/WIN command processor
 */
LIBW32_API int
w32_iscommand(const char *shell)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t wshell[WIN32_PATH_MAX];

        if (w32_utf2wc(shell, wshell, _countof(wshell)) > 0) {
            return w32_iscommandW(wshell);
        }
        return -1;
    }
#endif  //UTF8FILENAMES

    return w32_iscommandA(shell);
}


LIBW32_API int
w32_iscommandA(const char *shell)
{
    const int slen = (int)strlen(shell);

    if (cmdisA(shell, slen, "cmd") ||
            cmdisA(shell, slen, "cmd.exe") ||
            cmdisA(shell, slen, "command") ||
            cmdisA(shell, slen, "command.com") ||
            cmdisA(shell, slen, "command.exe")) {
        return TRUE;
    }
    return FALSE;
}


LIBW32_API int
w32_iscommandW(const wchar_t *shell)
{
    const int slen = (int)wcslen(shell);

    if (cmdisW(shell, slen, L"cmd") ||
            cmdisW(shell, slen, L"cmd.exe") ||
            cmdisW(shell, slen, L"command") ||
            cmdisW(shell, slen, L"command.com") ||
            cmdisW(shell, slen, L"command.exe")) {
        return TRUE;
    }
    return FALSE;
}


static int
cmdisA(const char *shell, int slen, const char *cmd)
{
    const int clen = (int)strlen(cmd);
    const char *p = shell + slen - clen;

    if (slen == clen || (slen > clen && (p[-1] == '\\' || p[-1] == '/'))) {
        if (0 == _stricmp(p, cmd)) {
            return TRUE;
        }
    }
    return FALSE;
}


static int
cmdisW(const wchar_t *shell, int slen, const wchar_t *cmd)
{
    const int clen = (int)wcslen(cmd);
    const wchar_t *p = shell + slen - clen;

    if (slen == clen || (slen > clen && (p[-1] == '\\' || p[-1] == '/'))) {
        if (0 == _wcsicmp(p, cmd)) {
            return TRUE;
        }
    }
    return FALSE;
}



/*
 *  w32_child_exec ---
 *      Setup a STARTUPINFO structure and launches a redirected child using
 *      the specified stdin/stdout and stderr handles.
 *
 *      A low level interface that expects the caller has setup the calling environment.
 */
LIBW32_API HANDLE
w32_child_execA(
    win32_spawn_t *args, HANDLE hStdIn, HANDLE hStdOut, HANDLE hStdErr)
{
    PROCESS_INFORMATION pi = {0};
    STARTUPINFOA si = {0};
    char *argblk = NULL;
    char *envblk = NULL;
    HANDLE hProc = 0;

    /*
     *  Build env and command line.
     */
    if (NULL == args || (NULL == args->cmd && NULL == args->argv)) {
        errno = EINVAL;
        return 0;
    }

    assert((NULL != args->cmd && NULL == args->argv) || (NULL == args->cmd && NULL != args->argv));
    if (! BuildVectorsA(args, &argblk, &envblk) != 0) {
        InternalError("BuildVector");
        return 0;
    }

    /*
     *  Set up the start up info struct
     *      USESTDHANDLES,
     *          hStdInput, hStdOutput, and hStdError members contain additional information.
     *
     *          If this flag is specified when calling one of the process creation functions,
     *          the handles must be inheritable and the function's bInheritHandles parameter
     *          must be set to TRUE. For more information;
     */
    (void) memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.wShowWindow = ((args->flags & W32_SWHIDE) ? SW_HIDE : SW_SHOW);
    si.hStdInput  = hStdIn;
    si.hStdOutput = hStdOut;
    si.hStdError  = hStdErr;

#if !defined(NDEBUG)
    if (hStdIn)  { DWORD flags; assert(GetHandleInformation(hStdIn,  &flags) && (HANDLE_FLAG_INHERIT & flags)); }
    if (hStdOut) { DWORD flags; assert(GetHandleInformation(hStdOut, &flags) && (HANDLE_FLAG_INHERIT & flags)); }
    if (hStdErr) { DWORD flags; assert(GetHandleInformation(hStdErr, &flags) && (HANDLE_FLAG_INHERIT & flags)); }
#endif

    si.dwFlags = STARTF_USESTDHANDLES;
    si.dwFlags |= STARTF_USESHOWWINDOW;

    /*
     *  Launch the process that you want to redirect, if lpApplicationName is NULL, search path is:
     *
     *   o The directory from which the application loaded.
     *   o The current directory for the parent process.
     *   o The 32-bit Windows system directory. Use the GetSystemDirectory function to get the path of this directory.
     *   o The 16-bit Windows system directory. There is no function that obtains the path of this directory, but it is searched. The name of this directory is System.
     *   o The Windows directory. Use the GetWindowsDirectory function to get the path of this directory.
     *   o The directories that are listed in the PATH environment variable.
     */
    if (0 == (hProc = ExecChildA(args, args->arg0, argblk, envblk, &si, &pi))) {
        const char *path, *cmd = (args->argv ? args->argv[0] : args->cmd);
        char *pfin, *buf = NULL;

        //  Complete if either:
        //
        //   o Error != ERROR_FILE_NOT_FOUND
        //   o arg0 contains a '/'.
        //   o PATH is NULL
        //
        if (WASNOT_ENOENT() || strchr(cmd, XSLASHCHAR) != NULL ||
                NULL == (path = GetenvA(args->envv, "PATH"))) {
            goto done;
        }

        if ((buf = calloc(_MAX_PATH + 1, sizeof(char))) == (const char *)NULL) {
            goto done;
        }

        while (NULL != (path = GetpathA(path, buf, _MAX_PATH - 1)) && (*buf)) {
            /* If necessary, append a SLASH */
            pfin = buf + strlen(buf) - 1;
            if (*pfin != SLASHCHAR && *pfin != XSLASHCHAR) {
                strcat(buf, SLASH);
            }

            /* Check length */
            if (strlen(buf) + strlen(cmd) < _MAX_PATH) {
                strcat(buf, cmd);
            } else {
                break;
            }

            // If successful, or if errno comes back with a value other than ENOENT
            // and the pathname is not a UNC name, return to the caller.
            //
            if ((hProc = ExecChildA(args, buf, argblk, envblk, &si, &pi)) != 0 ||
                    ((WASNOT_ENOENT()) && (!ISSLASH(*buf) || !ISSLASH(*(buf+1))))) {
                break;
            }
        }

done:;  free(buf);
    }

    /* Close any unnecessary handles. */
    if (hProc) {                                // success
        if (! CloseHandle(pi.hThread)) {
            InternalError("CloseHandle (thread)");
        }
    }

    free(argblk);
    free(envblk);
    return hProc;
}


/*
 *  w32_child_execW ---
 *      Setup a STARTUPINFO structure and launches a redirected child using
 *      the specified stdin/stdout and stderr handles.
 *
 *      A low level interface that expects the caller has setup the calling environment.
 */
LIBW32_API HANDLE
w32_child_execW(
    win32_spawnw_t *args, HANDLE hStdIn, HANDLE hStdOut, HANDLE hStdErr)
{
    PROCESS_INFORMATION pi = {0};
    STARTUPINFOW si = {0};
    wchar_t *argblk = NULL;
    wchar_t *envblk = NULL;
    HANDLE hProc = 0;

    /*
     *  Build env and command line.
     */
    if (NULL == args || (NULL == args->cmd && NULL == args->argv)) {
        errno = EINVAL;
        return 0;
    }

    assert((NULL != args->cmd && NULL == args->argv) || (NULL == args->cmd && NULL != args->argv));
    if (! BuildVectorsW(args, &argblk, &envblk)) {
        InternalError("building arg and env");
        return 0;
    }

    /*
     *  Set up the start up info struct
     *      USESTDHANDLES,
     *          hStdInput, hStdOutput, and hStdError members contain additional information.
     *
     *          If this flag is specified when calling one of the process creation functions,
     *          the handles must be inheritable and the function's bInheritHandles parameter
     *          must be set to TRUE. For more information;
     */
    (void) memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.wShowWindow = ((args->flags & W32_SWHIDE) ? SW_HIDE : SW_SHOW);
    si.hStdInput  = hStdIn;
    si.hStdOutput = hStdOut;
    si.hStdError  = hStdErr;

#if !defined(NDEBUG)
    if (hStdIn)  { DWORD flags; assert(GetHandleInformation(hStdIn,  &flags) && (HANDLE_FLAG_INHERIT & flags)); }
    if (hStdOut) { DWORD flags; assert(GetHandleInformation(hStdOut, &flags) && (HANDLE_FLAG_INHERIT & flags)); }
    if (hStdErr) { DWORD flags; assert(GetHandleInformation(hStdErr, &flags) && (HANDLE_FLAG_INHERIT & flags)); }
#endif

    si.dwFlags = STARTF_USESTDHANDLES;
    si.dwFlags |= STARTF_USESHOWWINDOW;

    /*
     *  Launch the process that you want to redirect, if lpApplicationName/arg0 is NULL, search path is:
     *
     *   o The directory from which the application loaded.
     *   o The current directory for the parent process.
     *   o The 32-bit Windows system directory. Use the GetSystemDirectory function to get the path of this directory.
     *   o The 16-bit Windows system directory. There is no function that obtains the path of this directory, but it is searched. The name of this directory is System.
     *   o The Windows directory. Use the GetWindowsDirectory function to get the path of this directory.
     *   o The directories that are listed in the PATH environment variable.
     */
    if (0 == (hProc = ExecChildW(args, args->arg0, argblk, envblk, &si, &pi))) {
        const wchar_t *path, *cmd = (args->argv ? args->argv[0] : args->cmd);
        wchar_t *pfin, *buf = NULL;

        //  Complete if either:
        //
        //   o Error != ERROR_FILE_NOT_FOUND
        //   o arg0 contains a '/'.
        //   o PATH is NULL
        //
        if (WASNOT_ENOENT() || wcschr(cmd, XSLASHCHAR) != NULL ||
                NULL == (path = GetenvW(args->envv, L"PATH"))) {
            goto done;
        }

        if ((buf = calloc(_MAX_PATH + 1, sizeof(wchar_t))) == (const wchar_t *)NULL) {
            goto done;
        }

        while (NULL != (path = GetpathW(path, buf, _MAX_PATH - 1)) && (*buf)) {
            /* If necessary, append a SLASH */
            pfin = buf + wcslen(buf) - 1;
            if (*pfin != SLASHCHAR && *pfin != XSLASHCHAR) {
                wcscat(buf, LSLASH);
            }

            /* Check length */
            if (wcslen(buf) + wcslen(cmd) < _MAX_PATH) {
                wcscat(buf, cmd);
            } else {
                break;
            }

            // If successful, or if errno comes back with a value other than ENOENT
            // and the pathname is not a UNC name, return to the caller.
            //
            if ((hProc = ExecChildW(args, buf, argblk, envblk, &si, &pi)) != 0 ||
                    ((WASNOT_ENOENT()) && (!ISSLASH(*buf) || !ISSLASH(*(buf+1))))) {
                break;
            }
        }

done:;  free(buf);
    }

    /* Close any unnecessary handles. */
    if (hProc) {                                // success
        if (! CloseHandle(pi.hThread)) {
            InternalError("CloseHandle (thread)");
        }
    }

    free(argblk);
    free(envblk);
    return hProc;
}


static HANDLE
ExecChildA(win32_spawn_t *args,
    const char *arg0, char *argv, char *envp, STARTUPINFOA *si, PROCESS_INFORMATION *pi)
{
    HANDLE hProc = 0;

    args->_dwFlags &= ~CREATE_UNICODE_ENVIRONMENT;
    if (CreateProcessA(
            arg0, argv,                         // [in]  application name/args.
            NULL, NULL,                         // [in]  SD's.
            TRUE,                               // [in]  handle inheritance options.
            args->_dwFlags,                     // [in]  creation flags.
            envp,                               // [in]  new envirnoment.
            args->dir,                          // [in]  working directory.
            si,                                 // [in]  startup information.
            pi)) {                              // [out] process information.
        args->_dwProcessId = pi->dwProcessId;
        hProc = pi->hProcess;

    } else if (WASNOT_ENOENT()) {
        DisplayErrorA(GetStdHandle(STD_OUTPUT_HANDLE), "CreateProcess", arg0);
    }
    return hProc;
}


static HANDLE
ExecChildW(win32_spawnw_t *args,
    const wchar_t *arg0, wchar_t *argv, wchar_t *envp, STARTUPINFOW *si, PROCESS_INFORMATION *pi)
{
    HANDLE hProc = 0;

    if (envp) args->_dwFlags |= CREATE_UNICODE_ENVIRONMENT;
    if (CreateProcessW(
            arg0, argv,                         // [in]  application name/args.
            NULL, NULL,                         // [in]  SD's.
            TRUE,                               // [in]  handle inheritance options.
            args->_dwFlags,                     // [in]  creation flags.
            envp,                               // [in]  new envirnoment.
            args->dir,                          // [in]  working directory.
            si,                                 // [in]  startup information.
            pi)) {                              // [out] process information.
        args->_dwProcessId = pi->dwProcessId;
        hProc = pi->hProcess;

    } else if (WASNOT_ENOENT()) {
        DisplayErrorW(GetStdHandle(STD_OUTPUT_HANDLE), L"CreateProcess", arg0);
    }
    return hProc;
}


static BOOL
WASNOT_ENOENT(void)
{
    const DWORD rc = GetLastError();

    if (rc == ERROR_PATH_NOT_FOUND ||
        rc == ERROR_FILE_NOT_FOUND) {
        return FALSE;
    }
    return TRUE;
}


/*
 *  w3_child_wait ---
 *      The wait function waits for the termination of the process ID of
 *      the specified process that is provided by hProc. The value of
 *      hProc passed should be the value returned by the call to the
 *      spawn function that created the specified process. If the process
 *      ID terminates before wait is called, wait returns immediately.
 *      wait can be used by any process to wait for any other known
 *      process for which a valid handle (procHandle) exists.
 *
 *      termstat points to a buffer where the return code of the specified
 *      process will be stored. The value of termstat indicates whether the
 *      specified process terminated "normally" by calling the Windows
 *      ExitProcess API. ExitProcess is called internally if the specified
 *      process calls exit or _exit, returns from main, or reaches the
 *      end of main.
 *
 *      See GetExitCodeProcess for more information regarding the value
 *      passed back through termstat. If wait is called with a NULL value
 *      for termstat, the return code of the specified process will not be
 *      stored.
 */
LIBW32_API BOOL
w32_child_wait(HANDLE hProc, int *status, int nowait)
{
    DWORD dwStatus = 0, rc;
    BOOL ret = FALSE;

    /*
     *  Explicitly check for process_id being -1 or -2.
     *
     *  In Windows NT, -1 is a handle on the current process, -2 is a handle to the
     *  current thread, and it is perfectly legal to to wait (forever) on either.
     */
    if (hProc == (HANDLE)-1 || hProc == (HANDLE)-2) {
        errno = ECHILD;

    /*
     *  Verify handle.
     */
    } else if (0 == GetExitCodeProcess(hProc, (LPDWORD)&dwStatus)) {
        errno = ECHILD;

    /*
     *  Wait for child process, then fetch its exit code.
     */
    } else if ((rc = WaitForSingleObject(hProc, (nowait ? 0 : INFINITE))) == WAIT_OBJECT_0 &&
                        GetExitCodeProcess(hProc, (LPDWORD)&dwStatus)) {
        /*
         *  Normal termination:     lo-byte = 0,            hi-byte = child exit code.
         *  Abnormal termination:   lo-byte = term status,  hi-byte = 0.
         */
        if (STILL_ACTIVE == dwStatus) {         // should occur yet!
            unsigned delay = 0;
            do {
                Sleep(100);                     // 10 * 100ms - 1 second
                if (GetExitCodeProcess(hProc, (LPDWORD)&dwStatus) && STILL_ACTIVE != dwStatus) {
                    break;  //done
                }
            } while (++delay < 10);
        }
        CloseHandle(hProc);                     // process complete
        if (status) {
            if (dwStatus > 0xff) {              // treat as signal
                switch (dwStatus) {
                case STATUS_ACCESS_VIOLATION:           // 0xC0000005L
                case STATUS_IN_PAGE_ERROR:              // 0xC0000006L
                case STATUS_INVALID_HANDLE:             // 0xC0000008L
#if !defined(STATUS_INVALID_PARAMETER)
#define STATUS_INVALID_PARAMETER 0xC000000DL
#endif
                case STATUS_INVALID_PARAMETER:          // 0xC000000DL
                case STATUS_NO_MEMORY:                  // 0xC0000017L
                    *status = SIGSEGV;
                    break;
                case STATUS_ILLEGAL_INSTRUCTION:        // 0xC000001DL
                    *status = SIGILL;
                    break;
                case STATUS_NONCONTINUABLE_EXCEPTION:   // 0xC0000025L
                case STATUS_INVALID_DISPOSITION:        // 0xC0000026L
                case STATUS_ARRAY_BOUNDS_EXCEEDED:      // 0xC000008CL
                    *status = SIGABRT;
                    break;
                case STATUS_FLOAT_DENORMAL_OPERAND:     // 0xC000008DL
                case STATUS_FLOAT_DIVIDE_BY_ZERO:       // 0xC000008EL
                case STATUS_FLOAT_INEXACT_RESULT:       // 0xC000008FL
                case STATUS_FLOAT_INVALID_OPERATION:    // 0xC0000090L
                case STATUS_FLOAT_OVERFLOW:             // 0xC0000091L
                case STATUS_FLOAT_STACK_CHECK:          // 0xC0000092L
                case STATUS_FLOAT_UNDERFLOW:            // 0xC0000093L
                    *status = SIGFPE;
                    break;
                case STATUS_INTEGER_DIVIDE_BY_ZERO:     // 0xC0000094L
                case STATUS_INTEGER_OVERFLOW:           // 0xC0000095L
                    *status = SIGFPE;
                    break;
                case STATUS_PRIVILEGED_INSTRUCTION:     // 0xC0000096L
                    *status = SIGILL;
                    break;
            //  case STATUS_STACK_OVERFLOW:             // 0xC00000FDL
            //  case STATUS_DLL_NOT_FOUND:              // 0xC0000135L
            //  case STATUS_ORDINAL_NOT_FOUND:          // 0xC0000138L
            //  case STATUS_ENTRYPOINT_NOT_FOUND:       // 0xC0000139L
                case STATUS_CONTROL_C_EXIT:             // 0xC000013AL
                    *status = SIGBREAK;
                    break;
            //  case STATUS_DLL_INIT_FAILED:            // 0xC0000142L
                case STATUS_FLOAT_MULTIPLE_FAULTS:      // 0xC00002B4L
                case STATUS_FLOAT_MULTIPLE_TRAPS:       // 0xC00002B5L
                    *status = SIGFPE;
                    break;
            //  case STATUS_REG_NAT_CONSUMPTION:        // 0xC00002C9L
            //  case STATUS_HEAP_CORRUPTION:            // 0xC0000374L
            //  case STATUS_STACK_BUFFER_OVERRUN:       // 0xC0000409L
            //  case STATUS_INVALID_CRUNTIME_PARAMETER: // 0xC0000417L
#if !defined(STATUS_ASSERTION_FAILURE)
#define STATUS_ASSERTION_FAILURE 0xC0000420L
#endif
                case STATUS_ASSERTION_FAILURE:          // 0xC0000420L
                    *status = SIGABRT;
                    break;
                default:
                    *status = 0x7f;
                    break;
                }

            } else {                            // application return value.
                *status = (int)(dwStatus >> 8);
            }
        }
        ret = TRUE;

    } else if (rc == WAIT_TIMEOUT) {            // onwait option
        errno = EAGAIN;

    } else {                                    // other error
        errno = ECHILD;
    }
    return ret;
}


/*
 *  BuildVectorsA --- build up command line/environ vectors.
 *
 *      Set up the block forms of the environment and the command line. If "envp" is
 *      null, "_environ" is used instead. File handle info is passed in the environment
 *      if _fileinfo is !0.
 */

static int
BuildVectorsA(win32_spawn_t *args, char **argblk, char **envblk)
{
    *envblk = NULL;

    if (NULL == (*argblk = BuildArgA(args->cmd, args->argv))) {
        return FALSE;
    }

    if (args->envv) {
        if (NULL == (*envblk = BuildEnvA(args->envv))) {
            free(*argblk);
            *argblk = NULL;
            return FALSE;
        }
    }

    return TRUE;
}


static char *
BuildArgA(const char *cmd, const char **argv)
{
    const char * const *vp;
    char *ret;
    int len;

    /*
     *  Allocate space for environment strings, count the number of bytes
     *  in the environment strings including nulls between strings
     */
    if (cmd) {
        len = (int)strlen(cmd) +  1 /*nul*/;
        assert(NULL == argv);
    } else {
        for (vp = argv, len = 2 /*quotes*/ + 2 /*delim*/; *vp; len += (int)strlen(*vp++) + 1 /*nul*/)
            /**/;
    }

    if (len > (int)(32 * 1024 * sizeof(char))) {
        errno = E2BIG;                          // command line too long >32k.
        return NULL;
    }

    if (NULL == (ret = (char *)calloc(len * sizeof(char), 1))) {
        return NULL;
    }

    /*
     *  Build the command line by concatenating the argument strings
     *  with spaces between, and two null bytes at the end.
     */
    if (cmd) {
        strcpy(ret, cmd);

    } else {
        char *cursor = ret;

        vp = argv;
        for (len = 0; *vp; ++len) {
            const char *arg = *vp++;
            int quote = FALSE;

            if (0 == len && *arg != '"' && strchr(arg, ' ')) {
                quote = TRUE;                   // quote, contains space.
            }

            if (quote) *cursor++ = '"';
            if (0 == len) {
                while (*arg) {                  // convert slashs within arg0.
                    *cursor++ = ('/' == *arg ? '\\' : *arg);
                    ++arg;
                }
            } else {
                strcpy(cursor, arg);
                cursor += strlen(arg);
            }
            if (quote) *cursor++ = '"';

            *cursor++ = ' ';                    // space delimiter.
        }

        cursor[-1] = '\0';                      // remove extra delimiter.
        *cursor = '\0';                         // terminator.
    }

    return ret;
}


static char *
BuildEnvA(const char **envv)
{
    /*
     *  Set up the block forms of the environment and the command line.
     *  If "envv" is null, "_environ" is used instead.
     */
    const char **envp =
#if defined(__WATCOMC__)
            (envv ? envv : (const char **)environ);
#else
            (envv ? envv : (const char **)_environ);
#endif
    const char * const *vp;
    char *ret, *cursor;
    int len;

    /*
     *  Allocate space for environment strings, count the number of bytes
     *  in the environment strings including nulls between strings
     */
    for (vp = envp, len = 2 /*padding*/; *vp; len += (int)strlen(*vp++) + 1 /*nul*/)
        /**/;

    if (NULL == (ret = (char *)calloc(len * sizeof(char), 1))) {
        return NULL;
    }

    /*
     *  Build the environment block by concatenating the environment
     *  strings with nulls between and two null bytes at the end
     */
    for (cursor = ret, vp = envp, len = 0; *vp; ++len, ++vp) {
        const size_t slen = strlen(*vp) + 1 /*nul*/;
        memcpy(cursor, *vp, slen * sizeof(char));
        cursor += slen;
    }

    if (cursor == ret) *cursor++ = '\0';
    *cursor = '\0';                             // final terminator.

    return ret;
}


/*
 *  BuildVectorsW --- build up command line/environ vectors.
 *
 *      Set up the block forms of the environment and the command line. If "envp" is
 *      null, "_wenviron" is used instead. File handle info is passed in the environment
 *      if _fileinfo is !0.
 */
static int
BuildVectorsW(win32_spawnw_t *args, wchar_t **argblk,  wchar_t **envblk)
{
    *envblk = NULL;

    if (NULL == (*argblk = BuildArgW(args->cmd, args->argv))) {
        return FALSE;
    }

    if (args->envv) {
        if (NULL == (*envblk = BuildEnvW(args->envv))) {
            free(*argblk);
            *argblk = NULL;
            return FALSE;
        }
    }

    return TRUE;
}


static wchar_t *
BuildArgW(const wchar_t *cmd, const wchar_t **argv)
{
    const wchar_t * const *vp;
    wchar_t *ret;
    int len;

    /*
     *  Allocate space for environment strings, count the number of bytes
     *  in the environment strings including nulls between strings
     */
    if (cmd) {
        len = (int)wcslen(cmd) +  1 /*nul*/;
        assert(NULL == argv);
    } else {
        for (vp = argv, len = 2 /*quotes*/ + 2 /*delim*/; *vp; len += (int)wcslen(*vp++) + 1 /*nul*/)
            /**/;
    }

    if (len > (int)(32 * 1024 * sizeof(wchar_t))) {
        errno = E2BIG;                          // command line too long >32k.
        return NULL;
    }

    if (NULL == (ret = (wchar_t *)calloc(len * sizeof(wchar_t), 1))) {
        return NULL;
    }

    /*
     *  Build the command line by concatenating the argument strings
     *  with spaces between, and two null bytes at the end.
     */
    if (cmd) {
        wcscpy(ret, cmd);

    } else {
        wchar_t *cursor = ret;

        vp = argv;
        for (len = 0; *vp; ++len) {
            const wchar_t *arg = *vp++;
            int quote = FALSE;

            if (0 == len && *arg != '"' && wcschr(arg, ' ')) {
                quote = TRUE;                   // quote, contains space.
            }

            if (quote) *cursor++ = '"';
            if (0 == len) {
                while (*arg) {                  // convert slashs within arg0.
                    *cursor++ = ('/' == *arg ? '\\' : *arg);
                    ++arg;
                }
            } else {
                wcscpy(cursor, arg);
                cursor += wcslen(arg);
            }
            if (quote) *cursor++ = '"';

            *cursor++ = ' ';                    // space delimiter.
        }

        cursor[-1] = '\0';                      // remove extra delimiter.
        *cursor = '\0';                         // terminator.
    }

    return ret;
}


static wchar_t *
BuildEnvW(const wchar_t **envv)
{
    /*
     *  Set up the block forms of the environment and the command line.
     *  If "envv" is null, "_environ" is used instead.
     */
    const wchar_t **envp =
#if defined(__WATCOMC__)
            (envv ? envv : (const wchar_t **)_wenviron);
#else
            (envv ? envv : (const wchar_t **)_wenviron);
#endif
    const wchar_t * const *vp;
    wchar_t *ret, *cursor;
    int len;

    /*
     *  Allocate space for environment strings, count the number of bytes
     *  in the environment strings including nulls between strings
     */
    for (vp = envp, len = 2 /*padding*/; *vp; len += (int)wcslen(*vp++) + 1 /*nul*/)
        /**/;

    if (NULL == (ret = (wchar_t *)calloc(len * sizeof(wchar_t), 1))) {
        return NULL;
    }

    /*
     *  Build the environment block by concatenating the environment
     *  strings with nulls between and two null bytes at the end
     */
    for (cursor = ret, vp = envp, len = 0; *vp; ++len, ++vp) {
        const size_t slen = wcslen(*vp) + 1 /*nul*/;
        memcpy(cursor, *vp, slen * sizeof(wchar_t));
        cursor += slen;
    }

    if (cursor == ret) *cursor++ = '\0';
    *cursor = '\0';                             // final terminator.

    return ret;
}


/*
 *  Getpath --- extract a pathname from an environment variable
 *
 *      To extract the next pathname from a semicolon-delimited list of pathnames
 *      (usually the value on an environment variable) and copy it to a
 *      caller-specified buffer. No check is done to see if the path is valid. The
 *      maximum number of characters copied to the buffer is maxlen - 1 (and then a
 *      '\0' is appended).
 *
 *      If we hit a quoted string, then allow any characters inside. For example, to
 *      put a semi-colon in a path, the user could have an environment variable that
 *      looks like:
 *
 *      PATH=C:\BIN;"D:\CRT\TOOLS;B1";C:\BINP
 */
static const char *
GetpathA(const char *src, char *dst, unsigned maxlen)
{
    const char *save_src;

    /* Strip off leading semi colons */
    while (*src == ';') {
        ++src;
    }

    /* Save original src pointer */
    save_src = src;

    /* Decrement maxlen to allow for the terminating _T('\0') */
    if (--maxlen == 0) {
        goto appendnull;
    }

    /* Get the next path in src string */
    while (*src && (*src != ';')) {
        if (*src != '"')  {                     // check for quote char
            *dst++ = *src++;
            if (--maxlen == 0) {
                save_src = src;                 // ensure NULL return
                goto appendnull;
            }

        } else {                                // quoted
            /* Copy all chars until we hit the final quote or the EOS */
            src++;                              // skip over opening quote
            while (*src && (*src != '"')) {
                *dst++ = *src++;
                if ( --maxlen == 0 ) {
                    save_src = src;             // ensure NULL return
                    goto appendnull;
                }
            }

            if (*src) {
                src++;                          // skip over closing quote
            }
        }
    }

    /*
     *  If we copied something and stopped because of a ';', skip ';'
     *  before returning
     */
    while (*src == ';') {
        ++src;
    }

    /*
     *  Store a terminating null.
     */
appendnull:
    *dst = '\0';
    return((save_src != src) ? (const char *)src : NULL);
}


static const wchar_t *
GetpathW(const wchar_t *src, wchar_t *dst, unsigned maxlen)
{
    const wchar_t *save_src;

    /* Strip off leading semi colons */
    while (*src == ';') {
        ++src;
    }

    /* Save original src pointer */
    save_src = src;

    /* Decrement maxlen to allow for the terminating _T('\0') */
    if (--maxlen == 0) {
        goto appendnull;
    }

    /* Get the next path in src string */
    while (*src && (*src != ';')) {
        if (*src != '"')  {                     // check for quote char
            *dst++ = *src++;
            if (--maxlen == 0) {
                save_src = src;                 // ensure NULL return
                goto appendnull;
            }

        } else {                                // quoted
            /* Copy all chars until we hit the final quote or the EOS */
            src++;                              // skip over opening quote
            while (*src && (*src != '"')) {
                *dst++ = *src++;
                if ( --maxlen == 0 ) {
                    save_src = src;             // ensure NULL return
                    goto appendnull;
                }
            }

            if (*src) {
                src++;                          // skip over closing quote
            }
        }
    }

    /*
     *  If we copied something and stopped because of a ';', skip ';'
     *  before returning
     */
    while (*src == ';') {
        ++src;
    }

    /*
     *  Store a terminating null.
     */
appendnull:
    *dst = '\0';
    return ((save_src != src) ? (const wchar_t *)src : NULL);
}


static const char *
GetenvA(const char *const *envp, const char *val)
{
    const char *p = NULL;

    if (envp) {                                 // Search local path
        size_t len;

        len = strlen(val);
        while (*envp) {
            if (strlen(*envp) > len && *(*envp + len) == '=' &&
                    strncmp(*envp, val, len) == 0) {
                p = *envp + len + 1;
                break;
            }
            ++envp;
        }
    }

    if (p == (const char *)NULL) {              // Global path
        p = getenv(val);
    }

    return (p);
}


static const wchar_t *
GetenvW(const wchar_t *const *envp, const wchar_t *val)
{
    const wchar_t *p = NULL;

    if (envp) {                                 // Search local path
        size_t len;

        len = wcslen(val);
        while (*envp) {
            if (wcslen(*envp) > len && *(*envp + len) == '=' &&
                    wcsncmp(*envp, val, len) == 0) {
                p = *envp + len + 1;
                break;
            }
            ++envp;
        }
    }

    if (p == (const wchar_t *)NULL) {           // Global path
        p = _wgetenv(val);
    }

    return (p);
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
    char t_rcbuffer[512], buffer[1024];
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
    wchar_t t_rcbuffer[512], buffer[1024];
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
