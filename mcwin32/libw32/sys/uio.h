#ifndef LIBW32_SYS_UIO_H_INCLUDED
#define LIBW32_SYS_UIO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_uio_h,"$Id: uio.h,v 1.4 2018/09/29 02:22:56 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 *  win32 sys/uio.h
 *
 * Copyright (c) 2012 - 2018, Adam Young.
 * All rights reserved.
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
 */

#include <sys/cdefs.h>

#include <stddef.h>         /* size_t */
#include <limits.h>         /* INT_MAX */

#define IOV_MAX             64
#define SSIZE_MAX           INT_MAX

__BEGIN_DECLS

typedef struct iovec {
    void *     iov_base;
    int        iov_len;
} iovec_t;

LIBW32_API size_t           readv(int, const struct iovec *, int);
LIBW32_API size_t           writev(int, const struct iovec *, int);

__END_DECLS

#endif /*LIBW32_SYS_UIO_H_INCLUDED */
