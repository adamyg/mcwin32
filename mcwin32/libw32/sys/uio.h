#ifndef WIN32_SYS_UIO_H_INCLUDED
#define WIN32_SYS_UIO_H_INCLUDED
/*
 *  win32 sys/uio.h
 *
 * Copyright (c) 2012 - 2017, Adam Young.
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

#endif /*WIN32_SYS_UIO_H_INCLUDED */
