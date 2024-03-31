#ifndef LIBW32_ERR_H_INCLUDED
#define LIBW32_ERR_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_err_h,"$Id: err.h,v 1.6 2024/02/25 16:50:07 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win <errt.h>
 *
 * Copyright (c) 1998 - 2024, Adam Young.
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
#include <stdarg.h>

/*
 *  <err.h> - Formatted error message.
 *
 *  The err() and warn() family of functions display a formatted error message on stderr. For a comparison of the members of this family, see err().
 *
 *  The vwarn() function produces a message that consists of:
 *
 *      o the last component of the program name, followed by a colon and a space
 *
 *      O the formatted message, followed by a colon and a space, if the fmt argument isn't NULL
 *
 *      O the string associated with the current value of errno
 *
 *      o a newline character
 *
 *  The vwarnx() function produces a similar message, except that it doesn't include the string associated with errno. The message consists of:
 *
 *      o the last component of the program name, followed by a colon and a space
 *
 *      o the formatted message, if the fmt argument isn't NULL
 *
 *      o a newline character
 */

__BEGIN_DECLS

/* "<program>: <format>: <errno>\n", on stderr. */
LIBW32_API void warn(const char *fmt, ...);
LIBW32_API void vwarn(const char *fmt, va_list ap);
LIBW32_API void warnc(int code, const char *fmt, ...);
LIBW32_API void vwarnc(int code, const char *fmt, va_list ap);

/* Likewise, but without ": " and the standard error string.  */
LIBW32_API void warnx(const char *fmt, ...);
LIBW32_API void vwarnx(const char *fmt, va_list ap);


/* Like above, but the exits using 'eval' */
LIBW32_API void err(int eval, const char *fmt, ...);
LIBW32_API void verr(int eval, const char *fmt, va_list ap);

LIBW32_API void errc(int eval, int code, const char *fmt, ...);
LIBW32_API void verrc(int eval, int code, const char *fmt, va_list);

LIBW32_API void errx(int eval, const char *fmt, ...);
LIBW32_API void verrx(int eval, const char *fmt, va_list ap);

LIBW32_API void err_set_file(void *);
LIBW32_API void err_set_exit(void (*)(int));

__END_DECLS

#endif /*LIBW32_ERR_H_INCLUDED*/
