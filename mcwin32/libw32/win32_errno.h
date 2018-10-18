#ifndef LIBW32_WIN32_ERRNO_H_INCLUDED
#define LIBW32_WIN32_ERRNO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_errno_h,"$Id: win32_errno.h,v 1.5 2018/10/09 16:03:48 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * <errno.h>
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

    /*
     *  Verify that MSVC POSIX errno number are not active; if the case warn and undef clashing constants
     *  Generally <sys/socket.h> is included before <errno.h> as such the following should not occur.
     */
#if defined(EADDRINUSE) && (EADDRINUSE != 10048)
#if defined(_MSC_VER)
#if (EADDRINUSE == 100)
#if !defined(_CRT_NO_POSIX_ERROR_CODES)
#pragma message("_CRT_NO_POSIX_ERROR_CODES should be defined: EADDRINUSE (and others) are inconsistent with WinSock aliases; redefining")
#else
#pragma message("_CRT_NO_POSIX_ERROR_CODES is defined: yet EADDRINUSE (and others) are inconsistent with WinSock aliases as <errno.h> include order is incorrect; redefining")
#endif
#else
#error unexpected EADDRINUSE value
#endif
#endif //_MSC_VER
#endif //EADDRINUSE != 10048

#if defined(_MSC_VER) || defined(__MAKEDEPEND__)
#undef EADDRINUSE           //100
#undef EADDRNOTAVAIL        //101
#undef EAFNOSUPPORT         //102
#undef EALREADY             //103
//#define EBADMSG           104
//#define ECANCELED         105
#undef ECONNABORTED         //106
#undef ECONNREFUSED         //107
#undef ECONNRESET           //108
#undef EDESTADDRREQ         //109
#undef EHOSTUNREACH         //110
//#define EIDRM             111
#undef EINPROGRESS          //112
#undef EISCONN              //113
#undef ELOOP                //114
#undef EMSGSIZE             //115g
#undef ENETDOWN             //116
#undef ENETRESET            //117
#undef ENETUNREACH          //118
#undef ENOBUFS              //119
//#define ENODATA           120
//#define ENOLINK           121
//#define ENOMSG            122
#undef ENOPROTOOPT          //123
//#define ENOSR             124
//#define ENOSTR            125
//#define ENOTCONN          126
#undef ENOTCONN             //126
#undef ENOTRECOVERABLE      //127
#undef ENOTSOCK             //128
//#define ENOTSUP           129
#undef EOPNOTSUPP           //130
//#define EOTHER            131
//#define EOVERFLOW         132
//#define EOWNERDEAD        133
//#define EPROTO            134
#undef EPROTONOSUPPORT      //135
#undef EPROTOTYPE           //136
//#define ETIME             137
#undef ETIMEDOUT            //138
//#define ETXTBSY           139
#undef EWOULDBLOCK          //140
#endif //EADDRINUSE

/*
 *  System <errno.h>
 */
#if defined(_MSC_VER) && \
        !defined(_CRT_NO_POSIX_ERROR_CODES)
#define _CRT_NO_POSIX_ERROR_CODES               /* disable POSIX error number, see <errno.h> (MSVC 2010+) */
#endif //_MSC_VER
#include <errno.h>

/*
 *  Addition UNIX style errno's, plus Windows Sockets errors redefined as regular Berkeley error constants.
 */

    /*
     *  General use error codes,
     *      which utilise their defined value under MSVC POSIX definition, see <errno.h>
     *
     *  MSVC: Their definitions are disabled using _CRT_NO_POSIX_ERROR_CODES,
     *      as many conflict with the WinSock aliases below,
     */
#define EBADMSG         104                     /* Bad message. */
#define ECANCELED       105                     /* Operation canceled. */
#define EIDRM           111                     /* Identifier removed. */
#define ENODATA         120                     /* No data. */
#define ENOLINK         121                     /* Virtual circuit is gone. */
#define ENOMSG          122                     /* No message. */
#define ENOSR           124                     /* No stream resources. */
#define ENOSTR          125                     /* Not a stream. */
#define ENOTRECOVERABLE 127                     /* Not recoverable. */
#define ENOTSUP         129                     /* Not supported. */
#define EOTHER          131                     /* Other error. */
#define EOVERFLOW       132                     /* Overflow. */
#define EOWNERDEAD      133                     /* Owner dead. */
#define ETIME           137                     /* Stream ioctl timeout. */
#define EPROTO          134
#if defined(_MSC_VER)
#define ETXTBSY         139                     /* Text file busy; OWC=33. */
#endif

    /* additional 141+ */
