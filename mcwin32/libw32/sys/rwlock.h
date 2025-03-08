#ifndef LIBW32_SYS_RWLOCK_H_INCLUDED
#define LIBW32_SYS_RWLOCK_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_rwlock_h,"$Id: rwlock.h,v 1.8 2025/03/08 16:40:00 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <rwlock.h> implementation
 *
 * Copyright (c) 1998 - 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
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
