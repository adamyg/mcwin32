/*
 * win32 socket () system calls
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

static void		sockclose(void);

static int x_sockinit = 0;                      /* initialisation status */
LIBW32_API int w32_h_errno = 0;                 /* lookup error */


/*
 *  w32_sockinit() system run-time initialisation.
 */
int
w32_sockinit(void)
{
    if (0 == x_sockinit) {
        WORD wVersionRequested;
        WSADATA wsaData;

        w32_sockfd_init();                      /* shadow file-descriptors */
        wVersionRequested = MAKEWORD(2, 2);     /* winsock2 */
        if (WSAStartup(wVersionRequested, &wsaData) != 0) {
            w32_sockerror();
            return -1;
        }
        x_sockinit = 1;
//      atexit(sockclose);
    }
    return 0;
}



static void
sockclose(void)
{
//  WSACleanup();
}



/*
 *  getaddrinfo() system/library call
 */
int
w32_getaddrinfo(const char *nodename, const char *servname,
        const struct addrinfo *hints, struct addrinfo **res)
{
    int done = 0, ret;

#undef getaddrinfo
retry:;
    if (0 != (ret = getaddrinfo(nodename, servname, hints, res))) {
        if (0 == done++) {
            if ((WSANOTINITIALISED == ret || (-1 == ret && WSANOTINITIALISED == WSAGetLastError())) &&
                    0 == w32_sockinit()) {
                goto retry;                     /* hide winsock initialisation */
            }
        }
        w32_sockerror();
    }
    return ret;
}


/*
 *  gethostbyname() system/library call
 */
struct hostent *
w32_gethostbyname(const char *host)
{
    struct hostent *hp;
    int done = 0;

#undef gethostbyname
retry:;
    if ((hp = gethostbyname(host)) != (struct hostent *)NULL) {
        w32_h_errno = 0;

    } else {
        const DWORD nerr = WSAGetLastError();
        if (0 == done++) {
            if (nerr == WSANOTINITIALISED && 0 == w32_sockinit()) {
                goto retry;                     /* hide winsock initialisation */
            }
        }
        w32_sockerror();
        w32_h_errno = nerr;                     /* lookup error */
    }
    return hp;
}


/*
 *  hstrerror() system/library call
 */
const char *
w32_hstrerror(int herrno)
{
    switch(herrno) {
    case 0:
        return "no error";
    case WSAHOST_NOT_FOUND:
        return "host not found";
    case WSATRY_AGAIN:
        return "try again";
    case WSANO_RECOVERY:
        return "no recovery";
    case WSANO_DATA:
        return "no data";
    }
    return "no address";
}


/*
 *  herror() system/library call
 */
void
w32_herror(const char *msg)
{
    fprintf(stderr, "%s: %s\n",                 /*XXX*/
        msg, w32_hstrerror(w32_h_errno));
}


/*
 *  socket() system call
 */
int
w32_socket(int af, int type, int protocol)
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
w32_connect(int fd, const struct sockaddr *name, int namelen)
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
w32_bind(int fd, const struct sockaddr *name, int namelen)
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
    return (ret);
}


/*
 *  getsockopt() system call
 */
int
w32_getsockopt(int fd, int level, int optname, void *optval, int *optlen)
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
    return (ret);
}


/*
 *  setsockopt() system call
 */
int
w32_setsockopt(
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
    return (ret);
}


/*
 *  listen() system call
 */
int
w32_listen(int fd, int num)
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
    return (ret);
}


/*
 *  accept() system call
 */
int
w32_accept(int fd, struct sockaddr *addr, int *addrlen)
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
        } else {
            /*
             *  WINNT has a misfeature that sockets are inherited
             *  by child processes by default, so disable.
             */
            SetHandleInformation((HANDLE)s, HANDLE_FLAG_INHERIT, 0);
            w32_sockfd_open(ret, s); /*associate file-descriptor */
        }
    }
    return (ret);
}


/*
 *  getpeername() system call
 */
int
w32_getpeername(int fd, struct sockaddr *name, int *namelen)
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
w32_getsockname(int fd, struct sockaddr *name, int *namelen)
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
w32_ioctlsocket(int fd, long cmd, int *argp)
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
 *  poll() system call
 */
