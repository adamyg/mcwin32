#ifndef LIBW32_SYS_UTYPES_H_INCLUDED
#define LIBW32_SYS_UTYPES_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_utypes_h,"$Id: utypes.h,v 1.20 2022/06/14 02:19:59 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* 
 * win32 unix types
 *
 * Copyright (c) 1998 - 2022, Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
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

#if defined(_MSC_VER)
#pragma warning(disable:4115)

#elif defined(__WATCOMC__)
#if (__WATCOMC__ < 1200)
#error utypes.h: old WATCOM Version, upgrade to OpenWatcom ...
#endif

#elif defined(__MINGW32__)

#else
#error utypes.h: Unknown compiler
#endif

#include <sys/types.h>                          /* System types */

#if !defined(_POSIX_SOURCE) && \
        !defined(_UNIXTYPES_T_DEFINED) && \
        !defined(u_char)
#define _UNIXTYPES_T_DEFINED
#if defined(_BSD_SOURCE)
#if !defined(_BSDTYPES_DEFINED)
typedef unsigned char u_char;                   /* BSD compatibility */
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
#define _BSDTYPES_DEFINED                       /* winsock[2].h and others */
#endif /*_BSDTYPES_DEFINED*/
#endif /*_BSD_SOURCE*/
typedef unsigned char uchar;                    /* Sys V compatibility */
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
#endif

/* [u]int8_t, [u]int16_t, [u]int32_t optional [u]int64_t */
#if defined(HAVE_STDINT_H)
#include <stdint.h>
#else
#if defined(_MSC_VER) && !defined(_MSC_STDINT_H_TYPES)
#if (_MSC_VER < 1300)
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#else
typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
#endif  /*1300*/
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#if (_MSC_VER < 1800)
typedef uint8_t uint_fast8_t;                   /* optional C11 */
typedef uint16_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;
#endif
#define _MSC_STDINT_H_TYPES
#endif  /*_MSC_STDINT_H_TYPES*/
#endif  /*stdint.h*/

#if defined(_BSD_SOURCE)
#ifndef u_int64_t
#define u_int64_t uint64_t
#endif
#ifndef u_int32_t
#define u_int32_t uint32_t
#endif
#ifndef u_int16_t
#define u_int16_t uint16_t
#endif
#ifndef u_int8_t
#define u_int8_t uint8_t
#define __uint8_t uint8_t
#endif
typedef char *caddr_t;                          /* core address */
typedef long daddr_t;                           /* disk address */
typedef unsigned long fixpt_t;                  /* fixed point number */
#endif  /*BSD_SOURCE*/

/* system identifiers */
/* Note: MSDN - Interprocess Communication Between 32-bit and 64-bit Applications
 *  64-bit versions of Windows use 32-bit handles for interoperability.
 *  When sharing a handle between 32-bit and 64-bit applications, only the lower 32 bits are significant, 
 *  so it is safe to truncate the handle (when passing it from 64-bit to 32-bit) or sign-extend the handle (when passing it from 32-bit to 64-bit).
 *  Handles that can be shared include handles to user objects such as windows (HWND), handles to GDI objects such as pens and brushes (HBRUSH and HPEN),
 *  and handles to named objects such as mutexes, semaphores, and file handles.
 */
#if !defined(HAVE_PID_T)
#if defined(_MSC_VER) || \
        (defined(__WATCOMC__) && (__WATCOMC__ < 1300 /*owc20*/)) || \
        (defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR))
typedef int pid_t;                              /* process identifier */
#endif
#define HAVE_PID_T 1
#endif

#if !defined(__MINGW32__) || \
        (defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR))
#if !defined(USECONDS_T)
#define USECONDS_T 1
#ifdef _WIN64
typedef unsigned long long useconds_t;
#else
typedef unsigned long useconds_t;
#endif
#endif /*USECONDS_T*/
#endif /*__MINHW32__*/

#ifdef _WIN64
typedef long long suseconds_t;
#else
typedef long suseconds_t;
#endif

#if defined(_MSC_VER) && \
        !defined(__WATCOMC__)
                /* check for !WATCOMC, at times we masquerade WC as MSVC */
#if !defined(uid_t) && !defined(gid_t)
typedef int uid_t;
typedef int gid_t;
#endif
#if !defined(id_t)
typedef int id_t;                               /* used as a general identifier; can contain least a pid_t, uid_t, or gid_t. */
#endif

#if !defined(ssize_t) && !defined(_SSIZE_T_DEFINED)
#define _SSIZE_T_DEFINED_ 1
#ifdef _WIN64
typedef __int64 ssize_t;
#else
typedef signed ssize_t;
#endif
#define ssize_t ssize_t
#endif

#if !defined(mode_t)
typedef unsigned short mode_t;
#define mode_t mode_t
#endif

#elif defined(__MINGW32__)
#if !defined(uid_t) && !defined(gid_t)
typedef int uid_t;
typedef int gid_t;
#endif
#if !defined(id_t)
typedef int id_t;                               /* used as a general identifier; can contain least a pid_t, uid_t, or gid_t. */
#endif

#endif /*_MSC_VER || __MINGW32__*/

#if !defined(HAVE_NLINK_T)
#if !defined(__WATCOMC__) || \
        (defined(__WATCOMC__) && (__WATCOMC__ < 1300 /*owc20*/))
typedef unsigned nlink_t;                       /* link count */
#endif
#define HAVE_NLINK_T
#endif

#ifndef major
#define major(devnum)   (((devnum) >> 8) & 0xff)
#endif
#ifndef minor
#define minor(devnum)   (((devnum) & 0xff))
#endif
#ifndef makedev
#define makedev(major,minor) \
                        ((((major) & 0xff) << 8) | ((minor) & 0xff))
#endif

#endif /*LIBW32_SYS_UTYPES_H_INCLUDED*/

