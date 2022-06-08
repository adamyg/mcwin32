/*
 *  libregexp config.h
 */

#if !defined(_BSD_SOURCE)
#define _BSD_SOURCE
#endif

#include "../config.h"

#include <stdio.h>                      /* snprintf */
#include <math.h>

#if defined(_WIN32)
#if !defined(_POSIX2_RE_DUP_MAX)
#define _POSIX2_RE_DUP_MAX              255
#endif
#endif

#undef INFINITY                         /*redef*/

#if !defined(_DIAGASSERT)
#define _DIAGASSERT(__x)                /*not used*/
#endif

#if !defined(__UNCONST)
#define __UNCONST(__s)                  ((char *) __s)
#endif

/*function mappings*/
#if defined(_MSC_VER) || defined(__WATCOMC__)
#if (_MSC_VER < 1500)	/* MSVC 2008 */
#define vsnprintf _vsnprintf
#endif
#if (_MSC_VER < 1700)	/* MSVC 2012 */
#define snprintf _snprintf
#endif /*1500*/
#define strdup _strdup
#define stricmp _stricmp
#define mktemp _mktemp
#ifndef readlink
#define readlink w32_readlink
#endif
#ifndef lstat
#define lstat w32_lstat
#endif
#endif	/*MSC_VER || WATCOM*/

/*end*/
