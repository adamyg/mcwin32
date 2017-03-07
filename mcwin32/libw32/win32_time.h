#ifndef LIBW32_WIN32TIME_H_INCLUDED
#define LIBW32_WIN32TIME_H_INCLUDED

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 time functionality.
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
 */

#include <sys/cdefs.h>

__BEGIN_DECLS

LIBW32_API unsigned int sleep(unsigned int);

struct timeval;
struct timezone;

LIBW32_API int          gettimeofday(struct timeval *tv, struct timezone *tz);

struct utimbuf;

LIBW32_API int          w32_utime(const char *path, const struct utimbuf *times);

__END_DECLS

#endif /*LIBW32_WIN32TIME_H_INCLUDED*/
