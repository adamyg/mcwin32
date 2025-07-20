#ifndef LIBW32_WIN32_CRTDLL_H_INCLUDED
#define LIBW32_WIN32_CRTDLL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_crtdll,"$Id: win32_crtdll.h,v 1.1 2025/07/20 06:59:27 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * crtdll, dll hooks.
 *
 * Copyright (c) 2025, Adam Young.
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

#include <sys/cdefs.h>

 /*
  * We need to put the following marker variables into the .CRT section.
  *
  * The .CRT section contains arrays of function pointers.
  * The compiler creates functions and adds pointers to this section
  * for things like C++ global constructors.
  *
  * The XIA, XCA etc are group names with in the section.
  * The compiler sorts the contributions by the group name.
  * For example, .CRT$XCA followed by .CRT$XCB, ... .CRT$XCZ.
  * The marker variables below let us get pointers
  * to the beginning/end of the arrays of function pointers.
  *
  * For example, standard groups are:
  *
  *      XCA         begin marker
  *      XCC         compiler inits
  *      XCL         library inits
  *      XCU         user inits
  *
  * Runtime hooks:
  *
  *      XCA/XCZ     C++ initializers
  *      XIA/XIZ     C initializers
  *      XPA/XPZ     C pre-terminators
  *      XTA/XTZ     C terminators
  */

#ifdef _UCRT
#include <corecrt_startup.h>
#else
typedef void (__cdecl *_PVFV)(void);
#endif

static void crtdll(DWORD reason);
static void WINAPI __crtdll(void *image, DWORD reason, void *pv);
    // typedef VOID (NTAPI *PIMAGE_TLS_CALLBACK) (PVOID DllHandle, DWORD Reason, PVOID Reserved);

#if !defined(CRTMODULE)
#error "You must define CRTMODULE to the name of your module."
    // CRTMODULE is used to identify the module for which the CRT section is being created.
    // It should be defined before including win32_crtinit.h.
#endif
#define ___CRTAPPEND(__prefix, __module) __prefix##__module
#define __CRTAPPEND(__prefix, __module) ___CRTAPPEND(__prefix, __module)
#define __CRTDLL __CRTAPPEND(__crtdll_, CRTMODULE)

#if defined(__GNUC__) || defined(__clang__)
__attribute__((section(".CRT$XLC"), used)) extern _PVFV __CRTDLL = __crtdll;

#elif defined(_MSC_VER)
#   pragma warning(disable:4152)
#   if (_MSC_VER >= 1600) || defined(_M_IA64)
#       pragma section(".CRT$XLC", long, read)
__declspec(allocate(".CRT$XLC")) extern void *__CRTDLL = __crtdll;
#   else
#       pragma data_seg(".CRT$XLC")
extern void *__CRTDLL = __crtdll;
#       pragma data_seg()
#   endif

#else
#   error Unsupported compiler is not supported.
#endif

static void WINAPI
__crtdll(void *image, DWORD reason, void *pv)
{
    (void) image;
    (void) pv;
#if defined(_MSC_VER)
    (void)__CRTDLL; // otherwise global optimisation may remove
#endif
    crtdll(reason);
}

#undef CRTMODULE

#endif /*LIBW32_WIN32_CRTDLL_H_INCLUDED*/
