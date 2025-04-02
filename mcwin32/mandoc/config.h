#ifndef MANDOC_CONFIG_H_INCLUDED
#define MANDOC_CONFIG_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/* $Id: config.h,v 1.1 2025/04/02 16:17:20 cvsuser Exp $
 * mandoc config.h
 *
 * Copyright (c) 2014 - 2025, Adam Young.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * ==end==
 *
 */

/*
 */

#include "w32config.h"

#if !defined(HAVE_PROGNAME)
#define HAVE_PROGNAME 1        /*libw32*/
#endif
#if !defined(HAVE_GETSUBOPT)
#define HAVE_GETSUBOPT 1       /*libw32*/
#endif

#include <stddef.h>
#include <malloc.h>
#include <unistd.h>

#ifndef  snprintf
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define  snprintf _snprintf    /*2015+*/
#endif
#endif

#ifndef  mktemp
#define  mktemp _mktemp
#endif
#ifndef  chdir
#define  chdir w32_chdir
#endif
#ifndef  mkdir
#define  mkdir w32_mkdir
#endif
#ifndef  rmdir
#define  rmdir w32_rmdir
#endif
#ifndef  getcwd
#define  getcwd w32_getcwd
#endif
#ifndef  realpath
#define  realpath w32_realpath
#endif
#ifndef  lstat
#define  lstat w32_lstat
#endif

#if !defined(__MINGW32__) && !defined(inline)
#define inline _inline
#endif

#include "../libw32/win32_child.h"

#include "mdocversion.h"        /*VERSION and binary names*/

#define PACKAGE_NAME "mcmandoc"

#define OSVERSTRING(__x) __OSVERSTRING(__x)
#define __OSVERSTRING(__x) #__x
#define OSNAME PACKAGE_NAME " " OSVERSTRING(VERSION_1) "." OSVERSTRING(ERSION_2)
#define OSENUM MANDOC_OS_OTHER
#define BINM_PAGER "less"

/*
 *  Toolchain specific
 */
     
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wformat="                  // unknown conversion type character 'z' in format
#pragma GCC diagnostic ignored "-Wformat-extra-args"        // too many arguments for format
#pragma GCC diagnostic ignored "-Wreturn-type"              // control reaches end of non-void function 
#pragma GCC diagnostic ignored "-Wsign-compare"             // operand of '?:' changes signedness from 'xxx' to 'yyy   
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"  // variable 'xxx' set but not used
#endif //__GNUC__
#if defined(__WATCOMC__)
#pragma disable_message(124)    // Comparison result always 0
#pragma disable_message(136)    // Comparison equivalent to 'unsigned == 0'
#pragma disable_message(202)    // Symbol 'xxx' has been defined, but not referenced
#pragma disable_message(303)    // Parameter 'xxx' has been defined, but not referenced
#endif
#if defined(_MSC_VER)
#pragma warning(disable:4146)   // unary minus operator applied to unsigned type, result still unsigned
#pragma warning(disable:4244)   // '=': conversion from 'xxx' to 'yyy', possible loss of data
#pragma warning(disable:4716)   // 'xxx': must return a value
#pragma warning(disable:4996)   // 'xxx': was declared deprecated
#endif

/*
 *  compat_err.c (1.13.4)
 *  compat_fgetln.c
 *  compat_getsubopt.c
 *  compat_reallocarray.c (1.13.4)
 *  compat_strlcat.c
 *  compat_strcasestr.c
 *  compat_strlcpy.c
 *  compat_strtonum (1.13.4)
 */

#if !defined(_GNU_SOURCE)
#if defined(linux) || defined(__CYGWIN__) //FIXME
#define _GNU_SOURCE             /*see: string.h*/
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#if !defined(HAVE_PROGNAME)
extern void                     setprogname(const char *);
extern const char *             getprogname(void);
#endif

#if !defined(HAVE_MKDTEMP)
extern char *                   mkdtemp(char *path);
#endif

#if !defined(HAVE_ERR)
extern void                     err(int eval, const char *fmt, ...);
extern void                     errx(int eval, const char *fmt, ...);
extern void                     warn(const char *fmt, ...);
extern void                     warnx(const char *fmt, ...);
#endif

#if !defined(HAVE_FGETLN)
extern char *                   fgetln(FILE *fp, size_t *len);
#endif

#if !defined(GAVE_GETLINE)
extern ssize_t                  getline(char **buf, size_t *bufsz, FILE *fp);
#endif

#if !defined(HAVE_GETSUBOPT)
extern char *suboptarg;
extern int                      getsubopt(char **optionp, char * const *tokens, char **valuep);
#endif

#if !defined(HAVE_REALLOCARRAY)
void *                          reallocarray(void *optr, size_t nmemb, size_t size);
#endif

#if !defined(HAVE_RECALLOCARRAY)
void *                          recallocarray(void *ptr, size_t oldnmemb, size_t newnmemb, size_t size);
#endif

#if !defined(HAVE_STRCASESTR) || defined(__CYGWIN__) /*missing?*/
extern char *                   strcasestr(const char *s, const char *find);
#endif

#if !defined(HAVE_STRLCPY)
extern size_t                   strlcpy(char *dst, const char *src, size_t siz);
#endif

#if !defined(HAVE_STRLCAT)
extern size_t                   strlcat(char *dst, const char *src, size_t siz);
#endif

#if !defined(HAVE_STRNDUP)
extern char *                   strndup(const char *str, size_t maxlen);
#endif

#if !defined(HAVE_STRTONUM)
extern long long                strtonum(const char *numstr, long long minval, long long maxval, const char **errstrp);
#endif

#include "libcompat.h"


/*
 *  __BEGIN_DECLS
 *  void my_declarations();
 *  __END_DECLS
 */

#if defined(HAVE_SYS_CDEFS_H)
#include <sys/cdefs.h>
#endif
#if defined(HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif
#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

#ifndef __BEGIN_DECLS
#  ifdef __cplusplus
#     define __BEGIN_DECLS      extern "C" {
#     define __END_DECLS        }
#  else
#     define __BEGIN_DECLS
#     define __END_DECLS
#  endif
#endif
#ifndef __P
#  if (__STDC__) || defined(__cplusplus) || \
         defined(_MSC_VER) || defined(__PARADIGM__) || defined(__GNUC__) || \
         defined(__BORLANDC__) || defined(__WATCOMC__)
#     define __P(x)             x
#  else
#     define __P(x)             ()
#  endif
#endif

#if !defined(__GNUC__) && !defined(__clang__)
#ifndef __attribute__           //FIXME: HAVE_ATTRIBUTE
#define __attribute__(__x)
#endif
#endif

#include "portable_endian.h"

#endif  /*MANDOC_CONFIG_H_INCLUDED*/
/*end*/
