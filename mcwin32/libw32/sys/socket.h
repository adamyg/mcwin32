#ifndef LIBW32_SYS_SOCKET_H_INCLUDED
#define LIBW32_SYS_SOCKET_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_socket_h,"$Id: socket.h,v 1.15 2023/12/28 17:31:16 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <sys/socket.h>
 *
 * Copyright (c) 2012 - 2023, Adam Young.
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
 */

#if !defined(_WINSOCK2_H)                       /* MINGW32 guard */
#include <win32_include.h>                      /* winsock and windows.h guard */
#endif
#include <win32_errno.h>

#include <sys/cdefs.h>
#include <time.h>

__BEGIN_DECLS

LIBW32_VAR int          w32_h_errno;

struct pollfd;

struct msghdr {
    void               *msg_name;
    socklen_t           msg_namelen;
    struct iovec       *msg_iov;
    size_t              msg_iovlen;
    void               *msg_control;
    socklen_t           msg_controllen;
    int                 msg_flags;
};

LIBW32_API int          w32_sockinit(void);
LIBW32_API int          w32_getaddrinfo(const char *nodename, const char *servname,
                            const struct addrinfo *hints, struct addrinfo **res);
LIBW32_API int          w32_getnameinfo(const struct sockaddr *sa, socklen_t salen,
                            char *node, socklen_t nodelen, char * service, socklen_t servicelen, int flags);
LIBW32_API struct hostent *w32_gethostbyname(const char *host);
LIBW32_API int          w32_gethostname(char *name, size_t namelen);
LIBW32_API int          w32_getdomainname(char *name, size_t namelen);
LIBW32_API const char * w32_hstrerror(int herrno);
LIBW32_API void         w32_herror(const char *msg);