#define EBADFD          141                     /* Bad file descriptor. */
#define EBADRQC         142
#define ENOTUNIQ        143

    /*
     *  WinSock errors are aliased to their BSD/POSIX counter part.
     *
     *  Note: This works for *most* errors, yet the following result in conflicts and are
     *      explicity remapped during i/o operations.
     *
#define EINTR           WSAEINTR                // 10004 "Interrupted system call"
#define EBADF           WSAEBADF                // 10009 "Bad file number"
#define EACCES          WSAEACCES               // 10013 "Permission denied"
#define EFAULT          WSAEFAULT               // 10014 "Bad address"
#define EINVAL          WSAEINVAL               // 10022 "Invalid argument"
#define EMFILE          WSAEMFILE               // 10024 "Too many open files"
#define ENAMETOOLONG    WSAENAMETOOLONG         // 10063 "File name too long"
#define ENOTEMPTY       WSAENOTEMPTY            // 10066 "Directory not empty"
     */

#endif  /*LIBW32_WIN32_ERRNO_H_INCLUDED*/

    /*
     *  Outside include guard, set/verify WinSock error mapping.
     *  Note: Force errors on inconsistent redefinitions
     */
#if defined(WSAEINTR)
    //  && !defined(LIBW32_ERRNO_WINSOCK)
    //  #define LIBW32_ERRNO_WINSOCK

#if !defined(EWOULDBLOCK)
#define EWOULDBLOCK     WSAEWOULDBLOCK          /* 10035 "Operation would block" */
#elif (EWOULDBLOCK != WSAEWOULDBLOCK)
#error Inconsistent EWOULDBLOCK definition ....
#endif

#if !defined(EINPROGRESS)
#define EINPROGRESS     WSAEINPROGRESS          /* 10036 "Operation now in progress" */
#elif (EINPROGRESS != WSAEINPROGRESS)
#error Inconsistent EINPROGRESS definition ....
#endif

#if !defined(EALREADY)
#define EALREADY        WSAEALREADY             /* 10037 "Operation already in progress" */
#elif (EALREADY != WSAEALREADY)
#error Inconsistent EALREADY definition ....
#endif

#if !defined(ENOTSOCK)
#define ENOTSOCK        WSAENOTSOCK             /* 10038 "Socket operation on non-socket" */
#elif (ENOTSOCK != WSAENOTSOCK)
#error Inconsistent ENOTSOCK definition ....
#endif

#if !defined(EDESTADDRREQ)
#define EDESTADDRREQ    WSAEDESTADDRREQ         /* 10039 "Destination address required" */
#elif (EDESTADDRREQ != WSAEDESTADDRREQ)
#error Inconsistent EDESTADDRREQ definition ....
#endif

#if !defined(EMSGSIZE)
#define EMSGSIZE        WSAEMSGSIZE             /* 10040 "Message too long" */
#elif (EMSGSIZE != WSAEMSGSIZE)
#error Inconsistent EMSGSIZE definition ....
#endif

#if !defined(EPROTOTYPE)
#define EPROTOTYPE      WSAEPROTOTYPE           /* 10041 "Protocol wrong type for socket" */
#elif (EPROTOTYPE != WSAEPROTOTYPE)
#error Inconsistent EPROTOTYPE definition ....
#endif

#if !defined(ENOPROTOOPT)
#define ENOPROTOOPT     WSAENOPROTOOPT          /* 10042 "Bad protocol option" */
#elif (ENOPROTOOPT != WSAENOPROTOOPT)
#error Inconsistent ENOPROTOOPT definition ....
#endif

#if !defined(EPROTONOSUPPORT)
#define EPROTONOSUPPORT WSAEPROTONOSUPPORT      /* 10043 "Protocol not supported" */
#elif (EPROTONOSUPPORT != WSAEPROTONOSUPPORT)
#error Inconsistent EPROTONOSUPPORT definition ....
#endif

