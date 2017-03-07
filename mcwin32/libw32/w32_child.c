/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 sub-process support
 *
 * Copyright (c) 2007, 2012 - 2017 Adam Young.
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
 *
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

static int              cmdis(const char *shell, int slen, const char *cmd);

static BOOL             SendCloseMessage(HANDLE hProc);
static BOOL             WASNOT_ENOENT(void);

static int              BuildVectors(win32_spawn_t *args, char **argblk, char **envblk);
static char *           Getpath(const char *src, char *dst, unsigned maxlen);
static const char *     Getenv(const char *const *envp, const char *val);
static HANDLE           ExecChild(win32_spawn_t *args,
			    const char *arg0, char *argv, char *envp, STARTUPINFO *si, PROCESS_INFORMATION *pi);
static void             DisplayError(HANDLE hOutput, const char *pszAPI, const char *args);
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
int
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
        if (w32_child_wait((HANDLE)pid, status, options & WNOHANG)) {
            ret = pid;
        }
    }
    return ret;
}


int
WEXITSTATUS(int status)
{
    return ((status) >> 8) & 0xff;
}


int
WCOREDUMP(int status)
{
    return 0;
}


int
WTERMSIG(int status)
{
    return (status & 0xff);
}


int
WIFSIGNALED(int status) 
{
    return (WTERMSIG (status) != 0);
}


int
WIFEXITED(int status)
{
    return (WTERMSIG (status) == 0);
}


