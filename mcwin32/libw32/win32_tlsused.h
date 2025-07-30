/* -*- mode: c; indent-width: 4; -*- */
/* $Id: win32_tlsused.h,v 1.3 2025/07/30 13:42:31 cvsuser Exp $ */
/*
 *  TLS Directory reference/definition.
 */

#if !defined(WINDOWS_MEAN_AND_LEAN)
#define WINDOWS_MEAN_AND_LEAN
#include <Windows.h>
#endif

//////////////////////////////////////////////////////////////////////////////////////
//      TLS support

#if defined(_MSC_VER)
#   if defined(_M_X64)                          // import TLS Directory.
#       pragma comment (linker, "/INCLUDE:_tls_used")
#   else
#       pragma comment (linker, "/INCLUDE:__tls_used")
#   endif
#endif


    //////////////////////////////////////////////////////////////////////////////////

#if defined(__WATCOMC__)

#if !defined(__NT__) || !defined(__386__)
#   error expect __NT__ __386__
#endif
#   if !defined(__SW_BD) && !defined(__SW_BM)
    // -bd (Win16/32 only):
    //
    //      This option causes the compiler to emit into the object file references to the run-time DLL startup code and,
    //      if required, special versions of the run-time libraries that support DLLs.
    //      The macro __SW_BD will be predefined if "bd" is selected.
    //
    //  -bm (Netware, OS/2, Win32 only):
    //
    //      This option causes the compiler to emit into the object file references to the appropriate multi-threaded library name(s).
    //      The macros __MT and __SW_BM will be predefined if "bm" is selected
    //
#       if defined(DLL) || defined(_WINDLL)
#           error NT DLL target assumed, -bd option required
#       else
#           error NT Application target assumed, -bm option required
#       endif
#   endif

#if defined(__cplusplus)
extern "C" {
#endif

//      Example:
//
//      Segment                Class          Group          Address         Size
//      =======                =====          =====          =======         ====
//
//      .tls                   TLS            TLS            00402000        00000004
//      .tls$                  TLS            TLS            00402004        00000004
//      .tls$ZZZ               TLS            TLS            00402008        00000004
//
//      .CRT$XLA               DATA           DGROUP         0040409c        00000004
//      .CRT$XLC               DATA           DGROUP         004040a0        00000004
//      .CRT$XLZ               DATA           DGROUP         004040a4        00000004
//
//      Address        Symbol
//      =======        ======
//
//      00402000       __tls_start
//                      : local tls variables
//      00402008       __tls_end
//

#if !defined(__PIMAGE_TLS_CALLBACK_T)           // missing fron OWC NT SDK
typedef void (NTAPI *PIMAGE_TLS_CALLBACK)(PVOID image, DWORD reason, PVOID reserved);
#define __PIMAGE_TLS_CALLBACK_T
#endif

ULONG __based( __segname( ".tls" ) ) _tls_start = (ULONG)&_tls_start;
ULONG __based( __segname( ".tls$ZZZ" ) ) _tls_end = (ULONG)&_tls_end;
ULONG _tls_index;

#pragma data_seg(".CRT$XLA", "DATA")
PIMAGE_TLS_CALLBACK _xl_a[] = { NULL };         // '__xl_a'; callback vector base
#pragma data_seg()

#pragma data_seg(".CRT$XLC", "DATA")
#pragma data_seg()

#pragma data_seg(".CRT$XLZ", "DATA")
PIMAGE_TLS_CALLBACK _xl_z[] = { NULL };         // '__xl_z'; NULL terminator
        // NULL terminator for TLS callback array.
        // This symbol, __xl_z, is never actually referenced, yet it must exist.
        // The OS loader code walks the TLS callback array, based at _xl_a, until it finds a NULL pointer,
        // so this makes sure the array is properly terminated.
#pragma data_seg()

#pragma data_seg(".rdata$T")
        // The Microsoft linker (and compatible linkers) uses the symbol __tls_used (on x86/x64 systems)
        // or _tls_used (on non-x86 systems) as the address of the TLS Directory.
        //
        // The TLS Directory contains a pointer to a zero terminated array of TLS callbacks.
        // So by creating a suitable TLS Directory and giving it the name __tls_used/_tls_used
        // you can have a TLS callback function in assembly code.
        //
ULONG _tls_used[] = {
        // IMAGE_TLS_DIRECTORY, TLS Directory
        // linker special symbol name '__tls_used'
        (ULONG)  &_tls_start,
        (ULONG)  &_tls_end,
        (ULONG)  &_tls_index,
        (ULONG) (&_xl_a + 1),                   // skip NULL
        0,
        0
};
#pragma data_seg()

extern ULONG _tls_array;                        // clbrdll.lib(tlsawnt.asm)

#pragma off (check_stack)                       // disable stack checks
void __declspec(naked)                          // generates code without prolog and epilog code.
_tls_region(void)
{       // reference declwnt.asm
        _asm {
                push      edx
                mov       eax, dword ptr fs:_tls_array
                mov       edx, dword ptr _tls_index
                mov       eax, dword ptr [eax + edx * 4]
                sub       eax, dword ptr _tls_used
                pop       edx
                ret
        }
}
#pragma aux _tls_region "_*";                   // '__tls_region'

#pragma on (check_stack)

#if defined(__cplusplus)
}
#endif

#endif // __WATCOMC__

//end

