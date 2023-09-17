#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_neterr_c,"$Id: w32_neterr.c,v 1.9 2023/09/17 13:04:58 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 network errno mapping support
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
 * ==extra==
 */

#include "win32_internal.h"
#include <unistd.h>
#include <time.h>

/*
 *     winsock error mapping to unix error.
 */
LIBW32_API int
w32_neterrno_map(int nerrno)
{
    if (nerrno < WSABASEERR) return nerrno;

    /*
     *  map a few basic winsock file i / o errno's to their unix equivalent.
     */
    switch (errno) {
    case WSAEINTR:              nerrno = EINTR; break;
    case WSAEBADF:              nerrno = EBADF; break;
    case WSAEACCES:             nerrno = EACCES; break;
    case WSAEFAULT:             nerrno = EFAULT; break;
    case WSAEINVAL:             nerrno = EINVAL; break;
    case WSAEMFILE:             nerrno = EMFILE; break;
    case WSAENAMETOOLONG:       nerrno = ENAMETOOLONG; break;
    case WSAENOTEMPTY:          nerrno = ENOTEMPTY; break;
    default:
        break;
    }
    return nerrno;
}


/*
 *      last network error to errno
 */
LIBW32_API int
w32_neterrno_set(void)
{
    const int nerrno = w32_neterrno_map((int)WSAGetLastError());
    errno = nerrno;
    return nerrno;
}


#if defined(DEFUNCT)           /* constants are no longer within a seperate namespace */
/*
 *  winsock error mapping to unix
 */
LIBW32_API int
w32_sockerrno_map(int nerrno)
{
    if (nerrno < WSABASEERR) return nerrno;
    switch (nerrno) {
    case WSAEINTR:              nerrno = EINTR; break;              // 10004L
    case WSAEBADF:              nerrno = EBADF; break;              // 10009L
    case WSAEACCES:             nerrno = EACCES; break;             // 10013L
    case WSAEFAULT:             nerrno = EFAULT; break;             // 10014L
    case WSAEINVAL:             nerrno = EINVAL; break;             // 10022L
    case WSAEMFILE:             nerrno = EMFILE; break;             // 10024L
    case WSAEWOULDBLOCK:        nerrno = EWOULDBLOCK; break;        // 10035L
    case WSAEINPROGRESS:        nerrno = EINPROGRESS; break;        // 10036L
    case WSAEALREADY:           nerrno = EALREADY; break;           // 10037L
    case WSAENOTSOCK:           nerrno = ENOTSOCK; break;           // 10038L
    case WSAEDESTADDRREQ:       nerrno = EDESTADDRREQ; break;       // 10039L
    case WSAEMSGSIZE:           nerrno = EMSGSIZE; break;           // 10040L
    case WSAEPROTOTYPE:         nerrno = EPROTOTYPE; break;         // 10041L
    case WSAENOPROTOOPT:        nerrno = ENOPROTOOPT; break;        // 10042L
    case WSAEPROTONOSUPPORT:    nerrno = EPROTONOSUPPORT; break;    // 10043L
    case WSAESOCKTNOSUPPORT:    nerrno = ESOCKTNOSUPPORT; break;    // 10044L
    case WSAEOPNOTSUPP:         nerrno = EOPNOTSUPP; break;         // 10045L
    case WSAEPFNOSUPPORT:       nerrno = EPFNOSUPPORT; break;       // 10046L
    case WSAEAFNOSUPPORT:       nerrno = EAFNOSUPPORT; break;       // 10047L
    case WSAEADDRINUSE:         nerrno = EADDRINUSE; break;         // 10048L
    case WSAEADDRNOTAVAIL:      nerrno = EADDRNOTAVAIL; break;      // 10049L
    case WSAENETDOWN:           nerrno = ENETDOWN; break;           // 10050L
    case WSAENETUNREACH:        nerrno = ENETUNREACH; break;        // 10051L
    case WSAENETRESET:          nerrno = ENETRESET; break;          // 10052L
    case WSAECONNABORTED:       nerrno = ECONNABORTED; break;       // 10053L
    case WSAECONNRESET:         nerrno = ECONNRESET; break;         // 10054L
    case WSAENOBUFS:            nerrno = ENOBUFS; break;            // 10055L
    case WSAEISCONN:            nerrno = EISCONN; break;            // 10056L
    case WSAENOTCONN:           nerrno = ENOTCONN; break;           // 10057L
    case WSAESHUTDOWN:          nerrno = ESHUTDOWN; break;          // 10058L
    case WSAETOOMANYREFS:       nerrno = ETOOMANYREFS; break;       // 10059L
    case WSAETIMEDOUT:          nerrno = ETIMEDOUT; break;          // 10060L
    case WSAECONNREFUSED:       nerrno = ECONNREFUSED; break;       // 10061L
    case WSAELOOP:              nerrno = ELOOP; break;              // 10062L
    case WSAENAMETOOLONG:       nerrno = ENAMETOOLONG; break;       // 10063L
    case WSAEHOSTDOWN:          nerrno = EHOSTDOWN; break;          // 10064L
    case WSAEHOSTUNREACH:       nerrno = EHOSTUNREACH; break;       // 10065L
    case WSAENOTEMPTY:          nerrno = ENOTEMPTY; break;          // 10066L
    case WSAEPROCLIM:           nerrno = EPROCLIM; break;           // 10067L
    case WSAEUSERS:             nerrno = EUSERS; break;             // 10068L
    case WSAEDQUOT:             nerrno = EDQUOT; break;             // 10069L
    case WSAESTALE:             nerrno = ESTALE; break;             // 10070L
    case WSAEREMOTE:            nerrno = EREMOTE; break;            // 10071L
//  case WSASYSNOTREADY:        nerrno = SYSNOTREADY; break;        // 10091L
//  case WSAVERNOTSUPPORTED:    nerrno = VERNOTSUPPORTED; break;    // 10092L
    case WSANOTINITIALISED:     nerrno = ENOTINITIALISED; break;    // 10093L
    case WSAEDISCON:            nerrno = EDISCON; break;            // 10101L
    case WSAENOMORE:            nerrno = ENOMORE; break;            // 10102L
    case WSAECANCELLED:         nerrno = ECANCELLED; break;         // 10103L
//  case WSAEINVALIDPROCTABLE:                                      // 10104L
//  case WSAEINVALIDPROVIDER:                                       // 10105L
//  case WSAEPROVIDERFAILEDINIT:                                    // 10106L
//  case WSASYSCALLFAILURE:                                         // 10107L
//  case WSASERVICE_NOT_FOUND:                                      // 10108L
//  case WSATYPE_NOT_FOUND:                                         // 10109L
//  case WSA_E_NO_MORE:                                             // 10110L
//  case WSA_E_CANCELLED:                                           // 10111L
    case WSAEREFUSED:           nerrno = EREFUSED; break;           // 10112L
    default:
        break;
    }
    return nerrno;
}
#endif //DEFUNCT
