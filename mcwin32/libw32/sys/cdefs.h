#ifndef LIBW32_SYS_CDEFS_H_INCLUDED
#define LIBW32_SYS_CDEFS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_cdefs_h,"$Id: cdefs.h,v 1.10 2022/06/14 02:19:59 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*-
 *
 * win32 declaration helpers
 *
 * Copyright (c) 1998 - 2022, Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
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
/*LINTLIBRARY*/

/*
 *  Disable (global) toolchain specific warnings:
 */
#if (defined(lint) || defined(_lint))

#elif (defined(_MSC_VER) && _MSC_VER >= 800)
#pragma warning(disable:4514)   /* unreferenced inline function has been removed */
#pragma warning(disable:4115)   /* forward reference of struct * */
#endif

/* 
 *  Library binding.
 */
#if !defined(LIBW32_API)

#if defined(LIBW32_DYNAMIC) && defined(LIBW32_STATIC)
#error LIBW32_DYNAMIC and LIBW32_STATIC are mutually exclusive
#endif

#if defined(LIBW32_DYNAMIC)
    #if defined(LIBW32_LIBRARY)     /* library source */
        #ifdef __GNUC__
            #define LIBW32_API __attribute__((dllexport)) extern
        #elif defined(__WATCOMC__)
            #define LIBW32_API extern __declspec(dllexport)
        #else
            #define LIBW32_API __declspec(dllexport)
        #endif
    #else
        #ifdef __GNUC__
            #define LIBW32_API __attribute__((dllimport)) extern
        #elif defined(__WATCOMC__)
            #define LIBW32_API extern __declspec(dllimport)
        #else
            #define LIBW32_API __declspec(dllimport)
        #endif
    #endif

#else   /*static*/
    #if defined(LIBW32_LIBRARY)     /* library source */
        #ifndef LIBW32_STATIC                   /* verify STATIC/DYNAMIC configuration */
            #error  LIBW32 static library yet LIB32_STATIC not defined.
        #endif
        #ifdef _WINDLL                          /*verify target configuration */
            #error  LIBW32 static library yet _WINDLL defined.
        #endif
    #endif
#endif

#ifndef LIBW32_API
#define LIBW32_API
#define LIBW32_VAR extern
#else
#define LIBW32_VAR LIBW32_API
#endif

#endif //!LIBW32_API


/*
 *  Binding:
 *
 * Usage:
 *      __BEGIN_DECLS
 *      void my_declarations();
 *      __END_DECLS
 */
#ifndef __BEGIN_DECLS
#  ifdef __cplusplus
#     define __BEGIN_DECLS      extern "C" {
#     define __END_DECLS        }
#  else
#     define __BEGIN_DECLS
#     define __END_DECLS
#  endif
#endif
#ifndef __P
#  if (__STDC__) || defined(__cplusplus) || \
         defined(_MSC_VER) || defined(__PARADIGM__) || defined(__GNUC__) || \
         defined(__BORLANDC__) || defined(__WATCOMC__)
#     define __P(x)             x
#  else
#     define __P(x)             ()
#  endif
#endif


/* Calling convention:
 *
 * Usage:
 *      void func_prototype();
 *      #ifdef  __PDECL2                        // Alternative form for typedefs
 *      typedef void (__PDECL2 *func_typedef)();
 *      #else
 *      typedef void __PDECL (*func_typdef)();
 *      #endif
 */
#ifndef __PDECL
#  if defined(WIN32)
#     if defined(_MSC_VER) || defined(__WATCOMC__)
#        define __PDECL         __cdecl         /* Calling convention */
#        define __PDECL2        __cdecl         /* Alternative form for typedefs */
#     elif defined(__MINGW32__)
#        define __PDECL
#        define __PDECL2
#     else
#        error cdefs.h: unknown target compiler ...
#     endif
#  else
#     define __PDECL            /**/
#  endif
#endif

/*
 * remove const cast-away warnings
 */
#ifndef __DECONST
#define __DECONST(__t,__a)      ((__t)(const void *)(__a))
#endif
#ifndef __UNCONST
#define __UNCONST(__a)          ((void *)(const void *)(__a))
#endif

/*
 * remove the volatile cast-away warnings
 */
#ifndef __UNVOLATILE
#define __UNVOLATILE(__a)       ((void *)(unsigned long)(volatile void *)(__a))
#endif

/*
 * CPP warnings
 *
 * Usage:
 *      #if defined(__PWARNING)
 *      #warning "blah blah blah"
 *      #elif defined(__PPRAGMA_MESSAGE)
 *      #pragma message( "warning: blah blah blah" )
 *      #endif
 */
#if defined(_MSC_VER) || \
         defined(__BORLANDC__) || defined(__PARADIGM__) || \
         defined(__WATCOMC__)
#    define __PPRAGMA_MESSAGE
#endif
#if defined(__GNUC__) || \
         defined(_MCC68K)
#    define __PWARNING
#endif

/* Unused variables/parameters:
 *
 * Usage:
 *      __PUNUSED(x)
 *      int x __PUNUSED_ATTRIBUTE__;
 */
#ifndef __PUNUSED
#  if (defined(lint) || defined(_lint))
            /* Note: lint -e530 says don't complain about uninitialized vars
             * for this. line +e530 turns that checking back on.  Error 527
             * has to do with unreachable code.
             */
