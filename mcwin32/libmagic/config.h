#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/*
 *  WIN32 config.h
 */

#include <../config.h>

/////////////////////////////////////////////////////////////////////////////////////////

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


/////////////////////////////////////////////////////////////////////////////////////////

/* Define to the full name of this package. */
#define PACKAGE_NAME        "file"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME     "file"

/* Define to the version of this package. */
#define PACKAGE_VERSION     "5.41"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING      "file 5.41"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT   "christos@astron.com"

/* Define to the home page for this package. */
#define PACKAGE_URL         ""

#define VERSION             PACKAGE_VERSION


/////////////////////////////////////////////////////////////////////////////////////////

/* Define if building universal (internal helper macro) */
    //#undef AC_APPLE_UNIVERSAL_BUILD

/* Define in built-in ELF support is used */
    //#undef BUILTIN_ELF

/* Define for ELF core file support */
    //#undef ELFCORE

/* Define to 1 if you have the `asctime_r' function. */
    //#undef HAVE_ASCTIME_R

/* Define to 1 if you have the `asprintf' function. */
    //#undef HAVE_ASPRINTF

/* Define to 1 if you have the `ctime_r' function. */
    //#undef HAVE_CTIME_R

/* HAVE_DAYLIGHT */
    //#undef HAVE_DAYLIGHT

/* Define to 1 if you have the declaration of `daylight', and to 0 if you don't. */
    //#undef HAVE_DECL_DAYLIGHT

/* Define to 1 if you have the declaration of `tzname', and to 0 if you don't. */
#undef HAVE_DECL_TZNAME

/* Define to 1 if you have the <dlfcn.h> header file. */
    //#undef HAVE_DLFCN_H

/* Define to 1 if you have the `dprintf' function. */
    //#undef HAVE_DPRINTF

/* Define to 1 if you have the <err.h> header file. */
#define HAVE_ERR_H

/* Define to 1 if you have the <fcntl.h> header file. */
    //#undef HAVE_FCNTL_H

/* Define to 1 if you have the `fmtcheck' function. */
    //#undef HAVE_FMTCHECK

/* Define to 1 if you have the `fork' function. */
    //#undef HAVE_FORK

/* Define to 1 if you have the `freelocale' function. */
    //#undef HAVE_FREELOCALE

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
    //#undef HAVE_FSEEKO

/* Define to 1 if you have the `getline' function. */
    //#define HAVE_GETLINE

/* Define to 1 if you have the <getopt.h> header file. */
#define HAVE_GETOPT_H

/* Define to 1 if you have the `getopt_long' function. */
#define HAVE_GETOPT_LONG

/* Define to 1 if you have the `getpagesize' function. */
    //#undef HAVE_GETPAGESIZE

/* Define to 1 if you have the `gmtime_r' function. */
    //#undef HAVE_GMTIME_R

/* Define to 1 if the system has the type `intptr_t'. */
    //#undef HAVE_INTPTR_T

/* Define to 1 if you have the <inttypes.h> header file. */
    //#undef HAVE_INTTYPES_H

/* Define to 1 if you have the `gnurx' library (-lgnurx). */
    //#undef HAVE_LIBGNURX

/* Define to 1 if you have the `z' library (-lz). */
#define HAVE_LIBZ

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H

/* Define to 1 if you have the <locale.h> header file. */
#if defined(__WATCOMC__)
#undef HAVE_LOCALE_H
#endif

/* Define to 1 if you have the `localtime_r' function. */
    //#undef HAVE_LOCALTIME_R

/* Define to 1 if mbrtowc and mbstate_t are properly declared. */
#define HAVE_MBRTOWC

/* Define to 1 if <wchar.h> declares mbstate_t. */
#define HAVE_MBSTATE_T

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H

/* Define to 1 if you have the `mkostemp' function. */
#define HAVE_MKOSTEMP

/* Define to 1 if you have the `mkstemp' function. */
#if defined(__WATCOMC__)
#define HAVE_MKSTEMP
#endif

/* Define to 1 if you have a working `mmap' system call. */
    //#undef HAVE_MMAP

/* Define to 1 if you have the `newlocale' function. */
    //#undef HAVE_NEWLOCALE

/* Define to 1 if you have the `pread' function. */
    //#undef HAVE_PREAD

