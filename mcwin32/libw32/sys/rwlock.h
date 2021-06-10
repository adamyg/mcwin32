#ifndef GR_SYS_RWLOCK_H_INCLUDED
#define GR_SYS_RWLOCK_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_rwlock_h,"$Id: rwlock.h,v 1.1 2021/06/10 15:45:04 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <rwlock.h> implementation
 *
 * Copyright (c) 1998 - 2018, Adam Young.
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
    unsigned int        opaque[16];
} rwlock_t;

#define RWLOCK_INITIALIZER      {0xffff}

__BEGIN_DECLS

LIBW32_API void         rwlock_init(struct rwlock *rw);
LIBW32_API void         rwlock_destroy(struct rwlock *rw);
LIBW32_API void         rwlock_rdlock(struct rwlock *rw);
LIBW32_API void         rwlock_wrlock(struct rwlock *rw);
LIBW32_API void         rwlock_rdunlock(struct rwlock *rw);
LIBW32_API void         rwlock_wrunlock(struct rwlock *rw);

__END_DECLS

#endif /*GR_SYS_RWLOCK_H_INCLUDED*/


