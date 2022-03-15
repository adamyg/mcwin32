#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_close_c,"$Id: w32_close.c,v 1.10 2022/02/17 16:04:59 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 close() system calls.
 *
 * Copyright (c) 2007, 2012 - 2022 Adam Young.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0501              /* enable xp+ features */
#endif

#include "win32_internal.h"
#include "win32_misc.h"
#include <unistd.h>


/*
//  NAME
//      close - close a file descriptor
//
//  SYNOPSIS
//
//      #include <unistd.h>
//
//      int close(int fildes);
//
//  DESCRIPTION
//
//      The close() function shall deallocate the file descriptor indicated by fildes. To
//      deallocate means to make the file descriptor available for return by subsequent
//      calls to open() or other functions that allocate file descriptors. All outstanding
//      record locks owned by the process on the file associated with the file descriptor
//      shall be removed (that is, unlocked).
//
//      If close() is interrupted by a signal that is to be caught, it shall return -1 with
//      errno set to [EINTR] and the state of fildes is unspecified. If an I/O error
//      occurred while reading from or writing to the file system during close(), it may
//      return -1 with errno set to [EIO]; if this error is returned, the state of fildes
//      is unspecified.
//
//      When all file descriptors associated with a pipe or FIFO special file are closed,
//      any data remaining in the pipe or FIFO shall be discarded.
//
//      When all file descriptors associated with an open file description have been closed,
//      the open file description shall be freed.
//
//      If the link count of the file is 0, when all file descriptors associated with the
//      file are closed, the space occupied by the file shall be freed and the file shall
//      no longer be accessible.
//
//      If a STREAMS-based fildes is closed and the calling process was previously
//      registered to receive a SIGPOLL signal for events associated with that STREAM, the
//      calling process shall be unregistered for events associated with the STREAM. The
//      last close() for a STREAM shall cause the STREAM associated with fildes to be
//      dismantled. If O_NONBLOCK is not set and there have been no signals posted for the
//      STREAM, and if there is data on the module's write queue, close() shall wait for an
//      unspecified time (for each module and driver) for any output to drain before
//      dismantling the STREAM. The time delay can be changed via an I_SETCLTIME ioctl()
//      request. If the O_NONBLOCK flag is set, or if there are any pending signals,
//      close() shall not wait for output to drain, and shall dismantle the STREAM
//      immediately.
//
//      If the implementation supports STREAMS-based pipes, and fildes is associated with
//      one end of a pipe, the last close() shall cause a hangup to occur on the other end
//      of the pipe. In addition, if the other end of the pipe has been named by fattach(),
//      then the last close() shall force the named end to be detached by fdetach(). If the
//      named end has no open file descriptors associated with it and gets detached, the
//      STREAM associated with that end shall also be dismantled. [Option End]
//
//      If fildes refers to the master side of a pseudo-terminal, and this is the last
//      close, a SIGHUP signal shall be sent to the controlling process, if any, for which
//      the slave side of the pseudo-terminal is the controlling terminal. It is
//      unspecified whether closing the master side of the pseudo-terminal flushes all
//      queued input and output. [Option End]
//
//      If fildes refers to the slave side of a STREAMS-based pseudo-terminal, a
//      zero-length message may be sent to the master. [Option End]
//
//      When there is an outstanding cancelable asynchronous I/O operation against fildes
//      when close() is called, that I/O operation may be canceled. An I/O operation that
//      is not canceled completes as if the close() operation had not yet occurred. All
//      operations that are not canceled shall complete as if the close() blocked until the
//      operations completed. The close() operation itself need not block awaiting such I/O
//      completion. Whether any I/O operation is canceled, and which I/O operation may be
//      canceled upon close(), is implementation-defined. [Option End]
//
//      If a shared memory object or a memory mapped file remains referenced at the last
//      close (that is, a process has it mapped), then the entire contents of the memory
//      object shall persist until the memory object becomes unreferenced. If this is the
//      last close of a shared memory object or a memory mapped file and the close results
//      in the memory object becoming unreferenced, and the memory object has been unlinked,
//      then the memory object shall be removed. [Option End]
//
//      If fildes refers to a socket, close() shall cause the socket to be destroyed. If
//      the socket is in connection-mode, and the SO_LINGER option is set for the socket
//      with non-zero linger time, and the socket has untransmitted data, then close()
//      shall block for up to the current linger interval until all data is transmitted.
//
//  RETURN VALUE
//
//      Upon successful completion, 0 shall be returned; otherwise, -1 shall be returned
//      and errno set to indicate the error.
//
//  ERRORS
//
//      The close() function shall fail if:
//
//      [EBADF]
//          The fildes argument is not a valid file descriptor.
//
//      [EINTR]
//          The close() function was interrupted by a signal.
//
//      The close() function may fail if:
//
//      [EIO]
//          An I/O error occurred while reading from or writing to the file system.
*/
LIBW32_API int
w32_close(int fildes)
{
    SOCKET s;
    int ret;

    if (fildes < 0) {
        errno = EBADF;
        ret = -1;
   } else if (w32_issockfd(fildes, &s)) {
        //
        //  To ensure that all data is sent and received on a connected socket
        //  before it is closed, an application should use shutdown to close the
        //  connection before calling closesocket.
        //
        //  Note:   The shutdown function does not block regardless of the
        //          SO_LINGER setting on the socket.
        //
#ifndef SD_BOTH
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02
#endif
        w32_sockfd_close(fildes, s);
        (void) shutdown(s, SD_BOTH);
        if ((ret = closesocket(s)) == SOCKET_ERROR) {
            w32_neterrno_set();
            ret = -1;
        }
    } else {
        ret = _close(fildes);
    }
    return ret;
}

/*end*/
