#ifndef LIBW32_WIN32_ERRNO_H_INCLUDED
#define LIBW32_WIN32_ERRNO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_errno_h,"$Id: win32_errno.h,v 1.11 2023/09/17 13:05:00 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * <errno.h>
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
 */

/*
 *  Verify that MSVC POSIX errno number are not active; if the case warn and undef clashing constants
 *  Generally <sys/socket.h> is included before <errno.h> as such the following should not occur.
 */
#if !defined(__MAKEDEPEND__)
#if defined(EADDRINUSE) && (EADDRINUSE != 10048)

#if defined(_MSC_VER) || \
        (defined(__MINGW32__) && defined(__MINGW64_VERSION_MAJOR))
#if (EADDRINUSE == 100)
#if defined(_CRT_NO_POSIX_ERROR_CODES)
#pragma message <system_error> is incompatible with _CRT_NO_POSIX_ERROR_CODES.
#endif
    /*
     *  RETHINK, as the following are assumed by the MinGW pthread package:
     *
     *      #define ETIMEDOUT   138
     *      #define ENOTSUP     129
     *      #define EWOULDBLOCK 140
     */
#include "msvc_errno.h"                         /* undef error codes */
#else
#error unexpected EADDRINUSE value
#endif
#endif //_MSC_VER

#endif //EADDRINUSE != 10048
#endif //__MAKEDEPEND__

/*
 *  import <errno.h>
 */
#if !defined(_CRT_NO_POSIX_ERROR_CODES)
#define  _CRT_NO_POSIX_ERROR_CODES              /* disable extended POSIX errno's */
#include <errno.h>
#undef   _CRT_NO_POSIX_ERROR_CODES
#else
#include <errno.h>
#endif

/*
 *  Addition UNIX style errno's, plus Windows Sockets errors redefined as regular Berkeley error constants.
 */

    /*
     *  General use error codes,
     *      which utilise their defined value under MSVC POSIX definition, see <errno.h>
     *
     *  MSVC: Their definitions can be disabled using _CRT_NO_POSIX_ERROR_CODES, as many conflict with the
     *      WinSock aliases below, but this generates complication errors within C++ code elements.
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
#if (defined(_MSC_VER) && !defined(__WATCOMC__))
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
     *      explicitly remapped during i/o operations: see w32_neterrno_map()
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
#if defined(WSAEINTR) && !defined(__MAKEDEPEND__)
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

#if defined(EWOULDBLOCK) && (EWOULDBLOCK != 10045)
    #if (EWOULDBLOCK == 140)                    /* MSVC/MINGW64 extra */
        #undef EWOULDBLOCK
    #else
        #error Inconsistent EWOULDBLOCK definition ....
    #endif
#endif
#if !defined(EWOULDBLOCK)
#define EWOULDBLOCK     10035                   /* 10035 "Operation would block" */
#endif

#if defined(EALREADY) && (EALREADY != 10037)
    #if (EALREADY == 103)                       /* MSVC/MINGW64 extra */
        #undef EALREADY
    #else
        #error Inconsistent EALREADY definition ....
    #endif
#endif
#if !defined(EALREADY)
#define EALREADY        10037                   /* 10037 "Operation already in progress" */
#endif

#if defined(EMSGSIZE) && (EMSGSIZE != 10040)
    #if (EMSGSIZE == 115)                       /* MSVC/MINGW64 extra */
        #undef EMSGSIZE
    #else
        #error Inconsistent EMSGSIZE definition ....
    #endif
#endif
#if !defined(EMSGSIZE)
#define EMSGSIZE        10040                   /* 10040 "Message too long" */
#endif

#if defined(EOPNOTSUPP) && (EOPNOTSUPP != 10045)
    #if (EOPNOTSUPP == 130)                     /* MSVC/MINGW64 extra */
        #undef EOPNOTSUPP
    #else
        #error Inconsistent EOPNOTSUPP definition ....
    #endif
#endif
#if !defined(EOPNOTSUPP)
#define EOPNOTSUPP      10045                   /* 10045 "Operation not supported on socket" */
#endif

#if defined(ENETUNREACH) && (ENETUNREACH != 10051)
    #if (ENETUNREACH == 118)
        #undef ENETUNREACH                      /* MSVC/MINGW64 */
    #else
        #error Inconsistent ENETUNREACH definition ....
    #endif
#endif
#if !defined(ENETUNREACH)
#define ENETUNREACH      10051                  /* 10051 "Network is unreachable" */
#endif

#if defined(ECONNRESET) && (ECONNRESET != 10054)
    #if (ECONNRESET == 108)                     /* MSVC/MINGW64 */
        #undef ECONNRESET
    #else
        #error Inconsistent ECONNRESET definition ....
    #endif
#endif
#if !defined(ECONNRESET)
#define ECONNRESET      10054                   /* 10054 "Connection reset by peer" */
#endif

#if defined(ENOBUFS) && (ENOBUFS != 10055)
    #if (ENOBUFS == 119)                     /* MSVC/MINGW64 */
        #undef ENOBUFS
    #else
        #error Inconsistent ENOBUFS definition ....
    #endif
#endif
#if !defined(ENOBUFS)
#define ENOBUFS         10055                   /* 10055 "No buffer space available" */
#endif

#if defined(ETIMEDOUT) && (ETIMEDOUT != 10060)
#   if (ETIMEDOUT == 138)                       /* MSVC/MINGW64 */
#       undef ETIMEDOUT
#   else
#       error Inconsistent ETIMEDOUT definition ....
#   endif
#endif
#if !defined(ETIMEDOUT)
#define ETIMEDOUT       10060                   /* 10060 "Connection timed out" */
#endif

#if defined(ELOOP) && (ELOOP != 10062)
#   if (ELOOP == 114)                           /* MSVC/MINGW64 */
#       undef ELOOP
#   else
#       error Inconsistent ELOOP definition ....
#   endif
#endif
#if !defined(ELOOP)
#define ELOOP           10062                   /* 10062 "Too many levels of symbolic links" */
#endif

#endif /*LIBW32_WIN32_ERRNO_H_INCLUDED*/

/*end*/
