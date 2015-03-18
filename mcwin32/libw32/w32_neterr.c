/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 network errno mapping support
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

#include "win32_internal.h"
#include <unistd.h>
#include <time.h>


int
w32_neterrno_set(void)
{
    int t_errno = WSAGetLastError();            /* last network error */

    switch (t_errno) {
    /*
     *  must map a few errno's as the errno namespaces isn't clean.
     */
    case WSAEINTR: t_errno = EINTR; break;
    case WSAEBADF: t_errno = EBADF; break;
    case WSAEACCES: t_errno = EACCES; break;
    case WSAEFAULT: t_errno = EFAULT; break;
    case WSAEINVAL: t_errno = EINVAL; break;
    case WSAEMFILE: t_errno = EMFILE; break;
    case WSAENAMETOOLONG: t_errno = ENAMETOOLONG; break;
    case WSAENOTEMPTY: t_errno = ENOTEMPTY; break;
    default:
        break;
    }
    errno = t_errno;
    return t_errno;
}
