#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_truncate_c,"$Id: w32_truncate.c,v 1.11 2023/09/17 13:05:00 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 ftruncate()/truncate() system calls.
 *
 * Copyright (c) 2007, 2012 - 2023 Adam Young.
 * All rights reserved.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#include "win32_internal.h"

#include <stdio.h>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#include <unistd.h>

/*
//    NAME
//       ftruncate, truncate - truncate a file to a specified length
//
//    SYNOPSIS
//
//       #include <unistd.h>
//
//       int ftruncate(int fildes, off_t length);
//       int truncate(const char *path, off_t length);
//
//    DESCRIPTION
//
//       The ftruncate() function causes the regular file referenced by fildes to have a
//       size of length bytes.
//
//       The truncate() function causes the regular file named by path to have a size of
//       length bytes.
//
//       If the file previously was larger than length, the extra data is discarded. If it
//       was previously shorter than length, it is unspecified whether the file is changed
//       or its size increased. If the file is extended, the extended area appears as if it
//       were zero-filled. If fildes references a shared memory object, ftruncate() sets the
//       size of the shared memory object to length. If the file is not a regular file or a
//       shared memory object, the result is unspecified.
//
//       With ftruncate(), the file must be open for writing; for truncate(), the process
//       must have write permission for the file.
//
//       If the effect of truncation is to decrease the size of a file or shared memory
//       object and whole pages beyond the new end were previously mapped, then the whole
//       pages beyond the new end will be discarded. References to the discarded pages
//       result in generation of a SIGBUS signal.
//
//       If the request would cause the file size to exceed the soft file size limit for the
//       process, the request will fail and the implementation will generate the SIGXFSZ
//       signal for the process.
//
//       These functions do not modify the file offset for any open file descriptions
//       associated with the file. On successful completion, if the file size is changed,
//       these functions will mark for update the st_ctime and st_mtime fields of the file,
//       and if the file is a regular file, the S_ISUID and S_ISGID bits of the file mode
//       may be cleared.
//
//    RETURN VALUE
//
//       Upon successful completion, ftruncate() and truncate() return 0. Otherwise a -1 is
//       returned, and errno is set to indicate the error.
//
//    ERRORS
//
//       The ftruncate() and truncate() functions will fail if:
//
//       [EINTR]
//           A signal was caught during execution.
//
//       [EINVAL]
//           The length argument was less than 0.
//
//       [EFBIG] or [EINVAL]
//           The length argument was greater than the maximum file size.
//
//       [EIO]
//           An I/O error occurred while reading from or writing to a file system.
//
//       The ftruncate() function will fail if:
//
//       [EBADF] or [EINVAL]
//           The fildes argument is not a file descriptor open for writing.
//
//       [EFBIG]
//           The file is a regular file and length is greater than the offset maximum
//           established in the open file description associated with fildes.
//
//       [EINVAL]
//           The fildes argument references a file that was opened without write permission.
//
//       [EROFS]
//           The named file resides on a read-only file system.
//
//       The truncate() function will fail if:
//
//       [EACCES]
//           A component of the path prefix denies search permission, or write
//           permission is denied on the file.
//
//       [EISDIR]
//           The named file is a directory.
//
//       [ELOOP]
//           Too many symbolic links were encountered in resolving path.
//
//       [ENAMETOOLONG]
//           The length of the specified pathname exceeds PATH_MAX bytes, or the length of a
//           component of the pathname exceeds NAME_MAX bytes.
//
//       [ENOENT]
//           A component of path does not name an existing file or path is an empty string.
//
//       [ENOTDIR]
//           A component of the path prefix of path is not a directory.
//
//       [EROFS]
//           The named file resides on a read-only file system.
//
//       The truncate() function may fail if:
//
//       [ENAMETOOLONG]
//           Pathname resolution of a symbolic link produced an intermediate result
//           whose length exceeds {PATH_MAX}.
*/
int
ftruncate(int fildes, off_t length)
{
    HANDLE handle;

    if (length < 0) {
        errno = EINVAL;
        return -1;

    } else if (fildes < 0 ||
                (handle = (HANDLE) _get_osfhandle(fildes)) == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return -1;
    }

    if (0xFFFFFFFF == SetFilePointer (handle, 0, NULL, FILE_CURRENT) ||
                0xFFFFFFFF == SetFilePointer (handle, length, NULL, FILE_BEGIN) ||
            !SetEndOfFile (handle)) {
        const DWORD rc = GetLastError();

        switch (rc) {
        case ERROR_INVALID_HANDLE:
            errno = EBADF;
            break;
        default:
            w32_errno_set();
            break;
        }
        return -1;
    }
    return 0;
}


int
truncate(const char *path, off_t length)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t wpath[WIN32_PATH_MAX];

        if (NULL == path) {
            errno = EFAULT;
            return -1;
        }

        if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
            return truncateW(wpath, length);
        }

        return -1;
    }
#endif  //UTF8FILENAMES

    return truncateA(path, length);
}


int
truncateA(const char *path, off_t length)
{
    HANDLE handle;

    if (length < 0) {
        errno = EINVAL;
        return -1;

    } else if (NULL == path || 0 == *path) {
        errno = EINVAL;
        return -1;
    }

    if (INVALID_HANDLE_VALUE == (handle =
            CreateFileA(path, GENERIC_WRITE, 0,
                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) ||

            0xFFFFFFFF == SetFilePointer (handle, 0, NULL, FILE_CURRENT) ||
                    0xFFFFFFFF == SetFilePointer (handle, length, NULL, FILE_BEGIN) ||
            !SetEndOfFile (handle)) {
        w32_errno_set();
        if (INVALID_HANDLE_VALUE != handle) {
            CloseHandle(handle);
        }
        return -1;
    }
    CloseHandle(handle);
    return 0;
}


int
truncateW(const wchar_t *path, off_t length)
{
    HANDLE handle;

    if (length < 0) {
        errno = EINVAL;
        return -1;

    } else if (NULL == path || 0 == *path) {
        errno = EINVAL;
        return -1;
    }

    if (INVALID_HANDLE_VALUE == (handle =
            CreateFileW(path, GENERIC_WRITE, 0,
                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) ||

            0xFFFFFFFF == SetFilePointer (handle, 0, NULL, FILE_CURRENT) ||
                    0xFFFFFFFF == SetFilePointer (handle, length, NULL, FILE_BEGIN) ||
            !SetEndOfFile (handle)) {
        w32_errno_set();
        if (INVALID_HANDLE_VALUE != handle) {
            CloseHandle(handle);
        }
        return -1;
    }
    CloseHandle(handle);
    return 0;
}


//  int
//  truncate64(const char *path, off64_t length)
//  {
//      errno = EIO;
//      return -1;
//  }

/*end*/
