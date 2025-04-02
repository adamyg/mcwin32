/*-
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <string.h>
#include <assert.h>

#include "win32_internal.h"
#include "libgen.h"

#if defined(_WIN32)
#define ISSEP(_c)   (_c == '/' || _c == '\\')
#else
#define ISSEP(_c)   (_c == '/')
#endif


/*
//  NAME
//
//      basename - return the last component of a pathname
//
//  SYNOPSIS
//
//      #include <libgen.h>
//
//      char *basename(char *path);
//
//  DESCRIPTION
//
//      The basename() function shall take the pathname pointed to by path and return a pointer
//      to the final component of the pathname, deleting any trailing '/' characters.
//
//      If the string pointed to by path consists entirely of the '/' character, basename() shall
//      return a pointer to the string "/". If the string pointed to by path is exactly "//", it
//      is implementation-defined whether '/' or "//" is returned.
//
//      If path is a null pointer or points to an empty string, basename() shall return a pointer
//      to the string ".".
//
//      The basename() function may modify the string pointed to by path, and may return a
//      pointer to static storage that may then be overwritten by a subsequent call to basename().
//
//      The basename() function need not be reentrant. A function that is not required to be
//      reentrant is not required to be thread-safe.
//
//  RETURN VALUE
//  
//      The basename() function shall return a pointer to the final component of path.
//
//  ERRORS
//
//      No errors are defined.
*/

LIBW32_API char *
w32_basename(char *path)
{
	return w32_basenameA(path);
}


LIBW32_API char *
w32_basenameA(char *path)
{
	char *ptr;

	/*
	 * If path is a null pointer or points to an empty string,
	 * basename() shall return a pointer to the string ".".
	 */
	if (path == NULL || *path == '\0')
		return (__DECONST(char *, "."));

	/* Find end of last pathname component and null terminate it. */
	ptr = path + strlen(path);
	while (ptr > path + 1 && ISSEP(ptr[-1]))
		--ptr;
	*ptr-- = '\0';

	/* Find beginning of last pathname component. */
	while (ptr > path && ! ISSEP(ptr[-1]))
		--ptr;
	return (ptr);
}


LIBW32_API wchar_t *
w32_basenameW(wchar_t *path)
{
	wchar_t *ptr;

	/*
	 * If path is a null pointer or points to an empty string,
	 * basename() shall return a pointer to the string ".".
	 */
	if (path == NULL || *path == '\0')
		return (__DECONST(wchar_t *, L"."));

	/* Find end of last pathname component and null terminate it. */
	ptr = path + wcslen(path);
	while (ptr > path + 1 && ISSEP(ptr[-1]))
		--ptr;
	*ptr-- = '\0';

	/* Find beginning of last pathname component. */
	while (ptr > path && ! ISSEP(ptr[-1]))
		--ptr;
	return (ptr);
}

//end
