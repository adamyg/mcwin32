#ifndef LIBW32_ERR_H_INCLUDED
#define LIBW32_ERR_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_err_h,"$Id: err.h,v 1.1 2020/05/21 15:27:02 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win <errt.h>
 *
 * Copyright (c) 1998 - 2020, Adam Young.
 * All rights reserved.
 *
 * This file is part of memcached-win32.
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
#include <stdarg.h>

__BEGIN_DECLS

/* "<program>: <format>: <errno>\n", on stderr. */
LIBW32_API void         warn(const char *fmt, ...);
LIBW32_API void         vwarn(const char *fmt, va_list ap);

/* Likewise, but without ": " and the standard error string.  */
LIBW32_API void         warnx(const char *fmt, ...);
LIBW32_API void         vwarnx(const char *fmt, va_list ap);

/* Like above, but the exits using 'eval' */
LIBW32_API void         err(int eval, const char *fmt, ...);
LIBW32_API void         verr(int eval, const char *fmt, va_list ap);

LIBW32_API void         errx(int eval, const char *fmt, ...);
LIBW32_API void         verrx(int eval, const char *fmt, va_list ap);

__END_DECLS

#endif /*WIN32_ERR_H_INCLUDED*/
