#ifndef WIN32_SYS_UTYPES_H_INCLUDED
#define WIN32_SYS_UTYPES_H_INCLUDED
/* -*- mode: c; tabs: 4 -*- */
/*
 *  win32 unix types
 *
 * ==end==
 */

#if defined(_MSC_VER)
#if (_MSC_VER != 1200)                          /* MSVC 6 */
#if (_MSC_VER != 1400)                          /* MSVC 8/2005 */
#if (_MSC_VER != 1600)                          /* MSVC 10/2010 */
#error utypes.h: untested MSVC c/c++ Version (CL 12.xx - 16.xx) only ...
#endif
#endif
#endif
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
#if !defined(_BSDTYPES_DEFINED)
#undef  u_char
typedef unsigned char   u_char;                 /* BSD compatibility */
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;
#define _BSDTYPES_DEFINED                       /* winsock[2].h and others */
#endif

typedef unsigned char   uchar;                  /* Sys V compatibility */
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;
#endif

#if !defined(HAVE_STDINT_H)
/* [u]int8_t, [u]int16_t, [u]int32_t optional [u]int64_t */
#if defined(_MSC_VER)
typedef signed char     int8_t;
typedef unsigned char   uint8_t;
#if (_MSC_VER < 1600)
typedef signed short    int16_t;
typedef unsigned short  uint16_t;
typedef signed long     int32_t;
typedef unsigned long   uint32_t;
typedef signed long     int32_t;
#if defined(HAVE_LONG_LONG_INT)
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#endif
#endif
#endif
#endif

typedef int pid_t;

#if defined(_MSC_VER)
typedef int uid_t;
typedef int gid_t;
typedef int uid_t;
typedef int gid_t;
typedef int ssize_t;
typedef int id_t;                               /* used as a general identifier; can contain least a pid_t, uid_t, or gid_t. */

typedef unsigned short mode_t;
#endif

typedef long suseconds_t;                       /* sys/types.h */


#if defined(BSD_SOURCE)
typedef char *          caddr_t;                /* core address */
typedef long            daddr_t;                /* disk address */
typedef unsigned long   fixpt_t;                /* fixed point number */
#endif  /*BSD_SOURCE*/

typedef unsigned        nlink_t;                /* link count */


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

#endif  /*WIN32_SYS_UTYPES_H_INCLUDED*/
