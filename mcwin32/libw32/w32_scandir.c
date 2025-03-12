 /*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1983, 1993
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

/*
 * Scan the directory dirname calling select to make a list of selected
 * directory entries then sort using qsort and compare routine dcomp.
 * Returns the number of entries and a pointer to a list of pointers to
 * struct dirent (through namelist). Returns -1 if there were any errors.
 */

/*
//  NAME
//      alphasort, scandir - scan a directory
//
//  SYNOPSIS
//      #include <dirent.h>
//
//      int alphasort(const struct dirent **d1, const struct dirent **d2);
//      int scandir(const char *dir, struct dirent ***namelist,
//              int (*sel)(const struct dirent *), 
//              int (*compar)(const struct dirent **, const struct dirent **));
//
//  DESCRIPTION
//      The alphasort() function can be used as the comparison function for the scandir()
//      function to sort the directory entries, d1 and d2, into alphabetical order. Sorting
//      happens as if by calling the strcoll() function on the d_name element of the dirent
//      structures passed as the two parameters. If the strcoll() function fails, the return
//      value of alphasort() is unspecified.
//
//      The alphasort() function shall not change the setting of errno if successful. Since no
//      return value is reserved to indicate an error, an application wishing to check for error
//      situations should set errno to 0, then call alphasort(), then check errno.
//
//      The scandir() function shall scan the directory dir, calling the function referenced by
//      sel on each directory entry. Entries for which the function referenced by sel returns
//      non-zero shall be stored in strings allocated as if by a call to malloc(), and sorted as
//      if by a call to qsort() with the comparison function compar, except that compar need not
//      provide total ordering. The strings are collected in array namelist which shall be
//      allocated as if by a call to malloc(). If sel is a null pointer, all entries shall be
//      selected. If the comparison function compar does not provide total ordering, the order in
//      which the directory entries are stored is unspecified.
//
//  RETURN VALUE
//      Upon successful completion, the alphasort() function shall return an integer greater than, 
//      equal to, or less than 0, according to whether the name of the directory entry pointed to
//      by d1 is lexically greater than, equal to, or less than the directory pointed to by d2
//      when both are interpreted as appropriate to the current locale. There is no return value
//      reserved to indicate an error.
//
//      Upon successful completion, the scandir() function shall return the number of entries in
//      the array and a pointer to the array through the parameter namelist. Otherwise, the
//      scandir() function shall return -1.
//
//  ERRORS
//      The scandir() function shall fail if:
//
//      [EACCES]
//          Search permission is denied for the component of the path prefix of dir or read
//          permission is denied for dir.
//
//      [ELOOP]
//          A loop exists in symbolic links encountered during resolution of the dir argument.
//
//      [ENAMETOOLONG]
//          The length of a component of a pathname is longer than {NAME_MAX}.
//
//      [ENOENT]
//          A component of dir does not name an existing directory or dir is an empty string.
//
//      [ENOMEM]
//          Insufficient storage space is available.
//
//      [ENOTDIR]
//          A component of dir names an existing file that is neither a directory nor a symbolic
//          link to a directory.
//
//      [EOVERFLOW]
//          One of the values to be returned or passed to a callback function cannot be
//          represented correctly. The scandir() function may fail if:
//
//      [ELOOP]
//          More than {SYMLOOP_MAX} symbolic links were encountered during resolution of the dir
//          argument.
//
//      [EMFILE]
//          All file descriptors available to the process are currently open.
//
//      [ENAMETOOLONG]
//          The length of a pathname exceeds {PATH_MAX}, or pathname resolution of a symbolic
//          link produced an intermediate result with a length that exceeds {PATH_MAX}.
//
//      [ENFILE]
//          Too many files are currently open in the system.
//
//
//      The following sections are informative.
//
//  EXAMPLES
//
//      An example to print the files in the current directory:
//
//      #include <dirent.h>
//      #include <stdio.h>
//      #include <stdlib.h>
//
//      ...
//
//      struct dirent **namelist;
//      int i,n;
//
//      n = scandir(".", &namelist, 0, alphasort);
//      if (n < 0)
//          perror("scandir");
//      else {
//          for (i = 0; i < n; i++) {
//              printf("%s\n", namelist[i]->d_name);
//              free(namelist[i]);
//          }
//      }
//      free(namelist);
//
//      ...
//
//  APPLICATION USAGE
//      If dir contains filenames that do not form character strings, or which contain characters
//      outside the domain of the collating sequence of the current locale, the alphasort()
//      function need not provide a total ordering. This condition is not possible if all
//      filenames within the directory consist only of characters from the portable filename
//      character set.
//
//      The scandir() function may allocate dynamic storage during its operation. If scandir() is
//      forcibly terminated, such as by longjmp() or siglongjmp() being executed by the function
//      pointed to by sel or compar, or by an interrupt routine, scandir() does not have a chance
//      to free that storage, so it remains permanently allocated. A safe way to handle
//      interrupts is to store the fact that an interrupt has occurred, then wait until scandir()
//      returns to act on the interrupt.
//
//      For functions that allocate memory as if by malloc(), the application should release such
//      memory when it is no longer required by a call to free(). For scandir(), this is namelist
//      (including all of the individual strings in namelist).
*/

