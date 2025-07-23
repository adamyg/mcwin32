#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_mkstemp_c, "$Id: w32_mkstemp.c,v 1.21 2025/07/23 15:05:00 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 mkstemp() implementation
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#include "win32_internal.h"
#include "win32_io.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#include <unistd.h>

#define DISABLE_HARD_ERRORS     (void)SetErrorMode (0);
#define ENABLE_HARD_ERRORS      (void)SetErrorMode (SEM_FAILCRITICALERRORS | \
                                        SEM_NOOPENFILEERRORBOX);


 /*
 //  NAME
 //      mkstemp - make a unique filename
 //
 //  SYNOPSIS
 //      #include <stdlib.h>
 //
 //      int mkstemp(char *template);
 //      int mkstemps(char *template, int suffixlen);
 //
 //  DESCRIPTION
 //
 //      The mkstemp() function shall replace the contents of the string pointed to by
 //      template by a unique filename, and return a file descriptor for the file open for
 //      reading and writing. The function thus prevents any possible race condition between
 //      testing whether the file exists and opening it for use.
 //
 //      The string in template should look like a filename with six trailing 'X' s;
 //      mkstemp() replaces each 'X' with a character from the portable filename character
 //      set. The characters are chosen such that the resulting name does not duplicate the
 //      name of an existing file at the time of a call to mkstemp().
 //
 //      Each successful call to mkstemp modifies template. In each subsequent call from the same
 //      process or thread with the same template argument, mkstemp checks for filenames that
 //      match names returned by mkstemp in previous calls. If no file exists for a given name,
 //      mkstemp returns that name. If files exist for all previously returned names, mkstemp
 //      creates a new name by replacing the alphabetic character it used in the previously
 //      returned name with the next available lowercase letter, in order, from 'a' through 'z'.
 //
 //  RETURN VALUE
 //
 //      Upon successful completion, mkstemp() shall return an open file descriptor.
 //      Otherwise, -1 shall be returned if no suitable file could be created.
 //
 //  ERRORS
 //      No errors are defined.
 */

#define GETTEMP_SUCCESS     1
#define GETTEMP_ERROR       0

#define DO_DEFAULT          0
#define DO_TEMPORARY        1
#define DO_MKDIR            2

static int                  gettempA_tmp(char *result, const char *path, int suffixlen, int *fildes, unsigned flags);
static int                  gettempA(char *path,  int suffixlen, int *fildes, unsigned flags, char *save);
static int                  gettempW_tmp(wchar_t *result, const wchar_t *path, int suffixlen, int *fildes, unsigned flags);
static int                  gettempW(wchar_t *path, int suffixlen, int *fildes, unsigned flags, wchar_t *save);


LIBW32_API int
w32_mkstemp(char *path)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t wpath[MAX_PATH];
        int fildes;

        w32_utf2wc(path, wpath, _countof(wpath));
        if ((fildes = w32_mkstempW(wpath)) >= 0) {
            w32_wc2utf(wpath, path, strlen(path) + 1);
            return fildes;
        }
        return -1;
    }
#endif  //UTF8FILENAMES

    return w32_mkstempA(path);
}


LIBW32_API int
w32_mkstempA(char *path)
{
    char t_path[MAX_PATH];
    int fildes = -1;
    if (GETTEMP_SUCCESS == gettempA(path, 0, &fildes, DO_DEFAULT, t_path) ||
            GETTEMP_SUCCESS == gettempA_tmp(path, t_path, 0, &fildes, DO_DEFAULT)) {
        return fildes;
    }
    return -1;
}


LIBW32_API int
w32_mkstempW(wchar_t *path)
{
    wchar_t t_path[MAX_PATH];
    int fildes = -1;
    if (GETTEMP_SUCCESS == gettempW(path, 0, &fildes, DO_DEFAULT, t_path) ||
            GETTEMP_SUCCESS == gettempW_tmp(path, t_path, 0, &fildes, DO_DEFAULT)) {
        return fildes;
    }
    return -1;
}


/////////////////////////////////////////////////////////////////////////////////////////

