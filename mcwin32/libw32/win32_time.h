#ifndef LIBW32_WIN32_TIME_H_INCLUDED
#define LIBW32_WIN32_TIME_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_time_h,"$Id: win32_time.h,v 1.18 2025/03/06 16:59:47 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 time functionality.
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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
 */

#include <sys/cdefs.h>
#include <sys/types.h>
#include <time.h>

__BEGIN_DECLS

#if !defined(USECONDS_T)
#define USECONDS_T 1
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
typedef unsigned long useconds_t __MINGW_ATTRIB_DEPRECATED;

#elif !defined(__MINGW32__)
#ifdef _WIN64
typedef unsigned long long useconds_t;
#else
typedef unsigned long useconds_t;
#endif
#endif /*__MINGW32__*/
#endif /*USECONDS_T*/

LIBW32_API unsigned int sleep(unsigned int);

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" /*useconds_t, POSIX.1-2008*/
#endif
LIBW32_API int          usleep(useconds_t useconds);
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
#pragma GCC diagnostic pop
#endif

struct timespec;
LIBW32_API int          nanosleep(const struct timespec *rqtp, struct timespec *rmtp /*notused*/);

struct timeval;
struct timezone;

LIBW32_API int          w32_gettimeofday(struct timeval *tv, struct timezone *tz);
#if !defined(HAVE_GETTIMEOFDAY)
#define HAVE_GETTIMEOFDAY
#if !defined(__MINGW32__)
LIBW32_API int          gettimeofday(struct timeval *tv, struct timezone *tz);
#endif
#endif

struct utimbuf;

LIBW32_API int          w32_utime(const char *path, const struct utimbuf *times);
LIBW32_API int          w32_utimeA(const char *path, const struct utimbuf *times);
LIBW32_API int          w32_utimeW(const wchar_t *path, const struct utimbuf *times);

#if (0) //libcompat
#if defined(_MSC_VER) || defined(__WATCOMC__)
LIBW32_API time_t       timegm(struct tm *tm);
#endif
#endif //libcompat

LIBW32_API char *       w32_strptime(const char *buf, const char *fmt, struct tm *tm);

__END_DECLS

#endif /*LIBW32_WIN32_TIME_H_INCLUDED*/
