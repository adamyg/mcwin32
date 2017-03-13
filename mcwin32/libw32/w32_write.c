/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 write() system calls.
 *
 * Copyright (c) 2007, 2012 - 2017 Adam Young.
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
#define WIN32_WINNT        0x0501              /* enable xp+ features */
#endif

#include "win32_internal.h"
#include "win32_misc.h"
#include <unistd.h>

#pragma comment(lib, "Ws2_32.lib")

/*
//  NAME
//  
//      pwrite, write - write on a file
//  
//  SYNOPSIS
//  
//      #include <unistd.h>
//  
//      ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset); [Option End]
//  
//      ssize_t write(int fildes, const void *buf, size_t nbyte);
//  
//  DESCRIPTION
//  
//      The write() function shall attempt to write nbyte bytes from the buffer pointed to
//      by buf to the file associated with the open file descriptor, fildes.
//  
//      Before any action described below is taken, and if nbyte is zero and the file is a
//      regular file, the write() function may detect and return errors as described below.
//      In the absence of errors, or if error detection is not performed, the write()
//      function shall return zero and have no other results. If nbyte is zero and the file
//      is not a regular file, the results are unspecified.
//  
//      On a regular file or other file capable of seeking, the actual writing of data
//      shall proceed from the position in the file indicated by the file offset associated
//      with fildes. Before successful return from write(), the file offset shall be
//      incremented by the number of bytes actually written. On a regular file, if this
//      incremented file offset is greater than the length of the file, the length of the
//      file shall be set to this file offset.
//  
//      On a file not capable of seeking, writing shall always take place starting at the
//      current position. The value of a file offset associated with such a device is
//      undefined.
//  
//      If the O_APPEND flag of the file status flags is set, the file offset shall be set
//      to the end of the file prior to each write and no intervening file modification
//      operation shall occur between changing the file offset and the write operation.
//  
//      If a write() requests that more bytes be written than there is room for (for
//      example, [XSI] [Option Start] the process' file size limit or [Option End] the
//      physical end of a medium), only as many bytes as there is room for shall be
//      written. For example, suppose there is space for 20 bytes more in a file before
//      reaching a limit. A write of 512 bytes will return 20. The next write of a non-zero
//      number of bytes would give a failure return (except as noted below).
//  
//      If the request would cause the file size to exceed the soft file size limit for the
//      process and there is no room for any bytes to be written, the request shall fail
//      and the implementation shall generate the SIGXFSZ signal for the thread. [Option End]
//  
//      If write() is interrupted by a signal before it writes any data, it shall return -1
//      with errno set to [EINTR].
//  
//      If write() is interrupted by a signal after it successfully writes some data, it
//      shall return the number of bytes written.
//  
//      If the value of nbyte is greater than {SSIZE_MAX}, the result is
//      implementation-defined.
//  
//      After a write() to a regular file has successfully returned:
//  
//          Any successful read() from each byte position in the file that was modified by
//          that write shall return the data specified by the write() for that position
//          until such byte positions are again modified.
//  
//          Any subsequent successful write() to the same byte position in the file shall
//          overwrite that file data.
//  
//      Write requests to a pipe or FIFO shall be handled in the same way as a regular file
//      with the following exceptions:
//  
//          There is no file offset associated with a pipe, hence each write request shall
//          append to the end of the pipe.
//  
//          Write requests of {PIPE_BUF} bytes or less shall not be interleaved with data
//          from other processes doing writes on the same pipe. Writes of greater than
//          {PIPE_BUF} bytes may have data interleaved, on arbitrary boundaries, with
//          writes by other processes, whether or not the O_NONBLOCK flag of the file
//          status flags is set.
//  
//          If the O_NONBLOCK flag is clear, a write request may cause the thread to block, 
//          but on normal completion it shall return nbyte.
//  
//          If the O_NONBLOCK flag is set, write() requests shall be handled differently, 
//          in the following ways:
//  
//              The write() function shall not block the thread.
//  
//              A write request for {PIPE_BUF} or fewer bytes shall have the following
//              effect: if there is sufficient space available in the pipe, write() shall
//              transfer all the data and return the number of bytes requested. Otherwise, 
//              write() shall transfer no data and return -1 with errno set to [EAGAIN].
//  
//              A write request for more than {PIPE_BUF} bytes shall cause one of the
//              following:
//  
//                  When at least one byte can be written, transfer what it can and return
//                  the number of bytes written. When all data previously written to the
//                  pipe is read, it shall transfer at least {PIPE_BUF} bytes.
//  
//                  When no data can be written, transfer no data, and return -1 with errno
//                  set to [EAGAIN].
//  
//      When attempting to write to a file descriptor (other than a pipe or FIFO) that
//      supports non-blocking writes and cannot accept the data immediately:
//  
//          If the O_NONBLOCK flag is clear, write() shall block the calling thread until
//          the data can be accepted.
//  
//          If the O_NONBLOCK flag is set, write() shall not block the thread. If some data
//          can be written without blocking the thread, write() shall write what it can and
//          return the number of bytes written. Otherwise, it shall return -1 and set errno
//          to [EAGAIN].
//  
//      Upon successful completion, where nbyte is greater than 0, write() shall mark for
//      update the st_ctime and st_mtime fields of the file, and if the file is a regular
//      file, the S_ISUID and S_ISGID bits of the file mode may be cleared.
//  
//      For regular files, no data transfer shall occur past the offset maximum established
//      in the open file description associated with fildes.
//  
//      If fildes refers to a socket, write() shall be equivalent to send() with no flags
//      set.
//  
//      If the O_DSYNC bit has been set, write I/O operations on the file descriptor shall
//      complete as defined by synchronized I/O data integrity completion.
//  
//      If the O_SYNC bit has been set, write I/O operations on the file descriptor shall
//      complete as defined by synchronized I/O file integrity completion. [Option End]
//  
//      If fildes refers to a shared memory object, the result of the write() function is
//      unspecified. [Option End]
//  
//      If fildes refers to a typed memory object, the result of the write() function is
//      unspecified. [Option End]
//  
//      If fildes refers to a STREAM, the operation of write() shall be determined by the
//      values of the minimum and maximum nbyte range (packet size) accepted by the STREAM.
//      These values are determined by the topmost STREAM module. If nbyte falls within the
//      packet size range, nbyte bytes shall be written. If nbyte does not fall within the
//      range and the minimum packet size value is 0, write() shall break the buffer into
//      maximum packet size segments prior to sending the data downstream (the last segment
//      may contain less than the maximum packet size). If nbyte does not fall within the
//      range and the minimum value is non-zero, write() shall fail with errno set to
//      [ERANGE]. Writing a zero-length buffer ( nbyte is 0) to a STREAMS device sends 0
//      bytes with 0 returned. However, writing a zero-length buffer to a STREAMS-based
//      pipe or FIFO sends no message and 0 is returned. The process may issue I_SWROPT
//      ioctl() to enable zero-length messages to be sent across the pipe or FIFO.
//  
//      When writing to a STREAM, data messages are created with a priority band of 0. When
//      writing to a STREAM that is not a pipe or FIFO:
//  
//          If O_NONBLOCK is clear, and the STREAM cannot accept data (the STREAM write
//          queue is full due to internal flow control conditions), write() shall block
//          until data can be accepted.
//  
//          If O_NONBLOCK is set and the STREAM cannot accept data, write() shall return -1
//          and set errno to [EAGAIN].
//  
//          If O_NONBLOCK is set and part of the buffer has been written while a condition
//          in which the STREAM cannot accept additional data occurs, write() shall
//          terminate and return the number of bytes written.
//  
//      In addition, write() shall fail if the STREAM head has processed an asynchronous
//      error before the call. In this case, the value of errno does not reflect the result
//      of write(), but reflects the prior error. [Option End]
//  
//      The pwrite() function shall be equivalent to write(), except that it writes into a
//      given position without changing the file pointer. The first three arguments to
//      pwrite() are the same as write() with the addition of a fourth argument offset for
//      the desired position inside the file. [Option End]
//  
//  RETURN VALUE
//  
//      Upon successful completion, write() [XSI] [Option Start] and pwrite() [Option End]
//      shall return the number of bytes actually written to the file associated with
//      fildes. This number shall never be greater than nbyte. Otherwise, -1 shall be
//      returned and errno set to indicate the error.
//  
//  ERRORS
//  
//      The write() and [XSI] [Option Start] pwrite() [Option End] functions shall fail if:
//  
//      [EAGAIN]
//          The O_NONBLOCK flag is set for the file descriptor and the thread would be
//          delayed in the write() operation.
//  
//      [EBADF]
//          The fildes argument is not a valid file descriptor open for writing.
//  
//      [EFBIG]
//          An attempt was made to write a file that exceeds the implementation-defined
//          maximum file size [XSI] [Option Start] or the process' file size limit, [Option
//          End] and there was no room for any bytes to be written.
//  
//      [EFBIG]
//          The file is a regular file, nbyte is greater than 0, and the starting position
//          is greater than or equal to the offset maximum established in the open file
//          description associated with fildes.
//  
//      [EINTR]
//          The write operation was terminated due to the receipt of a signal, and no data
//          was transferred.
//  
//      [EIO]
//          The process is a member of a background process group attempting to write to
//          its controlling terminal, TOSTOP is set, the process is neither ignoring nor
//          blocking SIGTTOU, and the process group of the process is orphaned. This error
//          may also be returned under implementation-defined conditions.
//  
//      [ENOSPC]
//          There was no free space remaining on the device containing the file.
//  
//      [EPIPE]
//          An attempt is made to write to a pipe or FIFO that is not open for reading by
//          any process, or that only has one end open. A SIGPIPE signal shall also be sent
//          to the thread.
//  
//      [ERANGE]
//          The transfer request size was outside the range supported by the STREAMS file
//          associated with fildes. [Option End]
//  
//      The write() function shall fail if:
//  
//      [EAGAIN] or [EWOULDBLOCK]
//          The file descriptor is for a socket, is marked O_NONBLOCK, and write would block.
//  
//      [ECONNRESET]
//          A write was attempted on a socket that is not connected.
//  
//      [EPIPE]
//          A write was attempted on a socket that is shut down for writing, or is no
//          longer connected. In the latter case, if the socket is of type SOCK_STREAM, a
//          SIGPIPE signal shall also be sent to the thread.
//  
//      The write() and [XSI] [Option Start] pwrite() [Option End] functions may fail if:
//  
//      [EINVAL]
//          The STREAM or multiplexer referenced by fildes is linked (directly or
//          indirectly) downstream from a multiplexer. [Option End]
//  
//      [EIO]
//          A physical I/O error has occurred.
//  
//      [ENOBUFS]
//          Insufficient resources were available in the system to perform the operation.
//  
//      [ENXIO]
//          A request was made of a nonexistent device, or the request was outside the
//          capabilities of the device.
//  
//      [ENXIO]
//          A hangup occurred on the STREAM being written to. [Option End]
//  
//      A write to a STREAMS file may fail if an error message has been received at the
//      STREAM head. In this case, errno is set to the value included in the error message.
//      [Option End]
//  
//      The write() function may fail if:
//  
//      [EACCES]
//          A write was attempted on a socket and the calling process does not have
//          appropriate privileges.
//  
//      [ENETDOWN]
//          A write was attempted on a socket and the local network interface used to reach
//          the destination is down.
//  
//      [ENETUNREACH]
//          A write was attempted on a socket and no route to the network is present.
//  
//      The pwrite() function shall fail and the file pointer remain unchanged if: [Option End]
//  
//      [EINVAL]
//          The offset argument is invalid. The value is negative. [Option End]
//  
//      [ESPIPE]
//          The fildes is associated with a pipe or FIFO. [Option End]
*/

int
w32_write(int fildes, const void *buffer, size_t nbyte)
{
    SOCKET s = -1;
    int ret;

    if (fildes < 0) {
        errno = EBADF;
        ret = -1;
	} else if (w32_issockfd(fildes, &s)) {
        if ((ret = sendto(s, buffer, (int)nbyte, 0, NULL, 0)) == SOCKET_ERROR) {
            w32_neterrno_set();
            ret = -1;
        }
    } else {
        ret = _write(fildes, buffer, (int)nbyte);
    }
    return ret;
}


