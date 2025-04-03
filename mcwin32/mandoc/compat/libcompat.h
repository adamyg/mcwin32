#pragma once
#ifndef LIBCOMPACT_H_INCLUDED
#define LIBCOMPACT_H_INCLUDED

//
//  libcompat - libmandoc support functions
//
//  Copyright (c) 2014 - 2025, Adam Young.
//  All rights reserved.
//
//  Permission to use, copy, modify, and distribute this software for any
//  purpose with or without fee is hereby granted, provided that the above
//  copyright notice and this permission notice appear in all copies.
//
//  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
//  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
//  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
//  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
//  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
//  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
//  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#include "w32config.h"

#if defined(LIBCOMPAT_SOURCE)
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <sys/cdefs.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#include <unistd.h>

__BEGIN_DECLS

#if !defined(HAVE_ASPRINTF)     /*stdio.h*/
extern int asprintf(char **str, const char *fmt, ...);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_ASPRINTF 1
#endif
#endif /*HAVE_ASPRINTF*/

#if !defined(HAVE_VASPRINTF)    /*stdio.h*/
extern int vasprintf(char **str, const char *fmt, va_list ap);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_VASPRINTF 1
#endif
#endif /*HAVE_VASPRINTF*/

#if !defined(HAVE_ISBLANK) && !defined(isblank)
extern int isblank(int ch);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_ISBLANK 1
#endif
#endif

#if !defined(HAVE_WCWIDTH)
extern int wcwidth(wchar_t ucs);
extern int wcswidth(const wchar_t *pwcs, size_t n);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_WCWIDTH 1
#define HAVE_WCSWIDTH 1
#endif
#endif

__END_DECLS

#endif /*LIBCOMPACT_H_INCLUDED*/
