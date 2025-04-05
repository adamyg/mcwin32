#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_err_c,"$Id: w32_err.c,v 1.5 2025/04/05 17:55:29 cvsuser Exp $")

/*-
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "win32_internal.h"
#include <unistd.h>
#include <err.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *err_file; /* file to use for error output */
static void (*err_exit)(int);

/*
 * This is declared to take a `void *' so that the caller is not required
 * to include <stdio.h> first.  However, it is really a `FILE *', and the
 * manual page documents it as such.
 */
LIBW32_API void
err_set_file(void *fp)
{
	if (fp)
		err_file = fp;
	else
		err_file = stderr;
}

LIBW32_API void
err_set_exit(void (*ef)(int))
{
	err_exit = ef;
}

LIBW32_API void
err(int eval, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verrc(eval, errno, fmt, ap);
	va_end(ap);
}

LIBW32_API void
verr(int eval, const char *fmt, va_list ap)
{
	verrc(eval, errno, fmt, ap);
}

LIBW32_API void
errc(int eval, int code, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verrc(eval, code, fmt, ap);
	va_end(ap);
}

LIBW32_API void
verrc(int eval, int code, const char *fmt, va_list ap)
{
	if (err_file == 0)
		err_set_file((FILE *)0);
	if (err_file) {
		fprintf(err_file, "%s: ", getprogname());
		if (fmt != NULL) {
			vfprintf(err_file, fmt, ap);
			fprintf(err_file, ": ");
		}
		fprintf(err_file, "%s\n", strerror(code));
		if (err_exit)
		err_exit(eval);
	}
	exit(eval);
}

LIBW32_API void
errx(int eval, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verrx(eval, fmt, ap);
	va_end(ap);
}

LIBW32_API void
verrx(int eval, const char *fmt, va_list ap)
{
	if (err_file == 0)
		err_set_file((FILE *)0);
	if (err_file) {
		fprintf(err_file, "%s: ", getprogname());
		if (fmt != NULL)
			vfprintf(err_file, fmt, ap);
		fprintf(err_file, "\n");
		if (err_exit)
			err_exit(eval);
	}
	exit(eval);
}

LIBW32_API void
warn(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vwarnc(errno, fmt, ap);
	va_end(ap);
}

LIBW32_API void
vwarn(const char *fmt, va_list ap)
{
	vwarnc(errno, fmt, ap);
}

LIBW32_API void
warnc(int code, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vwarnc(code, fmt, ap);
	va_end(ap);
}

LIBW32_API void
vwarnc(int code, const char *fmt, va_list ap)
{
	if (err_file == 0)
		err_set_file((FILE *)0);
	if (err_file) {
		fprintf(err_file, "%s: ", getprogname());
		if (fmt != NULL) {
			vfprintf(err_file, fmt, ap);
			fprintf(err_file, ": ");
		}
		fprintf(err_file, "%s\n", strerror(code));
	}
}

LIBW32_API void
warnx(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vwarnx(fmt, ap);
	va_end(ap);
}

LIBW32_API void
vwarnx(const char *fmt, va_list ap)
{
	if (err_file == 0)
		err_set_file((FILE *)0);
	if (err_file) {
		fprintf(err_file, "%s: ", getprogname());
		if (fmt != NULL)
			vfprintf(err_file, fmt, ap);
		fprintf(err_file, "\n");
	}
}

/*end*/
