#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_err_c,"$Id: w32_err.c,v 1.2 2021/11/30 13:06:19 cvsuser Exp $")

/*
 * Copyright (c) 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void vwarni(const char *, va_list);
static void vwarnxi(const char *, va_list);

/* "<program>: <format>", on stderr. */
static void
vwarnxi(const char *fmt, va_list ap)
{
	fprintf(stderr, "%s: ", getprogname());
	if (fmt != NULL)
		vfprintf(stderr, fmt, ap);
}

/* "<program>: <format>: <errno>\n", on stderr. */
static void
vwarni(const char *fmt, va_list ap)
{
	int sverrno;

	sverrno = errno;
	vwarnxi(fmt, ap);
	if (fmt != NULL)
		fputs(": ", stderr);
	fprintf(stderr, "%s\n", w32_strerror(sverrno));
}


LIBW32_API void
err(int eval, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwarni(fmt, ap);
	va_end(ap);
	exit(eval);
}


LIBW32_API void
verr(int eval, const char *fmt, va_list ap)
{
	vwarni(fmt, ap);
	exit(eval);
}


LIBW32_API void
errx(int eval, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwarnxi(fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	exit(eval);
}


LIBW32_API void
verrx(int eval, const char *fmt, va_list ap)
{
	vwarnxi(fmt, ap);
	fputc('\n', stderr);
	exit(eval);
}


LIBW32_API void
warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwarni(fmt, ap);
	va_end(ap);
}


LIBW32_API void
vwarn(const char *fmt, va_list ap)
{
	vwarni(fmt, ap);
}


LIBW32_API void
warnx(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwarnxi(fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}


LIBW32_API void
vwarnx(const char *fmt, va_list ap)
{
	vwarnxi(fmt, ap);
	fputc('\n', stderr);
}

/*end*/
