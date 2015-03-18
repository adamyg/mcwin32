/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 system io functionality
 * Note: NOT CALLED -- purely a compile time check of the mode namespace
 *
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

#include <w32config.h>

#include <win32_internal.h>

#if defined(HAVE_SYS_SOCKET_H) || defined(__MINGW32__)
#include <sys/socket.h>
#else
#error missing sys/socket.h
#endif

#if defined(HAVE_SYS_MOUNT_H) || defined(__MINGW32__)
#include <sys/mount.h>
#else
#error missing sys/mount.h
#endif

#if defined(HAVE_SYS_STATFS_H) || defined(__MINGW32__)
#include <sys/statfs.h>
#else
#error missing sys/statfs.h
#endif

#if defined(HAVE_SYS_STATVFS_H) || defined(__MINGW32__)
#include <sys/statvfs.h>
#else
#error missing sys/statvfs.h
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#error missing sys/time.h
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
#error missing unistd.h
#endif


/*
 *  check for unique mode bits ...
 */
int
__w32_check_attr(mode_t mode)
{
    switch (mode) {
#if defined(S_ISUID) && (S_ISUID)
    case S_ISUID:                               /* set user id on execution */
#endif
#if defined(S_ISGID) && (S_ISGID)
    case S_ISGID:                               /* set group id on execution */
#endif
#if defined(S_ISVTX) && (S_ISVTX)
    case S_ISVTX:                               /* sticky bit */
#endif
    case S_IRWXU:                               /* read, write, execute: owner */
    case S_IRUSR:                               /* read permission: owner */
    case S_IWUSR:                               /* write permission: owner */
    case S_IXUSR:                               /* execute permission: owner */
    case S_IRWXG:                               /* read, write, execute: group */
    case S_IRGRP:                               /* read permission: group */
    case S_IWGRP:                               /* write permission: group */
    case S_IXGRP:                               /* execute permission: group */
    case S_IRWXO:                               /* read, write, execute: other */
    case S_IROTH:                               /* read permission: other */
    case S_IWOTH:                               /* write permission: other */
    case S_IXOTH:                               /* execute permission: other */

    case S_IFDIR:                               /* directory */
    case S_IFCHR:                               /* character special */
    case S_IFIFO:                               /* pipe */
    case S_IFREG:                               /* regular */
    case S_IFLNK:                               /* link */

#if defined(S_IFBLK) && (S_IFBLK)
    case S_IFBLK:                               /* block device */
#endif
#if defined(S_IFSOCK) && (S_IFSOCK)
    case S_IFSOCK:                              /* socket */
#endif
#if defined(S_IFNAM) && (S_IFNAM)
    case S_IFNAM:                               /* special named files are widely used in QNX6 */
#endif
#if defined(S_IFDOOR) && (S_IFDOOR)
    case S_IFBLK:                               /* solaris special */
#endif
        return TRUE;
    }
    return FALSE;
}
/*end*/

