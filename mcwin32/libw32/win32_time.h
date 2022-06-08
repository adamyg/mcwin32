#ifndef LIBW32_WIN32_TIME_H_INCLUDED
#define LIBW32_WIN32_TIME_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_time_h,"$Id: win32_time.h,v 1.13 2022/06/08 09:51:45 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 time functionality.
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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
 */

#include <sys/cdefs.h>
#include <time.h>

__BEGIN_DECLS

LIBW32_API unsigned int sleep(unsigned int);

struct timeval;
struct timezone;

LIBW32_API int          w32_gettimeofday(struct timeval *tv, struct timezone *tz);
#if !defined(HAVE_GETTIMEOFDAY)
#define HAVE_GETTIMEOFDAY
#if !defined(__MINGW32__) && !defined(_MSC_VER)
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
