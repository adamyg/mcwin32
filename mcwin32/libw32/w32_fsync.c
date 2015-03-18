/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 2007, 2012 - 2015 Adam Young.
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
//      fsync - synchronize changes to a file
//  
//  SYNOPSIS
//  
//      #include <unistd.h>
//  
//      int fsync(int fildes);
//  
//  DESCRIPTION
//  
//      The fsync() function shall request that all data for the open file descriptor named
//      by fildes is to be transferred to the storage device associated with the file
//      described by fildes. The nature of the transfer is implementation-defined. The
//      fsync() function shall not return until the system has completed that action or
//      until an error is detected.
//  
//      If _POSIX_SYNCHRONIZED_IO is defined, the fsync() function shall force all
//      currently queued I/O operations associated with the file indicated by file
//      descriptor fildes to the synchronized I/O completion state. All I/O operations
//      shall be completed as defined for synchronized I/O file integrity completion.
//      [Option End]
//  
//  RETURN VALUE
//  
//      Upon successful completion, fsync() shall return 0. Otherwise, -1 shall be returned
//      and errno set to indicate the error. If the fsync() function fails, outstanding I/O
//      operations are not guaranteed to have been completed.
//  
//  ERRORS
//  
//      The fsync() function shall fail if:
//  
//      [EBADF]
//          The fildes argument is not a valid descriptor.
//  
//      [EINTR]
//          The fsync() function was interrupted by a signal.
//  
//      [EINVAL]
//          The fildes argument does not refer to a file on which this operation is possible.
//  
//      [EIO]
//          An I/O error occurred while reading from or writing to the file system.
//  
//      In the event that any of the queued I/O operations fail, fsync() shall return the
//      error conditions defined for read() and write().
*/
int
w32_fsync(int fd)
{
    HANDLE handle;
    int ret = 0;

    if (fd < 0) {
        errno = EBADF;
        ret = -1;
    } else if (fd >= WIN32_FILDES_MAX || 
            (handle = (HANDLE) _get_osfhandle(fd)) == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        ret = -1;

    } else if (! FlushFileBuffers(handle)) {
        w32_errno_set();
        ret = -1;
    }
    return ret;
}
