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
