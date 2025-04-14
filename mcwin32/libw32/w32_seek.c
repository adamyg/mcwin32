#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_seek_c, "$Id: w32_seek.c,v 1.1 2025/03/30 17:16:03 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 _LARGEFILE64_SOURCE support
 *
 *  lseek64, tell64, filelength64, fseek64, ftell64, fgetpos64
 *
 * Copyright (c) 2024 - 2025 Adam Young.
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

#if !defined(_LARGEFILE64_SOURCE)
#define _LARGEFILE64_SOURCE
#endif

#include "win32_internal.h"
#include "win32_io.h"
#include <unistd.h>

/*
//  NAME
//
//      lseek64 - move the read/write file offset
//
//  SYNOPSIS
//
//      #define _LARGEFILE64_SOURCE
//      #include <unistd.h>
//
//      off_t lseek64(int fildes, off_t offset, int whence);
//
//  DESCRIPTION
//
//      The lseek64() function shall set the file offset for the open file description
//      associated with the file descriptor fildes, as follows:
//
//          If whence is SEEK_SET, the file offset shall be set to offset bytes.
//
//          If whence is SEEK_CUR, the file offset shall be set to its current location plus offset.
//
//          If whence is SEEK_END, the file offset shall be set to the size of the file plus offset.
//
//      The symbolic constants SEEK_SET, SEEK_CUR, and SEEK_END are defined in <unistd.h>.
//
//      The behavior of lseek64() on devices which are incapable of seeking is
//      implementation-defined. The value of the file offset associated with such a
//      device is undefined.
//
//      The lseek64() function shall allow the file offset to be set beyond the end of
//      the existing data in the file. If data is later written at this point,
//      subsequent reads of data in the gap shall return bytes with the value 0 until
//      data is actually written into the gap.
//
//      The lseek64() function shall not, by itself, extend the size of a file.
//
//  RETURN VALUE
//
//      Upon successful completion, the resulting offset, as measured in bytes from the
//      beginning of the file, shall be returned. Otherwise, -1 shall be returned,
//      errno shall be set to indicate the error, and the file offset shall remain
//      unchanged.
//
//  NOTES
//
//      lseek64() is one of the functions that was specified in the Large File Summit
//      (LFS) specification that was completed in 1996. The purpose of the specification
//      was to provide transitional support that allowed applications on 32-bit systems
//      to access files whose size exceeds that which can be represented with a 32-bit
//      off_t type.
//
//      This symbol is exposed by header files if the _LARGEFILE64_SOURCE feature test
//      macro is defined.
//
//  ERRORS
//
//      The lseek64() function shall fail if:
//
//      [EBADF]
//          The fildes argument is not an open file descriptor.
//
//      [EINVAL]
//          The whence argument is not a proper value, or the resulting file offset would be
//          negative for a regular file, block special file, or directory.
//
//      [ESPIPE]
//          The fildes argument is associated with a pipe, FIFO, or socket.
*/

LIBW32_API off64_t
w32_lseek64(int fildes, off64_t offset, int whence)
{
#if (1)
    return _lseeki64(fildes, offset, whence);

#else
    HANDLE handle;
    LARGE_INTEGER liOffset, liResult;
    DWORD MoveMethod;

    unsigned int ret;
    int err;

    if ((handle = w32_osfhandle(fildes)) == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return -1;
    }

    liOffset.QuadPart = offset;
    switch (whence) {
    case SEEK_CUR:
	MoveMethod = FILE_CURRENT;
	break;
    case SEEK_SET:
	MoveMethod = FILE_BEGIN;
	break;
    case SEEK_END:
	MoveMethod = FILE_END;
	break;
    default:
        assert (0);
        errno = EINVAL;
        return ((off64_t) -1);
    }

    ret = SetFilePointerEx(handle, liOffset, &liResult, MoveMethod);
    if (ret == 0) {
        const DWORD rc = GetLastError();
        switch (rc) {
        case ERROR_INVALID_HANDLE:
            errno = EBADF;
            break;
        default:
            w32_errno_set();
            break;
        }
        return ((off64_t) -1);
    }

    return liResult.QuadPart;
#endif
}


