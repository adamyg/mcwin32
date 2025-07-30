#ifndef GR_ENDIAN_H_INCLUDED
#define GR_ENDIAN_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_endian_h,"$Id: endian.h,v 1.8 2025/07/20 07:03:17 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*-
 *
 * win32 <sys/endian.h> implementation
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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

#include <sys/types.h>
#include <stdint.h>

#define _LITTLE_ENDIAN      1234
#define _BIG_ENDIAN         4321
#if defined(_BSD_SOURCE)
#define LITTLE_ENDIAN       _LITTLE_ENDIAN
#define BIG_ENDIAN          _BIG_ENDIAN
#endif

#if !defined(__BYTE_ORDER)
#if defined(IS_LITTLE_ENDIAN)
#define __BYTE_ORDER        _LITTLE_ENDIAN
#elif defined(IS_BIG_ENDIAN)
#define __BYTE_ORDER        _BIG_ENDIAN
#else
#   if defined(_M_IA64) || defined(_M_AMD64) // 64-bit Intel/AMD
#       define IS_LITTLE_ENDIAN 1
#   elif defined(_M_IX86)   // 32-bit Intel
#       define IS_LITTLE_ENDIAN 1
#   elif defined(_M_MRX000)
#       define IS_LITTLE_ENDIAN 1
#   elif defined(_M_ALPHA)
#       define IS_LITTLE_ENDIAN 1
#   elif defined(_M_PPC)
#       define IS_LITTLE_ENDIAN 1
#   else
#       error unknown endian
#   endif
#endif
#endif /*__BYTE_ORDER*/
#if defined(_BSD_SOURCE)
#define BYTE_ORDER          __BYTE_ORDER
#endif

#if defined(_MSC_VER)
#pragma warning(disable:4514)                   /* unreferenced inline function has been removed */
#define __ENDIAN_INLINE__ __inline
#else
#define __ENDIAN_INLINE__ inline
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
/*
 *  big-endian
 */
#ifndef HTONS
#define HTONS(__x)          (__x)
#define NTOHS(__x)          (__x)
#define HTONL(__x)          (__x)
#define NTOHL(__x)          (__x)
#endif

#define be16toh(__x)        ((uint16_t)(__x))
#define htobe16(__x)        ((uint16_t)(__x))
#define be32toh(__x)        ((uint32_t)(__x))
#define htobe32(__x)        ((uint32_t)(__x))
#define be64toh(__x)        ((uint64_t)(__x))
#define htobe64(__x)        ((uint64_t)(__x))

#else
/*
 *  little-endian
 */
#ifndef HTONS
#define HTONS(__x)          (((((unsigned short)(__x) & 0xFF)) << 8) | \
                              (((unsigned short)(__x) & 0xFF00) >> 8))

#define NTOHS(__x)          (((((unsigned short)(__x) & 0xFF)) << 8) | \
                              (((unsigned short)(__x) & 0xFF00) >> 8))

#define HTONL(__x)          (((((unsigned long)(__x)  & 0xFF)) << 24) | \
                             ((((unsigned long)(__x)  & 0xFF00)) << 8) | \
                             ((((unsigned long)(__x)  & 0xFF0000)) >> 8) | \
                             ((((unsigned long)(__x)  & 0xFF000000)) >> 24))

#define NTOHL(__x)          (((((unsigned long)(__x)  & 0xFF)) << 24) | \
                             ((((unsigned long)(__x)  & 0xFF00)) << 8) | \
                             ((((unsigned long)(__x)  & 0xFF0000)) >> 8) | \
                             ((((unsigned long)(__x)  & 0xFF000000)) >> 24))
#endif

#define __HTON16(__x)       (((((uint16_t)(__x) & 0xFF)) << 8) | \
                              (((uint16_t)(__x) & 0xFF00) >> 8))

#define __NTOH16(__x)       (((((uint16_t)(__x) & 0xFF)) << 8) | \
                              (((uint16_t)(__x) & 0xFF00) >> 8))

#define __HTON32(__x)       (((((uint32_t)(__x) & 0xFF)) << 24) | \
                             ((((uint32_t)(__x) & 0xFF00)) << 8) | \
                             ((((uint32_t)(__x) & 0xFF0000)) >> 8) | \
                             ((((uint32_t)(__x) & 0xFF000000)) >> 24))

#define __NTOH32(__x)       (((((uint32_t)(__x) & 0xFF)) << 24) | \
                             ((((uint32_t)(__x) & 0xFF00)) << 8) | \
                             ((((uint32_t)(__x) & 0xFF0000)) >> 8) | \
                             ((((uint32_t)(__x) & 0xFF000000)) >> 24))

#define be16toh(__x)        __NTOH16(__x)
#define htobe16(__x)        __HTON16(__x)
#define be32toh(__x)        __NTOH32(__x)
#define htobe32(__x)        __HTON32(__x)
static __ENDIAN_INLINE__ uint64_t be64toh(uint64_t __x);
static __ENDIAN_INLINE__ uint64_t be64toh(uint64_t __x) {
    return (((uint64_t)be32toh(__x & (uint64_t)0xFFFFFFFFULL)) << 32) |
                ((uint64_t)be32toh((__x & (uint64_t)0xFFFFFFFF00000000ULL) >> 32));
}
static __ENDIAN_INLINE__ uint64_t htobe64(uint64_t __x);
static __ENDIAN_INLINE__ uint64_t htobe64(uint64_t __x) {
    return (((uint64_t)htobe32(__x & (uint64_t)0xFFFFFFFFULL)) << 32) |
                ((uint64_t)htobe32((__x & (uint64_t)0xFFFFFFFF00000000ULL) >> 32));
}

#endif /*LITTLE-ENDIAN*/

#if defined(WIN32_XTOX_INLINE)
#if defined(__ENDIAN_INLINE__)
static __ENDIAN_INLINE__ unsigned short htons(unsigned short);
static __ENDIAN_INLINE__ unsigned short htons(unsigned short __v) {
    return HTONS(__v);
}
static __ENDIAN_INLINE__ unsigned short ntohs(unsigned short);
static __ENDIAN_INLINE__ unsigned short ntohs(unsigned short __v) {
    return NTOHS(__v);
}
static __ENDIAN_INLINE__ unsigned long  htonl(unsigned long);
static __ENDIAN_INLINE__ unsigned long  htonl(unsigned long __v) {
    return HTONL(__v);
}
static __ENDIAN_INLINE__ unsigned long  ntohl(unsigned long);
static __ENDIAN_INLINE__ unsigned long  ntohl(unsigned long __v) {
    return NTOHL(__v);
}
#else
#define htons(__v)          HTONS(__v)
#define ntohs(__v)          NTOHS(__v)
#define htonl(__v)          HTONL(__v)
#define ntosl(__v)          NTOHL(__v)
#endif
#endif /*WIN32_XTOX_INLINE*/

#undef __ENDIAN_INLINE__

#endif /*GR_ENDIAN_H_INCLUDED*/
