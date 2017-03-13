#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/*
 *  WIN32 config.h
 */

#include <../config.h>

#define PACKAGE_NAME        "file"
#define PACKAGE_TARNAME     "file"
#define PACKAGE_VERSION     "5.29"		/* FIXME: generate buildinfo.h */
#define PACKAGE_STRING      "file 5.29"
#define PACKAGE_BUGREPORT   "christos@astron.com"
#define PACKAGE_URL         ""

#define VERSION             PACKAGE_VERSION

/*configuration*/
#define HAVE_SLANG 1
#undef  HAVE_SLANG_SLANG_H
#define HAVE_SLANG_H 1
#define HAVE_SYS_PARAM_H 1
#undef  HAVE_SYS_SELECT_H
#define HAVE_SYS_VFS_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_UNISTD_H 1
#define HAVE_TIME_H 1
#define HAVE_UTIME_H 1
#define HAVE_STDARG_H 1
#define HAVE_ASSERT_H 1
#define HAVE_STRING_H 1
#define HAVE_MEMORY_H 1
#define HAVE_PWD_H 1
#define HAVE_GRP_H 1
#undef  HAVE_STRUCT_STATVFS_F_BASETYPE
#undef  HAVE_STRUCT_STATVFS_F_FSTYPENAME
#undef  HAVE_STRUCT_STATFS_F_FSTYPENAME

#define HAVE_LIBMAGIC
#undef  HAVE_ASPELL
#undef  HAVE_SUBSHELL_SUPPORT
#define HAVE_CHARSET 1
#define HAVE_SLANG 1
#undef  HAVE_TEXTMODE_X11_SUPPORT
#undef  HAVE_LIBGPM

#undef  HAVE_REALPATH
#define HAVE_STRERROR 1
#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1
#if defined(__WATCOMC__)
#define HAVE_STRLCPY 1
#define HAVE_STRLCAT 1
#define HAVE_LOCALE_H 1
#endif

/*warnings*/
#if defined(_MSC_VER)
#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE                /* Deprecated warnings */
#endif
#pragma warning(disable : 4996)                 /* 'xxx': The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: _xxx. */

#elif defined(__WATCOMC__)
#if !defined(__cplusplus)
#pragma disable_message(124)                    /* Comparison result always 0 */
#pragma disable_message(136)                    /* Comparison equivalent to 'unsigned == 0' */
#endif
#pragma disable_message(201)                    /* Unreachable code */
#pragma disable_message(202)                    /* Symbol 'xxx' has been defined, but not referenced */
#pragma disable_message(302)                    /* Expression is only useful for its side effects */
#endif

/*standard includes*/
#include <stdio.h>
#include <unistd.h>
#if defined(WIN32_UNISTD_MAP)
#include <sys/socket.h>                         /* resolve order issues */
#endif
#if defined(_MSC_VER)
#include <sys/utime.h>                          /* utime() and utimbuf */
#endif

#ifndef UINT32_MAX
#define UINT32_MAX 0xffffffff
#endif
#ifndef intmax_t
#define intmax_t int
#endif

/*function mappings*/
#if defined(_MSC_VER) || defined(__WATCOMC__)
#if (_MSC_VER < 1500)   /* MSVC 2008 */
#define vsnprintf _vsnprintf
#endif
#if (_MSC_VER < 1700)   /* MSVC 2012 */
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
#endif  /*MSC_VER || WATCOM*/

#endif  /*CONFIG_H_INCLUDED*/