int
w32_poll(struct pollfd *fds, int cnt, int timeout)
{
    //  TODO -- dynamically load
    //      WINSOCK_API_LINKAGE int WSAAPI WSAPoll(LPWSAPOLLFD, ULONG, INT);
    //          ==> source: ws2_32.lib
    //
    struct timeval tmval;
    struct fd_set rfds;
    struct fd_set wfds;
    struct fd_set efds;
    SOCKET s[ FD_SETSIZE ];
    int wcnt;
    int ret;
    int i;

    if (cnt <= 0 || cnt > FD_SETSIZE) {
        errno = EINVAL;
        return (-1);
    }

    /* Build fd set */
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);
    wcnt = 0;

    for (i = 0; i < cnt; ++i)
        fds[i].revents = 0;

    for (i = 0; i < cnt; ++i) {
        SOCKET osf;

        osf = w32_sockhandle((int)fds[i].fd);
        if (osf == (SOCKET)INVALID_SOCKET) {
            fds[i].revents = POLLNVAL;
            return (1);
        }

        s[i] = (SOCKET) osf;
        FD_SET(s[i], &rfds);
        if (fds[i].events & POLLOUT) {
            FD_SET(s[i], &wfds);
            wcnt++;
        }
        FD_SET(s[i], &efds);
    }

    /* Select */
    if (timeout >= 0) {
        tmval.tv_sec = timeout / 1000;
        tmval.tv_usec = (timeout % 1000) * 1000;
    }

    ret = select(cnt, &rfds, (wcnt ? &wfds : NULL), &efds, (timeout == -1 ? NULL : &tmval));

    /* Update pollfd's */
    if (ret == SOCKET_ERROR) {
        DWORD nerr;

        nerr = WSAGetLastError();
        w32_sockerror();
        ret = -1;

        switch(nerr) {
        case WSANOTINITIALISED:     /* stack not initialisated */
            break;
        case WSAEFAULT:             /* invalid address */
            break;
        case WSAENETDOWN:           /* network shutdown */
            break;
        case WSAEINTR:              /* interrupted system call */
            break;
        case WSAENOTSOCK: {         /* invalid socket(s) */
                int len, type;

                ret = 0;
                for (i = 0; i < cnt; ++i) {
                                    /* mark bad sockets */
                    len = sizeof(type);
                    if (getsockopt(s[i], SOL_SOCKET, SO_TYPE, (char *)&type, &len) == SOCKET_ERROR) {
                        fds[i].revents = POLLNVAL;
                        ret++;
                    } else {
                        fds[i].revents = 0;
                    }
                }
            }
            break;

        case WSAEINPROGRESS:        /* shouldn't happen */
        case WSAEINVAL:
        default:                    /* misc */
            break;
        }

    } else if (ret >= 0) {          /* success, decode select return */
        /*
         -  A socket will be identified in a particular set
         -  when select returns if:
         -
         -  rfds:
         -      If listen has been called and a connection is pending, accept
         -      will succeed.
         -
         -      Data is available for reading (includes OOB data if
         -      SO_OOBINLINE is enabled).
         -
         -      Connection has been closed/reset/terminated.
         -
         -  wfds:
         -      If processing a connect call (nonblocking), connection has
         -      succeeded.
         -
         -      Data can be sent.
         -
         -  exceptfds:
         -      If processing a connect call (nonblocking), connection attempt
         -      failed.
         -
         -      OOB data is available for reading (only if SO_OOBINLINE is disabled).
        */
        BOOL    state;
        u_long  val;
        int     len;

        for (i = 0; i < cnt; ++i) {
            fds[i].revents = 0;

            if (FD_ISSET(s[i], &rfds)) {
                len = sizeof(state);            /* listening ? */
                state = 0;

                if ( getsockopt(s[i], SOL_SOCKET, SO_ACCEPTCONN, (char *)&state, &len) != 0 && 
                            WSAGetLastError() != WSAENOPROTOOPT ) {
                    fds[i].revents |= POLLERR;

                } else if (state) {
                    fds[i].revents |= POLLIN;   /* accept possible */

                } else if (ioctlsocket(s[i], FIONREAD, &val) != 0) {
                    fds[i].revents |= POLLERR;  /* hmmm */

                } else if (val > 0) {
                    fds[i].revents |= POLLIN;   /* read possible */

                } else {
                    fds[i].revents |= POLLHUP;  /* socket shutdown */
                }
            }

                                                /* write possible */
            if ((fds[i].events & POLLOUT) && FD_ISSET(s[i], &wfds)) {
                fds[i].revents |= POLLOUT;
            }

            if (FD_ISSET(s[i], &efds)) {        /* OOB data */
                fds[i].revents |= POLLPRI;
            }
        }
    }

    return ret;
}


/*
 *  poll() system call, using native WinSock sockets.
 */
