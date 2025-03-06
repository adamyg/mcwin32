/* $NetBSD: dirname.c,v 1.14 2018/09/27 00:45:34 kre Exp $ */

/*-
 * Copyright (c) 1997, 2002 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Klaus Klein and Jason R. Thorpe.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <string.h>
#include <assert.h>

#include "win32_internal.h"
#include "libgen.h"

#include <sys/param.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#if defined(_WIN32)
#define ISSEP(_c)   (_c == '/' || _c == '\\')
#else
#define ISSEP(_c)   (_c == '/')
#endif

#ifndef PATH_MAX
#define PATH_MAX    1024
#endif
#ifndef MIN
#define MIN(a, b)   ((a < b) ? a : b)
#endif


/*
//  NAME
//
//      dirname - report the parent directory name of a file pathname
//
//  SYNOPSIS
//
//      #include <libgen.h>
//
//      char *dirname(char *path);
//
//      size_t dirname_r(const char *path, char *buf, size_t maxlen);
//
//    DESCRIPTION
//
//      The dirname() function shall take a pointer to a character string that contains a
//      pathname, and return a pointer to a string that is a pathname of the parent directory of
//      that file. Trailing '/' characters in the path are not counted as part of the path.
//
//      If path does not contain a '/', then dirname() shall return a pointer to the string ".".
//      If path is a null pointer or points to an empty string, dirname() shall return a pointer
//      to the string "." .
//
//      The dirname() function need not be reentrant. A function that is not required to be
//      reentrant is not required to be thread-safe.
//
//    RETURN VALUE
//
//      The dirname() function shall return a pointer to a string that is the parent directory of
//      path. If path is a null pointer or points to an empty string, a pointer to a string "."
//      is returned.
//
//      The dirname() function may modify the string pointed to by path, and may return a pointer
//      to static storage that may then be overwritten by subsequent calls to dirname().
//
//  ERRORS
//
//      No errors are defined.
//
//  APPLICATION USAGE
//
//      The dirname() and basename() functions together yield a complete pathname. The expression
//      dirname(path) obtains the pathname of the directory where basename(path) is found.
//
//      Since the meaning of the leading "//" is implementation-defined, dirname(" //foo) may
//      return either "//" or '/' (but nothing else).
*/

char *
w32_dirname(char *path)
{
	static char result[PATH_MAX];		// TODO: TLS

	result[0] = 0;
#if defined(UTF8FILENAMES)
	if (w32_utf8filenames_state()) {
		if (path) {
			wchar_t wpath[WIN32_PATH_MAX], wresult[WIN32_PATH_MAX];

			if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
				(void)w32_dirnameW_r(wpath, wresult, _countof(wresult));
				w32_wc2utf(wresult, result, _countof(result));
				return result; // NOT MTSAFE
			}
		}
	}
#endif  //UTF8FILENAMES

	(void)w32_dirnameA_r(path, result, _countof(result));
	return result; //NOT MTSAFE
}


wchar_t *
w32_dirnameW(wchar_t *path)
{
	static wchar_t result[PATH_MAX];	// TODO: TLS

	result[0] = 0;
	(void)w32_dirnameW_r(path, result, _countof(result));
	return result; //NOT MTSAFE
}


char *
w32_dirnameA(char *path)
{
	static char result[PATH_MAX];		// TODO: TLS

	result[0] = 0;
	(void)w32_dirnameA_r(path, result, _countof(result));
	return result; //NOT MTSAFE
}


size_t
w32_dirnameA_r(const char *path, char *buf, size_t buflen)
{
	const char *endp;
	size_t len;

	/*
	 * If `path' is a null pointer or points to an empty string,
	 * return a pointer to the string ".".
	 */
	if (path == NULL || *path == '\0') {
		path = ".";
		len = 1;
		goto out;
	}

	/* Strip trailing slashes, if any. */
	endp = path + strlen(path) - 1;
	while (endp != path && ISSEP(*endp))
		endp--;

	/* Find the start of the dir */
	while (endp > path && !ISSEP(*endp))
		endp--;

	if (endp == path) {
		path = ISSEP(*endp) ? "/" : ".";
		len = 1;
		goto out;
	}

	do
		endp--;
	while (endp > path && ISSEP(*endp));

	len = endp - path + 1;
out:
	if (buf != NULL && buflen != 0) {
		buflen = MIN(len, buflen - 1);
		if (buf != path)
			memcpy(buf, path, buflen * sizeof(*buf));
		buf[buflen] = '\0';
	}
	return len;
}


size_t
w32_dirnameW_r(const wchar_t *path, wchar_t *buf, size_t buflen /*characters*/)
{
	const wchar_t *endp;
	size_t len;

	/*
	 * If `path' is a null pointer or points to an empty string,
	 * return a pointer to the string ".".
	 */
	if (path == NULL || *path == '\0') {
		path = L".";
		len = 1;
		goto out;
	}

	/* Strip trailing slashes, if any. */
	endp = path + wcslen(path) - 1;
	while (endp != path && ISSEP(*endp))
		endp--;

	/* Find the start of the dir */
	while (endp > path && !ISSEP(*endp))
		endp--;

	if (endp == path) {
		path = ISSEP(*endp) ? L"/" : L".";
		len = 1;
		goto out;
	}

	do
		endp--;
	while (endp > path && ISSEP(*endp));

	len = endp - path + 1;
out:
	if (buf != NULL && buflen != 0) {
		buflen = MIN(len, buflen - 1);
		if (buf != path)
			memcpy(buf, path, buflen * sizeof(*buf));
		buf[buflen] = '\0';
	}
	return len;
}

//end
