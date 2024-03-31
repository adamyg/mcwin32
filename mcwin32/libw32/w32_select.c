#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_select_c,"$Id: w32_select.c,v 1.15 2024/01/16 15:17:52 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 *  Windows 'select' compat interface
 *
 * Copyright (c) 2007, 2012 - 2024 Adam Young.
 * All rights reserved.
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

#include "win32_internal.h"

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#define  WIN32_SOCKET_H_CLEAN                   // disable mapping
#ifdef   HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

typedef struct Select {
    int             s_fd;                       // user supplied handle
    void          (*s_poll)(struct Select *);   // i/o poll interface
    HANDLE          s_handle;                   // system handle
    BYTE            s_type;                     // and handle type ... internal
#define FD_UNKNOWN      0
#define FD_CHAR         10
#define FD_CONSOLE      11
#define FD_BLOCK        20
#define FD_PIPE         30
#define FD_SOCK         31
    BYTE            s_wanted;                   // required streams
    BYTE            s_avail;                    // available streams
    BYTE            s_error;                    // and/or error on stream
#define T_READ          1
#define T_WRITE         2
#define T_EXCEPT        4
} Select_t;


SOCKET              w32_sockhandle(int);

static u_int        sel_build( int, fd_set *, u_int *, Select_t * );
static int          sel_wait( u_int cnt, Select_t *selfds, DWORD timeout );
static DWORD        sel_ticks( DWORD stick, DWORD etick );
static void         sel_console( Select_t *selfd );
static void         sel_block( Select_t *selfd );
static void         sel_pipe( Select_t *selfd );
static void         sel_socket( Select_t *selfd );
static void         sel_unknown( Select_t *selfd );


/*
 *  select() system call
 */
LIBW32_API int
w32_select(
    int nfs, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *tm)
{
    Select_t *selfds;
    u_int selcnt, invalid;
    DWORD timeout = 0;
    int ret;

    __CUNUSED(nfs)

    selcnt = (readfds ? readfds->fd_count : 0) + (writefds ? writefds->fd_count : 0) +
                (exceptfds ? exceptfds->fd_count : 0);

    if ((selfds = calloc(sizeof(Select_t), selcnt + 1)) == NULL) {
        return -1;
    }

    invalid =  sel_build( T_READ, readfds, &selcnt, selfds );
    invalid += sel_build( T_WRITE, writefds, &selcnt, selfds );
    invalid += sel_build( T_EXCEPT, exceptfds, &selcnt, selfds );

    timeout = 0;
    if (tm) {
        tm += (tm->tv_sec * 1000);
        tm += (tm->tv_usec / (1000000));
    }

    ret = sel_wait(selcnt, selfds, timeout);
    free(selfds);

    return ret;
}


static u_int
sel_build(
    int type, fd_set *fds, u_int *selcnt, Select_t *selfds )
{
    u_int invalid = 0, idx, fd;
    SOCKET osf = 0;

    if (fds == NULL) {
        return 0;
    }

    for (idx = 0; idx < fds->fd_count; ++idx) {
        // locate
        for (fd = 0; fd < *selcnt; fd++) {
            if (selfds[fd].s_fd == (int)fds->fd_array[idx])
                break;
        }

        // New handle, determine type
        if (fd >= *selcnt) {

            ++selcnt;

            selfds[fd].s_fd = (int)fds->fd_array[idx];

            if ((osf = w32_sockhandle((int)fds->fd_array[idx])) == INVALID_SOCKET) {
                selfds[fd].s_handle = (HANDLE)0;
                selfds[fd].s_type = FD_UNKNOWN;
                invalid++;

            } else {
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif
                selfds[fd].s_handle = (HANDLE)osf;

                switch (GetFileType((HANDLE)osf)) {
                case FILE_TYPE_CHAR: {          // char file
                        DWORD mode;

                        if (GetConsoleMode((HANDLE)osf, &mode ) == 0) {
                            selfds[fd].s_type = FD_CONSOLE;
                            selfds[fd].s_poll = sel_console;
                        } else {
                            selfds[fd].s_type = FD_CHAR;
                            selfds[fd].s_poll = sel_unknown;
                        }
                    }
                    break;
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif

                case FILE_TYPE_DISK:            // disk file
                    selfds[fd].s_type = FD_BLOCK;
                    selfds[fd].s_poll = sel_block;
                    break;

                case FILE_TYPE_PIPE: {          // pipe or socket
                        int state, len = sizeof(int);

                                                // what is the cost?? any alt method??
                        if (getsockopt(osf, SOL_SOCKET, SO_TYPE, (char *)&state, &len ) == 0 ||
                                WSAGetLastError() != WSAENOTSOCK) {
                            selfds[fd].s_type = FD_SOCK;
                            selfds[fd].s_poll = sel_socket;

                        } else {
                            selfds[fd].s_type = FD_PIPE;
                            selfds[fd].s_poll = sel_pipe;
                        }
                    }
                    break;

                case FILE_TYPE_REMOTE:          // unused
                case FILE_TYPE_UNKNOWN:         // unknown or error
                default:
                    selfds[fd].s_type = FD_UNKNOWN;
                    selfds[fd].s_poll = sel_unknown;
                    invalid++;
                    break;
                }
            }
        }

        // Assign type
        selfds[fd].s_wanted |= type;
    }
    return invalid;
}


