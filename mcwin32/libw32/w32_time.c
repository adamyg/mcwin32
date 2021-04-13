#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_time_c,"$Id: w32_time.c,v 1.6 2021/04/13 15:49:34 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 time system calls.
 *
 * Copyright (c) 2007, 2012 - 2018 Adam Young.
 * All rights reserved.
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

#include <sys/cdefs.h>

#include "win32_internal.h"
#include <win32_time.h>

#include <sys/socket.h>

#if defined(HAVE_SYS_UTIME_H) ||\
        defined(__MINGW32__)
#include <sys/utime.h>
#endif
#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif

#if defined(_MSC_VER) || defined(__WATCOMC__)
#include <sys/timeb.h>
#endif
#include <time.h>
#include <unistd.h>
#include <assert.h>


/*
//  NAME
//      sleep - suspend execution for an interval of time
//
//  SYNOPSIS
//      #include <unistd.h>
//
//      unsigned sleep(unsigned seconds);
//
//  DESCRIPTION
//      The sleep() function shall cause the calling thread to be suspended from execution
//      until either the number of realtime seconds specified by the argument seconds has
//      elapsed or a signal is delivered to the calling thread and its action is to invoke
//      a signal-catching function or to terminate the process. The suspension time may be
//      longer than requested due to the scheduling of other activity by the system.
//
//      If a SIGALRM signal is generated for the calling process during execution of
//      sleep() and if the SIGALRM signal is being ignored or blocked from delivery, it is
//      unspecified whether sleep() returns when the SIGALRM signal is scheduled. If the
//      signal is being blocked, it is also unspecified whether it remains pending after
//      sleep() returns or it is discarded.
//
//      If a SIGALRM signal is generated for the calling process during execution of
//      sleep(), except as a result of a prior call to alarm(), and if the SIGALRM signal
//      is not being ignored or blocked from delivery, it is unspecified whether that
//      signal has any effect other than causing sleep() to return.
//
//      If a signal-catching function interrupts sleep() and examines or changes either the
//      time a SIGALRM is scheduled to be generated, the action associated with the SIGALRM
//      signal, or whether the SIGALRM signal is blocked from delivery, the results are
//      unspecified.
//
//      If a signal-catching function interrupts sleep() and calls siglongjmp() or
//      longjmp() to restore an environment saved prior to the sleep() call, the action
//      associated with the SIGALRM signal and the time at which a SIGALRM signal is
//      scheduled to be generated are unspecified. It is also unspecified whether the
//      SIGALRM signal is blocked, unless the process' signal mask is restored as part of
//      the environment.
//
//      Interactions between sleep() and any of setitimer(), ualarm(), or usleep() are
//      unspecified.
//
//  RETURN VALUE
//      If sleep() returns because the requested time has elapsed, the value returned shall
//      be 0. If sleep() returns due to delivery of a signal, the return value shall be the
//      "unslept" amount (the requested time minus the time actually slept) in seconds.
//
//  ERRORS
//
//      No errors are defined.
*/
unsigned int
sleep (unsigned int secs)
{
    Sleep((DWORD)secs * 1000);
    return (0);
}

unsigned int
w32_sleep (unsigned int secs)
{
    Sleep((DWORD)secs * 1000);
    return (0);
}


/*
//  NAME
//
//      gettimeofday - get the date and time
//
//  SYNOPSIS
//
//      #include <sys/time.h>
//
//      int gettimeofday(struct timeval *restrict tp, void *restrict tzp);
//
//  DESCRIPTION
//
//      The  gettimeofday()  function  shall  obtain  the  current  time,
//      expressed  as  seconds  and  microseconds  since  the  Epoch, and
//      store  it  in  the  timeval  structure  pointed  to  by  tp.  The
//      resolution of the system clock is unspecified.
//
//      If tzp is not a null pointer, the behavior is unspecified.
//
//  RETURN VALUE
//
//      The  gettimeofday()  function  shall  return 0 and no value shall
//      be reserved to indicate an error.
//
//  ERRORS
//
//      No errors are defined.
//
*/

