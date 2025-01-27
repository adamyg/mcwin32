#if !defined(LOCAL_CONFIG_H_INCLUDED)
#define LOCAL_CONFIG_H_INCLUDED
/*
 *  local config.h
 */

#if defined(__WATCOMC__)
    /*
     *  close emulation of msvc; heavy reduction in code changes result.
     */
#define _MSC_VER 1200
#if !defined(__STDC_WANT_LIB_EXT1__)
#define __STDC_WANT_LIB_EXT1__ 1        /* extended library prototypes */
#endif
#endif //__WATCOMC__

    /*
     *  Public Glib configuration
     */
#undef  _WIN32_WINNT                    /* package dependent */
#undef  _WIN32_VER

#include "config.h.win32"               /* see: include-xx.xx.xx */
#undef HAVE_DIRENT_H

    /*
     *  Toolchain specific
     */
     
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wcast-function-type"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wempty-body"
#pragma GCC diagnostic ignored "-Wmemset-elt-size"
#pragma GCC diagnostic ignored "-Wint-in-bool-context"
#pragma GCC diagnostic ignored "-Wint-conversion"
#pragma GCC diagnostic ignored "-Wformat"
#endif //__GNUC__

#if defined(__WATCOMC__)
    /*
     *  --- WATCOMC 1.9+
     */
#if (__WATCOMC__ >= 1300)
#define HAVE_STDINT_H 1                 /* uintptr_t */
#endif

#define G_VA_COPY(dest,src) ((dest)[0]=(src)[0],(void)0)

extern void MemoryBarrier(void);
#pragma aux MemoryBarrier = "sfence";

#define _O_RANDOM       0
#define _O_SEQUENTIAL   0
#define _O_TEMPORARY    0               /* FIXME */

#if !defined(__cplusplus)
#pragma disable_message(124)            /* Comparison result always 0 */
#pragma disable_message(136)            /* Comparison equivalent to 'unsigned == 0' */
#endif
#pragma disable_message(107)            /* Missing return value for functio 'xxx' */
#pragma disable_message(201)            /* Unreachable code */
#pragma disable_message(202)            /* Symbol 'xxx' has been defined, but not referenced */
#pragma disable_message(302)            /* Expression is only useful for its side effects */
#pragma disable_message(303)            /* Parameter 'xxx' has been defined, but not referenced */

#elif defined(_MSC_VER)
    /*
     *  --- MSVC
     */
#include "msvc_recommended_pragmas.h"

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#elif defined(__MINGW32__)
    /*
     *  --- MINGW32/64
     */
#  if !defined(GLIB_STATIC_COMPILATION)
#    ifdef GLIB_COMPILATION
#      ifdef DLL_EXPORT
#        define GLIB_VAR extern __declspec(dllexport) /*remove multiple definition warnings */
#      else
#        define GLIB_VAR extern
#      endif
#    endif /* GLIB_COMPILATION */
#  endif /* GLIB_STATIC_COMPILATION */

#endif  /*WATCOMC || MSC_VER || MINGW32 */

#if defined(GPCRE_COMPILATION)
#define DG_LOG_DOMAIN   "GLib-GRegex"
#define NEWLINE         -1              /* issue passing "-1" as command line argument */
#endif

#endif  /*LOCAL_CONFIG_H_INCLUDED*/