#define _DIRENT_SOURCE
#include "win32_internal.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "win32_direct.h"

static int		bsd_strverscmp(const char *s1, const char *s2);
static void *		bsd_reallocarray(void *optr, size_t nmemb, size_t size);

#ifdef	I_AM_SCANDIR_B
#include "block_abi.h"
#define	SELECT(x)	CALL_BLOCK(sel, x)
#ifndef __BLOCKS__
void qsort_b(void *, size_t, size_t, void *);
#endif
#else
#define	SELECT(x)	sel(x)
#endif

#ifdef I_AM_SCANDIR_B
typedef DECLARE_BLOCK(int, select_block, const struct dirent *);
typedef DECLARE_BLOCK(int, dcomp_block, const struct dirent **, const struct dirent **);
#else
static int scandir_thunk_cmp(const void *p1, const void *p2, void *thunk);
#endif

static int
#ifdef I_AM_SCANDIR_B
scandir_b_dirp(DIR *dirp, struct dirent ***namelist, select_block sel, dcomp_block dcomp)
#else
scandir_dirp(DIR *dirp, struct dirent ***namelist,
    int (*sel)(const struct dirent *), int (*dcomp)(const struct dirent **, const struct dirent **))
#endif
{
	struct dirent *d, *p, **names = NULL;
	size_t arraysz, numitems;

	numitems = 0;
	arraysz = 32;			/* initial estimate of the array size */
	names = (struct dirent **)malloc(arraysz * sizeof(struct dirent *));
	if (names == NULL)
		goto fail;

	while ((d = readdir(dirp)) != NULL) {
		if (sel != NULL && !SELECT(d))
			continue;	/* just selected names */

		/*
		 * Make a minimum size copy of the data
		 */
		p = (struct dirent *)malloc(_GENERIC_DIRSIZ(d));
		if (p == NULL)
			goto fail;

		p->d_fileno = d->d_fileno;
		p->d_type   = d->d_type;
		p->d_reclen = d->d_reclen;
		p->d_namlen = d->d_namlen;
		bcopy(d->d_name, p->d_name, p->d_namlen + 1);
#if defined(_WIN32)
		p->d_ctime  = d->d_ctime;
		p->d_mtime  = d->d_mtime;
		p->d_size   = d->d_size;
		p->d_attr   = d->d_attr;
#endif

		/*
		 * Check to make sure the array has space left and realloc the maximum size.
		 */
		if (numitems >= arraysz) {
			struct dirent **names2;

			names2 = bsd_reallocarray(names, arraysz, 2 * sizeof(struct dirent *));
			if (names2 == NULL) {
				free(p);
				goto fail;
			}
			names = names2;
			arraysz *= 2;
		}
		names[numitems++] = p;
	}
	closedir(dirp);
	if (numitems && dcomp != NULL)
#ifdef I_AM_SCANDIR_B
		qsort_b(names, numitems, sizeof(struct dirent *), (void*)dcomp);
#else
#if defined(_WIN32)
		qsort_s(names, numitems, sizeof(struct dirent *), scandir_thunk_cmp, (void *)&dcomp);
#else
		qsort_r(names, numitems, sizeof(struct dirent *), scandir_thunk_cmp, &dcomp);
#endif
#endif
	*namelist = names;
	return (numitems);

fail:
	while (numitems > 0)
		free(names[--numitems]);
	free(names);
	closedir(dirp);
	return (-1);
}

int
#ifdef I_AM_SCANDIR_B
scandir_b(const char *dirname, struct dirent ***namelist, select_block select,
    dcomp_block dcomp)
#else
scandir(const char *dirname, struct dirent ***namelist,
    int (*sel)(const struct dirent *), int (*dcomp)(const struct dirent **, const struct dirent **))
