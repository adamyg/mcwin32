#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_getcwd_c,"$Id: w32_getcwd.c,v 1.14 2021/05/12 12:29:29 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 getcwd() implementation
 *
 * Copyright (c) 2007, 2012 - 2021 Adam Young.
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
w32_getcwd(char *path, int size)
{
    if (NULL == path || size <= 0) {
        errno = EINVAL;

    } else if (size < 64) {
        errno = ERANGE;

    } else {
        if (x_w32_vfscwd) {                     /* vfs chdir() */
            const char *in;
            char *out;

            for (in = x_w32_vfscwd, out = path; *in; ++in) {
                if (--size <= 0) {
                    errno = ENOMEM;
                    break;
                }
                *out++ = *in;
            }
            *out = 0;
            return path;
        }
    }

#if defined(UTF8FILENAMES)
    {   wchar_t *wpath;

        if (NULL != (wpath = alloca(sizeof(wchar_t) * (size + 1))) &&
                w32_getcwdW(wpath, size)) {
            w32_wc2utf(wpath, path, size);
            return path;
        }
        return NULL;
    }

#else

    return w32_getcwdA(path, size);

#endif  //UTF8FILENAMES
}


LIBW32_API char *
w32_getcwdA(char *path, int size)
{
    char t_path[WIN32_PATH_MAX];

    if (NULL == path || size <= 0) {
        errno = EINVAL;

    } else if (size < 64) {
        errno = ERANGE;

    } else {
        //  If the function succeeds, the return value is the length, in characters,
        //  of the string copied to lpszLongPath, not including the terminating 
        //  null character.
        //
        //  If the lpBuffer buffer is too small to contain the path, the return value 
        //  is the size, in characters, of the buffer that is required to hold 
        //  the path and the terminating null character.
        //
        DWORD ret;

        t_path[0] = 0;
        if ((ret = GetCurrentDirectoryA(sizeof(t_path), t_path)) == 0) {
            w32_errno_set();
            
        } else if (ret >= (DWORD)size || ret >= sizeof(t_path)) {
            errno = ENOMEM;

        } else {                                /* standardise to the system seperator */
            const char *in;
            char *out;

            for (in = t_path, out = path; *in; ++in) {
                if ('~' == *in) {               /* shortname expand */
                    (void) GetLongPathNameA(t_path, t_path, sizeof(t_path));
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
w32_getcwdW(wchar_t *path, int size)
{
    wchar_t t_path[WIN32_PATH_MAX];

    if (NULL == path || size <= 0) {
        errno = EINVAL;

    } else if (size < 64) {
        errno = ERANGE;

    } else {
        //  If the function succeeds, the return value is the length, in characters,
        //  of the string copied to lpszLongPath, not including the terminating 
        //  null character.
        //
        //  If the lpBuffer buffer is too small to contain the path, the return value 
        //  is the size, in characters, of the buffer that is required to hold 
        //  the path and the terminating null character.
        //
        DWORD ret;

        t_path[0] = 0;
        if ((ret = GetCurrentDirectoryW(_countof(t_path), t_path)) == 0) {
            w32_errno_set();
            
        } else if (ret >= (DWORD)size || ret >= _countof(t_path)) {
            errno = ENOMEM;

        } else {                                /* standardise to the system seperator */
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


LIBW32_API int
w32_getdrive(void)
{
    wchar_t t_path[WIN32_PATH_MAX];
    DWORD ret;

    t_path[0] = 0, t_path[1] = 0;
    if ((ret = GetCurrentDirectoryW(_countof(t_path), t_path)) >= 2) {
        if (t_path[1] == ':') {                 /* X: */
            const wchar_t ch = t_path[0];

            if (ch >= L'A' && ch <= L'Z') {
                return (ch - L'A') + 1;
            }

            if (ch >= L'a' && ch <= L'z') {
                return (ch - L'a') + 1;
            }
        }
    }
    return 0;   // UNC
}


LIBW32_API int
w32_getsystemdrive(void)
{
    wchar_t t_path[WIN32_PATH_MAX];
    DWORD ret;

    t_path[0] = 0, t_path[1] = 0;
    if ((ret = GetSystemDirectoryW(_countof(t_path), t_path)) >= 2) {
        if (t_path[1] == ':') {                 /* X: */
            const wchar_t ch = t_path[0];

            if (ch >= L'A' && ch <= L'Z') {
                return (ch - L'A') + 1;
            }

            if (ch >= L'a' && ch <= L'z') {
                return (ch - L'a') + 1;
            }
        }
    }
    return 0;
}


LIBW32_API int
w32_getlastdrive(void)
{
    return x_w32_cwdn;
}

/*end*/
