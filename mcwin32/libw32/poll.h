#ifndef LIBW32_POLL_H_INCLUDED
#define LIBW32_POLL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_poll_h,"$Id: poll.h,v 1.14 2025/03/06 17:15:29 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win <poll.h>
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
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
        defined(__WATCOMC__) || \
        (defined(__MINGW32__) && (_WIN32_WINNT < 0x600 || !defined(__MINGW64_VERSION_MAJOR)))
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
typedef struct pollfd {
    SOCKET  fd;
    SHORT   events;
    SHORT   revents;
} WSAPOLLFD, *PWSAPOLLFD, *LPWSAPOLLFD;

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

#if defined(__MINGW32__)                /* missing definitions */
__BEGIN_DECLS
int WSAAPI WSAPoll(LPWSAPOLLFD fdArray, ULONG fds, INT timeout);
__END_DECLS
#endif

#endif /*_MSC_VER || ___WATCOMC__ || _MINGW32__*/
#endif /*POLLING*/

__BEGIN_DECLS

LIBW32_API int          w32_poll_fd(struct pollfd *fds, int cnt, int timeout);
LIBW32_API int          w32_poll_native(struct pollfd *fds, int cnt, int timeout);

#if defined(LIBW32_SOCKET_MAP_FD) || defined(WIN32_SOCKET_MAP_FD)
#if !defined(WIN32_SOCKET_H_INCLUDED)
#define poll(a,b,c)             w32_poll_fd(a,b,c)
#endif

#elif defined(LIBW32_SOCKET_MAP_NATIVE) || defined(WIN32_SOCKET_MAP_NATIVE)
#if !defined(WIN32_SOCKET_H_INCLUDED)
#define poll(a,b,c)             w32_poll_native(a,b,c)
#endif
#endif /*WIN32_SOCKET_MAP_FD|NATIVE*/

__END_DECLS

#endif /*LIBW32_POLL_H_INCLUDED*/