LIBW32_API int
w32_mkstemps(char *path, int suffixlen)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t wpath[MAX_PATH];
        int fildes;

        w32_utf2wc(path, wpath, _countof(wpath));
        if ((fildes = w32_mkstempsW(wpath, suffixlen)) >= 0) {
            w32_wc2utf(wpath, path, strlen(path) + 1);
            return fildes;
        }
        return -1;
    }
#endif  //UTF8FILENAMES

    return w32_mkstempsA(path, suffixlen);
}


LIBW32_API int
w32_mkstempsA(char *path, int suffixlen)
{
    char t_path[MAX_PATH];
    int fildes = -1;
    if (GETTEMP_SUCCESS == gettempA(path, suffixlen, &fildes, DO_DEFAULT, t_path) ||
            GETTEMP_SUCCESS == gettempA_tmp(path, t_path, suffixlen, &fildes, DO_DEFAULT)) {
        return fildes;
    }
    return -1;
}


LIBW32_API int
w32_mkstempsW(wchar_t *path, int suffixlen)
{
    wchar_t t_path[MAX_PATH];
    int fildes = -1;
    if (GETTEMP_SUCCESS == gettempW(path, suffixlen, &fildes, DO_DEFAULT, t_path) ||
            GETTEMP_SUCCESS == gettempW_tmp(path, t_path, suffixlen, &fildes, DO_DEFAULT)) {
        return fildes;
    }
    return -1;
}


/////////////////////////////////////////////////////////////////////////////////////////

LIBW32_API int
w32_mkstempx(char *path)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t wpath[MAX_PATH];
        int fildes;

        w32_utf2wc(path, wpath, _countof(wpath));
        if ((fildes = w32_mkstempxW(wpath)) >= 0) {
            w32_wc2utf(wpath, path, strlen(path) + 1);
            return fildes;
        }
        return -1;
    }
#endif  //UTF8FILENAMES

    return w32_mkstempxA(path);
}


LIBW32_API int
w32_mkstempxA(char *path)
{
    char t_path[MAX_PATH];
    int fildes = -1;
    if (GETTEMP_SUCCESS == gettempA(path, 0, &fildes, DO_TEMPORARY, t_path) ||
            GETTEMP_SUCCESS == gettempA_tmp(path, t_path, 0, &fildes, DO_TEMPORARY)) {
        return fildes;
    }
    return -1;
}


LIBW32_API int
w32_mkstempxW(wchar_t *path)
{
    wchar_t t_path[MAX_PATH];
    int fildes = -1;
    if (GETTEMP_SUCCESS == gettempW(path, 0, &fildes, DO_TEMPORARY, t_path) ||
            GETTEMP_SUCCESS == gettempW_tmp(path, t_path, 0, &fildes, DO_TEMPORARY)) {
        return fildes;
    }
    return -1;
}


/*
//  NAME
//      mkdtemp - create a unique temporary directory.
//
//  SYNOPSIS
//      #include <stdlib.h>
//
//      char *mkdtemp(char *template);
//
//  DESCRIPTION
//      The mkdtemp() function shall create a directory with a unique name derived from
//      template. The application shall ensure that the string provided in template is a
//      pathname ending with at least six trailing 'X' characters. The mkdtemp() function
//      shall modify the contents of template by replacing six or more 'X' characters at
//      the end of the pathname with the same number of characters from the portable
//      filename character set. The characters shall be chosen such that the resulting
//      pathname does not duplicate the name of an existing file at the time of the call
//      to mkdtemp(). The mkdtemp() function shall use the resulting pathname to create
//      the new directory as if by a call to:
//
//          mkdir(pathname, S_IRWXU)
//
//      The mkstemp() function shall create a regular file with a unique name derived from
//      template and return a file descriptor for the file open for reading and writing.
//      The application shall ensure that the string provided in template is a pathname
//      ending with at least six trailing 'X' characters. The mkstemp() function shall
//      modify the contents of template by replacing six or more 'X' characters at the
//      end of the pathname with the same number of characters from the portable filename
//      character set. The characters shall be chosen such that the resulting pathname
//      does not duplicate the name of an existing file at the time of the call to mkstemp().
//      The mkstemp() function shall use the resulting pathname to create the file, and
//      obtain a file descriptor for it, as if by a call to:
//
//          open(pathname, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR)
//
//      By behaving as if the O_EXCL flag for open() is set, the function prevents any possible
//      race condition between testing whether the file exists and opening it for use.
//
//  RETURN VALUE
//      Upon successful completion, the mkdtemp() function shall return the value of template.
//      Otherwise, it shall return a null pointer and shall set errno to indicate the error.
*/