#if !defined(ESOCKTNOSUPPORT)
#define ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT      /* 10044 "Socket type not supported" */
#elif (ESOCKTNOSUPPORT != WSAESOCKTNOSUPPORT)
#error Inconsistent ESOCKTNOSUPPORT definition ....
#endif

#if !defined(EOPNOTSUPP)
#define EOPNOTSUPP      WSAEOPNOTSUPP           /* 10045 "Operation not supported on socket" */
#elif (EOPNOTSUPP != WSAEOPNOTSUPP)
#error Inconsistent EOPNOTSUPP definition ....
#endif

#if !defined(EPFNOSUPPORT)
#define EPFNOSUPPORT    WSAEPFNOSUPPORT         /* 10046 "Protocol family not supported" */
#elif (EPFNOSUPPORT != WSAEPFNOSUPPORT)
#error Inconsistent EPFNOSUPPORT definition ....
#endif

#if !defined(EAFNOSUPPORT)
#define EAFNOSUPPORT    WSAEAFNOSUPPORT         /* 10047 "Address family not supported by protocol family" */
#elif (EAFNOSUPPORT != WSAEAFNOSUPPORT)
#error Inconsistent EAFNOSUPPORT definition ....
#endif

#if !defined(EADDRINUSE)
#define EADDRINUSE      WSAEADDRINUSE           /* 10048 "Address already in use" */
#elif (EADDRINUSE != WSAEADDRINUSE)
#error Inconsistent EADDRINUSE definition ....
#endif

#if !defined(EADDRNOTAVAIL)
#define EADDRNOTAVAIL   WSAEADDRNOTAVAIL        /* 10049 "Can't assign requested address" */
#elif (EADDRNOTAVAIL != WSAEADDRNOTAVAIL)
#error Inconsistent EADDRNOTAVAIL definition ....
#endif

#if !defined(ENETDOWN)
#define ENETDOWN        WSAENETDOWN             /* 10050 "Network is down" */
#elif (ENETDOWN != WSAENETDOWN)
#error Inconsistent ENETDOWN definition ....
#endif

#if !defined(ENETUNREACH)
#define ENETUNREACH     WSAENETUNREACH          /* 10051 "Network is unreachable" */
#elif (ENETUNREACH != WSAENETUNREACH)
#error Inconsistent ENETUNREACH definition ....
#endif

#if !defined(ENETRESET)
#define ENETRESET       WSAENETRESET            /* 10052 "Net dropped connection or reset" */
#elif (ENETRESET != WSAENETRESET)
#error Inconsistent ENETRESET definition ....
#endif

#if !defined(ECONNABORTED)
#define ECONNABORTED    WSAECONNABORTED         /* 10053 "Software caused connection abort" */
#elif (ECONNABORTED != WSAECONNABORTED)
#error Inconsistent ECONNABORTED definition ....
#endif

#if !defined(ECONNRESET)
#define ECONNRESET      WSAECONNRESET           /* 10054 "Connection reset by peer" */
#elif (ECONNRESET != WSAECONNRESET)
#error Inconsistent ECONNRESET definition ....
#endif

#if !defined(ENOBUFS)
#define ENOBUFS         WSAENOBUFS              /* 10055 "No buffer space available" */
#elif (ENOBUFS != WSAENOBUFS)
#error Inconsistent ENOBUFS definition ....
#endif

#if !defined(EISCONN)
#define EISCONN         WSAEISCONN              /* 10056 "Socket is already connected" */
#elif (EISCONN != WSAEISCONN)
#error Inconsistent EISCONN definition ....
#endif

#if !defined(ENOTCONN)
#define ENOTCONN        WSAENOTCONN             /* 10057 "Socket is not connected" */
#elif (ENOTCONN != WSAENOTCONN)
#error Inconsistent ENOTCONN definition ....
#endif

#if !defined(ESHUTDOWN)
#define ESHUTDOWN       WSAESHUTDOWN            /* 10058 "Can't send after socket shutdown" */
#elif (ESHUTDOWN != WSAESHUTDOWN)
#error Inconsistent ESHUTDOWN definition ....
#endif

#if !defined(ETOOMANYREFS)
#define ETOOMANYREFS    WSAETOOMANYREFS         /* 10059 "Too many references, can't splice" */
#elif (ETOOMANYREFS != WSAETOOMANYREFS)
#error Inconsistent ETOOMANYREFS definition ...
#endif