#endif
{
	DIR *dirp;

	dirp = opendir(dirname);
	if (dirp == NULL)
		return (-1);
	return (
#ifdef I_AM_SCANDIR_B
	    scandir_b_dirp
#else
	    scandir_dirp
#endif
	    (dirp, namelist, sel, dcomp));
}

#ifndef I_AM_SCANDIR_B
//	int
//	scandirat(int dirfd, const char *dirname, struct dirent ***namelist,
//	    int (*sel)(const struct dirent *), int (*dcomp)(const struct dirent **,
//	    const struct dirent **))
//	{
//		DIR *dirp;
//		int fd;
//
//		fd = _openat(dirfd, dirname, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
//		if (fd == -1)
//			return (-1);
//		dirp = fdopendir(fd);
//		if (dirp == NULL) {
//			_close(fd);
//			return (-1);
//		}
//		return (scandir_dirp(dirp, namelist, sel, dcomp));
//	}

/*
 * Alphabetic order comparison routine for those who want it.
 * POSIX 2008 requires that alphasort() uses strcoll().
 */
int
alphasort(const struct dirent **d1, const struct dirent **d2)
{

	return (strcoll((*d1)->d_name, (*d2)->d_name));
}

int
versionsort(const struct dirent **d1, const struct dirent **d2)
{

	return (bsd_strverscmp((*d1)->d_name, (*d2)->d_name));
}

static int
scandir_thunk_cmp(const void *p1, const void *p2, void *thunk)
{
	int (*dc)(const struct dirent **, const struct dirent **);

	dc = *(int (**)(const struct dirent **, const struct dirent **))thunk;
	return (dc((const struct dirent **)p1, (const struct dirent **)p2));
}
#endif

static void *
bsd_reallocarray(void *optr, size_t nmemb, size_t size)
{
#define MUL_NO_OVERFLOW		((size_t)1 << (sizeof(size_t) * 4))

	if ((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) &&
		    nmemb > 0 && SIZE_MAX / nmemb < size) {
		errno = ENOMEM;
		return NULL;
	}
	return realloc(optr, size * nmemb);
}

static int
bsd_strverscmp(const char *s1, const char *s2)
{
	size_t digit_count_1, digit_count_2;
	size_t zeros_count_1, zeros_count_2;
	const unsigned char *num_1, *num_2;
	const unsigned char *u1 = __DECONST(const unsigned char *, s1);
	const unsigned char *u2 = __DECONST(const unsigned char *, s2);

	/*
	 * If pointers are the same, no need to go through to process of
	 * comparing them.
	 */
	if (s1 == s2)
		return (0);

	while (*u1 != '\0' && *u2 != '\0') {
		/* If either character is not a digit, act like strcmp(3). */

		if (!isdigit(*u1) || !isdigit(*u2)) {
			if (*u1 != *u2)
				return (*u1 - *u2);
			u1++;
			u2++;
			continue;
		}
		if (*u1 == '0' || *u2 == '0') {
			/*
			 * Treat leading zeros as if they were the fractional
			 * part of a number, i.e. as if they had a decimal point
			 * in front. First, count the leading zeros (more zeros
			 * == smaller number).
			 */
			zeros_count_1 = 0;
			zeros_count_2 = 0;
			for (; *u1 == '0'; u1++)
				zeros_count_1++;
			for (; *u2 == '0'; u2++)
				zeros_count_2++;
			if (zeros_count_1 != zeros_count_2)
				return (zeros_count_2 - zeros_count_1);

			/* Handle the case where 0 < 09. */
			if (!isdigit(*u1) && isdigit(*u2))
				return (1);
			if (!isdigit(*u2) && isdigit(*u1))
				return (-1);
		} else {
			/*
			 * No leading zeros; we're simply comparing two numbers.
			 * It is necessary to first count how many digits there
			 * are before going back to compare each digit, so that
			 * e.g. 7 is not considered larger than 60.
			 */
			num_1 = u1;
			num_2 = u2;

			/* Count digits (more digits == larger number). */
			for (; isdigit(*u1); u1++)
				;
			for (; isdigit(*u2); u2++)
				;
			digit_count_1 = u1 - num_1;
			digit_count_2 = u2 - num_2;
			if (digit_count_1 != digit_count_2)
				return (digit_count_1 - digit_count_2);

			/*
			 * If there are the same number of digits, go back to
			 * the start of the number.
			 */
			u1 = num_1;
			u2 = num_2;
		}

		/* Compare each digit until there are none left. */
		for (; isdigit(*u1) && isdigit(*u2); u1++, u2++) {
			if (*u1 != *u2)
				return (*u1 - *u2);
		}
	}
	return (*u1 - *u2);
}

//end