/* Define to 1 if you have the `setlocale' function. */
    //#undef HAVE_SETLOCALE

/* Define to 1 if you have the <signal.h> header file. */
    //#undef HAVE_SIGNAL_H

/* Have sig_t type */
    //#undef HAVE_SIG_T

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H

/* Define to 1 if you have the `strcasestr' function. */
    //#undef HAVE_STRCASESTR

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR

/* Define to 1 if you have the <strings.h> header file. */
    //#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H

/* Define to 1 if you have the `strlcat' function. */
    //#define HAVE_STRLCAT

/* Define to 1 if you have the `strlcpy' function. */
    //#define HAVE_STRLCPY

/* Define to 1 if you have the `strndup' function. */
    //#define HAVE_STRNDUP

/* Define to 1 if you have the `strtof' function. */
    //#undef HAVE_STRTOF

/* Define to 1 if you have the `strtoul' function. */
    //#define HAVE_STRTOUL

/* HAVE_STRUCT_OPTION */
#define HAVE_STRUCT_OPTION

/* Define to 1 if `st_rdev' is a member of `struct stat'. */
    //#undef HAVE_STRUCT_STAT_ST_RDEV

/* Define to 1 if `tm_gmtoff' is a member of `struct tm'. */
    //#undef HAVE_STRUCT_TM_TM_GMTOFF

/* Define to 1 if `tm_zone' is a member of `struct tm'. */
    //#undef HAVE_STRUCT_TM_TM_ZONE

/* Define to 1 if you have the <sys/mman.h> header file. */
    //#undef HAVE_SYS_MMAN_H

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H

/* Define to 1 if you have the <sys/stat.h> header file. */
    //#undef HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/time.h> header file. */
    //#undef HAVE_SYS_TIME_H

/* Define to 1 if you have the <sys/types.h> header file. */
    //#undef HAVE_SYS_TYPES_H

/* Define to 1 if you have the <sys/utime.h> header file. */
    //#undef HAVE_SYS_UTIME_H

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
    //#undef HAVE_SYS_WAIT_H

/* HAVE_TM_ISDST */
    //#undef HAVE_TM_ISDST

/* HAVE_TM_ZONE */
    //#undef HAVE_TM_ZONE

/* HAVE_TZNAME */
    //#undef HAVE_TZNAME

/* Define to 1 if the system has the type `uintptr_t'. */
    //#undef HAVE_UINTPTR_T

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H

/* Define to 1 if you have the `uselocale' function. */
    //#undef HAVE_USELOCALE

/* Define to 1 if you have the `utime' function. */
    //#undef HAVE_UTIME

/* Define to 1 if you have the `utimes' function. */
    //#undef HAVE_UTIMES

/* Define to 1 if you have the <utime.h> header file. */
    //#undef HAVE_UTIME_H

/* Define to 1 if you have the `vasprintf' function. */
    //#undef HAVE_VASPRINTF

/* Define to 1 if you have the `vfork' function. */
    //#undef HAVE_VFORK

/* Define to 1 if you have the <vfork.h> header file. */
    //#undef HAVE_VFORK_H

/* Define to 1 or 0, depending whether the compiler supports simple visibility declarations. */
    //#undef HAVE_VISIBILITY

/* Define to 1 if you have the <wchar.h> header file. */
#define HAVE_WCHAR_H

/* Define to 1 if you have the <wctype.h> header file. */
#define HAVE_WCTYPE_H

/* Define to 1 if you have the `wcwidth' function. */
    //#define HAVE_WCWIDTH

/* Define to 1 if `fork' works. */
    //#undef HAVE_WORKING_FORK

/* Define to 1 if `vfork' works. */
    //#undef HAVE_WORKING_VFORK

/* Define to 1 if you have the <xlocale.h> header file. */
    //#undef HAVE_XLOCALE_H

/* Define to 1 if you have the <zlib.h> header file. */
#define HAVE_ZLIB_H

/* Define to the sub-directory in which libtool stores uninstalled libraries. */
    //#undef LT_OBJDIR

/* Define to 1 if `major', `minor', and `makedev' are declared in <mkdev.h>. */
    //#undef MAJOR_IN_MKDEV

