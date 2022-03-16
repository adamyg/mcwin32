#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_fallocate_c, "$Id: w32_fallocate.c,v 1.5 2022/03/16 13:46:59 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 posix_fallocate() system calls
 *
 * Copyright (c) 2018 - 2022 Adam Young.
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
#define _WIN32_WINNT        0x0501              /* enable xp+ features */
#endif

#include "win32_internal.h"
#include <unistd.h>

static BOOL                 FileTell(HANDLE hFile, uint64_t *pos);
static BOOL                 FileSeek(HANDLE hFile, uint64_t pos);


/*
NAME
    posix_fallocate - file space control (ADVANCED REALTIME)

SYNOPSIS
    #include <fcntl.h>

    int posix_fallocate(int fd, off_t offset, off_t len);

DESCRIPTION

    The posix_fallocate() function shall ensure that any required storage for
    regular file data starting at offset and continuing for len bytes is allocated
    on the file system storage media. If posix_fallocate() returns successfully,
    subsequent writes to the specified file data shall not fail due to the lack
    of free space on the file system storage media.

    If the offset+ len is beyond the current file size, then posix_fallocate()
    shall adjust the file size to offset+ len. Otherwise, the file size shall not
    be changed.

    It is implementation-defined whether a previous posix_fadvise() call influences
    allocation strategy.

    Space allocated via posix_fallocate() shall be freed by a successful call to
    creat() or open() that truncates the size of the file. Space allocated via
    posix_fallocate() may be freed by a successful call to ftruncate() that reduces
    the file size to a size smaller than offset+ len.

RETURN VALUE

    Upon successful completion, posix_fallocate() shall return zero;
    otherwise, an error number shall be returned to indicate the error.

ERRORS

    The posix_fallocate() function shall fail if:

    [EBADF]
         The fd argument is not a valid file descriptor.

    [EBADF]
        The fd argument references a file that was opened without write permission.

    [EFBIG]
        The value of offset+ len is greater than the maximum file size.

    [EINTR]
        A signal was caught during execution.

    [EINVAL]
        The len argument was zero or the offset argument was less than zero.

    [EIO]
        An I/O error occurred while reading from or writing to a file system.

    [ENODEV]
        The fd argument does not refer to a regular file.

    [ENOSPC]
        There is insufficient free space remaining on the file system storage media.

    [ESPIPE]
        The fd argument is associated with a pipe or FIFO.

*/

LIBW32_API int
posix_fallocate(int fd, off_t offset, off_t len)
{
    HANDLE handle;
    int ret = -1;

    if (fd < 0) {
        errno = EBADF;
    } else if (fd >= WIN32_FILDES_MAX ||
            (handle = (HANDLE) _get_osfhandle(fd)) == INVALID_HANDLE_VALUE) {
        errno = EBADF;
    } else if (0 == (offset + len)) {
        errno = EINVAL;
    } else if ((offset + len) >= ((16LL * 1028 * 1024 * 1024) - (4 * 1024))) {
        errno = EFBIG;                          // 16GB
    } else {
#if (0) //TODO - additional testing
        uint64_t newpos = offset + len, oldpos = 0;
        DWORD err = 0;
        char buf = 0;

        if (newpos <= oldpos) {
            ret = 0;                            // success; no change

        } else if (! FileTell(handle, &oldpos)) {
            err = GetLastError();               // access error etc

        //  Seek and hen set the new physical file size for the specified file, extending the file.
        } else if (! FileSeek(handle, newpos) || ! SetEndOfFile(handle)) {
            err = GetLastError();
            (void) FileSeek(handle, oldpos);    // attempt to restore position.

        //  Sets the valid data length of the specified file:
        //    If SetFileValidData is used on a file, the potential performance gain is obtained by not filling the
        //    allocated clusters for the file with zeros. Therefore, reading from the file will return whatever
        //    the allocated clusters contain, potentially content from other users.
        //
        } else if (SetFileValidData(handle, newpos)) {
            ret = 0;                            // success

        } else {        // Alternative, mark file as sparse and zero-fill.
            if (DeviceIoControl(handle, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &buf, NULL)) {
                FILE_ZERO_DATA_INFORMATION zero = {0};

                zero.FileOffset.QuadPart = oldpos;
                zero.BeyondFinalZero.QuadPart = newpos;
                if (DeviceIoControl(handle, FSCTL_SET_ZERO_DATA, &zero, sizeof(zero), NULL, 0, &buf, NULL)) {
                    ret = 0;                    // success
                }

            } else {    // Not-supported
                (void) FileSeek(handle, oldpos);
                errno = EOPNOTSUPP;
            }
        }

        if (ret && err) {
            w32_errno_setas(err);
        }

#else   //TODO - additional testing
        errno = EOPNOTSUPP;
#endif
    }
    return ret;
}


static BOOL
FileTell(HANDLE hFile, uint64_t *pos)
{
    LARGE_INTEGER liDistanceToMove = {0}, liNewFilePointer = {0};

    if (SetFilePointerEx(hFile, liDistanceToMove, &liNewFilePointer, FILE_CURRENT)) {
        *pos = liNewFilePointer.QuadPart;
        return TRUE;
    }
    return FALSE;
}


static BOOL
FileSeek(HANDLE hFile, uint64_t pos)
{
    LARGE_INTEGER liDistanceToMove = {0}, liNewFilePointer = {0};

    liDistanceToMove.QuadPart = pos;
    if (SetFilePointerEx(hFile, liDistanceToMove, &liNewFilePointer, FILE_BEGIN)) {
        return TRUE;
    }
    return FALSE;
}

/*end*/
