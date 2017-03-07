/*
 *  libintl, namespace.h
 */

#ifdef  _MSC_VER
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#pragma warning(disable:4996)                   /* 'xxx' was declared deprecated */
#endif

#include <sys/types.h>
#include <sys/utypes.h>
#include <limits.h>
#include <unistd.h>

#ifndef __RCSID
#define __RCSID(__rcsid)
#endif

#ifndef snprintf
#if (_MSC_VER < 1500) /* MSVC 2008 (v9.0) */
#define vsnprintf       _vsnprintf
#endif /*1500*/
#if (_MSC_VER < 1900) /* MSVC 2015 (v9.0) */
#define snprintf        _snprintf
#endif /*1900*/
#endif /*snprintf*/

#ifndef open
#if (_MSC_VER < 1500) /* MSVC 2008 (v9.0) */
#define open            _open
#define read            _read
#define write           _write
#endif /*1500*/
#endif /*open*/

#ifndef strdup
#define strdup(__a)     _strdup(__a)
#endif

#ifndef HAVE_GETCWD
#define getcwd(__a,__b) w32_getcwd(__a,__b)
#endif

#ifndef HAVE_STRSEP
extern char *           libintl_strsep(char **stringp, const char *delim);
#define strsep(__a,__b) libintl_strsep(__a,__b)
#endif

#if defined(_MSC_VER)
#define LC_MESSAGES     (LC_MAX + 1)
#endif

/*end*/


