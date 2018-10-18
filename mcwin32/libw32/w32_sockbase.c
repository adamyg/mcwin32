#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_sockbase_c,"$Id: w32_sockbase.c,v 1.4 2018/10/12 00:52:04 cvsuser Exp $")

/*
 * win32 socket () system calls
 * Base functionality.
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

static int x_sockinit = 0;                      /* initialisation status */
LIBW32_API int w32_h_errno = 0;                 /* lookup error */


/*
 *  w32_sockinit() system run-time initialisation.
 */
LIBW32_API int
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
    }
    return 0;
}


/*
 *  getaddrinfo() system/library call
 */
LIBW32_API int
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
 *  getnameinfo() system/library call
 */
LIBW32_API int
w32_getnameinfo(const struct sockaddr *sa, socklen_t salen,
    char *node, socklen_t nodelen, char * service, socklen_t servicelen, int flags)
{
    int done = 0, ret;

#undef getnameinfo
retry:;
    if (0 != (ret = getnameinfo(sa, salen, node, nodelen, service, servicelen, flags))) {
        if (0 == done++) {
            if ((WSANOTINITIALISED == ret || (-1 == ret && WSANOTINITIALISED == WSAGetLastError())) &&
                    0 == w32_sockinit()) {
                goto retry;                     /* hide winsock initialisation */
            }
        }
        w32_sockerror(); /*???*/
    }
    return ret;
}


/*
 *  gethostbyname() system/library call
 *
 *  Note: The gethostbyname function has been deprecated by the introduction of the
 *      getaddrinfo function. Developers creating Windows Sockets 2 applications are urged
 *      to use the getaddrinfo function instead of gethostbyname.
 */
LIBW32_API struct hostent *
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
LIBW32_API const char *
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
LIBW32_API void
w32_herror(const char *msg)
{
    fprintf(stderr, "%s: %s\n",                 /*XXX*/
        (msg ? msg : "(nul)"), w32_hstrerror(w32_h_errno));
}


/*
 *  export the last socket error.
 */
LIBW32_API int
w32_sockerror(void)
{
    return w32_neterrno_set();
}

/*end*/