int
w32_poll_native(struct pollfd *fds, int cnt, int timeout)
{
    //  TODO -- dynamically load
    //      WINSOCK_API_LINKAGE int WSAAPI WSAPoll(LPWSAPOLLFD, ULONG, INT);
    //          ==> source: ws2_32.lib
    //
    struct timeval tmval;
    struct fd_set rfds;
    struct fd_set wfds;
    struct fd_set efds;
    SOCKET s[ FD_SETSIZE ];
    int wcnt;
    int ret;
    int i;

    if (cnt <= 0 || cnt > FD_SETSIZE) {
        errno = EINVAL;
        return (-1);
    }

    /* Build fd set */
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);
    wcnt = 0;

    for (i = 0; i < cnt; ++i)
        fds[i].revents = 0;

    for (i = 0; i < cnt; ++i) {
        s[i] = (SOCKET) fds[i].fd; //native!
        FD_SET(s[i], &rfds);
        if (fds[i].events & POLLOUT) {
            FD_SET(s[i], &wfds);
            wcnt++;
        }
        FD_SET(s[i], &efds);
    }

    /* Select */
    if (timeout >= 0) {
        tmval.tv_sec = timeout / 1000;
        tmval.tv_usec = (timeout % 1000) * 1000;
    }

    ret = select(cnt, &rfds, (wcnt ? &wfds : NULL), &efds, (timeout == -1 ? NULL : &tmval));

    /* Update pollfd's */
    if (ret == SOCKET_ERROR) {
        DWORD nerr;

        nerr = WSAGetLastError();
        w32_sockerror();
        ret = -1;

        switch(nerr) {
        case WSANOTINITIALISED:     /* stack not initialisated */
            break;
        case WSAEFAULT:             /* invalid address */
            break;
        case WSAENETDOWN:           /* network shutdown */
            break;
        case WSAEINTR:              /* interrupted system call */
            break;
        case WSAENOTSOCK: {         /* invalid socket(s) */
                int len, type;

                ret = 0;
                for (i = 0; i < cnt; ++i) {
                                    /* mark bad sockets */
                    len = sizeof(type);
                    if (getsockopt(s[i], SOL_SOCKET, SO_TYPE, (char *)&type, &len) == SOCKET_ERROR) {
                        fds[i].revents = POLLNVAL;
                        ret++;
                    } else {
                        fds[i].revents = 0;
                    }
                }
            }
            break;

        case WSAEINPROGRESS:        /* shouldn't happen */
        case WSAEINVAL:
        default:                    /* misc */
            break;
        }

    } else if (ret >= 0) {          /* success, decode select return */
       BOOL    state;
        u_long  val;
        int     len;

        for (i = 0; i < cnt; ++i) {
            fds[i].revents = 0;

            if (FD_ISSET(s[i], &rfds)) {
                len = sizeof(state);            /* listening ? */
                state = 0;

                if (getsockopt(s[i], SOL_SOCKET, SO_ACCEPTCONN, (char *)&state, &len) != 0 &&
                            WSAGetLastError() != WSAENOPROTOOPT) {
                    fds[i].revents |= POLLERR;

                } else if (state) {
                    fds[i].revents |= POLLIN;   /* accept possible */

                } else if (ioctlsocket(s[i], FIONREAD, &val) != 0) {
                    fds[i].revents |= POLLERR;  /* hmmm */

                } else if (val > 0) {
                    fds[i].revents |= POLLIN;   /* read possible */

                } else {
                    fds[i].revents |= POLLHUP;  /* socket shutdown */
                }
            }

                                                /* write possible */
            if ((fds[i].events & POLLOUT) && FD_ISSET(s[i], &wfds)) {
                fds[i].revents |= POLLOUT;
            }

            if (FD_ISSET(s[i], &efds)) {        /* OOB data */
                fds[i].revents |= POLLPRI;
            }
        }
    }
    return ret;
}


/*
 *  send() system call
 */
int
w32_send(int fd, const void *buf, size_t len, int flags)
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
w32_sendto(int fd, const void *buf, size_t len, int flags,
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
w32_recv(int fd, char *buf, int len, int flags)
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
w32_recvfrom(int fd, char *buf, int len, int flags,
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
w32_sockwrite(int fd, const void *buffer, unsigned int cnt)
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
w32_sockread(int fd, void *buf, unsigned int nbyte)
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
w32_shutdown(int fd, int how)
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


/*
 *  export the last socket error. 
 */
int
w32_sockerror(void)
{
    int nerrno = WSAGetLastError();             /* last network error */

    /*
     *  must map a few errno's as the MSVC errno namespaces isn't clean.
     */
    switch( errno ) {
    case WSAEINTR: nerrno = EINTR; break;
    case WSAEBADF: nerrno = EBADF; break;
    case WSAEACCES: nerrno = EACCES; break;
    case WSAEFAULT: nerrno = EFAULT; break;
    case WSAEINVAL: nerrno = EINVAL; break;
    case WSAEMFILE: nerrno = EMFILE; break;
    case WSAENAMETOOLONG: nerrno = ENAMETOOLONG; break;
    case WSAENOTEMPTY: nerrno = ENOTEMPTY; break;
    default: break;
    }
    errno = nerrno;
    return nerrno;
}
/*end*/

