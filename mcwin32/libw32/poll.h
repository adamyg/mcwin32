#ifndef LIBW32_POLL_H_INCLUDED
#define LIBW32_POLL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_poll_h,"$Id: poll.h,v 1.7 2022/02/17 16:04:59 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win <poll.h>
 *
 * Copyright (c) 2007, 2012 - 2022 Adam Young.
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
 * ==end==
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0600              /* enable vista+ features (WSAPoll) */
#endif

#include <win32_include.h>
#include <sys/cdefs.h>

struct w32_pollfd {
    int                 fd;
    short               events;
    short               revents;
};


#if !defined(POLLIN)
#if (defined(_MSC_VER) && (_MSC_VER >= 1400)) || \
        defined(__MINGW32__) || defined(__WATCOMC__)
/*
 *  POLLRDNORM          Data on priority band 0 may be read.
 *  POLLRDBAND          Data on priority bands greater than 0 may be read.
 *  POLLIN              Same effect as POLLRDNORM | POLLRDBAND.
 *  POLLPRI             High priority data may be read.
 *  POLLOUT             Same value as POLLWRNORM.
 *  POLLWRNORM          Data on priority band 0 may be written.
 *  POLLWRBAND          Data on priority bands greater than 0 may be
 *                      written. This event only examines bands that have
 *                      been written to at least once.
 *  POLLERR             An error has occurred (revents only).
 *  POLLHUP             Device has been disconnected (revents only).
 *  POLLNVAL            Invalid fd member (revents only).
 */
struct pollfd {
    SOCKET              fd;
    SHORT               events;
    SHORT               revents;
};

#define POLLRDNORM      0x0100
#define POLLRDBAND      0x0200
#define POLLIN          (POLLRDNORM | POLLRDBAND)
#define POLLPRI         0x0400
#define POLLWRNORM      0x0010
#define POLLOUT         POLLWRNORM
#define POLLWRBAND      0x0020
#define POLLERR         0x0001
#define POLLHUP         0x0002
#define POLLNVAL        0x0004
#endif /*_MSC_VER || __MINGW32__ || __WATCOMC__ */
#endif /*POLLING*/

__BEGIN_DECLS

LIBW32_API int          w32_poll_fd(struct pollfd *fds, int cnt, int timeout);
LIBW32_API int          w32_poll_native(struct pollfd *fds, int cnt, int timeout);

#if defined(WIN32_SOCKET_MAP_FD)
#if !defined(WIN32_SOCKET_H_INCLUDED)
#define poll(a,b,c)             w32_poll_fd(a,b,c)
#endif

#elif defined(WIN32_SOCKET_MAP_NATIVE)
#if !defined(WIN32_SOCKET_H_INCLUDED)
#define poll(a,b,c)             w32_poll_native(a,b,c)
#endif
#endif /*WIN32_SOCKET_MAP_FD|NATIVE*/

__END_DECLS

#endif /*LIBW32_POLL_H_INCLUDED*/
