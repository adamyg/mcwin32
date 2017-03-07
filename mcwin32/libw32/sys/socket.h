#ifndef WIN32_SYS_SOCKET_H_INCLUDED
#define WIN32_SYS_SOCKET_H_INCLUDED
/* -*- mode: c; tabs: 4 -*- */
/*
 * win32 <sys/socket.h>
 *
 * Copyright (c) 2012 - 2017, Adam Young.
 * All rights reserved.
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
 */

#include <win32_include.h>                      /* winsock and windows.h guard */

/*
 *  errno.h
 *
 *      Addition UNIX style errno's, plus Windows Sockets errors redefined
 *      as regular Berkeley error constants.  These are normally commented
 *      out in winsock.h in Windows NT to avoid conflicts with errno.h
 */
#include <unistd.h>                             /* for redef checks */

#if !defined(_MSC_VER) || (_MSC_VER < 1600)

#if !defined(EALREADY)
#define EALREADY        WSAEALREADY
#elif (EALREADY != WSAEALREADY)
#error  Inconsistent EALREADY definition ....
#endif

//#define ENOTINITIALISED WSANOTINITIALISED

#if !defined(EWOULDBLOCK)
#define EWOULDBLOCK     WSAEWOULDBLOCK
#elif (EWOULDBLOCK != WSAEWOULDBLOCK)
#error  Inconsistent EWOULDBLOCK definition ....
#endif

#if !defined(EINPROGRESS)
#define EINPROGRESS     WSAEINPROGRESS
#elif (EINPROGRESS != WSAEINPROGRESS)
#error  Inconsistent EINPROGRESS definition ....
#endif

#if !defined(ENOTSOCK)
#define ENOTSOCK        WSAENOTSOCK
#elif (ENOTSOCK != WSAENOTSOCK)
#error  Inconsistent ENOTSOCK definition ....
#endif

#if !defined(EDESTADDRREQ)
#define EDESTADDRREQ    WSAEDESTADDRREQ
#elif (EDESTADDRREQ != WSAEDESTADDRREQ)
#error  Inconsistent EDESTADDRREQ definition ....
#endif

#if !defined(EMSGSIZE)
#define EMSGSIZE        WSAEMSGSIZE
#elif (EMSGSIZE != WSAEMSGSIZE)
#error  Inconsistent EMSGSIZE definition ....
#endif

//      #define EPROTOTYPE      WSAEPROTOTYPE
//      #define ENOPROTOOPT     WSAENOPROTOOPT
//      #define EPROTONOSUPPORT WSAEPROTONOSUPPORT
//      #define ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT

#if !defined(EOPNOTSUPP)
#define EOPNOTSUPP      WSAEOPNOTSUPP
#elif (EOPNOTSUPP != WSAEOPNOTSUPP)
#error  Inconsistent EOPNOTSUPP definition ....
#endif

//      #define EPFNOSUPPORT    WSAEPFNOSUPPORT
//      #define EAFNOSUPPORT    WSAEAFNOSUPPORT
//      #define EADDRINUSE      WSAEADDRINUSE
//      #define EADDRNOTAVAIL   WSAEADDRNOTAVAIL
//      #define ENETDOWN        WSAENETDOWN

#if !defined(ENETUNREACH)
#define ENETUNREACH     WSAENETUNREACH
#elif (ENETUNREACH != WSAENETUNREACH)
#error  Inconsistent ENETUNREACH definition ....
#endif

//      #define ENETRESET       WSAENETRESET
//      #define ECONNABORTED    WSAECONNABORTED

#if !defined(ECONNRESET)
#define ECONNRESET      WSAECONNRESET
#elif (ECONNRESET != WSAECONNRESET)
#error  Inconsistent ECONNRESET definition ....
#endif

//      #define ENOBUFS         WSAENOBUFS
//      #define EISCONN         WSAEISCONN
//      #define ENOTCONN        WSAENOTCONN

#if !defined(ESHUTDOWN)
#define ESHUTDOWN       WSAESHUTDOWN
#elif (ESHUTDOWN != WSAESHUTDOWN)
#error  Inconsistent ESHUTDOWN definition ....
#endif

#if !defined(ETOOMANYREFS)
#define ETOOMANYREFS    WSAETOOMANYREFS
#elif (ETOOMANYREFS != WSAETOOMANYREFS)
#error  Inconsistent ETOOMANYREFS definition ...
#endif

#if !defined(ETIMEDOUT)
#define ETIMEDOUT       WSAETIMEDOUT
#elif (ETIMEDOUT != WSAETIMEDOUT)
#error  Inconsistent ETIMEDOUT definition ....
#endif

#if !defined(ECONNREFUSED)
#define ECONNREFUSED    WSAECONNREFUSED
#elif (ECONNREFUSED != WSAECONNREFUSED)
#error  Inconsistent ECONNREFUSED definition ...
#endif

#if !defined(ELOOP)
#define ELOOP           WSAELOOP
#elif (ELOOP != WSAELOOP)
#error  Inconsistent ELOOP definition ....
#endif

#if !defined(ENAMETOOLONG)
#define ENAMETOOLONG    WSAENAMETOOLONG
#endif

