#ifndef LIBW32_WIN32_ICONV_H_INCLUDED
#define LIBW32_WIN32_ICONV_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_iconv_h,"$Id: win32_iconv.h,v 1.7 2022/02/17 16:05:01 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 iconv dynamic loader.
 *
 * Copyright (c) 2007, 2012 - 2022, Adam Young.
 * All rights reserved.
 *
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
#include <sys/utypes.h>

__BEGIN_DECLS

LIBW32_API void             w32_iconv_dllname(const char *dllname);

LIBW32_API int              w32_iconv_connect(int verbose);
LIBW32_API void             w32_iconv_shutdown(void);

LIBW32_API void *           w32_iconv_open(const char *to, const char *from);
LIBW32_API int              w32_iconv(void *fd, const char **from, size_t *fromlen, char **to, size_t *tolen);
LIBW32_API void             w32_iconv_close(void *fd);

LIBW32_API void *           w32native_iconv_open(const char *tocode, const char *fromcode);
LIBW32_API int              w32native_iconv_close(void *fd);
LIBW32_API size_t           w32native_iconv(void *fd, const char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft);

#if defined(WIN32_ICONV_MAP)
typedef void *iconv_t;

#define iconv_open(__to, __from) w32_iconv_open(__to, __from)
#define iconv(__fd, __from, __fromlen, __to, __tolen) w32_iconv(__fd, __from, __fromlen, __to, __tolen)
#define iconv_close(__fd)   w32_iconv_close(__fd)
#endif /*WIN32_ICONV_MAP*/

__END_DECLS

#endif /*LIBW32_WIN32_ICONV_H_INCLUDED*/
