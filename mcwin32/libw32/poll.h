#ifndef LIBW32_POLL_H_INCLUDED
#define LIBW32_POLL_H_INCLUDED

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win <poll.h>
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
	defined(__MINGW32__)
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
#endif /*_MSC_VER || __MINGW32__*/
#endif /*POLLING*/

__BEGIN_DECLS

int                     w32_poll(struct pollfd *fds, int cnt, int timeout);

__END_DECLS

#endif /*LIBW32_POLL_H_INCLUDED*/