//      #define EHOSTDOWN       WSAEHOSTDOWN
//      #define EHOSTUNREACH    WSAEHOSTUNREACH

#if !defined(ENOTEMPTY)
#define ENOTEMPTY       WSAENOTEMPTY
#endif

//      #define EPROCLIM        WSAEPROCLIM
//      #define EUSERS          WSAEUSERS
//      #define EDQUOT          WSAEDQUOT
//      #define ESTALE          WSAESTALE
//      #define EREMOTE         WSAEREMOTE

#endif  /*1600*/

__BEGIN_DECLS

LIBW32_API extern int   w32_h_errno;

struct pollfd;

LIBW32_API int          w32_sockinit(void);
LIBW32_API int          w32_getaddrinfo(const char *nodename, const char *servname,
                            const struct addrinfo *hints, struct addrinfo **res);
LIBW32_API struct hostent *w32_gethostbyname(const char *host);
LIBW32_API int          w32_gethostname(char *name, size_t namelen);
LIBW32_API int          w32_getdomainname(char *name, size_t namelen);
LIBW32_API const char * w32_hstrerror(int herrno);
LIBW32_API void         w32_herror(const char *msg);
LIBW32_API int          w32_socket(int af, int type, int protocol);
LIBW32_API int          w32_connect(int fd, const struct sockaddr *name, int namelen);
LIBW32_API int          w32_getsockopt(int fd, int level, int optname, void *optval, int *optlen);
LIBW32_API int          w32_setsockopt(int fd, int level, int optname, const void *optval, int optlen);
LIBW32_API int          w32_getpeername(int fd, struct sockaddr *name, int *namelen);
LIBW32_API int          w32_getsockname(int fd, struct sockaddr *name, int *namelen);
LIBW32_API int          w32_ioctlsocket(int fd, long cmd, int *argp);
LIBW32_API int          w32_bind(int fd, const struct sockaddr *name, int namelen);
LIBW32_API int          w32_listen(int fd, int num);
LIBW32_API int          w32_accept(int fd, struct sockaddr * addr, int * addrlen);
#if !defined(GR_POLL_H_INCLUDED)
LIBW32_API int          w32_poll(struct pollfd *fds, int cnt, int timeout);
LIBW32_API int          w32_poll_native(struct pollfd *fds, int cnt, int timeout);
#endif
LIBW32_API int          w32_send(int fd, const void *buf, size_t len, int flags);
LIBW32_API int          w32_sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
LIBW32_API int          w32_recv(int fd, char *buf, int len, int flags);
LIBW32_API int          w32_recvfrom(int fd, char *buf, int len, int flags, struct sockaddr *from_addr, int *fromlen);
LIBW32_API int          w32_sockwrite(int fd, const void *buffer, unsigned int cnt);
LIBW32_API int          w32_sockread(int fd, void *buf, unsigned int nbyte);
LIBW32_API int          w32_shutdown(int fd, int flags);
LIBW32_API int          w32_sockerror(void);
LIBW32_API int          w32_sockerrno_map(int nerrno);

#if !defined(WIN32_SOCKET_H_CLEAN) && defined(WIN32_SOCKET_MAP)
#define HOST_NOT_FOUND          WSAHOST_NOT_FOUND
#define NO_AGAIN                WSANO_AGAIN
#define NO_RECOVERY             WSANO_RECOVERY
#define NO_DATA                 WSANO_DATA
#define NO_ADDRESSS             -1

#undef  h_errno
#define h_errno                 w32_h_errno

#ifndef gethostname //unistd.h
#define gethostname(a,b)        w32_gethostname(a,b)
#endif
#ifndef getdomainname //unistd.h
#define getdomainname(a,b)      w32_getdomainname(a,b)
#endif

#if !defined(_MSC_VER) || (_MSC_VER < 1400)
#define getaddrinfo(a,b,c,d)    w32_getaddrinfo(a,b,c,d)
#endif
#define gethostbyname(a)        w32_gethostbyname(a)
#define hstrerror(a)            w32_hstrerror(a)
#define herror(a)               w32_herror(a)

#define socket(a,b,c)           w32_socket(a,b,c)
#define connect(a,b,c)          w32_connect(a,b,c)
#define setsockopt(a,b,c,d,e)   w32_setsockopt(a,b,c,d,e)
#define getsockopt(a,b,c,d,e)   w32_getsockopt(a,b,c,d,e)
#define getpeername(a,b,c)      w32_getpeername(a,b,c)
#define getsockname(a,b,c)      w32_getsockname(a,b,c)
#define bind(a,b,c)             w32_bind(a,b,c)
#define listen(a,b)             w32_listen(a,b)
#define accept(a,b,c)           w32_accept(a,b,c)
#define poll(a,b,c)             w32_poll(a,b,c)
#define send(a,b,c,d)           w32_send(a,b,c,d)
#define sendto(a,b,c,d,e)       w32_sendto(a,b,c,d,e)
#define recv(a,b,c,d)           w32_recv(a,b,c,d)
#define recvfrom(a,b,c,d,e)     w32_sendto(a,b,c,d,e)
#define shutdown(a,b)           w32_shutdown(a,b)

#endif /*SOCKET_MAPCALLS*/

__END_DECLS

#endif /*SYS_SOCKET_H_INCLUDED*/

