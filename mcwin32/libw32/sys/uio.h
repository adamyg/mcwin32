#ifndef LIBW32_SYS_UIO_H_INCLUDED
#define LIBW32_SYS_UIO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_uio_h,"$Id: uio.h,v 1.8 2022/06/14 02:19:59 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 sys/uio.h
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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

#include <sys/cdefs.h>

#include <stddef.h>         /* size_t */
#include <limits.h>         /* INT_MAX */

#define IOV_MAX             64
#if !defined(__MINGW32__)
#if !defined(SSIZE_MAX)
#define SSIZE_MAX           INT_MAX
#endif
#endif

__BEGIN_DECLS

struct iovec {
    void *     iov_base;
    int        iov_len;
};

LIBW32_API int /*ssize_t*/  readv(int, const struct iovec *, int);
LIBW32_API int /*ssize_t*/  writev(int, const struct iovec *, int);

//  LIBW32_API ssize_t      preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset);
//  LIBW32_API ssize_t      pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset);
//  LIBW32_API ssize_t      preadv2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags);
//  LIBW32_API ssize_t      pwritev2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags);

__END_DECLS

#endif /*LIBW32_SYS_UIO_H_INCLUDED*/