/*
//  NAME
//
//      tell64 - return the read/write file pointer
//
//  SYNOPSIS
//
//      #define _LARGEFILE64_SOURCE
//      #include <unistd.h>
//
//      off64_t tell64 (int fildes);
//
//  DESCRIPTION
//      filedes is a file descriptor returned from a creat, open, dup, fcntl, pipe, or
//      ioctl, system call. tell and tell64 return the file pointer associated with filedes.
//
//      The two differ in that tell returns an off_t and tell64 returns an off64_t. The
//      64-bit offset returned by tell64 is useful for 32 bit applications working with 64
//      bit files. This is because the 32 bit offset returned by tell might not be large
//      enough to represent the current file offset.
//
*/

LIBW32_API off64_t
w32_tell64(int fildes)
{
    return _telli64(fildes);
}


/*
//  NAME
//
//      filelength - return the number of bytes in an open file.
//
//  SYNOPSIS
//
//      #define _LARGEFILE64_SOURCE
//      #include <unistd.h>
//
//      off64_t filelength64 (int fildes);
//
//  DESCRIPTION
//      filedes is a file descriptor returned from a creat, open, dup, fcntl, pipe, or
//      ioctl, system call. tell and tell64 return the file pointer associated with filedes.
//
//      The filelength() function returns the number of bytes in the opened file indicated
//      by the file descriptor filedes.
//
//  RETURN
//      The number of bytes in the file. If an error occurs, (-1L) is returned, and errno
//      indicates the type of error detected.
*/

LIBW32_API off64_t
w32_filelength64(int fildes)
{
    return _filelengthi64(fildes);
}


/*
//  NAME
//
//      fseeko64 - reposition a file-position indicator in a stream.
//
//  SYNOPSIS
//
//      #define _LARGEFILE64_SOURCE
//      #include <unistd.h>
//
//      int fseeko64(FILE *stream, off64_t offset, int whence);
//
//  DESCRIPTION
//      The fseek64() function shall set the file-position indicator for the stream
//      pointed to by stream. If a read or write error occurs, the error indicator for
//      the stream shall be set and fseek() fails.
//
//  RETURN VALUE
//      Upon successful completion, the resulting offset, as measured in bytes from the
//      beginning of the file, shall be returned. Otherwise, -1 shall be returned,
//      errno shall be set to indicate the error, and the file offset shall remain
//      unchanged.
*/

LIBW32_API off64_t
w32_fseeko64 (FILE *stream, off64_t offset, int whence)
{
    return _fseeki64(stream, offset, whence);
}


/*
//  NAME
//
//      ftell64 - return the current position of a stream.
//
//  SYNOPSIS
//
//      #define _LARGEFILE64_SOURCE
//      #include <unistd.h>
//
//      off64_t ftello64(FILE *stream);
//
//  DESCRIPTION
//      The ftello64() function shall obtain the current value of the file-position indicator
//      for the stream pointed to by stream.
//
//  RETURN VALUE
//      Upon successful completion, ftell65() shall return the current value of the
//      file-position indicator for the stream measured in bytes from the beginning
//      of the file.
//
//      Otherwise, ftell64() shall return -1, cast off64_t, and set errno to indicate the error.
*/

LIBW32_API off64_t
w32_ftello64 (FILE *stream)
{
    return _ftelli64(stream);
}


/*
//  NAME
//
//      fgetpos64 - get current file position information
//
//  SYNOPSIS
//      #define _LARGEFILE64_SOURCE
//      #include <unistd.h>
//
//      int fgetpos64(FILE *stream, fpos64_t *pos);
//
//  DESCRIPTION
//      The fgetpos64() function shall not change the setting of errno if successful.
//
//  RETURN VALUE
//      Upon successful completion, fgetpos64() shall return 0; otherwise, it shall
//      return a non-zero value and set errno to indicate the error.
//
//  ERRORS
//      The fgetpos64() function shall fail if:
//
//      [EBADF]
//          The file descriptor underlying stream is not valid.
//
//      [EOVERFLOW]
//          The current value of the file position cannot be represented correctly in
//          an object of type fpos64_t.
//
//      [ESPIPE]
//          The file descriptor underlying stream is associated with a pipe, FIFO, or socket.
//
*/

LIBW32_API int
w32_fgetpos64 (FILE *stream, fpos64_t *pos)
{
    off64_t t_pos;

    if (NULL == pos) {
        errno = EFAULT;
        return -1;
    }

    if ((t_pos = _ftelli64(stream)) == (off64_t)-1) {
        return -1;
    }

    *pos = t_pos;
    return 0;
}

//end