LIBW32_API char *
w32_mkdtemp(char *path)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t wpath[MAX_PATH];

        w32_utf2wc(path, wpath, _countof(wpath));
        if (w32_mkdtempW(wpath)) {
            w32_wc2utf(wpath, path, strlen(path) + 1);
            return path;
        }
        return NULL;
    }
#endif  //UTF8FILENAMES

    return w32_mkdtempA(path);
}


LIBW32_API char *
w32_mkdtempA(char *path)
{
    return (GETTEMP_SUCCESS == gettempA(path, 0, NULL, DO_MKDIR, NULL) ? path : NULL);
}


LIBW32_API wchar_t *
w32_mkdtempW(wchar_t *path)
{
    return (GETTEMP_SUCCESS == gettempW(path, 0, NULL, DO_MKDIR, NULL) ? path : NULL);
}


LIBW32_API char *
w32_mkdtemps(char *path, int suffixlen)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t wpath[MAX_PATH];

        w32_utf2wc(path, wpath, _countof(wpath));
        if (w32_mkdtempsW(wpath, suffixlen)) {
            w32_wc2utf(wpath, path, strlen(path) + 1);
            return path;
        }
        return NULL;
    }
#endif  //UTF8FILENAMES

    return w32_mkdtempsA(path, suffixlen);
}


LIBW32_API char *
w32_mkdtempsA(char *path, int suffixlen)
{
    return (GETTEMP_SUCCESS == gettempA(path, suffixlen, NULL, DO_MKDIR, NULL) ? path : NULL);
}


LIBW32_API wchar_t *
w32_mkdtempsW(wchar_t *path, int suffixlen)
{
    return (GETTEMP_SUCCESS == gettempW(path, suffixlen, NULL, DO_MKDIR, NULL) ? path : NULL);
}


/////////////////////////////////////////////////////////////////////////////////////////
//  Implementation

#if defined(_O_TEMPORARY)
#if defined(_O_NOINHERIT)
#define O_MODEX     (O_CREAT|O_EXCL|O_RDWR|O_BINARY|_O_TEMPORARY|_O_NOINHERIT)
#else
#define O_MODEX     (O_CREAT|O_EXCL|O_RDWR|O_BINARY|_O_TEMPORARY)
#endif
#define ISTEMPORARY 1

#elif defined(O_TEMPORARY)
#if defined(O_NOINHERIT)
#define O_MODEX     (O_CREAT|O_EXCL|O_RDWR|O_BINARY|O_TEMPORARY|O_NOINHERIT)
#else
#define O_MODEX     (O_CREAT|O_EXCL|O_RDWR|O_BINARY|O_TEMPORARY)
#endif
#define ISTEMPORARY 1

#else
#define O_MODEX     (O_CREAT|O_EXCL|O_RDWR|O_BINARY)
#endif

#define O_MODE      (O_CREAT|O_EXCL|O_RDWR|O_BINARY)


static unsigned
generate_seed(void)
{
    static unsigned seed;
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:28159) // GetTickCount()
#endif
    if (0 == seed) seed = (WIN32_GETPID() * GetTickCount());
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    seed = (1103515245 * seed + 12345);
    return seed;
}


static int
gettempA_tmp(char *result, const char *path, int suffixlen, int *fildes, unsigned flags)
{
    /*
     *  "/tmp/", reference system temporary path
     */
    if (path && 0 == memcmp(path, "/tmp/", 5)) {
        char t_path[MAX_PATH], *p;
        size_t pathlen = strlen(path + 5),
            tmplen = (int)GetTempPathA(_countof(t_path), t_path);
                // TMP, TEMP, USERPROFILE environment variables, default windows directory.

        if (pathlen && tmplen) {
            if ((pathlen + tmplen) >= (int)_countof(t_path)) {
                errno = ENAMETOOLONG;

            } else {
                if (t_path[tmplen-1] != '\\') {
                    t_path[tmplen-1] = '\\', ++tmplen;
                }
                memcpy(t_path + tmplen, path + 5, pathlen + 1 /*nul*/);
                for (p = t_path; NULL != (p = strchr(p, '/'));) {
                    *p++ = '\\';                /* convert */
                }

                if (GETTEMP_SUCCESS == gettempA(t_path, suffixlen, fildes, flags, NULL)) {
                    strcpy(result, "/tmp/");
                    strcpy(result + 5, t_path + tmplen);
                    return GETTEMP_SUCCESS;
                }
            }
        }
    }
    return GETTEMP_ERROR;
}


