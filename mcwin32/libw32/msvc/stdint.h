#ifndef WIN32_STDINT_H_INCLUDED
#define WIN32_STDINT_H_INCLUDED
/* -*- mode: c; tabs: 4 -*- */
/*
 *  Visual C/C++ compat header file (integer types)
 *
 */

#if defined(_MSC_VER)
#if (_MSC_VER != 1200)                          /* MSVC 6 */
#if (_MSC_VER != 1400)                          /* MSVC 8/2005 */
#if (_MSC_VER != 1600)                          /* MSVC 10/2010 */
#error stdint.h: Untested MSVC C/C++ Version (CL 12.xx - 16.xx) only ...
#endif
#endif
#endif
#pragma warning(disable:4115)

#elif defined(__WATCOMC__)
#if (__WATCOMC__ < 1200)
#error stdint.h: Old WATCOM Version, upgrade to OpenWatcom ...
#endif

#else
#error stdint.h: Unknown compiler
#endif

#include <sys/types.h>                          /* System types */
#include <limits.h>                             /* System limits */

/* Exact-width types. */
typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef signed short        int16_t;
typedef unsigned short      uint16_t;
#if !defined(__WATCOMC__)
typedef signed long         int32_t;
typedef unsigned long       uint32_t;
#endif
typedef long long           int64_t;
typedef unsigned long long  uint64_t;

/* Minimum-width types. */
typedef signed   char       int_least8_t;
typedef signed   short      int_least16_t;
typedef signed   long       int_least32_t;
typedef signed   long long  int_least64_t;

typedef unsigned char       uint_least8_t;
typedef unsigned short      uint_least16_t;
typedef unsigned long       uint_least32_t;
typedef unsigned long long  uint_least64_t;

/* Fastest minimum-width types. */
typedef signed   int        int_fast8_t;
typedef signed   int        int_fast16_t;
typedef signed   long       int_fast32_t;
typedef signed   long long  int_fast64_t;

typedef unsigned int        uint_fast8_t;
typedef unsigned int        uint_fast16_t;
typedef unsigned long       uint_fast32_t;
typedef unsigned long long  uint_fast64_t;

/* Integer types able to hold *object* pointers. */
// typedef long             intptr_t;
// typedef unsigned long    uintptr_t;

/* Greatest-width types. */
typedef long long           intmax_t;
typedef unsigned long long  uintmax_t;

#endif  /*WIN32_STDINT_H_INCLUDED*/
