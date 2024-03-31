//$Id: crypto_globals.c,v 1.7 2024/03/29 13:51:55 cvsuser Exp $
//
//  libmbedcrypto support -
//      retrieve the dynamic fprintf/snprintf/printf implementations (if required)
//

#include "crypto_globals.h"

#if defined(_MSC_VER) || defined(__WATCOMC__)
#pragma comment(lib, "BCrypt.lib") // BCryptGenRandom()
#endif

/*
 *  Global instances
 */

#if defined(MBEDTLS_PLATFORM_FPRINTF_ALT)
CRYPTO_MBEDAPI mbedtls_fprintf_t
get_mbedtls_fprintf(void) {
#undef mbedtls_fprintf
    return mbedtls_fprintf;
}
#endif


#if defined(MBEDTLS_PLATFORM_PRINTF_ALT)
CRYPTO_MBEDAPI mbedtls_printf_t
get_mbedtls_printf(void) {
#undef mbedtls_printf
    return mbedtls_printf;
}
#endif


#if defined(MBEDTLS_PLATFORM_SNPRINTF_ALT)
CRYPTO_MBEDAPI mbedtls_snprintf_t
get_mbedtls_snprintf(void) {
#undef mbedtls_snprintf
    return mbedtls_snprintf;
}
#else
int mbedtls_platform_set_snprintf(int (*snprintf_func)(char *s, size_t n, const char *format, ...)) {
    return 0; /*not-enabled*/
}
#endif


#if defined(MBEDTLS_PLATFORM_VSNPRINTF_ALT)
CRYPTO_MBEDAPI mbedtls_vsnprintf_t
get_mbedtls_vsnprintf(void) {
#undef mbedtls_vsnprintf
    return mbedtls_vsnprintf;
}
#else
int mbedtls_platform_set_vsnprintf(int (*vsnprintf_func)(char *s, size_t n, const char *format, va_list arg)) {
    return 0; /*not-enabled*/
}
#endif


/*
 *  Optional toolchain components, supply allowing common def's
 *
 *  see: platform.h/.c
 */

#if defined(__MINGW32__) || (defined(_MSC_VER) && _MSC_VER <= 1900) || defined(__WATCOMC__)
#define HAVE_MBEDTLS_PLATFORM_HAS_NON_CONFORMING_SNPRINTF
#define HAVE_MBEDTLS_PLATFORM_HAS_NON_CONFORMING_VSNPRINTF
#endif


#if !defined(HAVE_MBEDTLS_PLATFORM_HAS_NON_CONFORMING_SNPRINTF)
int mbedtls_platform_win32_snprintf( char *s, size_t n, const char *fmt, ... )
{
    int ret = -1;
    va_list argp;

    va_start( argp, fmt );
    ret = mbedtls_vsnprintf( s, n, fmt, argp );
    va_end( argp );

    return( ret );
}
#endif  //HAVE_MBEDTLS_PLATFORM_HAS_NON_CONFORMING_SNPRINTF


#if !defined(HAVE_MBEDTLS_PLATFORM_HAS_NON_CONFORMING_VSNPRINTF)
int mbedtls_platform_win32_vsnprintf( char *s, size_t n, const char *fmt, va_list arg )
{
    int ret = -1;

    /* Avoid calling the invalid parameter handler by checking ourselves */
    if( s == NULL || n == 0 || fmt == NULL )
        return( -1 );

#if defined(_TRUNCATE)
    ret = vsnprintf_s( s, n, _TRUNCATE, fmt, arg );
#else
    ret = vsnprintf( s, n, fmt, arg );
    if( ret < 0 || (size_t) ret == n )
    {
        s[n-1] = '\0';
        ret = -1;
    }
#endif

    return( ret );
}
#endif  //HAVE_MBEDTLS_PLATFORM_HAS_NON_CONFORMING_VSNPRINTF

//end

