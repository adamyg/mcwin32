/* -*- mode: c; indent-width: 4; -*- */
/* $Id: win32_tlsinit.h,v 1.4 2025/07/29 09:08:49 cvsuser Exp $ */
/*
 * tls initialisation hooks.
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

 /*
  * See:
  * https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#the-tls-section
  * https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#tls-callback-functions
  *
  * We need to put the following marker variables into the .CRT section.
  *
  * The .CRT section contains arrays of function pointers.
  * The compiler creates functions and adds pointers to this section
  * for things like C++ global constructors.
  *
  * Implementation Example:
  *
        #pragma section(".CRT$XLA", long, read) // First Loader TLS Callback
        #pragma section(".CRT$XLC", long, read) // CRT TLS Constructor
        #pragma section(".CRT$XLD", long, read) // CRT TLS Terminator
        #pragma section(".CRT$XLZ", long, read) // Last Loader TLS Callback

        extern _SEGALLOC(".CRT$XLA") PIMAGE_TLS_CALLBACK __xl_a[];
        extern _SEGALLOC(".CRT$XLZ") PIMAGE_TLS_CALLBACK __xl_z[];

        static void _calltls(PIMAGE_TLS_CALLBACK *a, PIMAGE_TLS_CALLBACK *b) {
            while (a != b) {
                if (*a) {
                    (**a)(DllHandle, Reason, Reserved);
                }
                ++a;
            }
        }

        void NTAPI crt_tls_callback(PVOID DllHandle, DWORD Reason, PVOID Reserved) {
            _calltls(__xl_a, __xl_z, DllHandle, Reason, Reserved);
        }

      either

        void WINAPI _DllMainCRTStartup() {
            crt_tls_callback();
        }

      or

        ULONG _tls_index = 0;

        _SEGALLOC(".tls") char *_tls_start = NULL;
        _SEGALLOC(".tls$ZZZ") char *_tls_end = NULL;

        //
        // The linker looks for this memory image and uses the data there to create the TLS directory.
        // Other compilers that support TLS and work with the Microsoft linker must use this same technique.
        //

        _SEGALLOC(".rdata$T")
        const IMAGE_TLS_DIRECTORY _tls_used = {
            (ULONG_PTR)  &_tls_start,
            (ULONG_PTR)  &_tls_end,
            (ULONG_PTR)  &_tls_index,
            (ULONG_PTR) (&__xl_a + 1),  // +1, assuming _xl_a[0] = {NULL}
            (ULONG) 0,
            (ULONG) 0
        };

        _SEGALLOC(".CRT$XLC") PIMAGE_TLS_CALLBACK _crtdef = _calltls;

 */

#if !defined(WINDWS_MEAN_AND_LEAN)
#include <windows.h>
#endif

#if !defined(_UCRT)
#if !defined(__PIMAGE_TLS_CALLBACK_T)           // missing fron OWC NT SDK
typedef void (NTAPI *PIMAGE_TLS_CALLBACK)(PVOID image, DWORD reason, PVOID reserved);
#define __PIMAGE_TLS_CALLBACK_T
#endif
#endif
#define __TLSCALLBACK PIMAGE_TLS_CALLBACK

#if !defined(TLSINIT)
#error "You must define TLSINIT to the name of interface before including."
    // TLSINIT is used to identify the module for which the CRT section is being created.
    // It should be defined before including win32_crtinit.h.
#endif

static void TLSINIT (DWORD reason);             // simplified user callback

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(__TLSCONCAT)
#define ___TLSCONCAT(__prefix, __module) __prefix##__module
#define __TLSCONCAT(__prefix, __module) ___TLSCONCAT(__prefix, __module)
#endif

#define __TLSINITI __TLSCONCAT(__tlsi_, TLSINIT) // initialisation object
#define __TLSINITC __TLSCONCAT(__tlsc_, TLSINIT) // callback

static void NTAPI __TLSINITC (PVOID image, DWORD reason, PVOID reserved);

#if defined(__GNUC__) || defined(__clang__)
__attribute__((section(".CRT$XLC"), used)) __TLSCALLBACK __TLSINITI = __TLSINITC;

#elif defined(_MSC_VER)
#   pragma warning(disable:4152)
#   if defined(_M_X64)                          // import TLS Directory (CRT:tlssup.asm)
#       pragma comment (linker, "/INCLUDE:_tls_used")
#   else
#       pragma comment (linker, "/INCLUDE:__tls_used")
#   endif
#   if (_MSC_VER >= 1600) || defined(_M_IA64)
#       pragma section(".CRT$XLC", long, read)
__declspec(allocate(".CRT$XLC")) extern __TLSCALLBACK __TLSINITI = __TLSINITC;
#   else
#       pragma data_seg(".CRT$XLC")
extern __TLSCALLBACK = __TLSINITC;
#       pragma data_seg()
#   endif

#elif defined(__WATCOMC__)
#   if !defined(__NT__) || !defined(__386__)
#   error NT-DLL32 target assumed, __NT__ and __386__ excepted
#   endif
#   if !defined(__SW_BD) && !defined(__SW_BM)
    // -bd (Win16/32 only):
    //
    //      This option causes the compiler to emit into the object file references to the run-time DLL startup code and,
    //      if required, special versions of the run-time libraries that support DLLs.
    //      The macro __SW_BD will be predefined if "bd" is selected.
    //
    //  -bm (Netware, OS/2, Win32 only):
    //      This option causes the compiler to emit into the object file references to the appropriate multi-threaded library name(s).
    //      The macros __MT and __SW_BM will be predefined if "bm" is selected
    //
#       if defined(DLL) || defined(_WINDLL)
#           error NT DLL target assumed, -bd option required
#       else
#           error NT Application target assumed, -bm option required
#       endif
#   endif

#pragma data_seg(".CRT$XLC", "DATA")
extern __TLSCALLBACK __TLSINITI = __TLSINITC;
#pragma data_seg()

#else
#   error Unsupported compiler
#endif


#if defined(__WATCOMC__)
#pragma off (check_stack)
#endif

static void NTAPI
__TLSINITC (void *image, DWORD reason, void *reserved)
{
    (void) image;
    (void) reserved;

#if defined(_MSC_VER)
    (void) __TLSINITI;                          // otherwise global optimisation may remove
#endif

    // DLL_THREAD_ATTACH, DLL_THREAD_DETACH, DLL_PROCESS_ATTACH and DLL_PROCESS_DETACH
    TLSINIT (reason);
}

#if defined(__WATCOMC__)
#pragma on (check_stack)
#endif

#if defined(__cplusplus)
}
#endif

#undef __TLSINITI
#undef __TLSINITC
#undef TLSINIT

//end