#       define __PUNUSED(x) \
            /*lint -e527 -e530 */ \
            { (x) = (x); }        \
            /*lint +e527 +e530 */

#  elif ((defined(_MSC_VER) && _MSC_VER >= 800) || \
         defined(__BORLANDC__) || defined(__PARADIGM__))
#       define __PUNUSED(x)     (void)x;
#  elif defined(__WATCOMC__)
#       define __PUNUSED(x)     (void)x;
#  elif defined(__GNUC__)
#       define __PUNUSED(x)     (void)x;
#       define __PUNUSED_ATTRIBUTE__    __attribute__((unused))
#  endif

#  ifndef __PUNUSED
#        define __PUNUSED(x)    /*default*/
#  endif
#  ifndef __PUNUSED_ATTRIBUTE__
#        define __PUNUSED_ATTRIBUTE__   /*default*/
#  endif
#endif
#ifndef __CUNUSED
#  define __CUNUSED(x)          __PUNUSED(x)
#endif


/*
 * Prototype attributes:
 *
 * Usage:
 *      void MyPrintf(const char *, ...)
 *          __ATTRIBUTE__ ((__format__ (__printf__, 1, 2)));
 *
 */
#ifndef __ATTRIBUTE__
#  if defined(__GNUC__)
#     define __ATTRIBUTE__(x)   __attribute__(x)
#  else
#     define __ATTRIBUTE__(x)   /**/
#  endif
#endif


/* Structure pack helper, see also sys/pack?.h:
 *
 * Usage:
 *      #include <sys/pack1.h>
 *      __packed_pre__ struct mypackedstruct {
 *              :
 *      } __packed_post__;
 *      #include <sys/pack0.h>
 */
#ifndef __packed_pre__
#  if (defined(lint) || defined(_lint))
#     define __packed_pre__     /**/
#     define __packed_post__    /**/
#  elif ((defined(_MSC_VER) && _MSC_VER >= 800) || \
            defined(__BORLANDC__) || defined(__PARADIGM__) )
#     define __packed_pre__     /**/
#     define __packed_post__    /**/
#  elif defined(__WATCOMC__)
#     define __packed_pre__     /**/
#     define __packed_post__    /**/
#  elif defined(_MCC68K)
#     define __packed_pre__     packed
#     define __packed_post__    /**/
#  elif defined(__GNUC__)
#     define __packed_pre__     /**/
#     define __packed_post__    __attribute__((packed, aligned(1)))
#  elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#     define __packed_pre__     /**/
#     define __packed_post__    /**/
#  else
#     error cdefs.h: Packing method unknown for this compiler ...
#  endif
#endif

/*
 * ---- BSD stuff -----
 */

/*
 * Assert
 */
#ifndef _DIAGASSERT
#define _DIAGASSERT(_a)         /**/
#endif

#ifndef __RCSID
#define __RCSID(__rcsid)        /**/
#endif


/*
 * Compiler-dependent macros to help declare dead (non-returning) and
 * pure (no side effects) functions, and unused variables.  They are
 * null except for versions of gcc that are known to support the features
 * properly (old versions of gcc-2 supported the dead and pure features
 * in a different (wrong) way).
 */
#if !defined(__GNUC__) || \
        (__GNUC__) < 2 || __GNUC__ == 2 && __GNUC_MINOR__ < 5
#define __dead2
#define __pure2
#define __unused
#elif __GNUC__ == 2 && __GNUC_MINOR__ >= 5 && __GNUC_MINOR__ < 7
#define __dead2                 __attribute__((__noreturn__))
#define __pure2                 __attribute__((__const__))
#define __unused
#elif __GNUC__ >= 2 || (_GNUC__ == 2 && __GNUC_MINOR__ >= 7)
#define __dead2                 __attribute__((__noreturn__))
#define __pure2                 __attribute__((__const__))
#define __unused                __attribute__((__unused__))
#endif

/*
 * Compiler-dependent structure packing attribute.
 */
#if defined(__GNUC__)
#define __packed                __attribute__((packed, aligned(1)))
#else
#define __packed                /**/
#endif

/*
 * Compiler-dependent macros to declare that functions take printf-like
 * or scanf-like arguments.  They are null except for versions of gcc
 * that are known to support the features properly (old versions of gcc-2
 * didn't permit keeping the keywords out of the application namespace).
 */
#if __GNUC__ < 2 || __GNUC__ == 2 && __GNUC_MINOR__ < 7
#define __printflike(fmtarg, firstvararg)
#define __scanflike(fmtarg, firstvararg)
#else
#define __printflike(fmtarg, firstvararg) \
                        __attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#define __scanflike(fmtarg, firstvararg) \
                                __attribute__((__format__ (__scanf__, fmtarg, firstvararg)))
#endif

/* Compiler-dependent macros that rely on FreeBSD-specific extensions. */
#if __FreeBSD_cc_version >= 300001
#define __printf0like(fmtarg, firstvararg) \
                                __attribute__((__format__ (__printf0__, fmtarg, firstvararg)))
#else
#define __printf0like(fmtarg, firstvararg)
#endif

#endif /*LIBW32_SYS_CDEFS_H_INCLUDED*/