#if !defined(ISTEMPORARY)
static int
OpenTemporaryA(const char *path)
{
    const DWORD desiredAccess = GENERIC_READ | GENERIC_WRITE; // O_RDWR
    const DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE; // 0600
    const DWORD creationDisposition = CREATE_NEW; // O_EXCL
    const DWORD flagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY;
    HANDLE handle;
    int fd = -1;

    handle = CreateFileA(path,
        desiredAccess, shareMode, NULL /*O_NOINHERIT*/, creationDisposition, flagsAndAttributes, NULL);
    if (INVALID_HANDLE_VALUE != handle) {
        (void) DeleteFileA(path); // delete-on-close, needs FILE_SHARE_DELETE
        if ((fd = _open_osfhandle((intptr_t)handle, _O_RDWR | _O_BINARY)) < 0) {
            CloseHandle(handle);
            errno = ENFILE; // file limit
        }
    }
    return fd;
}
#endif //ISTEMPORARY


static int
gettempA(char *path, int suffixlen, int *fildes, unsigned flags, char *save)
{
    register char *start, *trv, *end, c;
    struct stat sbuf;
    unsigned seed;
    int rc;

    if (save) {
        for (trv = path; (*save = *trv) != 0; ++save) {
            if ((++trv - path) >= MAX_PATH) {
                errno = ENAMETOOLONG;
                return GETTEMP_ERROR;
            }
        }
    } else {
        for (trv = path; *trv; ++trv);
        if ((trv - path) >= MAX_PATH) {
            errno = ENAMETOOLONG;
            return GETTEMP_ERROR;
        }
    }

    trv -= suffixlen;
    end = trv;
    if (suffixlen < 0 || trv <= path ||         /* out-of-bounds? */
            NULL != strchr(end, '/') || NULL != strchr(end, '\\')) {
        errno = EINVAL;
        return GETTEMP_ERROR;
    }

    seed = generate_seed();
    while (--trv >= path && *trv == 'X') {
        *trv = (char)((seed % 10) + '0');
        seed /= 10;                             /* extra X's get set to 0's */
    }

    if ((trv + 1) == end) {                     /* missing template? */
        errno = EINVAL;
        return GETTEMP_ERROR;
    }

    /*
     *  check the target directory; if you have six X's and it
     *  doesn't exist this runs for a *very* long time.
     */
    for (start = trv + 1;; --trv) {
        if (trv <= path) {
            break;
        }

        if ((c = *trv) == '/' || c == '\\') {
            if (trv[-1] == ':') {
                break;
            }
            *trv = '\0';
            DISABLE_HARD_ERRORS
            rc = w32_statA(path, &sbuf);
            ENABLE_HARD_ERRORS
            *trv = c;
            if (rc) {
                return GETTEMP_ERROR;
            }
            if (!(sbuf.st_mode & S_IFDIR)) {
                errno = ENOTDIR;
                return GETTEMP_ERROR;
            }
            break;
        }
    }


    /*
     *  Create file as temporary; file is deleted when last file descriptor is closed.
     */
    for (;;) {
        errno = 0;
        if (DO_MKDIR & flags) {
            if (0 == w32_mkdirA(path, 0700)) {
                return GETTEMP_SUCCESS;
            }

            if (EEXIST != errno) {
                return GETTEMP_ERROR;
            }

        } else if (fildes) {
#if defined(ISTEMPORARY)
            if ((*fildes = WIN32_OPEN(path, ((DO_TEMPORARY & flags) ? O_MODEX : O_MODE), 0600)) >= 0) {
                if (DO_TEMPORARY & flags) {
                    DWORD attrs;

                    if (INVALID_FILE_ATTRIBUTES != (attrs = GetFileAttributesA(path))) {
                        if (0 == (attrs & FILE_ATTRIBUTE_TEMPORARY)) { // signal temporary storage
                            (void) SetFileAttributesA(path, FILE_ATTRIBUTE_TEMPORARY | attrs);
                        }
                    }
                }
                return GETTEMP_SUCCESS;
            }
#else
            if ((*fildes = ((DO_TEMPORARY & flags) ? OpenTemporaryA(path) :
                    WIN32_OPEN(path, O_MODE, 0600))) >= 0) {
                return GETTEMP_SUCCESS;
            }
#endif //ISTEMPORARY

            if (EEXIST != errno) {
                return GETTEMP_ERROR;
            }

        } else {
            DISABLE_HARD_ERRORS
            rc = w32_statA(path, &sbuf);
            ENABLE_HARD_ERRORS
            if (rc) {
#ifndef ENMFILE
                return (((ENOENT == errno)) ? GETTEMP_SUCCESS : GETTEMP_ERROR);
#else
                return (((ENOENT == errno) || (ENMFILE == errno)) ? GETTEMP_ERROR);
#endif
            }
        }

        /* next is sequence */
        for (trv = start;;) {
            if (trv == end) {
                return GETTEMP_ERROR;           /* EOS */
            }

            if ('z' == *trv) {                  /* 0..9a..z */
                *trv++ = 'a';
            } else {
                if (isdigit(*trv)) {
                    *trv = 'a';
                } else {
                    ++*trv;
                }
                break;
            }
        }
    }
    /*NOTREACHED*/
}


