#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_flock_c,"$Id: w32_flock.c,v 1.2 2021/11/30 13:06:19 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 flockc() system calls
 *
 * Copyright (c) 2020, Adam Young.
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


#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501                     /* enable xp+ features */
#endif

#include "win32_internal.h"
#include <sys/file.h>
#include <unistd.h>

/*
//  NAME
//      flock -- apply or remove an advisory lock on an open file
//
//  SYNOPSIS
//      #include <sys/file.h>
//
//      #define LOCK_SH     0x01        // shared file lock.
//      #define LOCK_EX     0x02        // exclusive file lock.
//      #define LOCK_NB     0x04        // do not block	when locking.
//      #define LOCK_UN     0x08        // unlock file.
//
//  DESCRIPTION
//      Apply or remove an advisory lock on the open file specified by fd. The argument operation is one of the following:
//
//          LOCK_SH         Place a shared lock. More than one process may hold a shared lock for a given file at a given time.
//          LOCK_EX         Place an exclusive lock. Only one process may hold an exclusive lock for a given file at a given time.
//          LOCK_UN         Remove an existing lock held by this process.
//
//      A call to flock() may block if an incompatible lock is held by another process. To make a nonblocking request,
//      include LOCK_NB (by ORing) with any of the above operations.
//
//      A single file may not simultaneously have both shared and exclusive locks.
//
//      Locks created by flock() are associated with an open file table entry. This means that duplicate file descriptors
//      (created by, for example, fork(2) or dup(2)) refer to the same lock, and this lock may be modified or released using any of these descriptors.
//      Furthermore, the lock is released either by an explicit LOCK_UN operation on any of these duplicate descriptors, or when all such descriptors have been closed.
//
//      If a process uses open(2) (or similar) to obtain more than one descriptor for the same file, these descriptors are
//      treated independently by flock(). An attempt to lock the file using one of these file descriptors may be denied by
//      a lock that the calling process has already placed via another descriptor.
//
//      A process may only hold one type of lock (shared or exclusive) on a file. Subsequent flock() calls on an already
//      locked file will convert an existing lock to the new lock mode.
//
//      Locks created by flock() are preserved across an execve(2).
//
//      A shared or exclusive lock can be placed on a file regardless of the mode in which the file was opened.
//
//  RETURN VALUE
//      On success, zero is returned. On error, -1 is returned, and errno is set appropriately.
//
//      EBADF               fd is not an open file descriptor.
//      EINTR               While waiting to acquire a lock, the call was interrupted by delivery of a signal caught by a handler; see signal(7).
//      EINVAL              operation is invalid.
//      ENOLCK              The kernel ran out of memory for allocating lock records.
//      EWOULDBLOCK         The file is locked and the LOCK_NB flag was selected.
*/

LIBW32_API int
w32_flock(int fd, int operation)
{
    HANDLE handle = 0;
    int ret = -1;

    if (fd < 0) {
        errno = EBADF;

    } else if (fd >= WIN32_FILDES_MAX ||
            (handle = (HANDLE) _get_osfhandle(fd)) == INVALID_HANDLE_VALUE) {
        errno = EBADF;

    } else {
        DWORD szLower = 0, szUpper = 0;
        OVERLAPPED ov = {0};

        szLower = GetFileSize(handle, &szUpper);
        if (LOCK_UN & operation) {              // unlock
            if (UnlockFileEx(handle, 0, szLower, szUpper, &ov)) {
                ret = 0;
            } else {
                w32_errno_set();
            }

        } else {                                // lock
            DWORD dwFlags = 0;

            if (0 == (LOCK_SH & operation))     // not-shared
                dwFlags |= LOCKFILE_EXCLUSIVE_LOCK;

            if (0 != (LOCK_NB & operation))     // non-blocking
                dwFlags |= LOCKFILE_FAIL_IMMEDIATELY;

            if (LockFileEx(handle, dwFlags, 0, szLower, szUpper, &ov)) {
                ret = 0;
            } else {
                const DWORD err = GetLastError();
                switch (err) {
                case ERROR_LOCK_VIOLATION:
                    errno = EWOULDBLOCK;
                    break;
                default:
                    w32_errno_setas(err);
                    break;
                }
            }
        }
    }
    return ret;
}

/*end*/
