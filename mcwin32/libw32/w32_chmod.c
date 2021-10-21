#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_chmod_c,"$Id: w32_chmod.c,v 1.4 2021/05/24 15:10:33 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 chmod() system calls.
 *
 * Copyright (c) 2020 - 2021 Adam Young.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0501              /* enable xp+ features */
#endif

#include "win32_internal.h"
#include <unistd.h>


/*
//  NAME
//      chmod
//
//  SYNOPSIS
//      #include <sys/stat.h>
//
//      int chmod(const char *pathname, mode_t mode);
//
//  DESCRIPTION
//      The chmod() and fchmod() system calls change a files mode bits.
//      chmod() changes the mode of the file specified whose pathname is given
//      in pathname, which is dereferenced if it is a symbolic link.
//
//      The file mode consists of the file permission bits plus the set-user-ID,
//      set-group-ID, and sticky bits)  These system calls differ only in how the
//      file is specified.
//
//  ERRORS
//
//      The chmod() function will fail if:
//
//      [EACCES]
//          Search permission is denied on a component of the path prefix.
//      [ELOOP]
//          Too many symbolic links were encountered in resolving path.
//      [ENAMETOOLONG]
//          The length of the path argument exceeds {PATH_MAX} or a pathname component is longer than {NAME_MAX}.
//      [ENOTDIR]
//          A component of the path prefix is not a directory.
//      [ENOENT]
//          A component of path does not name an existing file or path is an empty string.
//      [EPERM]
//          The effective user ID does not match the owner of the file and the process does not have appropriate privileges.
//      [EROFS]
//          The named file resides on a read-only file system.
//
//      The chmod() function may fail if:
//
//      [EINTR]
//          A signal was caught during execution of the function.
//      [EINVAL]
//          The value of the mode argument is invalid.
//      [ENAMETOOLONG]
//          Pathname resolution of a symbolic link produced an intermediate result whose length exceeds {PATH_MAX}.
*/

LIBW32_API int
w32_chmod(const char *pathname, mode_t mode)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t wpathname[WIN32_PATH_MAX];

        if (NULL == pathname) {
            errno = EFAULT;
            return -1;
        }

        if (w32_utf2wc(pathname, wpathname, _countof(wpathname)) > 0) {
            return w32_chmodW(wpathname, mode);
        }

        return -1;
    }
#endif  //UTF8FILENAMES

    return w32_chmodA(pathname, mode);
}


LIBW32_API int
w32_chmodA(const char *pathname, mode_t mode)
{
#undef chmod
    return chmod(pathname, mode);
}


LIBW32_API int
w32_chmodW(const wchar_t *pathname, mode_t mode)
{
    return _wchmod(pathname, mode);
}

/*end*/
