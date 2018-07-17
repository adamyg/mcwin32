/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 getcwd() implementation
 *
 * Copyright (c) 2007, 2012 - 2018 Adam Young.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * The Midnight Commander is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained 
 * online at http://www.opengroup.org/unix/online.html.
 * ==end==
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
char *
w32_getcwd(char *path, int size)
{
    char t_path[1024];

    if (NULL == path || size <= 0) {
        errno = EINVAL;

    } else if (size < 64) {
        errno = ERANGE;

    } else {
        DWORD ret;

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

        //  If the function succeeds, the return value is the length, in characters,
        //  of the string copied to lpszLongPath, not including the terminating 
        //  null character.
        //
        //  If the lpBuffer buffer is too small to contain the path, the return value 
        //  is the size, in characters, of the buffer that is required to hold 
        //  the path and the terminating null character.
        //
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


char *
w32_getcwdd(char drive, char *path, int size)
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
        char t_path[1024];
        char pathrel[4] = {"X:."}, *file = NULL;
        DWORD ret;

        if (x_w32_cwdd[nDrive]) {               /* chdir() cache */
            const char *in;
            char *out;

            for (in = x_w32_cwdd[nDrive], out = path; *in; ++in) {
                if (--size <= 0) {
                    errno = ENOMEM;
                    break;
                }
                *out++ = *in;
            }
            *out = 0;
            return path;
        }

        //  If the function succeeds, the return value is the length, in characters,
        //  of the string copied to lpszLongPath, not including the terminating 
        //  null character.
        //
        //  If the lpBuffer buffer is too small to contain the path, the return value 
        //  is the size, in characters, of the buffer that is required to hold 
        //  the path and the terminating null character.
        //
        pathrel[0] = ('A' + nDrive);            /* A ... Z */

        if ((ret = GetFullPathNameA(pathrel, sizeof(t_path), t_path, &file)) == 0) {
            w32_errno_set();

        } else if (ret >= (DWORD)size || ret >= sizeof(t_path)) {
            errno = ENOMEM;
            
        } else {
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
