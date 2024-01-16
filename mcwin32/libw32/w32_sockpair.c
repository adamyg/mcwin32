#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_sockpair_c,"$Id: w32_sockpair.c,v 1.12 2023/11/06 15:07:42 cvsuser Exp $")

/*
 * win32 socket file-descriptor support
 *
 * Copyright (c) 2007, 2012 - 2023 Adam Young.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 * or (at your option) any later version.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==notice==
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0601              /* enable vista+ features (WSAPoll) */
#endif

#include "win32_include.h"
#include "win32_internal.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

/*
//  NAME
//
//      socketpair - create a pair of connected sockets
//
//  SYNOPSIS
//
//      #include <sys/socket.h>
//
//      int socketpair(int domain, int type, int protocol, int socket_vector[2]);
//
//  DESCRIPTION
//
//      The socketpair() function creates an unbound pair of connected sockets in a
//      specified domain, of a specified type, under the protocol optionally specified by
//      the protocol argument. The two sockets are identical. The file descriptors used in
//      referencing the created sockets are returned in socket_vector[0] and
//      socket_vector[1].
//
//      domain
//          Specifies the communications domain in which the sockets are to be created.
//
//      type
//          Specifies the type of sockets to be created.
//
//      protocol
//          Specifies a particular protocol to be used with the sockets. Specifying a
//          protocol of 0 causes socketpair() to use an unspecified default protocol
//          appropriate for the requested socket type.
//
//      socket_vector
//          Specifies a 2-integer array to hold the file descriptors of the created socket
//          pair.
//
//      The type argument specifies the socket type, which determines the semantics of
//      communications over the socket. The socket types supported by the system are
//      implementation-dependent. Possible socket types include:
//
//      SOCK_STREAM
//          Provides sequenced, reliable, bidirectional, connection-mode byte streams, and
//          may provide a transmission mechanism for out-of-band data.
//
//      SOCK_DGRAM
//          Provides datagrams, which are connectionless-mode, unreliable messages of fixed
//          maximum length.
//
//      SOCK_SEQPACKET
//          Provides sequenced, reliable, bidirectional, connection-mode transmission path
//          for records. A record can be sent using one or more output operations and
//          received using one or more input operations, but a single operation never
//          transfers part of more than one record. Record boundaries are visible to the
//          receiver via the MSG_EOR flag.
//
//      If the protocol argument is non-zero, it must specify a protocol that is supported
//      by the address family. The protocols supported by the system are
//      implementation-dependent.
//
//      The process may need to have appropriate privileges to use the socketpair()
//      function or to create some sockets.
//
//  RETURN VALUE
//
//      Upon successful completion, this function returns 0. Otherwise, -1 is returned and
//      errno is set to indicate the error.
//
//  ERRORS
//
//      The socketpair() function will fail if:
//
//      [EAFNOSUPPORT]
//          The implementation does not support the specified address family.
//
//      [EMFILE]
//          No more file descriptors are available for this process.
//
//      [ENFILE]
//          No more file descriptors are available for the system.
//
//      [EOPNOTSUPP]
//          The specified protocol does not permit creation of socket pairs.
//
//      [EPROTONOSUPPORT]
//          The protocol is not supported by the address family, or the protocol is not supported by the implementation.
//
//      [EPROTOTYPE]
//          The socket type is not supported by the protocol.
//
//      The socketpair() function may fail if:
//
//      [EACCES]
//          The process does not have appropriate privileges.
//
//      [ENOBUFS]
//          Insufficient resources were available in the system to perform the operation.
//
//      [ENOMEM]
//          Insufficient memory was available to fulfill the request.
//
//      [ENOSR]
//          There were insufficient STREAMS resources available for the operation to complete.
//
//  APPLICATION USAGE
//
//      The documentation for specific address families specifies which protocols each
//      address family supports. The documentation for specific protocols specifies which
//      socket types each protocol supports.
//
//      The socketpair() function is used primarily with UNIX domain sockets and need not
//      be supported for other domains.
*/