int
WIFSTOPPED(int status)
{
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
int
w32_kill(int pid, int value)
{
    if (pid > 0) {
        HANDLE hProc = (HANDLE)pid;

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
            || GetExitCodeProcess(info->hProcess, &status)
                && status == STILL_ACTIVE       // value = 259
                && PostMessage(hwnd, WM_CLOSE, 0, 0);
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
int
w32_iscommand(const char *shell)
{
    const int slen = (int)strlen(shell);

    if (cmdis(shell, slen, "cmd") ||
            cmdis(shell, slen, "cmd.exe") ||
            cmdis(shell, slen, "command") ||
            cmdis(shell, slen, "command.com") ||
            cmdis(shell, slen, "command.exe")) {
        return TRUE;
    }
    return FALSE;
}


static int
cmdis(const char *shell, int slen, const char *cmd)
{
    const int clen = (int)strlen(cmd);
    const char *p = shell+slen-clen;

    if (slen == clen || (slen > clen && (p[-1] == '\\' || p[-1] == '/'))) {
        if (_stricmp(p, cmd) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}


/*
 *  w32_child_exec ---
 *      Setup a STARTUPINFO structure and launches redirected child using
 *      the specified stdin/stdout and stderr handles.
 *
 *      This is a low level interface and expects the caller has setup the 
 *      calling environment.
 */
HANDLE
w32_child_exec(
    struct win32_spawn *args, HANDLE hStdIn, HANDLE hStdOut, HANDLE hStdErr)
{
    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = {0};
    HANDLE hProc = 0;
    char *argblk;
    char *envblk;

    /* 
     *  Set up the start up info struct 
     *      USESTDHANDLES,
     *          The hStdInput, hStdOutput, and hStdError members contain additional information.
     *
     *          If this flag is specified when calling one of the process creation functions, 
     *          the handles must be inheritable and the function's bInheritHandles parameter 
     *          must be set to TRUE. For more information;
     */
    (void) memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.wShowWindow = SW_HIDE;                   //SW_NORMAL, SW_SHOWMINIMIZED
    si.hStdInput  = hStdIn;
    si.hStdOutput = hStdOut;
    si.hStdError  = hStdErr;

    if (hStdIn)  { DWORD flags; assert(GetHandleInformation(hStdIn,  &flags) && (HANDLE_FLAG_INHERIT & flags)); }
    if (hStdOut) { DWORD flags; assert(GetHandleInformation(hStdOut, &flags) && (HANDLE_FLAG_INHERIT & flags)); }
    if (hStdErr) { DWORD flags; assert(GetHandleInformation(hStdErr, &flags) && (HANDLE_FLAG_INHERIT & flags)); }

    si.dwFlags = STARTF_USESTDHANDLES;
    si.dwFlags |= STARTF_USESHOWWINDOW;

    /* 
     *  Build env and command line. 
     */
    if (BuildVectors(args, &argblk, &envblk) != 0) {
        InternalError("building arg and env");
    }

    /* 
     *  Launch the process that you want to redirect. 
     */
    if (0 == (hProc = ExecChild(args, NULL, argblk, envblk, &si, &pi))) {
        const char *path, *cmd =
                (args->argv ? args->argv[0] : args->cmd);
        char *pfin, *buf = NULL;

        //  Complete if,
        //      Error != ERROR_FILE_NOT_FOUND
        //      arg0 contains a '/'.
        //  or  PATH is NULL
        //
        if (WASNOT_ENOENT() || strchr(cmd, XSLASHCHAR) != NULL ||
                NULL == (path = Getenv(args->envp, "PATH"))) {
            goto done;
        }

        if ((buf = calloc(_MAX_PATH + 1, 1)) == (const char *)NULL) {
            goto done;
        }

        while (NULL != (path = Getpath(path, buf, _MAX_PATH-1)) && (*buf)) {
            /* If necessary, append a SLASH */
            pfin = buf + strlen(buf) - 1;
            if (*pfin != SLASHCHAR && *pfin != XSLASHCHAR)
                (void) strcat(buf, SLASH);

            /* Check length */
            if (strlen(buf) + strlen(cmd) < _MAX_PATH) {
                strcat(buf, cmd);
            } else {
                break;
            }

            //  Try spawning it. if successful, or if errno comes back
            //  with a value other than ENOENT and the pathname is not
            //  a UNC name, return to the caller.
            //
            if ((hProc = ExecChild(args, buf, argblk, envblk, &si, &pi)) != 0 ||
                    ((WASNOT_ENOENT()) && (!ISSLASH(*buf) || !ISSLASH(*(buf+1))))) {
                break;
            }
        }

done:;  if (buf != NULL) {
            free(buf);
        }
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
ExecChild(win32_spawn_t *args,
    const char *arg0, char *argv, char *envp, STARTUPINFO *si, PROCESS_INFORMATION *pi)
{
    HANDLE hProc = 0;

    if (CreateProcess(
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

    } else if (WASNOT_ENOENT()) {               // XXX, current or hStdError ??
        DisplayError(GetStdHandle(STD_OUTPUT_HANDLE), "CreateProcess", argv);
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
BOOL
w32_child_wait(HANDLE hProc, int *status, int nowait)
{
    DWORD dwStatus, rc;
    BOOL ret = FALSE;

    /*
     *  Explicitly check for process_id being -1 or -2.
     *
     *  In Windows NT, -1 is a handle on the current process, -2 is the
     *  current thread, and it is perfectly legal to to wait (forever) on either.
     */
    if (hProc == (HANDLE)-1 || hProc == (HANDLE)-2) {
        errno = ECHILD;

    /*
     *  Wait for child process, then fetch its exit code
     */
    } else if ((rc = WaitForSingleObject(hProc, (nowait ? 0 : INFINITE))) == WAIT_OBJECT_0 &&
                        GetExitCodeProcess(hProc, (LPDWORD)&dwStatus)) {
        /*
         *  Normal termination:     lo-byte = 0,            hi-byte = child exit code.
         *  Abnormal termination:   lo-byte = term status,  hi-byte = 0.
         */
        CloseHandle(hProc);                     // process complete
        if (status) {
            if (0 == (dwStatus & 0xff)) {
                *status = (int)dwStatus >> 8;
            } else {
                *status = (int)dwStatus;
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
 *  BuildVectors --- build up command line/environ vectors.
 *
 *      Set up the block forms of the environment and the command line. If "envp" is
 *      null, "_environ" is used instead. File handle info is passed in the environment
 *      if _fileinfo is !0.
 */
static int
BuildVectors(win32_spawn_t *args, char **argblk, char **envblk)
{
    const char **envp =
#if defined(__WATCOMC__)
            (args->envp ? args->envp : (const char **)environ);
#else
            (args->envp ? args->envp : (const char **)_environ);
#endif
    const char * const *vp;
    int tmp;
    char *cptr;

    /*
     *  Allocate space for command line string.  tmp counts the number of
     *  bytes in the command line string including spaces between arguments
     *  plus two charcater for possible quoting.
     */
    if (args->cmd) {
        tmp = (int)strlen(args->cmd) + 1;
    } else {
        for (vp = args->argv, tmp = 2+2; *vp; tmp += strlen(*vp++) + 1)
            /**/;
    }

    if (tmp > 32767) {
        errno = E2BIG;                          // command line too long >32k
        return -1;
    }

    /* Allocate space for the command line plus 2 null bytes */
    if ((*argblk = calloc(tmp * sizeof(char), 1)) == NULL) {
        *envblk = NULL;
        return -1;
    }

    /*
     *  Allocate space for environment strings tmp counts the number of bytes
     *  in the environment strings including nulls between strings
     */
    for (vp = envp, tmp = 2; *vp; tmp += strlen(*vp++) + 1)
        /**/;

    /* Allocate space for the environment strings plus extra null byte */
    if (NULL == (*envblk = calloc(tmp * sizeof(char), 1))) {
        free(*argblk);
        *argblk = NULL;
        return -1;
    }

    /*
     *  Build the command line by concatenating the argument strings
     *  with spaces between, and two null bytes at the end.
     */
    cptr = *argblk;

    if (args->cmd) {
        strcpy(cptr, args->cmd);
        cptr += strlen(args->cmd);

    } else {
        vp = args->argv;
        for (tmp = 0; *vp; ++tmp) {
            const char *arg = *vp++;
            int quote = FALSE;

            if (0 == tmp && *arg != '"' && strchr(arg, ' ')) {
                ++quote;                        // quote, contains space
            }

            if (quote) *cptr++ = '"';
            if (0 == tmp) {
                while (*arg) {                  // convert slashs within arg0
                    *cptr++ = ('/' == *arg ? '\\' : *arg);
                    ++arg;
                }
            } else {
                strcpy(cptr, arg);
                cptr += strlen(arg);
            }
            if (quote) *cptr++ = '"';

            *cptr++ = ' ';
        }
        *cptr = cptr[-1] = '\0';                // remove extra blank
    }


    /*
     *  Build the environment block by concatenating the environment
     *  strings with nulls between and two null bytes at the end
     */
    cptr = *envblk;
    vp = envp;

    for (tmp = 0; *vp; ++tmp) {
        (void) strcpy(cptr, *vp);
        cptr += strlen(*vp++) + 1;
    }

    /*
     *  Empty environment block ... this requires two nulls.
     */
    if (cptr != NULL) {
        if (cptr == *envblk) {
            *cptr++ = '\0';
        }
        *cptr = '\0';                           // Extra \0 terminates
    }
    return 0;
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
static char *
Getpath(const char *src, char *dst, unsigned maxlen)
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
    while (*src == ';')
        ++src;

    /*
     *  Store a terminating null.
     */
appendnull:
    *dst = '\0';
    return((save_src != src) ? (char *)src : NULL);
}


static const char *
Getenv(const char *const *envp, const char *val)
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
            envp++;
        }
    }

    if (p == (const char *)NULL) {              // Global path
        p = getenv(val);
    }

    return (p);
}


/*
 *  WaiInternalError ---
 *      Displays the error number and corresponding message.
 */
static void
DisplayError(
    HANDLE hOutput, const char *pszAPI, const char *args)
{
    const DWORD rc = GetLastError();
    LPVOID  lpvMessageBuffer;
    CHAR    szPrintBuffer[512];
    DWORD   nCharsWritten;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, rc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&lpvMessageBuffer, 0, NULL);

    _snprintf(szPrintBuffer, sizeof(szPrintBuffer),
        "Internal Error: %s = %d (%s).\n%s%s", pszAPI, rc, (char *)lpvMessageBuffer,
            args ? args : "", args ? "\n" : "" );

    WriteConsole(hOutput, szPrintBuffer, lstrlen(szPrintBuffer), &nCharsWritten, NULL);
    LocalFree(lpvMessageBuffer);
}


static void
InternalError(
    const char *pszAPI)
{
    DisplayError(GetStdHandle(STD_OUTPUT_HANDLE), pszAPI, NULL);
    ExitProcess(GetLastError());
}