#if !defined(ETIMEDOUT)
#define ETIMEDOUT       WSAETIMEDOUT            /* 10060 "Connection timed out" */
#elif (ETIMEDOUT != WSAETIMEDOUT)
#error Inconsistent ETIMEDOUT definition ....
#endif

#if !defined(ECONNREFUSED)
#define ECONNREFUSED    WSAECONNREFUSED         /* 10061 "Connection refused" */
#elif (ECONNREFUSED != WSAECONNREFUSED)
#error Inconsistent ECONNREFUSED definition ...
#endif

#if !defined(ELOOP)
#define ELOOP           WSAELOOP                /* 10062 "Too many levels of symbolic links" */
#elif (ELOOP != WSAELOOP)
#error Inconsistent ELOOP definition ....
#endif

#if !defined(EHOSTDOWN)
#define EHOSTDOWN       WSAEHOSTDOWN            /* 10064 "Host is down" */
#elif (EHOSTDOWN != WSAEHOSTDOWN)
#error Inconsistent EHOSTDOWN definition ....
#endif

#if !defined(EHOSTUNREACH)
#define EHOSTUNREACH    WSAEHOSTUNREACH         /* 10065 "No Route to Host" */
#elif (EHOSTUNREACH != WSAEHOSTUNREACH)
#error Inconsistent EHOSTUNREACH definition ....
#endif

#if !defined(EPROCLIM)
#define EPROCLIM        WSAEPROCLIM             /* 10067 "Too many processes" */
#elif (EPROCLIM != WSAEPROCLIM)
#error Inconsistent EPROCLIM definition ....
#endif

#if !defined(EUSERS)
#define EUSERS          WSAEUSERS               /* 10068 "Too many users" */
#elif (EUSERS != WSAEUSERS)
#error Inconsistent EUSERS definition ....
#endif

#if !defined(EDQUOT)
#define EDQUOT          WSAEDQUOT               /* 10069 "Disc Quota Exceeded" */
#elif (EDQUOT != WSAEDQUOT)
#error Inconsistent EDQUOT definition ....
#endif

#if !defined(ESTALE)
#define ESTALE          WSAESTALE               /* 10070 "Stale NFS file handle" */
#elif (ESTALE != WSAESTALE)
#error Inconsistent ESTALE definition ....
#endif

#if !defined(ENONET)
#define ENONET          WSASYSNOTREADY          /* 10091 "Network SubSystem is unavailable" */
#elif (ENONET != WSASYSNOTREADY)
#error Inconsistent ENONET definition ....
#endif

#if !defined(ENOTINITIALISED)
#define ENOTINITIALISED WSANOTINITIALISED       /* 10093 "Successful WSASTARTUP not yet performed" */
#elif (ENOTINITIALISED != WSANOTINITIALISED)
#error Inconsistent ENOTINITIALISED definition ....
#endif

#if !defined(EREMOTE)
#define EREMOTE         WSAEREMOTE              /* 10071 "Too many levels of remote in path" */
#elif (EREMOTE != WSAEREMOTE)
#error Inconsistent EREMOTE definition ....
#endif

#else   //LIBW32_ERRNO_WINSOCK
    /*
     *  Map not socket specific errors.
     */

#if !defined(EWOULDBLOCK)
#define EWOULDBLOCK     10035                   /* 10035 "Operation would block" */
#endif

#if !defined(EALREADY)
#define EALREADY        10037                   /* 10037 "Operation already in progress" */
#endif

#if !defined(EMSGSIZE)
#define EMSGSIZE        10040                   /* 10040 "Message too long" */
#endif

#if !defined(EOPNOTSUPP)
#define EOPNOTSUPP      10045                   /* 10045 "Operation not supported on socket" */
#endif

#if !defined(ENOBUFS)
#define ENOBUFS         10055                   /* 10055 "No buffer space available" */
#endif

#if !defined(ETIMEDOUT)
#define ETIMEDOUT       10060                   /* 10060 "Connection timed out" */
#endif

#if !defined(ELOOP)
#define ELOOP           10062                   /* 10062 "Too many levels of symbolic links" */
#endif

#endif  //!LIBW32_ERRNO_WINSOCK

/*end*/
