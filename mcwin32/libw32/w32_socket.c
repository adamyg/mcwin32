/*
 * win32 socket () system calls
 *
 * Copyright (c) 2007, 2012 - 2018 Adam Young.
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
#define _WIN32_WINNT        0x0601              /* enable vista+ features (WSAPoll) */
#endif

#include "win32_include.h"
#include "win32_internal.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#pragma comment(lib, "Ws2_32.lib")          

/*
 *  socket() system call
 */
int
w32_socket_fd(int af, int type, int protocol)
{
    int done = 0, ret;
    SOCKET s;

#undef socket
retry:;
    if ((s = socket(af, type, protocol)) == INVALID_SOCKET) {
        if (0 == done++) {
            if (WSAGetLastError() == WSANOTINITIALISED && 0 == w32_sockinit()) {
                goto retry;                     /* hide winsock initialisation */
            }
        }
        w32_sockerror();
        ret = -1;
    } else if ((ret = (int)s) < WIN32_FILDES_MAX &&
                    (ret = _open_osfhandle((long)s, 0)) == -1) {
        closesocket(s);
        errno = EMFILE;
    } else {
        SetHandleInformation((HANDLE)s, HANDLE_FLAG_INHERIT, 0);
        w32_sockfd_open(ret, s);                /* associate file-descriptor */
    }
    return ret;
}


/*
 *  connect() system call
 */
int
w32_connect_fd(int fd, const struct sockaddr *name, socklen_t namelen)
{
    SOCKET osf;
    int ret = 0;

#undef connect
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if (connect((SOCKET)osf, name, namelen) != 0) {
        w32_sockerror();
        ret = -1;
    }
    return ret;
}


/*
 *  bind() system call
 */
int
w32_bind_fd(int fd, const struct sockaddr *name, socklen_t namelen)
{
    SOCKET osf;
    int ret = 0;

#undef bind
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if (bind((SOCKET)osf, name, namelen) != 0) {
        w32_sockerror();
        ret = -1;
    }
    return ret;
}


/*
 *  getsockopt() system call
 */
int
w32_getsockopt_fd(int fd, int level, int optname, void *optval, int *optlen)
{
    SOCKET osf;
    int ret = 0;

#undef getsockopt
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if (getsockopt((SOCKET)osf, level, optname, optval, optlen) != 0) {
        w32_sockerror();
        ret = -1;
    }
    return ret;
}


/*
 *  setsockopt() system call
 */
int
w32_setsockopt_fd(
    int fd, int level, int optname, const void *optval, int optlen )
{
    SOCKET osf;
    int ret = 0;

#undef setsockopt
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if (setsockopt((SOCKET)osf, level, optname, optval, optlen) != 0) {
        w32_sockerror();
        ret = -1;
    }
    return ret;
}


/*
 *  listen() system call
 */
int
w32_listen_fd(int fd, int num)
{
    SOCKET osf;
    int ret = 0;

#undef listen
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if (listen((SOCKET)osf, num) != 0) {
        w32_sockerror();
        ret = -1;
    }
    return ret;
}


/*
 *  accept() system call
 */
int
w32_accept_fd(int fd, struct sockaddr *addr, int *addrlen)
{
    SOCKET osf;
    int ret = 0;

#undef accept
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else {
        SOCKET s;

        if ((s = accept((SOCKET)osf, addr, addrlen)) == INVALID_SOCKET) {
            w32_sockerror();
            ret = -1;
        } else if ((ret = (int)s) < WIN32_FILDES_MAX && 
                         (ret = _open_osfhandle((long)s, 0)) == -1) {
            (void) closesocket(s);
            errno = EMFILE;
        } else {
            /*
             *  WINNT has a misfeature that sockets are inherited
             *  by child processes by default, so disable.
             */
            SetHandleInformation((HANDLE)s, HANDLE_FLAG_INHERIT, 0);
            w32_sockfd_open(ret, s); /*associate file-descriptor */
        }
    }
    return ret;
}


/*
 *  getpeername() system call
 */
int
w32_getpeername_fd(int fd, struct sockaddr *name, socklen_t *namelen)
{
    SOCKET osf;
    int ret;

#undef getpeername
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if ((ret = getpeername((SOCKET)osf, name, namelen)) == -1 /*SOCKET_ERROR*/) {
        w32_sockerror();
    }
    return ret;
}


