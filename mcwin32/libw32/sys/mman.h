#ifndef LIBW32_SYS_MMAN_H_INCLUDED
#define LIBW32_SYS_MMAN_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_mman_h,"$Id: mman.h,v 1.13 2025/03/08 16:40:00 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 mmap implementation
 * Copyright (c) 2012 - 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
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

#include <sys/utypes.h>                         /* off_t */
#include <stddef.h>                             /* size_t */

/*
 * Protections are chosen from these bits, or-ed together.
 * Note - not all implementations literally provide all possible
 * combinations.  PROT_WRITE is often implemented as (PROT_READ |
 * PROT_WRITE) and (PROT_EXECUTE as PROT_READ | PROT_EXECUTE).
 * However, no implementation will permit a write to succeed
 * where PROT_WRITE has not been set.  Also, no implementation will
 * allow any access to succeed where prot is specified as PROT_NONE.
 */

#define PROT_READ       0x1                     /* pages can be read */
#define PROT_WRITE      0x2                     /* pages can be written */
#define PROT_EXEC       0x4                     /* pages can be executed */
#define PROT_NONE       0x0                     /* pages cannot be accessed */

/* sharing types:  must choose either SHARED or PRIVATE */
#define MAP_SHARED      1                       /* share changes */
#define MAP_PRIVATE     2                       /* changes are private */
#define MAP_TYPE        0xf                     /* mask for share type */

/* mapping type */
#define MAP_FILE        0                       /* regular file */

/* other flags to mmap (or-ed in to MAP_SHARED or MAP_PRIVATE) */
#define MAP_FIXED       0x10                    /* user assigns address */
#define MAP_ANONYMOUS   0x20                    /* allocated from memory, swap space */
#define MAP_ANON        MAP_ANONYMOUS
#define MAP_NORESERVE   0x40                    /* don't reserve needed swap area */

/* return value on failure */
#if !defined (MAP_FAILED)                       /* Failure return value. */
#define MAP_FAILED      ((void *) -1)
#endif

/* flags to msync */
#define MS_SYNC         0x4                     /* wait for msync */
#define MS_ASYNC        0x1                     /* return immediately */
#define MS_INVALIDATE   0x2                     /* invalidate caches */

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */

__BEGIN_DECLS

LIBW32_API void *       mmap __P((void *addr, size_t len, int prot, int flags, int fildes, off_t off));
LIBW32_API int          mprotect __P((void *addr, size_t len, int prot));
LIBW32_API int          msync __P((void *addr, size_t len, int flags));
LIBW32_API int          munmap __P((void *addr, size_t len));
LIBW32_API int          mlock __P((const void *, size_t));
LIBW32_API int          munlock __P((const void *, size_t));

__END_DECLS

#endif /*LIBW32_SYS_MMAN_H_INCLUDED*/