static int
gettempW_tmp(wchar_t *result, const wchar_t *path, int suffixlen, int *fildes, unsigned flags)
{
    /*
     *  "/tmp/", reference system temporary path
     */
    if (path && 0 == wmemcmp(path, L"/tmp/", 5)) {
        wchar_t t_path[MAX_PATH], *p;
        size_t pathlen = wcslen(path + 5),
            tmplen = (int)GetTempPathW(_countof(t_path), t_path);
                // TMP, TEMP, USERPROFILE environment variables, default windows directory.

        if (pathlen && tmplen) {
            if ((pathlen + tmplen) >= (int)(_countof(t_path) + 1)) {
                errno = ENAMETOOLONG;

            } else {
                if (t_path[tmplen-1] != '\\') {
                    t_path[tmplen-1] = '\\', ++tmplen;
                }
                wmemcpy(t_path + tmplen, path + 5, pathlen + 1 /*nul*/);
                for (p = t_path; NULL != (p = wcschr(p, '/'));) {
                    *p++ = '\\';                /* convert */
                }

                if (GETTEMP_SUCCESS == gettempW(t_path, suffixlen, fildes, flags, NULL)) {
                    wcscpy(result, L"/tmp/");
                    wcscpy(result + 5, t_path + tmplen);
                    return GETTEMP_SUCCESS;
                }
            }
        }
    }
    return GETTEMP_ERROR;
}


#if !defined(ISTEMPORARY)
static int
OpenTemporaryW(const wchar_t *path)
{
    const DWORD desiredAccess = GENERIC_READ | GENERIC_WRITE; // O_RDWR
    const DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE; // 0600 + delete
    const DWORD creationDisposition = CREATE_NEW; // O_EXCL
    const DWORD flagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY;
    HANDLE handle;
    int fd = -1;

    handle = CreateFileW(path,
    desiredAccess, shareMode, NULL /*O_NOINHERIT*/, creationDisposition, flagsAndAttributes, NULL);
    if (INVALID_HANDLE_VALUE != handle) {
        (void) DeleteFileW(path); // delete-on-close, needs FILE_SHARE_DELETE
        if ((fd = _open_osfhandle((intptr_t)handle, _O_RDWR|_O_BINARY)) < 0) {
            CloseHandle(handle);
            errno = ENFILE; // file limit
        }
    }
    return fd;
}
#endif //ISTEMPORARY


