#ifndef LIBW32_EDIDENTIFIER_H_INCLUDED
#define LIBW32_EDIDENTIFIER_H_INCLUDED

/* -*- mode: c; indent-width: 4; -*- */
/*  $Id: edidentifier.h,v 1.5 2022/03/16 13:46:58 cvsuser Exp $
 *  Compiler specific object identify functionality.
 *
 *      __CIDENT(description)
 *      __CIDENT_RCSID(tag, rcsid)
 *      __CPRAGMA_ONCE
 *
 *
 * Copyright (c) 2007, 2012 - 2022 Adam Young.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 * or (at your option) any later version.
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

    /*
     *  warnings
     */
#if defined(__GNUC__)
#pragma GCC diagnostic ignored  "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored  "-Wmissing-braces"
#pragma GCC diagnostic ignored  "-Wunused-function"
#pragma GCC diagnostic ignored  "-Wunused-parameter"
#pragma GCC diagnostic ignored  "-Wunknown-pragmas"
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE                /* disable deprecate warnings */
#endif
#endif

    /*
     *  object level source code identification string, see the following
     *
     *      man ident
     *      man strings
     *      man mcs             (Solaris specific)
     *
     *  C99 introduced the _Pragma operator.
     *      This feature addresses a major problem with `#pragma': being a directive, it
     *      cannot be produced as the result of macro expansion. _Pragma is an operator,
     *      much like sizeof or defined, and can be embedded in a macro.
     */
#if !defined(__CIDENT)
#if defined(__GNUC__) && !defined(_AIX)
#   define __CIDENT_ASM(__asm)  __asm__(#__asm);
#   define __CIDENT(__id)       __CIDENT_ASM(.ident __id)

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#   if (!defined(__cplusplus) ||(_SUNPRO_CC >= 0x590))
#   define __CIDENT_XX(__id)    _Pragma(#__id)  /* cc or CC (>= Studio12) */
#   define __CIDENT(__id)       __CIDENT_XX(ident __id)
#   endif

#elif defined(__IBMC__) || defined(__IBMCPP__)
#   define __CIDENT_XX(__id)    _Pragma(__id)
#   define __CIDENT(__id)       __CIDENT_XX(comment (user, #__id))

#elif defined(_MSC_VER)
#if (_MSC_VER >= 1600)
#   define __CIDENT_XX(__id)    __pragma(__id)
#   define __CIDENT(__id)       __CIDENT_XX(comment (user, #__id))
#endif

#elif defined(__HP_cc) || defined(__HP_aCC)
#   define __CIDENT_XX(__id)    _Pragma(#__id)  /* ANSI mode assumed, -Aa and -Ae */
#   define __CIDENT(__id)       __CIDENT_XX(version __id)

#endif

#if !defined(__CIDENT)
#   define __CIDENT(__id)       /*unknown*/
#endif
#endif  /*__CIDENT*/

    /*
     *  load once directive
     */
#if !defined(__CIDENT_JOIN)
#define ____CIDENT_JOIN(_x,_y)  _x ## _y
#define __CIDENT_JOIN(_x,_y)    ____CIDENT_JOIN(_x,_y)
#endif

#if !defined(__CPRAGMA_ONCE)
#if defined(__GNUC__)
#   define __CPRAGMA_ONCE       _Pragma("once")

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#   define __CPRAGMA_ONCE       /*not supported*/

#elif defined(__IBMC__) || defined(__IBMCPP__)
#   define __CPRAGMA_ONCE       _Pragma("once")

#elif defined(_MSC_VER)
#if (_MSC_VER >= 1600) && defined(__cplusplus)
#   define __CPRAGMA_ONCE       __pragma(once)
#else
#   define __CPRAGMA_ONCE       /*not supported*/
#endif

#elif defined(__WATCOMC__)
#   define __CPRAGMA_ONCE       /*not supported*/

#elif defined(__HP_cc) || defined(__HP_aCC)
#   define __CPRAGMA_ONCE       /*not supported*/

#endif

#if !defined(__CPRAGMA_ONCE)
#   define __CPRAGMA_ONCE       /*unknown*/
#endif
#endif  /*__CPRAGMA_ONCE*/


    /*
     *  string concatenation support, allowing expansion of arguments
     */
#if !defined(__CIDENT_JOIN)
#define __XXCIDENT_JOIN(_x,_y)  _x ## _y
#define __CIDENT_JOIN(_x,_y)    __XXCIDENT_JOIN(__x,__y)
#endif

    /*
     *  high-level RCS based source identification
     */
#if !defined(__CIDENT_RCSID)
#if defined(__GNUC__)
#   if !defined(_AIX)
#       define __CIDENT_RCSID(__tag,__rcsid)    __CIDENT(__rcsid)
#   else
#       define __CIDENT_RCSID(__tag,__rcsid)    /* local vars */    \
static char __attribute__ ((unused)) __CIDENT_JOIN(RCSID_,__tag)[] = __rcsid; \
static void __CIDENT_JOIN(RCSFN_,__tag)(const char *tag) {          \
    const char *t_tag = __CIDENT_JOIN(RCSID_,__tag);                \
    __CIDENT_JOIN(RCSFN_,__tag)(tag ? tag : t_tag);                 \
}
#   endif

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#   if (!defined(__cplusplus) || (_SUNPRO_CC >= 0x590))
#       define __CIDENT_RCSID(__tag,__rcsid)    __CIDENT(__rcsid)
#   endif

#elif defined(__IBMC__) || defined(__IBMCPP__)

#elif defined(_MSC_VER)
#if (_MSC_VER >= 1600) && defined(__cplusplus)
#   define __CIDENT_RCSID(__tag,__rcsid)        __CIDENT(__rcsid)
#endif

#elif defined(__HP_cc) || defined(__HP_aCC)
#   define __CIDENT_RCSID(__tag,__rcsid)        __CIDENT(__rcsid)
#endif

#if !defined(__CIDENT_RCSID)
#   define __CIDENT_RCSID(__tag,__rcsid)        /* local vars */    \
static char __CIDENT_JOIN(RCSID_,__tag)[] = __rcsid;                \
static void __CIDENT_JOIN(RCSFN_,__tag)(const char *tag) {          \
    const char *t_tag = __CIDENT_JOIN(RCSID_,__tag);                \
    __CIDENT_JOIN(RCSFN_,__tag)(tag ? tag : t_tag);                 \
}
#endif
#endif  /*__CIDENT_RCSID*/

#ifndef __RCSID
#define __RCSID(__rcsid) __CIDENT_RCSID(rcsid_,__rcsid)
#endif

__CPRAGMA_ONCE

#endif  /*LIBW32_EDIDENTIFIER_H_INCLUDED*/
