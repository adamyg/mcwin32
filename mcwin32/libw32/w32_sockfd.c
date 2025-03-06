#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_sockfd_c,"$Id: w32_sockfd.c,v 1.17 2025/03/06 16:59:47 cvsuser Exp $")

/*
 * win32 socket file-descriptor support
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0601              /* enable vista+ features (WSAPoll) */
#endif

#include "win32_include.h"
#include "win32_internal.h"
#include "win32_misc.h"

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

static int                  x_fdhard;           /* hard limit */
static int                  x_fdlimit = WIN32_FILDES_DEF; /* soft limit */
static SOCKET *             x_fdsockets;


/*
 *  file-descriptor association initialisation.
 */
LIBW32_API void
w32_fdsetinit(void)
{
    if (0 == x_fdhard) {
        unsigned s;

        if (NULL != (x_fdsockets = (SOCKET *)calloc(WIN32_FILDES_MAX, sizeof(SOCKET)))) {
            for (s = 0; s < WIN32_FILDES_MAX; ++s) {
                x_fdsockets[s] = INVALID_SOCKET;
            }
            x_fdhard = WIN32_FILDES_MAX;
        }
    }
}


/*
 *  update the file-descriptor limit
 */
LIBW32_API int
w32_fdregister(int fd)
{
    assert(fd >= -1 /*error*/ && fd < WIN32_FILDES_MAX);
    if (fd > x_fdlimit) {
        if ((x_fdlimit = fd) > WIN32_FILDES_MAX) {
            x_fdlimit = WIN32_FILDES_MAX;       /* cap */
        }
    }
    return fd;
}


/*
 *  associate a file-descriptor with a socket.
 */
LIBW32_API void
w32_fdsockopen(int fd, SOCKET s)
{
    assert(fd >= 0 && fd < WIN32_FILDES_MAX);

    w32_fdsetinit();
    w32_fdregister(fd);

    if (x_fdsockets && fd >= 0 && fd < x_fdhard) {
        assert(s != INVALID_SOCKET);
        x_fdsockets[fd] = s;
    }
}


/*
 *  retrieve the socket associated with a file-descriptor.
 */
LIBW32_API SOCKET
w32_fdsockget(int fd)
{
    if (fd >= WIN32_FILDES_MAX) {               /* not an osf handle; hard limit */
        return fd;

    } else if (fd >= 0 && fd < x_fdhard) {
        if (x_fdsockets) {                      /* local socket mapping */
            return x_fdsockets[fd];

        /*
         *  MSVC 2015+ no longer suitable without fdlimit; asserts when out-of-range
         */
        } else if (fd < x_fdlimit) {
            SOCKET s;
            if ((s = _get_osfhandle(fd)) != (SOCKET)INVALID_HANDLE_VALUE) {
                return s;
            }
        }
    }
    return INVALID_SOCKET;
}


/*
 *  disassociate a file-descriptor with a socket.
 */
LIBW32_API void
w32_fdsockclose(int fd, SOCKET s)
{
    if (fd >= 0 && fd < x_fdhard) {
        if (s == INVALID_SOCKET || x_fdsockets[fd] == s) {
            x_fdsockets[fd] = INVALID_SOCKET;
        }
    }
}


/*
 *  determine whether a socket file descriptor, for read/write/close usage.
 */

//  static int
//  IsStdHandle(int fd)
//  {
//      switch((DWORD)fd) {
//	case STD_INPUT_HANDLE:	// (DWORD)-10   The standard input device.  Initially, this is the console input buffer, CONIN$.
//      case STD_OUTPUT_HANDLE: // (DWORD)-11   The standard output device. Initially, this is the active console screen buffer, CONOUT$.
//      case STD_ERROR_HANDLE:  // (DWORD)-12   The standard error device.  Initially, this is the active console screen buffer, CONOUT$.
//          return 1;
//      }
//	return 0;
//  }

static int
IsSocket(HANDLE h)
{
    return (GetFileType(h) == FILE_TYPE_PIPE &&
                0 == GetNamedPipeInfo(h, NULL, NULL, NULL, NULL));
}

LIBW32_API int
w32_issockfd(int fd, SOCKET *s)
{
    SOCKET t_s = INVALID_SOCKET;

    if (fd >= 0) {
        if (fd >= WIN32_FILDES_MAX) {           /* not an osf handle; hard limit */
            /*
             *  TODO: restrict logic further
             *      HANDLES should always be DWORD aligned, hence must be "(fd & 0x7) == 0"
             *      Confirm and can this go further??
             *          Minimal handle value??
             */
            t_s = (SOCKET)fd;

        } else if (fd >= x_fdhard ||            /* local socket mapping */
                    (t_s = x_fdsockets[fd]) == INVALID_SOCKET) {

            /*
             *  MSVC 2015+ no longer suitable; asserts when out-of-range.
             *  Unfortunately socket handles can be small numeric values yet so are file descriptors.
             */
            if (fd >= 0x80 && 0 == (fd & 0x3) && IsSocket(w32_ITOH(fd))) {
                t_s = (SOCKET)fd;

            } else if (fd >= x_fdlimit ||
                    _get_osfhandle(fd) == (OSFHANDLE)INVALID_HANDLE_VALUE) {
                t_s = (SOCKET)fd;               /* invalid assume socket; otherwise file */
            }
        }
    }

    if (s) *s = t_s;
    return (t_s != INVALID_SOCKET);
}

/*end*/