static int
gettempW(wchar_t *path, int suffixlen, int *fildes, unsigned flags, wchar_t *save)
{
    register wchar_t *start, *trv, *end, c;
    struct stat sbuf;
    unsigned seed;
    int rc;

    if (save) {
        for (trv = path; (*save = *trv) != 0; ++save) {
            if ((++trv - path) >= MAX_PATH) {
                errno = ENAMETOOLONG;
                return GETTEMP_ERROR;
            }
        }
    } else {
        for (trv = path; *trv; ++trv);
        if ((trv - path) >= MAX_PATH) {
            errno = ENAMETOOLONG;
            return GETTEMP_ERROR;
        }
    }

    trv -= suffixlen;
    end = trv;
    if (suffixlen < 0 || trv <= path ||         /* out-of-bounds? */
            NULL != wcschr(end, '/') || NULL != wcschr(end, '\\')) {
        errno = EINVAL;
        return GETTEMP_ERROR;
    }

    seed = generate_seed();
    while (--trv >= path && *trv == 'X') {
        *trv = (char)((seed % 10) + '0');
        seed /= 10;                             /* extra X's get set to 0's */
    }

    if ((trv + 1) == end) {                     /* missing template? */
        errno = EINVAL;
        return GETTEMP_ERROR;
    }

    /*
     *  check the target directory; if you have six X's and it
     *  doesn't exist this runs for a *very* long time.
     */
    for (start = trv + 1;; --trv) {
        if (trv <= path) {
            break;
        }

        if ((c = *trv) == '/' || c == '\\') {
            if (trv[-1] == ':') {
                break;
            }
            *trv = '\0';
            DISABLE_HARD_ERRORS
            rc = w32_statW(path, &sbuf);
            ENABLE_HARD_ERRORS
            *trv = c;
            if (rc) {
                return GETTEMP_ERROR;
            }
            if (!(sbuf.st_mode & S_IFDIR)) {
                errno = ENOTDIR;
                return GETTEMP_ERROR;
            }
            break;
        }
    }

    /*
     *  Create file as temporary; file is deleted when last file descriptor is closed.
     */
    for (;;) {
        errno = 0;
        if (DO_MKDIR & flags) {
            if (0 == w32_mkdirW(path, 0700)) {
                return GETTEMP_SUCCESS;
            }

            if (EEXIST != errno) {
                return GETTEMP_ERROR;
            }

        } else if (fildes) {
#if defined(ISTEMPORARY)
            if ((*fildes = WIN32_WOPEN(path, ((DO_TEMPORARY & flags) ? O_MODEX : O_MODE), 0600)) >= 0) {
                if (DO_TEMPORARY & flags) {
                    DWORD attrs;

                    if (INVALID_FILE_ATTRIBUTES != (attrs = GetFileAttributesW(path))) {
                        if (0 == (attrs & FILE_ATTRIBUTE_TEMPORARY)) { // signal temporary storage
                            (void) SetFileAttributesW(path, FILE_ATTRIBUTE_TEMPORARY | attrs);
                        }
                    }
                }
                return GETTEMP_SUCCESS;
            }
#else
            if ((*fildes = ((DO_TEMPORARY & flags) ? OpenTemporaryW(path) :
                    WIN32_WOPEN(path, O_MODE, 0600))) >= 0) {
                return GETTEMP_SUCCESS;
            }
#endif //ISTEMPORARY

            if (EEXIST != errno) {
                return GETTEMP_ERROR;
            }

        } else {
            DISABLE_HARD_ERRORS
            rc = w32_statW(path, &sbuf);
            ENABLE_HARD_ERRORS
            if (rc) {
#ifndef ENMFILE
                return (((ENOENT == errno)) ? GETTEMP_SUCCESS : GETTEMP_ERROR);
#else
                return (((ENOENT == errno) || (ENMFILE == errno)) ? GETTEMP_ERROR);
#endif
            }
        }

        /* next is sequence */
        for (trv = start;;) {
            if (trv == end) {
                return GETTEMP_ERROR;           /* EOS */
            }

            if ('z' == *trv) {                  /* 0..9a..z */
                *trv++ = 'a';
            } else {
                if (isdigit(*trv)) {
                    *trv = 'a';
                } else {
                    ++*trv;
                }
                break;
            }
        }
    }
    /*NOTREACHED*/
}

/*end*/
