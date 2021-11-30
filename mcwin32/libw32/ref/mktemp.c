/* -*- c: tabs: 9 -*-
 * Copyright (c) 1987, 1993
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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

#if !defined(lint) && defined(sccs)
static char sccsid[] = "@(#)mktemp.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "libcompat.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#if !defined(unix)
#include <process.h>
#include <io.h>

#if defined(WIN32) || defined(_WIN32)
#define  WINDOWS_MEAN_AND_LEAN
#include <Windows.h>
#endif

#if defined(_MSC_VER)
#define getpid() _getpid()
#endif

#if (defined(WIN32) || defined(_WIN32)) && !defined(S_ISDIR)
#define S_ISDIR(m) (m & _S_IFDIR)
#endif
#endif

static int _gettemp(char *path, int *doopen);


#if !defined(HAVE_MKSTEMP)
int
mkstemp(char *path)
{
	int fd = -1;

	if (_gettemp(path, &fd)) {
		return fd;
	}

#if defined(WIN32) || defined(_WIN32)
	/* "/tmp/", reference system temporary path */
	if (path && 0 == memcmp(path, "/tmp/", 5)) {

		int  pathlen, tmplen;
		char t_path[256], *p;

		tmplen = (int)GetTempPathA(sizeof(t_path), t_path);

		pathlen = strlen(path + 4); /*include trailing '/'*/
		if (tmplen && (pathlen + tmplen) < sizeof(t_path)) {

			if (t_path[tmplen-1] == '\\') --tmplen;
			(void) memcpy(t_path + tmplen, path + 4, pathlen + 1 /*nul*/);
			for (p = t_path; NULL != (p = strchr(p, '/'));) {
				*p++ = '\\';
			}

			if (_gettemp(t_path, &fd)) {

				strcpy(path + 4, t_path + tmplen);
				if ('\\' == path[4]) path[4] = '/';
				return fd;
			}
		}
	}
#endif

	assert(0);
	return -1;
}
#endif  //HAVE_MKSTEMP


#if !defined(HAVE_MKTEMP)
char *
mktemp(char *path)
{
	return(_gettemp(path, (int *)NULL) ? path : (char *)NULL);
}
#endif  //HAVE_MKTEMP)


//
//  xmktemp - extension
//
char *
xmktemp(char *path, char *result, size_t length)
{
	if (_gettemp(path, (int *)NULL)) {
		if (result && length) { 	/* return resulting path*/
			strncpy(result, (const char *)path, length);
			result[length-1]=0;
		}
		return result;
	}

#if defined(WIN32) || defined(_WIN32)
	if (path && result && length && 0 == memcmp(path, "/tmp/", 5)) {

		int  pathlen, tmplen;
		char t_path[256], *p;

		tmplen = (int)GetTempPathA(sizeof(t_path), t_path);

		pathlen = strlen(path + 4);	/*include trailing '/'*/

		if (tmplen && (pathlen + tmplen) < sizeof(t_path)) {

			if (t_path[tmplen-1] == '\\') --tmplen;
			(void) memcpy(t_path + tmplen, path + 4, pathlen + 1 /*nul*/);
			for (p = t_path; NULL != (p = strchr(p, '/'));) {
				*p++ = '\\';
			}

			if (_gettemp(t_path, NULL)) {

				strcpy(path + 4, t_path + tmplen);
				if ('\\' == path[4]) path[4] = '/';
				strncpy(result, t_path, length);

				return result;
			}
		}
	}
#endif

	assert(0);
	return NULL;
}


static int
_gettemp(char *path, register int *doopen)
{
	register char *start, *trv;
	struct stat sbuf;
	unsigned int pid;
#if !defined(unix)
	int slash;
#endif

	pid = getpid();
	for (trv = path; *trv; ++trv); /* extra X's get set to 0's */
	while (*--trv == 'X') {
		*trv = (pid % 10) + '0';
		pid /= 10;
	}

	/*
	 * check the target directory; if you have six X's and it
	 * doesn't exist this runs for a *very* long time.
	 */
	for (start = trv + 1;; --trv) {
		if (trv <= path)
			break;
#if !defined(unix)
		if (*trv == '/' || *trv == '\\') { /*}*/
			slash = *trv;
#else
		if (*trv == '/') {
#endif
			*trv = '\0';
			if (stat(path, &sbuf)) {
				return 0;
			}
			if (!S_ISDIR(sbuf.st_mode)) {
				errno = ENOTDIR;
				return 0;
			}
#if !defined(unix)
			*trv = slash;
#else
			*trv = '/';
#endif
			break;
		}
	}

	for (;;) {
		if (doopen) {
#if defined(_MSC_VER) || defined(__WATCOMC__)
			if ((*doopen = _open(path, O_CREAT|O_EXCL|O_RDWR|O_BINARY, 0600)) >= 0)
#else
			if ((*doopen = open(path, O_CREAT|O_EXCL|O_RDWR|O_BINARY, 0600)) >= 0)
#endif
				return(1);
			if (errno != EEXIST)
				return(0);
		} else if (stat(path, &sbuf)) {
			return(errno == ENOENT ? 1 : 0);
		}

		/* tricky little algorithm for backward compatibility */
		for (trv = start;;) {
			if (!*trv)
				return(0);
			if (*trv == 'z')
				*trv++ = 'a';
			else {
				if (isdigit(*trv))
					*trv = 'a';
				else
					++*trv;
				break;
			}
		}
	}
	/*NOTREACHED*/
}

