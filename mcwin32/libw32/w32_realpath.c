#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_realpath_c, "$Id: w32_realpath.c,v 1.13 2023/12/28 17:30:52 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 directory access services ...
 *
 *      realpath
 *
 * Copyright (c) 2018 - 2023 Adam Young.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT            0x0501          /* reparse */
#endif

#define _DIRENT_SOURCE
#include "win32_internal.h"

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

#include "win32_io.h"


//  NAME
//	realpath - resolve a pathname
//
//  SYNOPSIS
//	#include <stdlib.h>
//	char *realpath(const char *restrict file_name, char *restrict resolved_name);
//
//  DESCRIPTION
//	The realpath() function shall derive, from the pathname pointed to by file_name, an absolute
//	pathname that names the same file, whose resolution does not involve '.', '..', or symbolic
//	links. The generated pathname shall be stored as a null-terminated string, up to a maximum of
//	{PATH_MAX} bytes, in the buffer pointed to by resolved_name.
//
//	If resolved_name is a null pointer, the behavior of realpath() is implementation-defined.
//
//  RETURN VALUE
//	Upon successful completion, realpath() shall return a pointer to the resolved name. Otherwise,
//	realpath() shall return a null pointer and set errno to indicate the error, and the contents of
//	the buffer pointed to by resolved_name are undefined.
//
//  ERRORS
//	The realpath() function shall fail if:
//
//	[EACCES]
//	    Read or search permission was denied for a component of file_name.
//	[EINVAL]
//	    The file_name argument is a null pointer.
//	[EIO]
//	    An error occurred while reading from the file system.
//	[ELOOP]
//	    A loop exists in symbolic links encountered during resolution of the file_name argument.
//	[ENAMETOOLONG]
//	    The length of the file_name argument exceeds { PATH_MAX} or a pathname component is longer
//	    than {NAME_MAX}.
//	[ENOENT]
//	    A component of file_name does not name an existing file or file_name points to an empty string.
//	[ENOTDIR]
//	    A component of the path prefix is not a directory.
//
//	The realpath() function may fail if:
//
//	[ELOOP]
//	    More than {SYMLOOP_MAX} symbolic links were encountered during resolution of the file_name argument.
//	[ENAMETOOLONG]
//	    Pathname resolution of a symbolic link produced an intermediate result whose length exceeds {PATH_MAX}.
//	[ENOMEM]
//	    Insufficient storage space is available.
//
LIBW32_API char *
w32_realpath(const char *path, char *resolved_path /*[PATH_MAX]*/)
{
    return w32_realpath2(path, resolved_path, PATH_MAX);
}


LIBW32_API char *
w32_realpath2(const char *path, char *resolved_path, size_t maxlen)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t *wresolved_path, wpath[WIN32_PATH_MAX];

        w32_utf2wc(path, wpath, _countof(wpath));

        if (NULL != (wresolved_path = alloca(sizeof(wchar_t *) * WIN32_PATH_MAX))) {
            if (w32_realpathW(wpath, wresolved_path, WIN32_PATH_MAX)) {
                if (NULL == resolved_path) {    // dynamic; implementation specific.
                    maxlen = WIN32_PATH_MAX;
                    if (NULL == (resolved_path = (char *)malloc(sizeof(char) * maxlen))) {
                        return NULL;
                    }
                }

                if (resolved_path) {
                    w32_wc2utf(wresolved_path, resolved_path, maxlen);
                    return resolved_path;
                }
            }
        }
        return NULL;
    }
#endif  //UTF8FILENAMES

    return w32_realpathA(path, resolved_path, maxlen);
}


LIBW32_API char *
w32_realpathA(const char *path, char *resolved_path, size_t maxlen)
{
    char *result = NULL, symlink[WIN32_PATH_MAX];

    if (path && *path) {
        if (NULL != (result = resolved_path)) { // user buffer.
            if (maxlen < 4) {                   // "X:/\0"
                errno = EINVAL;                 // invalid length.
                result = NULL;
            }
        } else {                                // glibc style extension.
            maxlen = WIN32_PATH_MAX;
            result = (char *)malloc(sizeof(char) * maxlen);
                // malloc() shall set errno on error.
        }

        if (result) {                           // root
            /*
             *  Generic chdir("/") behaviour is context specific, meaning goto root of current drive,
             *  mount point or current UNC. Normalisze this behaviour to root of current/last drive.
             */
            if (('/' == path[0] || '\\' == path[0]) && 0 == path[1]) {
                int driveno = w32_getdrive();   // also see chdir()
                if (driveno <= 0) driveno = w32_getlastdrive();
                if (driveno <= 0) driveno = w32_getsystemdrive();
                if (driveno > 0) {
                    result[0] = (char)(driveno + ('A' - 1));
                    result[1] = ':';
                    result[2] = '/';
                    result[3] = 0;
                    return result;
                }
            }
        }

        if (result) {                           // resolve symlink component
            if (w32_lnkexpandA(path, symlink, _countof(symlink), SHORTCUT_COMPONENT)) {
                path = symlink;
            }
        }

        if (result) {
            const size_t size =                 // resolve, including .. and . components
                GetFullPathNameA(path, (DWORD)maxlen, result, 0);

            //
            //  GetFullPathNameA() returns a size larger than buffer if buffer is too small
            if (size >= (size_t)maxlen) {
                if (result != resolved_path) {
                    free((void *)result);

                    maxlen = size + 1 /*plus null*/;
                    result = (char *)malloc(sizeof(char) * maxlen);
                        // malloc() shall set errno on error.

                    if (result) {
                        const size_t new_size =
                            GetFullPathNameA(path, (DWORD)maxlen, result, 0);
                        if (new_size >= (size_t)maxlen) {
                            free((void *)result);
                            errno = ENAMETOOLONG;
                            result = NULL;
                            return NULL;
                        }
                    }
                } else {                        // resolved_path buffer isn't big enough
                    errno = ENAMETOOLONG;
                    result = NULL;
                }
            }

            //
            //  GetFullPathNameA() returns 0 if some path resolve problem occured
            if (0 == size) {
                switch (GetLastError()) {
                case ERROR_ACCESS_DENIED:
                case ERROR_SHARING_VIOLATION:
                case ERROR_PRIVILEGE_NOT_HELD:
                    errno = EACCES;  break;
                case ERROR_FILE_NOT_FOUND:
                    errno = ENOENT;  break;
                case ERROR_PATH_NOT_FOUND:
                case ERROR_INVALID_DRIVE:
                    errno = ENOTDIR; break;
                default:
                    errno = EIO;
                    break;
                }

                if (result != resolved_path) {
                    free((void *)result);
                }
                result = NULL;
            }

            //
            //  Success, verify the result.
            if (result) {
                struct stat sb = { 0 };
                if (w32_statA(result, &sb)) {
                    if (result != resolved_path) {
                        free(result);
                    }
                    result = NULL;
                        // stat() shall set errno on error.
                }
            }
        }
    } else {
        errno = EINVAL;                         // invalid source.
    }

    if (result) {
        if (result[0] && result[1] == ':') {    // normalize directory slashes.
            char *cursor;
            for (cursor = result; *cursor; ++cursor) {
                if ('\\' == *cursor) *cursor = '/';
            }
        }
    }
    return result;
}