LIBW32_API int
w32_gettimeofday(
    struct timeval *tv, /*struct timezone*/ void *tz)
{
    __CUNUSED(tz)
    if (tv) {
#if defined(_MSC_VER) || defined(__WATCOMC__)
        struct _timeb lt;

        _ftime(&lt);
        tv->tv_sec = (long)(lt.time + lt.timezone);
        tv->tv_usec = lt.millitm * 1000;
        assert(4 == sizeof(tv->tv_sec));

#elif defined(__MINGW32__)
#undef gettimeofday
        return gettimeofday(tv, tz)
 
#else //DEFAULT
        FILETIME ft;
        long long hnsec;
 
        (void) GetSystemTimeAsFileTime(&ft);
        hnsec = filetime_to_hnsec(&ft);
        tv->tv_sec = hnsec / 10000000;
        tv->tv_usec = (hnsec % 10000000) / 10;
#endif
        return 0;
    }
    errno = EINVAL;
    return -1;
}


#if !defined(__MINGW32__)
LIBW32_API int
gettimeofday(
    struct timeval *tv, struct timezone *tz)
{
    return w32_gettimeofday(tv, tz);
}
#endif


/*
//  NAME
//
//      utime - set file access and modification times
//
//  SYNOPSIS
//
//      #include <utime.h>
//
//      int utime(const char *path, const struct utimbuf *times);
//
//  DESCRIPTION
//
//      The  utime()  function  shall  set  the  access  and modification
//      times of the file named by the path argument.
//
//      If  times  is  a  null pointer, the access and modification times
//      of  the  file  shall  be  set  to the current time. The effective
//      user  ID  of  the  process  shall match the owner of the file, or
//      the  process  has write permission to the file or has appropriate
//      privileges, to use utime() in this manner.
//
//      If  times  is not a null pointer, times shall be interpreted as a
//      pointer  to  a  utimbuf structure and the access and modification
//      times  shall  be  set  to  the values contained in the designated
//      structure.  Only  a  process  with the effective user ID equal to
//      the   user   ID  of  the  file  or  a  process  with  appropriate
//      privileges may use utime() this way.
//
//      The  utimbuf  structure  is  defined in the <utime.h> header. The
//      times  in  the  structure  utimbuf  are measured in seconds since
//      the Epoch.
//
//      Upon  successful  completion,  utime() shall mark the time of the
//      last   file   status  change,   st_ctime,   to  be  updated;  see
//      <sys/stat.h>.
//
//  RETURN VALUE
//
//      Upon  successful  completion,  0 shall be returned. Otherwise, -1
//      shall  be  returned and errno shall be set to indicate the error,
//      and the file times shall not be affected.
//
//  ERRORS
//
//      The utime() function shall fail if:
//
//      [EACCES]
//          Search  permission  is  denied  by  a  component  of the path
//          prefix;  or  the  times  argument  is  a null pointer and the
//          effective  user  ID  of  the process does not match the owner
//          of  the  file, the process does not have write permission for
//          the   file,   and  the  process  does  not  have  appropriate
//          privileges.
//
//      [ELOOP]
//          A   loop   exists   in   symbolic  links  encountered  during
//          resolution of the path argument.
//
//      [ENAMETOOLONG]
//          The  length  of  the  path  argument  exceeds {PATH_MAX} or a
//          pathname component is longer than {NAME_MAX}.
//
//      [ENOENT]
//          A  component  of  path does not name an existing file or path
//          is an empty string.
//
//      [ENOTDIR]
//          A component of the path prefix is not a directory.
//
//      [EPERM]
//          The  times  argument  is  not  a null pointer and the calling
//          process'  effective  user  ID does not match the owner of the
//          file  and  the  calling process does not have the appropriate
//          privileges.
//
//      [EROFS]
//          The file system containing the file is read-only.
//
//      The utime() function may fail if:
//
//      [ELOOP]
//          More  than  {SYMLOOP_MAX}  symbolic  links  were  encountered
//          during resolution of the path argument.
//
//      [ENAMETOOLONG]
//          As  a  result  of  encountering a symbolic link in resolution
//          of   the  path  argument,   the  length  of  the  substituted
//          pathname string exceeded {PATH_MAX}.
//
*/
LIBW32_API int
w32_utime(const char *path, const struct utimbuf *times)
{
#if defined(__MINGW32__)
#undef utime
    return utime(path, (struct utimbuf *)times);
#else
    return _utime(path, (struct _utimbuf *)times);
#endif
}

/*end*/

