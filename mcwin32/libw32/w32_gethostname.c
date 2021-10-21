#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_gethostname_c,"$Id: w32_gethostname.c,v 1.9 2021/06/10 12:42:33 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 gethostname
 *
 * Copyright (c) 1998 - 2021, Adam Young.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#include "win32_internal.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>

#if !defined(WINDOWS_MEAN_AND_LEAN)
#define WINDOWS_MEAN_AND_LEAN
#endif
#include <windows.h>

#if defined(__WATCOMC__) && (__WATCOMC__ <= 1900)
typedef enum _COMPUTER_NAME_FORMAT {
    ComputerNameNetBIOS,
    ComputerNameDnsHostname,
    ComputerNameDnsDomain,
    ComputerNameDnsFullyQualified,
    ComputerNamePhysicalNetBIOS,
    ComputerNamePhysicalDnsHostname,
    ComputerNamePhysicalDnsDomain,
    ComputerNamePhysicalDnsFullyQualified,
    ComputerNameMax
} COMPUTER_NAME_FORMAT;
BOOL WINAPI
GetComputerNameExA(COMPUTER_NAME_FORMAT NameType, LPSTR lpBuffer, LPDWORD nSize);
#endif


/*
//  NAME
//      gethostname - get name of current host
//
//  SYNOPSIS
//      #include <unistd.h>
//
//      int gethostname(char *name, size_t namelen);
//
//  DESCRIPTION
//      The gethostname() function shall return the standard host name for the
//      current machine. The namelen argument shall specify  the  size  of  the
//      array  pointed  to  by  the  name argument.  The returned name shall be
//      null-terminated, except that if namelen is an  insufficient  length  to
//      hold the host name, then the returned name shall be truncated and it is
//      unspecified whether the returned name is null-terminated.
//
//      Host names are limited to {HOST_NAME_MAX} bytes.
//
//  RETURN VALUE
//      Upon successful completion, 0 shall be returned; otherwise, -1 shall be
//      returned.
//
//  ERRORS
//      No errors are defined.
//
//      The following sections are informative.
*/
LIBW32_API int
w32_gethostname(char *name, size_t namelen)
{
    int done = 0, ret;
    const char *host;

    if (NULL == name || 0 == namelen) {
        errno = EINVAL;
        return -1;
    }

#undef gethostname
retry:;
    if (0 == (ret = gethostname(name, namelen))) {
        return 0;
    } else {
        DWORD dwSize = namelen;

        if (0 == done++) {                      /* WSAStartup call must occur before using this function. */
            if ((SOCKET_ERROR == ret && WSANOTINITIALISED == WSAGetLastError()) &&
                    0 == w32_sockinit()) {
                goto retry;                     /* hide winsock initialisation */
            }
        }
        w32_sockerror();

        if (GetComputerNameExA(ComputerNameDnsHostname, name, &dwSize) && dwSize) {
            // The DNS host name of the local computer.
            // If the local computer is a node in a cluster, lpBuffer receives the DNS host name of the cluster virtual server.
            return 0;
        }
    }

    host = getenv("COMPUTERNAME");
    if (NULL == host) host = "pccomputer";
    strncpy(name, (const char *)host, namelen);
    name[namelen - 1] = 0;

    return 0;
}

/*end*/