/* Define to 1 if `major', `minor', and `makedev' are declared in <sysmacros.h>. */
    //#undef MAJOR_IN_SYSMACROS

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
    //#define TM_IN_SYS_TIME

/* Enable extensions on AIX 3, Interix.  */
    //#ifndef _ALL_SOURCE
    //# undef _ALL_SOURCE
    //#endif

/* Enable GNU extensions on systems that have them.  */
    //#ifndef _GNU_SOURCE
    //# undef _GNU_SOURCE
    //#endif

/* Enable threading extensions on Solaris.  */
    //#ifndef _POSIX_PTHREAD_SEMANTICS
    //# undef _POSIX_PTHREAD_SEMANTICS
    //#endif

/* Enable extensions on HP NonStop.  */
    //#ifndef _TANDEM_SOURCE
    //# undef _TANDEM_SOURCE
    //#endif

/* Enable general extensions on Solaris.  */
    //#ifndef __EXTENSIONS__
    //# undef __EXTENSIONS__
    //#endif

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most significant byte first (like Motorola and SPARC, unlike Intel). */
    //#if defined AC_APPLE_UNIVERSAL_BUILD
    //# if defined __BIG_ENDIAN__
    //#  define WORDS_BIGENDIAN 1
    //# endif
    //#else
    //# ifndef WORDS_BIGENDIAN
    //#  undef WORDS_BIGENDIAN
    //# endif
    //#endif

/* Enable zlib compression support */
#define ZLIBSUPPORT

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
    //#undef _FILE_OFFSET_BITS

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
    //#undef _LARGEFILE_SOURCE

/* Define for large files, on AIX-style hosts. */
    //#undef _LARGE_FILES

/* Define to 1 if on MINIX. */
    //#undef _MINIX

/* Define to 2 if the system does not provide POSIX.1 features except with this defined. */
    //#undef _POSIX_1_SOURCE

/* Define to 1 if you need to in order for `stat' and other things to work. */
    //#undef _POSIX_SOURCE

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the #define below would cause a syntax error. */
    //#undef _UINT32_T

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the #define below would cause a syntax error. */
    //#undef _UINT64_T

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the #define below would cause a syntax error. */
    //#undef _UINT8_T

/* Define to empty if `const' does not conform to ANSI C. */
    //#undef const

/* Define to the type of a signed integer type of width exactly 32 bits if such a type exists and the standard includes do not define it. */
    //#undef int32_t

/* Define to the type of a signed integer type of width exactly 64 bits if such a type exists and the standard includes do not define it. */
    //#undef int64_t

/* Define to the type of a signed integer type wide enough to hold a pointer, if such a type exists, and if the system does not define it. */
    //#undef intptr_t

/* Define to a type if <wchar.h> does not define. */
    //#undef mbstate_t

/* Define to `long int' if <sys/types.h> does not define. */
    //#undef off_t

/* Define to `int' if <sys/types.h> does not define. */
    //#undef pid_t

/* Define to `unsigned int' if <sys/types.h> does not define. */
    //#undef size_t

/* Define to the type of an unsigned integer type of width exactly 16 bits if such a type exists and the standard includes do not define it. */
    //#undef uint16_t

/* Define to the type of an unsigned integer type of width exactly 32 bits if such a type exists and the standard includes do not define it. */
    //#undef uint32_t

/* Define to the type of an unsigned integer type of width exactly 64 bits if such a type exists and the standard includes do not define it. */
    //#undef uint64_t

/* Define to the type of an unsigned integer type of width exactly 8 bits if such a type exists and the standard includes do not define it. */
    //#undef uint8_t

/* Define to the type of an unsigned integer type wide enough to hold a pointer, if such a type exists, and if the system does not define it. */
    //#undef uintptr_t

/* Define as `fork' if `vfork' does not work. */
    //#undef vfork


/////////////////////////////////////////////////////////////////////////////////////////

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

#if defined(__MINGW32__)
#if !defined(lstat)
#define lstat w32_lstat
#define readlink w32_readlink
#endif
#endif

#if defined(__WATCOMC__) || \
        (defined(_MSC_VER) && !defined(PRIx64))
#define PRIx64 "ull"
#endif

/////////////////////////////////////////////////////////////////////////////////////////

#endif  /*CONFIG_H_INCLUDED*/

