#ifndef LIBW32_WIN32_CRTINIT_H_INCLUDED
#define LIBW32_WIN32_CRTINIT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_crtinit,"$Id: win32_crtinit.h,v 1.4 2025/07/29 09:27:43 cvsuser Exp $")
#endif //LIBW32_WIN32_CRTINIT_H_INCLUDED

/* -*- mode: c; indent-width: 4; -*- */
/*
 * crtinit, application run-time initialization hooks.
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

/* Usage:
 *
 *  #define CRTINIT rtinit
 *  #include "win32_crtinit.h"
 *
 *  static void
 *  rtinit(void)
 *  {
 *      tls = TlsAlloc();
 *      InitializeCriticalSection(&lock, 0x400);
 *  }
 *
 */

/*  MSVC/Mingw64
 *
 *  We need to put the following marker variables into the .CRT section.
 *
 *  The .CRT section contains arrays of function pointers.
 *  The compiler creates functions and adds pointers to this section
 *  for things like C++ global constructors.
 *
 *  The XIA, XCA etc are group names with in the section.
 *  The compiler sorts the contributions by the group name.
 *  For example, .CRT$XCA followed by .CRT$XCB, ... .CRT$XCZ.
 *  The marker variables below let us get pointers
 *  to the beginning/end of the arrays of function pointers.
 *
 *  For example, standard groups are:
 *
 *      XCA         Begin marker
 *      XCC         Compiler initialisations
 *      XCL         Library initialisations
 *      XCU         User initialisations
 *      XCZ         End marker
 *
 *  Runtime hooks:
 *
 *      XIA/XIZ     C initializers (int return, non-zero terminates)
 *      XCA/XCZ     C++ initializers (void return)
 *
 *      XPA/XPZ     C pre-terminators (void return)
 *      XTA/XTZ     C terminators (void return)
 *
 *  Example Implementation:
 *
        extern _CRTALLOC(".CRT$XIA") _PIFV __xi_a[];
        extern _CRTALLOC(".CRT$XIZ") _PIFV __xi_z[];    // C initializers
        extern _CRTALLOC(".CRT$XCA") _PVFV __xc_a[];
        extern _CRTALLOC(".CRT$XCZ") _PVFV __xc_z[];    // C++ initializers
        extern _CRTALLOC(".CRT$XPA") _PVFV __xp_a[];
        extern _CRTALLOC(".CRT$XPZ") _PVFV __xp_z[];    // C pre-terminators
        extern _CRTALLOC(".CRT$XTA") _PVFV __xt_a[];
        extern _CRTALLOC(".CRT$XTZ") _PVFV __xt_z[];    // C terminators

        static void _initterm_e(_PIFV *a, _PIFV *b) {
           while (a != b) {
                if (*a) {
                    const int result = (**a)();         // non-zero terminate
                    if (result != 0) {
                        if (IsDebuggerPresent()) __debugbreak();
                        ExitProcess(0);
                    }
                }
                ++a;
            }
        }

        static void _initterm(_PVFV *a, _PVFV *b) {
            while (a != b) {
                if (*a) {
                    (**a)();
                }
                ++a;
            }
        }

        void w32_crtinit() {
            _initterm_e(__xi_a, __xi_z);                // C initializers
            _initterm(__xc_a, __xc_z);                  // C++ initializers
        }

        void w32_crtexit() {
            _initterm(__xp_a, __xp_z);                  // C pre-terminators
            _initterm(__xt_a, __xt_z);                  // C terminators
        }

 *
 */

#ifdef _UCRT
#include <corecrt_startup.h>
#else
typedef void (__cdecl *_PVFV)(void);
#endif

#if !defined(CRTINIT)
#error "You must define CRTINIT to the name of interface before including."
    // CRTMODULE is used to identify the module for which the CRT section is being created.
    // It should be defined before including win32_crtinit.h.
#endif

static void CRTINIT (void);                     // user callback

#if !defined(__CRTCONCAT)
#define ___CRTCONCAT(__prefix, __module) __prefix##__module
#define __CRTCONCAT(__prefix, __module) ___CRTCONCAT(__prefix, __module)
#endif

#define __CRTINITI __CRTCONCAT(__crti_, CRTINIT) // initialisation object
#define __CRTINITC __CRTCONCAT(__crtc_, CRTINIT) // callback; if required


#if defined(_MSC_VER)
#   if defined(_M_X64)                          // import TLS Directory.
#       pragma comment (linker, "/INCLUDE:_tls_used")
#   else
#       pragma comment (linker, "/INCLUDE:__tls_used")
#   endif
#endif


#if defined(__GNUC__) || defined(__clang__)
static void __cdecl __CRTINITC (void);
__attribute__((section(".CRT$XCU"), used)) _PVFV __CRTINITI = __CRTINITC;

#elif defined(_MSC_VER)
static void __cdecl __CRTINITC (void);
#   pragma warning(disable:4152)
#   if (_MSC_VER >= 1600) || defined(_M_IA64)
#       pragma section(".CRT$XCU", long, read)
__declspec(allocate(".CRT$XCU")) extern _PVFV __CRTINITI = __CRTINITC;
#   else
#       pragma data_seg(".CRT$XCU")
extern _PVFV __CRTINITI = __CRTINITC;
#       pragma data_seg()
#   endif

#elif defined(__WATCOMC__)
#   if !defined(__NT__)
#       error __NT__ not defined
#   endif
#   if !defined(__386__)
#       error __386__ not defined
#   endif

#if !defined(RTINIT)
#   pragma warning(disable:4103)
#   pragma pack(push,1)
struct rt_init {        // see: watcom/rtinit.h
    unsigned char type;
    unsigned char priority;
    void __near (*fn)(void);
};
#   pragma pack(pop)

#   define RTINIT(seg, label, routine, priority) \
struct rt_init __based( __segname( seg ) ) label = \
        { 0, priority, routine };
#   define INIT_PRIORITY_PROGRAM 64

typedef unsigned crinit_assert_t[sizeof(struct rt_init) == 6 ? 1 : -1]; // negative size on failure
#endif
#undef __CRTINITC       // callback not required
RTINIT("XI", __CRTINITI, CRTINIT, INIT_PRIORITY_PROGRAM)

#else
#   error Unsupported compiler
#endif

#if defined(__CRTINITC)
static void __cdecl
__CRTINITC (void)
{
#if defined(_MSC_VER)
    (void) __CRTINITI;  // otherwise global optimisation may remove
#endif
    CRTINIT ();         // application hook
}
#endif

#undef __CRTINITI
#undef __CRTINITC
#undef CRTINIT

//end
