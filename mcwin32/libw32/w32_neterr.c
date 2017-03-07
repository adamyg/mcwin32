/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 network errno mapping support
 *
 * Copyright (c) 2007, 2012 - 2017 Adam Young.
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

 /*
  *	winsock error mapping to unix error.
  */
int
w32_neterrno_map(int nerrno)
{
    if (nerrno < WSABASEERR) return nerrno;
	/*
	 *	map a few basic winsock file i / o errno's to their unix equivalent.
	 */
    switch (errno) {
    case WSAEINTR:          nerrno = EINTR; break;
    case WSAEBADF:          nerrno = EBADF; break;
    case WSAEACCES:         nerrno = EACCES; break;
    case WSAEFAULT:         nerrno = EFAULT; break;
    case WSAEINVAL:         nerrno = EINVAL; break;
    case WSAEMFILE:         nerrno = EMFILE; break;
    case WSAENAMETOOLONG:   nerrno = ENAMETOOLONG; break;
    case WSAENOTEMPTY:		nerrno = ENOTEMPTY; break;
    default:
        break;
    }
    return nerrno;
}


/*
 *	last network error to errno
 */
int
w32_neterrno_set(void)
{
	int t_errno = w32_neterrno_map(WSAGetLastError());
    errno = t_errno;
    return t_errno;
}