static int
sel_wait(u_int cnt, Select_t *selfds, DWORD timeout)
{
    DWORD  stick, ret;
    HANDLE waitfor[MAXIMUM_WAIT_OBJECTS];       // system limit
    u_int i = 0;

    if (cnt > sizeof(waitfor)/sizeof(waitfor[0]))
        return -EINVAL;

    while (i < cnt) {                           // build waitfor array
        waitfor[i] = selfds[i].s_handle;
        i++;
    }

    assert(0 == WAIT_OBJECT_0);
    stick = GetTickCount();                     // start tick

    for (;;) {
        // wait for event/timeout
        if ((ret = WaitForMultipleObjects (cnt, waitfor, FALSE, timeout)) == WAIT_FAILED) {
            return -EIO;
        }

        // timeout
        if (ret == WAIT_TIMEOUT) {
            break;
        }

        // event
        if (ret <= (DWORD)(WAIT_OBJECT_0 + cnt + 1)) {
            i = ret - WAIT_OBJECT_0;
            assert(waitfor[i] == selfds[i].s_handle);
            (selfds[i].s_poll)( selfds+i );
            if (selfds[i].s_error || selfds[i].s_avail)
                return i+1;
        }

        // calculate next timeout frame...
        if (timeout != INFINITE) {
            DWORD ctick, ttick;                 // current and total ticks

            ctick = GetTickCount();
            if ((ttick = sel_ticks( stick, ctick )) > timeout) {
                break;
            }
            timeout -= ttick;
            stick = ctick;
        }
    }
    return 0;
}


static DWORD
sel_ticks(DWORD stick, DWORD etick)
{
    if (etick >= stick) {                       // normal case
        return (etick - stick);
    }
    return (0xffffffff - stick) + etick;        // ticks has wrapped
}


static void
sel_console(Select_t *selfd)
{
    INPUT_RECORD k;
    DWORD count;
    HANDLE h = selfd->s_handle;

    if (selfd->s_wanted & T_WRITE)
        selfd->s_avail |= T_WRITE;

    if (selfd->s_wanted & T_READ)
#if defined(USE_UNICODE)
        while (PeekConsoleInputW(h, &k, 1, &count) && count) {
#else
        while (PeekConsoleInputA(h, &k, 1, &count) && count) {
#endif
            if (k.EventType == KEY_EVENT) {
                if (k.Event.KeyEvent.bKeyDown) {
                    selfd->s_avail |= T_READ;
                    break;
                }
            }

#if defined(USE_UNICODE)
	    (void) ReadConsoleInputW(h, &k, 1, &count);
#else
	    (void) ReadConsoleInputA(h, &k, 1, &count);
#endif
        }
}


static void
sel_block(Select_t *selfd)
{
    __PUNUSED(selfd)
    assert(0);                                  // TODO
}


static void
sel_pipe(Select_t *selfd)
{
    __PUNUSED(selfd)
    assert(0);                                  // TODO
}


static void
sel_socket(Select_t *selfd)
{
    struct timeval tmval;
    struct fd_set rfds, wfds, efds;
    SOCKET socket = (SOCKET) selfd->s_handle;
    BYTE wanted = selfd->s_wanted;

    tmval.tv_sec = 0;
    tmval.tv_usec = 0;                          /* XXX - nonblocking */

    FD_ZERO(&rfds); if (wanted & T_READ) FD_SET(socket, &rfds);
    FD_ZERO(&wfds); if (wanted & T_WRITE) FD_SET(socket, &wfds);
    FD_ZERO(&efds); if (wanted & T_EXCEPT) FD_SET(socket, &efds);

    if (select(1, &rfds, &wfds, &efds, &tmval) == SOCKET_ERROR) {
        ++selfd->s_error;

    } else {
        if (FD_ISSET(socket, &rfds)) selfd->s_avail |= T_READ;
        if (FD_ISSET(socket, &wfds)) selfd->s_avail |= T_WRITE;
        if (FD_ISSET(socket, &efds)) selfd->s_avail |= T_EXCEPT;
    }
}


static void
sel_unknown(Select_t *selfd)
{
    ++selfd->s_error;
}

/*end*/
