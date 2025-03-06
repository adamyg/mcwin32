#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_getcwdd_c,"$Id: w32_getcwdd.c,v 1.12 2025/03/06 16:59:46 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 getcwdd() implementation
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
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
#include <ctype.h>
#include <errno.h>

/*
//  NAME
//
//      getcwd, getcwdd - get the pathname of the current working directory
//
//  SYNOPSIS
//
//      #include <unistd.h>
//
//      char *getcwd(char *buf, size_t size);
//      char *getcwdd(char drive, char *buf, size_t size);
//
//  DESCRIPTION
//
//      The getcwd() function shall place an absolute pathname of the current working
//      directory in the array pointed to by buf, and return buf. The pathname copied to
//      the array shall contain no components that are symbolic links. The size argument is
//      the size in bytes of the character array pointed to by the buf argument. If buf is
//      a null pointer, the behavior of getcwd() is unspecified.
//
//      The getcwdd() function retrieves the absolute pathname of the current working for the
//      specificed drive (A thru Z).
//
//  RETURN VALUE
//
//      Upon successful completion, getcwd() shall return the buf argument. Otherwise,
//      getcwd() shall return a null pointer and set errno to indicate the error. The
//      contents of the array pointed to by buf are then undefined.
//
//  ERRORS
//
//      The getcwd() function shall fail if:
//
//      [EINVAL]
//          The size argument is 0.
//
//      [ERANGE]
//          The size argument is greater than 0, but is smaller than the length of the pathname +1.
//
//      The getcwd() function may fail if:
//
//      [EACCES]
//          Read or search permission was denied for a component of the pathname.
//
//      [ENOMEM]
//          Insufficient storage space is available.
*/

LIBW32_API char *
w32_getcwdd(char drive, char *path, size_t size)
{
    if (NULL == path || size <= 0) {
        errno = EINVAL;

    } else if (size < 64) {
        errno = ERANGE;

    } else {
#if defined(UTF8FILENAMES)
        if (w32_utf8filenames_state()) {
            wchar_t *wpath;

            if (NULL != (wpath = alloca(sizeof(wchar_t) * (size + 1))) &&
                    w32_getcwddW(drive, wpath, size)) {
                w32_wc2utf(wpath, path, size);
                return path;
            }
            return NULL;
        }
#endif  //UTF8FILENAMES

        return w32_getcwddA(drive, path, size);
    }
    return NULL;
}


LIBW32_API char *
w32_getcwddA(char drive, char *path, size_t size)
{
    const unsigned nDrive =
            (isalpha((unsigned char)drive) ? (toupper(drive) - 'A') : 0xff);

    if (NULL == path || size <= 0) {
        errno = EINVAL;

    } else if (size < 64) {
        errno = ERANGE;

    } else if (nDrive >= 26) {
        errno = EINVAL;

    } else {
        //  If the function succeeds, the return value is the length, in characters,
        //  of the string copied to lpszLongPath, not including the terminating
        //  null character.
        //
        //  If the lpBuffer buffer is too small to contain the path, the return value
        //  is the size, in characters, of the buffer that is required to hold
        //  the path and the terminating null character.
        //
        char t_path[WIN32_PATH_MAX];
        char rel[4] = {"X:."}, *file = NULL;
        DWORD ret;

        rel[0] = (char)('A' + nDrive);      /* A ... Z */

        t_path[0] = 0;
        if ((ret = GetFullPathNameA(rel, _countof(t_path), t_path, &file)) == 0) {
            w32_errno_set();

        } else if (ret >= (DWORD)size || ret >= _countof(t_path)) {
            errno = ENOMEM;

        } else {
            const char *in;
            char *out;

            for (in = t_path, out = path; *in; ++in) {
                if ('~' == *in) {           /* shortname expand */
                    (void) GetLongPathNameA(t_path, t_path, _countof(t_path));
                    for (in = t_path, out = path; *in; ++in) {
                        *out++ = ('\\' == *in ? '/' : *in);
                    }
                    break;
                }
                *out++ = ('\\' == *in ? '/' : *in);
            }
            *out = 0;
            return path;
        }
    }

    if (path && size > 0) path[0] = 0;
    return NULL;
}



LIBW32_API wchar_t *
w32_getcwddW(char drive, wchar_t *path, size_t size)
{
    const unsigned nDrive =
            (isalpha((unsigned char)drive) ? (toupper(drive) - 'A') : 0xff);

    if (NULL == path || size <= 0) {
        errno = EINVAL;

    } else if (size < 64) {
        errno = ERANGE;

    } else if (nDrive >= 26) {
        errno = EINVAL;

    } else {
        //  If the function succeeds, the return value is the length, in characters,
        //  of the string copied to lpszLongPath, not including the terminating
        //  null character.
        //
        //  If the lpBuffer buffer is too small to contain the path, the return value
        //  is the size, in characters, of the buffer that is required to hold
        //  the path and the terminating null character.
        //
        wchar_t t_path[WIN32_PATH_MAX];
        wchar_t rel[4] = {L"X:."}, *file = NULL;
        DWORD ret;

        rel[0] = (wchar_t)('A' + nDrive);       /* A ... Z */

        t_path[0] = 0;
        if ((ret = GetFullPathNameW(rel, _countof(t_path), t_path, &file)) == 0) {
            w32_errno_set();

        } else if (ret >= (DWORD)size || ret >= _countof(t_path)) {
            errno = ENOMEM;

        } else {
            const wchar_t *in;
            wchar_t *out;

            for (in = t_path, out = path; *in; ++in) {
                if ('~' == *in) {               /* shortname expand */
                    (void) GetLongPathNameW(t_path, t_path, _countof(t_path));
                    for (in = t_path, out = path; *in; ++in) {
                        *out++ = ('\\' == *in ? '/' : *in);
                    }
                    break;
                }
                *out++ = ('\\' == *in ? '/' : *in);
            }
            *out = 0;
            return path;
        }
    }

    if (path && size > 0) path[0] = 0;
    return NULL;
}

/*end*/