LIBW32_API int          w32_socket_fd(int af, int type, int protocol);
LIBW32_API int          w32_socket_native(int af, int type, int protocol);
LIBW32_API int          w32_connect_fd(int fd, const struct sockaddr *name, socklen_t namelen);
LIBW32_API int          w32_connect_native(int fd, const struct sockaddr *name, socklen_t namelen);
LIBW32_API int          w32_getsockopt_fd(int fd, int level, int optname, void *optval, int *optlen);
LIBW32_API int          w32_getsockopt_native(int fd, int level, int optname, void *optval, int *optlen);
LIBW32_API int          w32_setsockopt_fd(int fd, int level, int optname, const void *optval, int optlen);
LIBW32_API int          w32_setsockopt_native(int fd, int level, int optname, const void *optval, int optlen);
LIBW32_API int          w32_getpeername_fd(int fd, struct sockaddr *name, socklen_t *namelen);
LIBW32_API int          w32_getpeername_native(int fd, struct sockaddr *name, socklen_t *namelen);
LIBW32_API int          w32_getsockname_fd(int fd, struct sockaddr *name, socklen_t *namelen);
LIBW32_API int          w32_getsockname_native(int fd, struct sockaddr *name, socklen_t *namelen);
LIBW32_API int          w32_ioctlsocket_fd(int fd, long cmd, int *argp);
LIBW32_API int          w32_ioctlsocket_native(int fd, long cmd, int *argp);
LIBW32_API int          w32_bind_fd(int fd, const struct sockaddr *name, socklen_t namelen);
LIBW32_API int          w32_bind_native(int fd, const struct sockaddr *name, socklen_t namelen);
LIBW32_API int          w32_listen_fd(int fd, int num);
LIBW32_API int          w32_listen_native(int fd, int num);
LIBW32_API int          w32_accept_fd(int fd, struct sockaddr * addr, int * addrlen);
LIBW32_API int          w32_accept_native(int fd, struct sockaddr * addr, int * addrlen);
LIBW32_API int          w32_send_fd(int fd, const void *buf, size_t len, int flags);
LIBW32_API int          w32_send_native(int fd, const void *buf, size_t len, int flags);
LIBW32_API int          w32_sendto_fd(int fd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
LIBW32_API int          w32_sendto_native(int fd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
LIBW32_API int          w32_sendmsg_fd(int fd, const struct msghdr *message, int flags);
LIBW32_API int          w32_sendmsg_native(int fd, const struct msghdr *message, int flags);
LIBW32_API int          w32_recv_fd(int fd, char *buf, int len, int flags);
LIBW32_API int          w32_recv_native(int fd, char *buf, int len, int flags);
LIBW32_API int          w32_recvfrom_fd(int fd, char *buf, int len, int flags, struct sockaddr *from_addr, int *fromlen);
LIBW32_API int          w32_recvfrom_native(int fd, char *buf, int len, int flags, struct sockaddr *from_addr, int *fromlen);
LIBW32_API int          w32_shutdown_fd(int fd, int flags);
LIBW32_API int          w32_shutdown_native(int fd, int flags);

LIBW32_API int          w32_socknonblockingio_fd(int fd, int enabled);
LIBW32_API int          w32_socknonblockingio_native(int fd, int enabled);

LIBW32_API int          w32_sockinheritable_fd(int fd, int enabled);
LIBW32_API int          w32_sockinheritable_native(int fd, int enabled);

LIBW32_API int          w32_sockwrite_fd(int fd, const void *buffer, unsigned int cnt);
LIBW32_API int          w32_sockwrite_native(int fd, const void *buffer, unsigned int cnt);
LIBW32_API int          w32_sockread_fd(int fd, void *buf, unsigned int nbyte);
LIBW32_API int          w32_sockread_native(int fd, void *buf, unsigned int nbyte);
LIBW32_API int          w32_sockclose_fd(int fd);
LIBW32_API int          w32_sockclose_native(int fd);
LIBW32_API int          w32_shutdown_fd(int fd, int flags);
LIBW32_API int          w32_shutdown_native(int fd, int flags);

LIBW32_API int          w32_sockerror(void);
LIBW32_API int          w32_sockerrno_map(int nerrno);

LIBW32_API int          w32_socketpair_fd(int af, int type, int proto, int sock[2]);
LIBW32_API int          w32_socketpair_native(int af, int type, int proto, int sock[2]);

LIBW32_API int          w32_poll_fd(struct pollfd *fds, int cnt, int timeout);
LIBW32_API int          w32_poll_native(struct pollfd *fds, int cnt, int timeout);


/*
 *  Compile time bindings
 */
#if defined(LIBW32_SOCKET_MAP_FD)
#if !defined(WIN32_SOCKET_MAP_FD)
#define WIN32_SOCKET_MAP_FD 1
#endif
#endif
#if defined(LIBW32_SOCKET_MAP_NATIVE)
#if !defined(WIN32_SOCKET_MAP_NATIVE)
#define WIN32_SOCKET_MAP_NATIVE 1
#endif
#endif

#if defined(WIN32_SOCKET_MAP_FD) && defined(WIN32_SOCKET_MAP_NATIVE)
#error WIN32_SOCKET_MAP_FD and WIN32_SOCKET_MAP_NATIVE are mutually exclusive
#endif


#if defined(WIN32_SOCKET_MAP_FD) || defined(WIN32_SOCKET_MAP_NATIVE)
/*
 *  Socket interface, generic functions.
 */
#define HOST_NOT_FOUND          WSAHOST_NOT_FOUND
#define NO_AGAIN                WSANO_AGAIN
#define NO_RECOVERY             WSANO_RECOVERY
#define NO_DATA                 WSANO_DATA
#define NO_ADDRESSS             -1

#undef  h_errno
#define h_errno                 w32_h_errno

#ifndef gethostname     /*<unistd.h>*/
#define gethostname(a,b)        w32_gethostname(a,b)
#endif
#ifndef getdomainname   /*<unistd.h>*/
#define getdomainname(a,b)      w32_getdomainname(a,b)
#endif

#if !defined(_MSC_VER) || (_MSC_VER < 1400)
#define getaddrinfo(a,b,c,d)    w32_getaddrinfo(a,b,c,d)
#define getnameinfo(a,b,c,d,e,f,g) w32_getnameinfo(a,b,c,d,e,f,g)
#endif
#define gethostbyname(a)        w32_gethostbyname(a)
#define hstrerror(a)            w32_hstrerror(a)
#define herror(a)               w32_herror(a)

#endif  //WIN32_SOCKET_MAP || WIN32_SOCKET_MAP_NATIVE

#if defined(WIN32_SOCKET_MAP_FD)
/*
 *  Socket interface, file-descriptor.
 */
#define socket(a,b,c)           w32_socket_fd(a,b,c)
#define connect(a,b,c)          w32_connect_fd(a,b,c)
#define setsockopt(a,b,c,d,e)   w32_setsockopt_fd(a,b,c,d,e)
#define getsockopt(a,b,c,d,e)   w32_getsockopt_fd(a,b,c,d,e)
#define getpeername(a,b,c)      w32_getpeername_fd(a,b,c)
#define getsockname(a,b,c)      w32_getsockname_fd(a,b,c)
#define ioctlsocket(a,b,c)      w32_ioctlsocket_native(a,b,c)
#define bind(a,b,c)             w32_bind_fd(a,b,c)
#define listen(a,b)             w32_listen_fd(a,b)
#define accept(a,b,c)           w32_accept_fd(a,b,c)
#define poll(a,b,c)             w32_poll_fd(a,b,c)
#define send(a,b,c,d)           w32_send_fd(a,b,c,d)
#define sendto(a,b,c,d,e,f)     w32_sendto_fd(a,b,c,d,e,f)
#define sendmsg(a,b,c)          w32_sendmsg_fd(a,b,c)
#define recv(a,b,c,d)           w32_recv_fd(a,b,c,d)
#define recvfrom(a,b,c,d,e,f)   w32_recvfrom_fd(a,b,c,d,e,f)
#define shutdown(a,b)           w32_shutdown_fd(a,b)
#if !defined(LIBW32_SYS_POLL_H_INCLUDED)
#define poll(a,b,c)             w32_poll_fd(a,b,c)
#endif

#define socknonblockingio(a,b)  w32_socknonblockingio_fd(a,b)
#define sockinheritable(a,b)    w32_sockinheritable_fd(a,b)
#define sockread(a,b,c)         w32_sockread_fd(a,b,c)
#define sockwrite(a,b,c)        w32_sockwrite_fd(a,b,c)
#define sockclose(a)            w32_sockclose_fd(a)

#define socketpair(a,b,c,d)     w32_socketpair_fd(a,b,c,d)

#elif defined(WIN32_SOCKET_MAP_NATIVE)
/*
 *  Socket interface, native sockets.
 */
#define socket(a,b,c)           w32_socket_native(a,b,c)
#define connect(a,b,c)          w32_connect_native(a,b,c)
#define setsockopt(a,b,c,d,e)   w32_setsockopt_native(a,b,c,d,e)
#define getsockopt(a,b,c,d,e)   w32_getsockopt_native(a,b,c,d,e)
#define getpeername(a,b,c)      w32_getpeername_native(a,b,c)
#define getsockname(a,b,c)      w32_getsockname_native(a,b,c)
#define ioctlsocket(a,b,c)      w32_ioctlsocket_native(a,b,c)
#define bind(a,b,c)             w32_bind_native(a,b,c)
#define listen(a,b)             w32_listen_native(a,b)
#define accept(a,b,c)           w32_accept_native(a,b,c)
#define poll(a,b,c)             w32_poll_native(a,b,c)
#define send(a,b,c,d)           w32_send_native(a,b,c,d)
#define sendto(a,b,c,d,e,f)     w32_sendto_native(a,b,c,d,e,f)
#define sendmsg(a,b,c)          w32_sendmsg_native(a,b,c)
#define recv(a,b,c,d)           w32_recv_native(a,b,c,d)
#define recvfrom(a,b,c,d,e,f)   w32_recvfrom_native(a,b,c,d,e,f)
#define shutdown(a,b)           w32_shutdown_native(a,b)
#if !defined(LIBW32_SYS_POLL_H_INCLUDED)
#define poll(a,b,c)             w32_poll_native(a,b,c)
#endif /*SOCKET_MAPCALLS*/

#define socknonblockingio(a,b)  w32_socknonblockingio_native(a,b)
#define sockinheritable(a,b)    w32_sockinheritable_native(a,b)
#define sockread(a,b,c)         w32_sockread_native(a,b,c)
#define sockwrite(a,b,c)        w32_sockwrite_native(a,b,c)
#define sockclose(a)            w32_sockclose_native(a)

#define socketpair(a,b,c,d)     w32_socketpair_native(a,b,c,d)

#endif /*WIN32_SOCKET_MAP_FD|NATIVE*/

LIBW32_API int                  w32_select(int, fd_set *, fd_set *, fd_set *, const struct timeval *timeout);

__END_DECLS

/* missing definitions */

#if defined(_MSC_VER) || \
    defined(__MINGW64_VERSION_MAJOR) /* MingGW-w64/32 */
#include <Iphlpapi.h>                           /* if_nametoindex() */
#endif

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
INT WSAAPI inet_pton(INT Family, PCSTR pszAddrString, PVOID pAddrBuf);
PCSTR WSAAPI inet_ntop(INT Family, const VOID *pAddr, PSTR pStringBuf, size_t StringBufSize);

ULONG WINAPI if_nametoindex(PCSTR InterfaceName);

typedef struct addrinfo ADDRINFOA, *PADDRINFOA;
    //see: w32_sockbase.c
    //INT WSAAPI getaddrinfo(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA *pHints, PADDRINFOA *ppResult);
VOID WSAAPI freeaddrinfo(PADDRINFOA pAddrInfo);
#endif

#if defined(__WATCOMC__)
#if !defined(HAVE_TIMESPEC)                     /* missing definitions */
#define HAVE_TIMESPEC
#endif
#if !defined(_TIMESPEC_DEFINED) && (__WATCOMC__ < 1300)
#define _TIMESPEC_DEFINED                       /* OWC1.9=1290, OWC2.0=1300 */
struct timespec {
        time_t tv_sec;
        long tv_nsec;
};
#else
#include <signal.h>
#endif  /*TIMESPEC_STRUCT_T*/

ULONG WINAPI if_nametoindex(PCSTR InterfaceName);
#endif

#endif /*LIBW32_SYS_SOCKET_H_INCLUDED*/
