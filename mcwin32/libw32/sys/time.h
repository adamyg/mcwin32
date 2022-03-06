#ifndef LIBW32_SYS_TIME_H
#define LIBW32_SYS_TIME_H
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_time_h,"$Id: time.h,v 1.7 2022/02/24 15:33:51 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 sys/time.h implementation.
 *
 * Copyright (c) 2012 - 2022, Adam Young.
 * All rights reserved.
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
 */

#include <sys/cdefs.h>
#include <sys/utypes.h>                 /* suseconds_t */
#include <sys/select.h>
#include <time.h>

#if defined(NEED_TIMEVAL)
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
struct timeval {
    long                tv_sec;         /* seconds */
    long                tv_usec;        /* and microseconds */
};
#endif
#endif

struct w32_timeval {
    time_t              tv_sec;         /* seconds */
    suseconds_t         tv_usec;        /* and microseconds */
};

struct itimerval {
    struct timeval      it_interval;    /* timer interval */
    struct timeval      it_value;       /* current value */
};

/*
 -  struct timezone {
 -      int tz_minuteswest;             // minutes west of Greenwich
 -      int tz_dsttime;                 // type of dst correction
 -  };
 */

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
#define ITIMER_VIRTUAL  1               /* Decrements in process virtual time. */
#define ITIMER_PROF     2               /* Decrements both in process virtual time and when the system is running on behalf of the process. */

LIBW32_API int          getitimer(int which, struct itimerval *value);
LIBW32_API int          setitimer(int which, const struct itimerval *value, struct itimerval *ovalue);

#if defined(_WINSOCKAPI_) || defined(_WINSOCK2API_)
LIBW32_API int          w32_gettimeofday(struct timeval *, /*struct timezone*/ void *);
LIBW32_API int          w32_select(int, fd_set *, fd_set *, fd_set *, struct timeval *timeout);
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

#endif  /*LIBW32_SYS_TIME_H*/

