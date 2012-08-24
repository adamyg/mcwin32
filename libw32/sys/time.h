#ifndef WIN32_SYS_TIME_H
#define WIN32_SYS_TIME_H

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 sys/time.h implementation.
 *
 *
 * ==end==
 */
#include <sys/cdefs.h>
#include <sys/utypes.h>                         /* suseconds_t */
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
    long                tv_sec;                 /* seconds */            
    long                tv_usec;                /* and microseconds */
};
#endif
#endif

struct w32_timeval {
    time_t              tv_sec;                 /* seconds */            
    suseconds_t         tv_usec;                /* and microseconds */
};

struct itimerval {
    struct timeval      it_interval;            /* timer interval */
    struct timeval      it_value;               /* current value */ 
};


#include <sys/cdefs.h>

__BEGIN_DECLS

int                     getitimer(int, struct itimerval *);
int                     setitimer(int, const struct itimerval *__restrict,
                                    struct timeval *__restrict);

#if defined(_WINSOCKAPI_) || defined(_WINSOCK2API_)
int                     w32_gettimeofday(struct timeval *__restrict, /*struct timezone*/ void *__restrict);
int                     w32_select(int, fd_set *__restrict, fd_set *__restrict, fd_set *__restrict,
                                    struct timeval *timeout);
#endif

#if defined(NEED_TIMEVAL) || \
        defined(_WINSOCKAPI_) || defined(_WINSOCK2API_)
int                     utimes(const char *, const struct timeval[2]);
#endif

/*
 *  POSIX 1003.1c -- <time.h>
 */

char * __PDECL          ctime_r(const time_t *ctm, char *buf);
char * __PDECL          asctime_r(const struct tm *tm, char *buf);
struct tm * __PDECL     localtime_r(const time_t *ctm, struct tm *res);
struct tm * __PDECL     gmtime_r(const time_t *ctm, struct tm *res);

__END_DECLS

#endif  /*WIN32_SYS_TIME_H*/
