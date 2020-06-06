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
#define __STDC_WANT_LIB_EXT1__ 1	/*extended library prototypes*/

    /*
     *  WINAPI_FAMILY should be set to be one of these (8.1 greater):
     *      WINAPI_FAMILY_APP           1
     *      WINAPI_FAMILY_DESKTOP_APP   2
     *      WINAPI_FAMILY_PHONE_APP     3
     */
#endif  //__WATCOMC__

    /*
     *  Public Glib configuration
     */
#include "config.h.win32"

    /*
     *  Toolchain specific
     */

#if defined(__WATCOMC__)
    /*
     *  --- WATCOMC 1.9
     */
#define G_VA_COPY(dest,src) ((dest)[0]=(src)[0],(void)0)

extern void MemoryBarrier(void);
#pragma aux MemoryBarrier = "sfence";

#define _O_RANDOM       0
#define _O_SEQUENTIAL   0
#define _O_TEMPORARY    0               /*FIXME*/

#if !defined(__cplusplus)
#pragma disable_message(124)            /* Comparison result always 0 */
#pragma disable_message(136)            /* Comparison equivalent to 'unsigned == 0' */
#endif
#pragma disable_message(107)            /* Missing return value for functio 'xxx' */
#pragma disable_message(201)            /* Unreachable code */
#pragma disable_message(202)            /* Symbol 'xxx' has been defined, but not referenced */
#pragma disable_message(302)		/* Expression is only useful for its side effects */

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

#endif  /*WATCOMC || MSC_VER*/

#if defined(GPCRE_COMPILATION)
#define DG_LOG_DOMAIN	"GLib-GRegex"
#define NEWLINE         -1              /* issue passing "-1" as command line argument */
#endif

#endif  /*LOCAL_CONFIG_H_INCLUDED*/

