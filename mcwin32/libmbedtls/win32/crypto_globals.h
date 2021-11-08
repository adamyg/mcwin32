#ifndef CRYPTO_GLOBALS_H_INCLUDED
#define CRYPTO_GLOBALS_H_INCLUDED
#pragma once
//$Id: crypto_globals.h,v 1.3 2021/11/08 13:13:02 cvsuser Exp $
//
//  libmbedcrypto support
//

#if defined(_WIN32)
#   if defined(LIBMBED_DYNAMIC)
#       if defined(LIBMBED_STATIC)
#           error LIBMBED_DYNAMIC and LIBMBED_STATIC defined
#       endif
#       if defined(LIBMBEDCRYPTO_SOURCE)
#           define CRYPTO_MBEDAPI __declspec(dllexport)
#       else
#           define CRYPTO_MBEDAPI __declspec(dllimport)
#       endif
#   else
#       if !defined(LIBMBED_STATIC)
#           define LIBMBED_STATIC
#       endif
#       define CRYPTO_MBEDAPI extern
#   endif
#else
#   define CRYPTO_MBEDAPI extern
#endif

#include "mbedtls/platform.h"
#include <stdarg.h>

typedef int (*mbedtls_fprintf_t)(FILE *stream, const char *format, ...);
typedef int (*mbedtls_printf_t)(const char *format, ...);
typedef int (*mbedtls_snprintf_t)(char * s, size_t n, const char * format, ...);
typedef int (*mbedtls_vsnprintf_t)(char * s, size_t n, const char * format, va_list arg);

#if defined(MBEDTLS_PLATFORM_FPRINTF_ALT)
CRYPTO_MBEDAPI mbedtls_fprintf_t get_mbedtls_fprintf(void);
#endif
#if defined(MBEDTLS_PLATFORM_PRINTF_ALT)
CRYPTO_MBEDAPI mbedtls_printf_t get_mbedtls_printf(void);
#endif
#if defined(MBEDTLS_PLATFORM_SNPRINTF_ALT)
CRYPTO_MBEDAPI mbedtls_snprintf_t get_mbedtls_snprintf(void);
#endif
#if defined(MBEDTLS_PLATFORM_VSNPRINTF_ALT)
CRYPTO_MBEDAPI mbedtls_vsnprintf_t get_mbedtls_vsnprintf(void);
#endif

#endif //CRYPTO_GLOBALS_H_INCLUDED

