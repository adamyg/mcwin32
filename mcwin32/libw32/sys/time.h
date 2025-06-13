#ifndef LIBW32_SYS_TIME_H_INCLUDED
#define LIBW32_SYS_TIME_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_time_h,"$Id: time.h,v 1.15 2025/06/11 17:33:57 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 sys/time.h implementation.
 *
 * Copyright (c) 1998 - 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ==end==
 */

#include <sys/cdefs.h>
#if defined(__MINGW32__)
#include_next <sys/time.h>              /* struct timeval */
#else
#include <sys/socket.h>                 /* struct timeval */
#endif
#include <sys/utypes.h>                 /* suseconds_t */
#include <time.h>

#if defined(NEED_TIMEVAL) /*|| defined(__MINGW64_VERSION_MAJOR)*/
#if !defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
//
//  The <sys/time.h> header shall define the timeval structure that includes at
//  least the following members:
//
//      time_t         tv_sec      Seconds.
//      suseconds_t    tv_usec     Microseconds.
//
//  yet current winsock definitions are as follows.
//
#ifndef _TIMEVAL_DEFINED
#define _TIMEVAL_DEFINED
struct timeval {
    long                tv_sec;         /* seconds */
    long                tv_usec;        /* and microseconds */
};
#endif //_TIMEVAL_DEFINED
#endif //_WINSOCK2API_
#endif //NEED_TIMEVAL

struct w32_timeval {
    time_t              tv_sec;         /* seconds */
    suseconds_t         tv_usec;        /* and microseconds */
};

struct itimerval {
    struct timeval      it_interval;    /* timer interval */
    struct timeval      it_value;       /* current value */
};

#if !defined(TIMEVAL_TO_TIMESPEC)
#define TIMEVAL_TO_TIMESPEC(tv, ts) {       \
    (ts)->tv_sec = (tv)->tv_sec;            \
    (ts)->tv_nsec = (tv)->tv_usec * 1000;   \
}
#define TIMESPEC_TO_TIMEVAL(tv, ts) {       \
    (tv)->tv_sec = (ts)->tv_sec;            \
    (tv)->tv_usec = (ts)->tv_nsec / 1000;   \
}
#endif

/* Operations on timevals. */
#if !defined(timerisset)
#define timerisset(tvp)         ((tvp)->tv_sec || (tvp)->tv_usec)
#endif

#if !defined(timercmp)
#define timercmp(tvp, uvp, cmp)             \
    (((tvp)->tv_sec == (uvp)->tv_sec) ?     \
        ((tvp)->tv_usec cmp (uvp)->tv_usec) : \
        ((tvp)->tv_sec cmp (uvp)->tv_sec))
#endif

#if !defined(timerclear)
#define timerclear(tvp)         (tvp)->tv_sec = (tvp)->tv_usec = 0
#endif

#define timeradd(tvp, uvp, vvp)             \
    do {                                    \
        (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec; \
        (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec; \
        if ((vvp)->tv_usec >= 1000000) {    \
            (vvp)->tv_sec++;                \
            (vvp)->tv_usec -= 1000000;      \
        }                                   \
    } while (0)

#define timersub(tvp, uvp, vvp)             \
    do {                                    \
        (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec; \
        (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec; \
        if ((vvp)->tv_usec < 0) {           \
            (vvp)->tv_sec--;                \
            (vvp)->tv_usec += 1000000;      \
        }                                   \
    } while (0)

#include <sys/cdefs.h>

__BEGIN_DECLS

#define ITIMER_REAL     0               /* Decrements in real time. */
    //#define ITIMER_VIRTUAL  1               /* unsupported - Decrements in process virtual time. */
    //#define ITIMER_PROF     2               /* unsupported - Decrements both in process virtual time and when the system is running on behalf of the process. */

LIBW32_API int          getitimer(int which, struct itimerval *value);
LIBW32_API int          setitimer(int which, const struct itimerval *value, struct itimerval *ovalue);

#if defined(_WINSOCKAPI_) || defined(_WINSOCK2API_)
struct timeval;
struct timezone;

LIBW32_API int          w32_gettimeofday(struct timeval *tv, struct timezone *tz);
LIBW32_API int          w32_select(int, fd_set *, fd_set *, fd_set *, const struct timeval *timeout);
#endif

#if defined(NEED_TIMEVAL) || \
        defined(_WINSOCKAPI_) || defined(_WINSOCK2API_)
LIBW32_API int          utimes(const char *, const struct timeval[2]);
#endif

#if defined(_MSC_VER) || defined(__WATCOMC__)
LIBW32_API time_t       timegm(struct tm *tm);
#endif

/*
 *  POSIX 1003.1c -- <time.h>
 *
LIBW32_API char *       ctime_r(const time_t *ctm, char *buf);
LIBW32_API char *       asctime_r(const struct tm *tm, char *buf);
LIBW32_API struct tm *  localtime_r(const time_t *ctm, struct tm *res);
LIBW32_API struct tm *  gmtime_r(const time_t *ctm, struct tm *res);
 */

__END_DECLS

#endif /*LIBW32_SYS_TIME_H_INCLUDED*/