LIBW32_API int
w32_socketpair_fd(int af, int type, int proto, int sock[2])
{
    /*
     *  Note: Currently supports TCP/IPv4 socket pairs only.
     */
    int ret;

    if (0 == (ret = w32_socketpair_native(af, type, proto, sock))) {
        int s0 = -1, s1 = -1;

        if (sock[0] < WIN32_FILDES_MAX ||
                (s0 = _open_osfhandle((OSFHANDLE)sock[0], 0)) == -1 ||
            sock[1] < WIN32_FILDES_MAX ||
                (s1 = _open_osfhandle((OSFHANDLE)sock[1], 0)) == -1) {

            closesocket((SOCKET)sock[1]);
            if (s0 >= 0) _close(s0);
            else closesocket((SOCKET)sock[0]);
            errno = EMFILE;
            ret = -1;

        } else {
            w32_sockfd_open(s0, sock[0]);       /* associate file-descriptor */
            sock[0] = s0;

            w32_sockfd_open(s0, sock[1]);       /* associate file-descriptor */
            sock[1] = s1;
        }
    }
    return ret;
}


LIBW32_API int
w32_socketpair_native(int af, int type, int proto, int sock[2])
{
    /*
     *  Note: Currently supports TCP/IPv4 socket pairs only.
     */
    SOCKET listen_sock;
    SOCKADDR_IN addr1;
    SOCKADDR_IN addr2;
    int addr1_len = sizeof (addr1);
    int addr2_len = sizeof (addr2);
    int nerr;

    sock[0] = -1;
    sock[1] = -1;

    assert(af == AF_INET && type == SOCK_STREAM && (0 == proto || IPPROTO_IP == proto || IPPROTO_TCP == proto));

#undef socket
    if (0 == proto) proto = IPPROTO_IP;
    if ((listen_sock = socket(af, type, proto)) == INVALID_SOCKET)
        goto error;

    memset((void*)&addr1, 0, sizeof(addr1));
#if defined(__MINGW32__)
    addr1.sin_family = (short)af;
#else
    addr1.sin_family = (ADDRESS_FAMILY)af;
#endif
    addr1.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr1.sin_port = 0;

#undef bind
    if (bind(listen_sock, (SOCKADDR*)&addr1, addr1_len) == SOCKET_ERROR)
        goto error;

#undef getsockname
    if (getsockname(listen_sock, (SOCKADDR*)&addr1, &addr1_len) == SOCKET_ERROR)
        goto error;

#undef listen
    if (listen(listen_sock, 1))
        goto error;

    // note: safe to convert handles from 64 to 32; only lower 32-bits are used
    if ((sock[0] = (int)socket(af, type, proto)) == (int)INVALID_SOCKET)
        goto error;

#undef connect
    if (connect(sock[0], (SOCKADDR*)&addr1, addr1_len))
        goto error;

#undef accept
    if ((sock[1] = (int)accept(listen_sock, 0, 0)) == (int)INVALID_SOCKET)
        goto error;

#undef getpeername
    if (getpeername(sock[0], (SOCKADDR*)&addr1, &addr1_len) == SOCKET_ERROR)
        goto error;

#undef getsockname
    if (getsockname(sock[1], (SOCKADDR*)&addr2, &addr2_len) == SOCKET_ERROR)
        goto error;

    if (addr1_len != addr2_len
        || addr1.sin_addr.s_addr != addr2.sin_addr.s_addr
        || addr1.sin_port        != addr2.sin_port)
        goto error;

#undef closesocket
    closesocket(listen_sock);

    return 0;

error:
    nerr = WSAGetLastError();

    if (listen_sock != INVALID_SOCKET)
        closesocket(listen_sock);

    if (sock[0] != (int)INVALID_SOCKET)
        closesocket(sock[0]);

    if (sock[1] != (int)INVALID_SOCKET)
        closesocket(sock[1]);

    WSASetLastError(nerr);
    w32_sockerror();

    return -1; //SOCKET_ERROR
}

/*end*/
