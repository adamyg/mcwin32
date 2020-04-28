#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_poll_c,"$Id: w32_poll.c,v 1.5 2020/04/25 20:17:11 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 poll system calls
 *
 * Copyright (c) 1998 - 2018, Adam Young.
 *
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
#define _WIN32_WINNT 0x0601                     /* enable vista+ features (WSAPoll) */
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

#if defined(_MSC_VER)
#pragma warning(disable : 4244) // conversion from 'xxx' to 'xxx', possible loss of data
#endif

#pragma comment(lib, "Ws2_32.lib")

static int w32_poll(int native, struct pollfd *fds, int cnt, int timeout);

/*
//  NAME
//
//      poll - input/output multiplexing
//
//  SYNOPSIS
//
//      #include <<poll.h>
//      int poll(struct pollfd fds[], nfds_t nfds, int timeout);
//
//  DESCRIPTION
//
//      The poll() function provides applications with a mechanism for multiplexing
//      input/output over a set of file descriptors. For each member of the array
//      pointed to by fds, poll() examines the given file descriptor for the event(s)
//      specified in events. The number of pollfd structures in the fds array is
//      specified by nfds. The poll() function identifies those file descriptors on
//      which an application can read or write data, or on which certain events have
//      occurred.
//
//      The fds argument specifies the file descriptors to be examined and the events
//      of interest for each file descriptor. It is a pointer to an array with one
//      member for each open file descriptor of interest. The array`s members are
//      pollfd structures within which fd specifies an open file descriptor and events
//      and revents are bitmasks constructed by OR-ing a combination of the following
//      event flags:
//
//      POLLIN
//          Data other than high-priority data may be read without blocking. For
//          STREAMS, this flag is set in revents even if the message is of zero length.
//
//      POLLRDNORM
//          Normal data (priority band equals 0) may be read without blocking. For
//          STREAMS, this flag is set in revents even if the message is of zero length.
//
//      POLLRDBAND
//          Data from a non-zero priority band may be read without blocking. For
//          STREAMS, this flag is set in revents even if the message is of zero length.
//
//      POLLPRI
//          High-priority data may be received without blocking. For STREAMS, this flag
//          is set in revents even if the message is of zero length.
//
//      POLLOUT
//          Normal data (priority band equals 0) may be written without blocking.
//
//      POLLWRNORM
//          Same as POLLOUT.
//
//      POLLWRBAND
//          Priority data (priority band > 0) may be written. If any priority band has
//          been written to on this STREAM, this event only examines bands that have
//          been written to at least once.
//
//      POLLERR
//          An error has occurred on the device or stream. This flag is only valid in
//          the revents bitmask; it is ignored in the events member.
//
//      POLLHUP
//          The device has been disconnected. This event and POLLOUT are mutually
//          exclusive; a stream can never be writable if a hangup has occurred. However,
//          this event and POLLIN, POLLRDNORM, POLLRDBAND or POLLPRI are not mutually
//          exclusive. This flag is only valid in the revents bitmask; it is ignored in
//          the events member.
//
//      POLLNVAL
//          The specified fd value is invalid. This flag is only valid in the revents
//          member; it is ignored in the events member.
//
//      If the value of fd is less than 0, events is ignored and revents is set to 0 in
//      that entry on return from poll().
//
//      In each pollfd structure, poll() clears the revents member except that where
//      the application requested a report on a condition by setting one of the bits of
//      events listed above, poll() sets the corresponding bit in revents if the
//      requested condition is true. In addition, poll() sets the POLLHUP, POLLERR and
//      POLLNVAL flag in revents if the condition is true, even if the application did
//      not set the corresponding bit in events.
//
//      If none of the defined events have occurred on any selected file descriptor,
//      poll() waits at least timeout milliseconds for an event to occur on any of the
//      selected file descriptors. If the value of timeout is 0, poll() returns
//      immediately. If the value of timeout is -1, poll() blocks until a requested
//      event occurs or until the call is interrupted.
//
//      Implementations may place limitations on the granularity of timeout intervals.
//      If the requested timeout interval requires a finer granularity than the
//      implementation supports, the actual timeout interval will be rounded up to the
//      next supported value.
//
//      The poll() function is not affected by the O_NONBLOCK flag.
//
//      The poll() function supports regular files, terminal and pseudo-terminal
//      devices, STREAMS-based files, FIFOs and pipes. The behaviour of poll() on
//      elements of fds that refer to other types of file is unspecified.
//
//      Regular files always poll TRUE for reading and writing.
//
//  RETURN VALUE
//
//      Upon successful completion, poll() returns a non-negative value. A positive
//      value indicates the total number of file descriptors that have been selected
//      (that is, file descriptors for which the revents member is non-zero). A value
//      of 0 indicates that the call timed out and no file descriptors have been
//      selected. Upon failure, poll() returns -1 and sets errno to indicate the error.
//
//    ERRORS
//
//      The poll() function will fail if:
//
//      [EAGAIN]
//          The allocation of internal data structures failed but a subsequent request
//          may succeed.
//
//      [EINTR]
//          A signal was caught during poll().
//
//      [EINVAL]
//          The nfds argument is greater than {OPEN_MAX}, or one of the fd members
//          refers to a STREAM or multiplexer that is linked (directly or indirectly)
//          downstream from a multiplexer.
//
//  SEE ALSO
//      getmsg(), putmsg(), read(), select(), write(), <poll.h>
*/

LIBW32_API int
w32_poll_fd(struct pollfd *fds, int cnt, int timeout)
{
    return w32_poll(FALSE, fds, cnt, timeout);
}


LIBW32_API int
w32_poll_native(struct pollfd *fds, int cnt, int timeout)
{
    return w32_poll(TRUE, fds, cnt, timeout);
}


static int
w32_poll(int native, struct pollfd *fds, int cnt, int timeout)
{
    //  TODO -- dynamically load
    //      WINSOCK_API_LINKAGE int WSAAPI WSAPoll(LPWSAPOLLFD, ULONG, INT);
    //          ==> source: ws2_32.lib
    //
    struct timeval tmval = {0};
    struct fd_set rfds;
    struct fd_set wfds;
    struct fd_set efds;
    SOCKET s[ FD_SETSIZE ];
    int badcnt, wcnt, ret;
    int i;

    if (cnt <= 0 || cnt > FD_SETSIZE) {
        errno = EINVAL;
        return -1;
    }

    /* Build fd set */
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);

    for (i = 0; i < cnt; ++i)
        fds[i].revents = 0;

    for (i = 0, badcnt = 0, wcnt = 0; i < cnt; ++i) {
        SOCKET osf;

        if (native) {
            osf = fds[i].fd;
        } else {
            osf = w32_sockhandle((int)fds[i].fd);
            if (osf == (SOCKET)INVALID_SOCKET) {
                fds[i].revents = POLLNVAL;
                ++badcnt;
            }
        }
        s[i] = osf;
        FD_SET(s[i], &rfds);
        if (fds[i].events & POLLOUT) {
            FD_SET(s[i], &wfds);
            ++wcnt;
        }
        FD_SET(s[i], &efds);
    }

    if (badcnt) return badcnt;

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
        w32_neterrno_set();
        ret = -1;

        switch(nerr) {
        case WSANOTINITIALISED:     /* stack not initialisated; should not occurr */
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
                        ++ret;
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
        u_long  val = 0;
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

/*end*/