LIBW32_API wchar_t *
w32_realpathW(const wchar_t *path, wchar_t *resolved_path, size_t maxlen)
{
    wchar_t *result = NULL, symlink[WIN32_PATH_MAX];

    if (path && *path) {
        if (NULL != (result = resolved_path)) { // user buffer.
            if (maxlen < 4) {                   // "X:/\0"
                errno = EINVAL;                 // invalid length.
                result = NULL;
            }
        } else {                                // glibc style extension.
            maxlen = WIN32_PATH_MAX;
            result = (wchar_t *)malloc(sizeof(wchar_t) * maxlen);
                // malloc() shall set errno on error.
        }

        if (result) {                           // root
            /*
             *  Generic chdir("/") behaviour is context specific, meaning goto root of current drive,
             *  mount point or current UNC. Normalisze this behaviour to root of current/last drive.
             */
            if (('/' == path[0] || '\\' == path[0]) && 0 == path[1]) {
                int driveno = w32_getdrive();
                if (driveno <= 0) driveno = w32_getlastdrive();
                if (driveno <= 0) driveno = w32_getsystemdrive();
                if (driveno > 0) {
                    result[0] = (wchar_t)(driveno + ('A' - 1));
                    result[1] = ':';
                    result[2] = '/';
                    result[3] = 0;
                    return result;
                }
            }
        }

        if (result) {                           // resolve symlink component
            if (w32_lnkexpandW(path, symlink, _countof(symlink), SHORTCUT_COMPONENT)) {
                path = symlink;
            }
        }

        if (result) {
            const size_t size =                 // resolve, including .. and . components
                GetFullPathNameW(path, (DWORD)maxlen, result, 0);

            //
            //  GetFullPathNameA() returns a size larger than buffer if buffer is too small
            if (size >= (size_t)maxlen) {
                if (result != resolved_path) {
                    free((void *)result);

                    maxlen = size + 1 /*plus null*/;
                    result = (wchar_t *)malloc(sizeof(wchar_t) * maxlen);
                        // malloc() shall set errno on error.

                    if (result) {
                        const size_t new_size =
                            GetFullPathNameW(path, (DWORD)maxlen, result, 0);
                        if (new_size >= (size_t)maxlen) {
                            free((void *)result);
                            errno = ENAMETOOLONG;
                            result = NULL;
                            return NULL;
                        }
                    }
                } else {                        // resolved_path buffer isn't big enough
                    errno = ENAMETOOLONG;
                    result = NULL;
                }
            }

            //
            //  GetFullPathNameA() returns 0 if some path resolve problem occured
            if (0 == size) {
                switch (GetLastError()) {
                case ERROR_ACCESS_DENIED:
                case ERROR_SHARING_VIOLATION:
                case ERROR_PRIVILEGE_NOT_HELD:
                    errno = EACCES;  break;
                case ERROR_FILE_NOT_FOUND:
                    errno = ENOENT;  break;
                case ERROR_PATH_NOT_FOUND:
                case ERROR_INVALID_DRIVE:
                    errno = ENOTDIR; break;
                default:
                    errno = EIO;
                    break;
                }

                if (result != resolved_path) {
                    free((void *)result);
                }
                result = NULL;
            }

            //
            //  Success, verify the result.
            if (result) {
                struct stat sb = { 0 };
                if (w32_statW(result, &sb)) {
                    if (result != resolved_path) {
                        free(result);
                    }
                    result = NULL;
                        // stat() shall set errno on error.
                }
            }
        }
    } else {
        errno = EINVAL;                         // invalid source.
    }

    if (result) {
        if (result[0] && result[1] == ':') {    // normalize directory slashes.
            wchar_t *cursor;
            for (cursor = result; *cursor; ++cursor) {
                if ('\\' == *cursor) *cursor = '/';
            }
        }
    }
    return result;
}

/*end*/