/*
 *  getsockname() system call.
 */
int
w32_getsockname_fd(int fd, struct sockaddr *name, socklen_t *namelen)
{
    SOCKET osf;
    int ret;

#undef getsockname
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if ((ret = getsockname((SOCKET)osf, name, namelen)) == -1 /*SOCKET_ERROR*/) {
        w32_sockerror();
    }
    return ret;
}


/*
 *  ioctl() system call; aka read() for sockets.
 */
int
w32_ioctlsocket_fd(int fd, long cmd, int *argp)
{
    SOCKET osf;
    int ret;

#undef ioctlsocket
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else {
        u_long t_arg = (u_long)*argp;
        if ((ret = ioctlsocket((SOCKET)osf, cmd, &t_arg)) == -1 /*SOCKET_ERROR*/) {
            w32_sockerror();
        } else {
            *argp = (int)t_arg;
        }
    }
    return ret;
}


/*
 *  send() system call
 */
int
w32_send_fd(int fd, const void *buf, size_t len, int flags)
{
    SOCKET osf;
    int ret;

#undef send
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if ((ret = send((SOCKET)osf, buf, (int)len, flags)) == -1 /*SOCKET_ERROR*/) {
        w32_sockerror();
    }
    return ret;
}


/*
 *  sendto() system call
 */
int
w32_sendto_fd(int fd, const void *buf, size_t len, int flags,
        const struct sockaddr *dest_addr, socklen_t addrlen)
{
    SOCKET osf;
    int ret;

#undef sendto
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if ((ret = sendto((SOCKET)osf, buf, (int)len, flags, dest_addr, addrlen)) == -1 /*SOCKET_ERROR*/) {
        w32_sockerror();
    }
    return ret;
}


/*
 *  recv() system call
 */
int
w32_recv_fd(int fd, char *buf, int len, int flags)
{
    SOCKET osf;
    int ret;

#undef recv
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if ((ret = recv((SOCKET)osf, buf, len, flags)) == -1 /*SOCKET_ERROR*/) {
        w32_sockerror();
    }
    return ret;
}


/*
 *  recvfrom() system call
 */
int
w32_recvfrom_fd(int fd, char *buf, int len, int flags,
        struct sockaddr *from_addr, int *fromlen)
{
    SOCKET osf;
    int ret;

#undef recvfrom
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if ((ret = recvfrom((SOCKET)osf, buf, len, flags, from_addr, fromlen)) == -1 /*SOCKET_ERROR*/) {
        w32_sockerror();
    }
    return ret;
}


/*
 *  sockwrite() system call; aka write() for sockets.
 */
int
w32_sockwrite_fd(int fd, const void *buffer, unsigned int cnt)
{
    SOCKET osf;
    int ret;

#undef sendto
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if ((ret = sendto(osf, buffer, cnt, 0, NULL, 0)) == -1 /*SOCKET_ERROR*/) {
        if (w32_sockerror() == ENOTSOCK) {
            ret = _write(fd, buffer, cnt);
        }      
    }
    return ret;
}


/*
 *  sockwrite() system call; aka read() for sockets.
 */
int
w32_sockread_fd(int fd, void *buf, unsigned int nbyte)
{
    SOCKET osf;
    int ret;

#undef recvfrom
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if ((ret = recvfrom(osf, buf, nbyte, 0, NULL, 0)) == -1 /*SOCKET_ERROR*/) {
        if (w32_sockerror() == ENOTSOCK) {
            ret = _read(fd, buf, nbyte);
        }
    }
    return ret;
}


/*
 *  shutdown() system call
 */
int
w32_shutdown_fd(int fd, int how)
{
    SOCKET osf;
    int ret;

#undef shutdown
    if ((osf = w32_sockhandle(fd)) == (SOCKET)INVALID_SOCKET) {
        ret = -1;
    } else if ((ret = shutdown((SOCKET)osf, how)) == -1) {
        w32_sockerror();
    }
    return ret;
}


/*
 *  determine whether a valid socket file descriptor. 
 */
SOCKET
w32_sockhandle(int fd)
{
    SOCKET ret;
    if ((ret = w32_sockfd_get(fd)) == INVALID_SOCKET) {
        errno = EBADF;
    }
    return ret;
}

/*end*/