#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_itimer_c,"$Id: w32_itimer.c,v 1.1 2020/03/28 00:26:03 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 itimer system calls -- INCOMPLETE, signal interface required.
 *
 * Copyright (c) 2018 - 2019, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#include <sys/cdefs.h>
#include "win32_internal.h"
#include <win32_time.h>
#if defined(HAVE_SYS_UTIME_H) ||\
        defined(__MINGW32__)
#include <sys/utime.h>
#endif
#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>


/*
//  NAME
//
//      getitimer, setitimer - get and set value of interval timer
//
//  SYNOPSIS
//
//      #include <sys/time.h>
//
//      int getitimer(int which, struct itimerval *value);
//      int setitimer(int which, const struct itimerval *restrict value, struct itimerval *restrict ovalue);
//
//  DESCRIPTION
//
//      The getitimer() function shall store the current value of the timer specified by which into the structure pointed to by value.
//      The setitimer() function shall set the timer specified by which to the value specified in the structure pointed to by value,
//      and if ovalue is not a null pointer, store the previous value of the timer in the structure pointed to by ovalue.
//
//      A timer value is defined by the itimerval structure, specified in <sys/time.h>. If it_value is non-zero, it shall indicate the
//      time to the next timer expiration. If it_interval is non-zero, it shall specify a value to be used in reloading it_value when
//      the timer expires. Setting it_value to 0 shall disable a timer, regardless of the value of it_interval. Setting it_interval
//      to 0 shall disable a timer after its next expiration (assuming it_value is non-zero).
//
//      Implementations may place limitations on the granularity of timer values. For each interval timer, if the requested timer value
//      requires a finer granularity than the implementation supports, the actual timer value shall be rounded up to the next supported value.
//
//      An XSI-conforming implementation provides each process with at least three interval timers, which are indicated by the which argument:
//
//      ITIMER_PROF
//          Decrements both in process virtual time and when the system is running on behalf of the process.
//          It is designed to be used by interpreters in statistically profiling the execution of interpreted programs.
//          Each time the ITIMER_PROF timer expires, the SIGPROF signal is delivered.
//
//      ITIMER_REAL
//          Decrements in real time. A SIGALRM signal is delivered when this timer expires.
//
//      ITIMER_VIRTUAL
//          Decrements in process virtual time. It runs only when the process is executing.
//          A SIGVTALRM signal is delivered when it expires.
//
//      The interaction between setitimer() and alarm() or sleep() is unspecified.
//
//  RETURN VALUE
//
//      Upon successful completion, getitimer() or setitimer() shall return 0; otherwise, -1 shall be returned and errno set to indicate the error.
//
//  ERRORS
//
//      The setitimer() function shall fail if:
//
//      [EINVAL]
//          The value argument is not in canonical form. (In canonical form, the number of microseconds is a non-negative integer less
//          than 1000000 and the number of seconds is a non-negative integer.)
//
//      The getitimer() and setitimer() functions may fail if:
//
//      [EINVAL]
//          The which argument is not recognized.
//
*/

struct itimer {
#define USE_THREAD
    HANDLE handle, task;
    CRITICAL_SECTION lock;
    struct itimerval value;
};

static struct itimer itimer = {INVALID_HANDLE_VALUE};

LIBW32_API int
getitimer(int which, struct itimerval *value)
{
    assert(value);

    if (value == NULL) {
        errno = EFAULT;
        return -1;
    }
    *value = itimer.value;
    return 0;
}


#if defined(USE_THREAD)
static DWORD WINAPI
timer_thread(LPVOID param)
{
    DWORD waitms = INFINITE;

    for (;;) {
        int rc;
        rc = WaitForSingleObjectEx(itimer.handle, waitms, FALSE);
        if (WAIT_OBJECT_0 == rc) {
            EnterCriticalSection(&itimer.lock);

            if (itimer.value.it_value.tv_sec == 0 &&
                    itimer.value.it_value.tv_usec == 0) {
                waitms = INFINITE;              /* cancel */

            } else {                            /* reschedule */
                waitms = (itimer.value.it_value.tv_usec + 999) / 1000 +
                itimer.value.it_value.tv_sec * 1000;
            }

            ResetEvent(itimer.handle);
            LeaveCriticalSection(&itimer.lock);

        } else if (WAIT_TIMEOUT == rc) {        /* trigger */
            w32_raise(SIGALRM);
            waitms = INFINITE;

        } else {
            assert(false); //debug trap
        }
    }
    return 0;
}

#else
static void CALLBACK
timer_completion(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
    //TODO
    w32_raise(SIGALRM);
}
#endif


LIBW32_API int
setitimer(int which, const struct itimerval *value, struct itimerval *ovalue)
{
    int ret = 0;

    assert(value);
    assert(which == ITIMER_REAL);

    //
    //  initialisation
    if (INVALID_HANDLE_VALUE == itimer.handle) {
        InitializeCriticalSection(&itimer.lock);
        EnterCriticalSection(&itimer.lock);
            //possible initialisation race; rare if never.
#if defined(USE_THREAD)
        if (NULL != (itimer.handle = CreateEvent(NULL, TRUE, FALSE, NULL))) {
            if (INVALID_HANDLE_VALUE == (itimer.task = CreateThread(NULL, 0, timer_thread, NULL, 0, NULL))) {
                CloseHandle(itimer.handle);
                itimer.handle = NULL;
            }
        }
#else
        itimer.handle = CreateWaitableTimer(NULL, TRUE, NULL);
#endif
        LeaveCriticalSection(&itimer.lock);
    }

    if (NULL == itimer.handle) {
        errno = EINVAL;
        return -1;
    }

    if (NULL == value) {
        errno = EFAULT;
        return -1;
    }

    //
    //  execute
    EnterCriticalSection(&itimer.lock);

    if (ovalue) *ovalue = itimer.value;
    itimer.value = *value;

#if !defined(USE_THREAD)
    if (0 == value->it_value.tv_sec && 0 == value->it_value.tv_usec) {
        CancelWaitableTimer(itimer.handle);

    } else {
        LARGE_INTEGER due;
        due.QuadPart = -(value->it_value.tv_usec * 10 + value->it_value.tv_sec * 10000000L);
            // Create a negative 64-bit integer that will be used to signal the timer xx seconds from now.

        if (! SetWaitableTimer(itimer.handle, &due, 0, timer_completion, NULL, FALSE)) {
            errno = EINVAL;
            ret = -1;
        }
    }
#endif

    LeaveCriticalSection(&itimer.lock);

#if defined(USE_THREAD)
    SetEvent(itimer.handle);
#endif

    return ret;
}

/*end*/




