#ifndef LIBW32_SYS_RWLOCK_H_INCLUDED
#define LIBW32_SYS_RWLOCK_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_rwlock_h,"$Id: rwlock.h,v 1.6 2024/01/16 15:17:52 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <rwlock.h> implementation
 *
 * Copyright (c) 1998 - 2024, Adam Young.
 * All rights reserved.
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
 */

#include <sys/cdefs.h>

typedef struct rwlock {
#if defined(_WIN64)
    unsigned char       opaque[128];
#else
    unsigned char       opaque[64];
#endif
} rwlock_t;

#define RWLOCK_INITIALIZER      {0xffff}

__BEGIN_DECLS

LIBW32_API void         rwlock_init(struct rwlock *rw);
LIBW32_API void         rwlock_destroy(struct rwlock *rw);
LIBW32_API void         rwlock_rdlock(struct rwlock *rw);
LIBW32_API void         rwlock_wrlock(struct rwlock *rw);
LIBW32_API void         rwlock_rdunlock(struct rwlock *rw);
LIBW32_API void         rwlock_wrunlock(struct rwlock *rw);
LIBW32_API void         rwlock_unlock(struct rwlock *rw);
LIBW32_API int          rwlock_status(struct rwlock *rw);

__END_DECLS

#endif /*WIN32_SYS_RWLOCK_H_INCLUDED*/
