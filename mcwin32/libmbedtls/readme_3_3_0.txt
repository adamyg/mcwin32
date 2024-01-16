
    Source: mbed TLS 3.3.0

        Makefile.in:

                o CRYPTO_SOURCES, updated
                o X509_SOURCES, updated
                o TLS_SOURCES, updated

        Definitions:

                libmbedcrypto.def, updated
                libmbedtls.def, updated
                libmbedx509.def, updated
                libmbedtls.rc, updated

        Compiler tweaks:

             library\psa_crypto.c(x16)

                -  psa_key_attributes_t attributes = {
                -          .core = slot->attr
                -  };
                +  psa_key_attributes_t attributes = {0}; //OWC
                +  attributes.core = slot->attr;


             library\ssl_debug_helpers_generated.c

                - [MBEDTLS_SSL_HELLO_REQUEST] = "MBEDTLS_SSL_HELLO_REQUEST",

                Note: language feature not supported, converted to switch ().

                + switch( value ) ...


             library\ssl_tls12_server.c(948):

                - MBEDTLS_SSL_DEBUG_MSG( 2, ( "=> parse client hello" ) );

                + MBEDTLS_SSL_DEBUG_MSG( 2, ( "=> parse client hello" ) );
                        moved after read_record_header_label:;

             include\mbedtls\platform.h:

                -  #if defined(__MINGW32__) || (defined(_MSC_VER) && _MSC_VER <= 1900)
                +  #if defined(__MINGW32__) || (defined(_MSC_VER) && _MSC_VER <= 1900) || defined(__WATCOMC__)
                   #define MBEDTLS_PLATFORM_HAS_NON_CONFORMING_SNPRINTF
                   #define MBEDTLS_PLATFORM_HAS_NON_CONFORMING_VSNPRINTF

                        -------

                   #if defined(MBEDTLS_PLATFORM_FPRINTF_ALT)
                   /* We need FILE * */
                   #include <stdio.h>
                +  #if defined(LIBMBED_WIN32) && !defined(LIBMBEDCRYPTO_SOURCE) //stdlib, linkage
                +  #include "crypto_globals.h"
                +  #define mbedtls_fprintf get_mbedtls_fprintf()
                +  #else
                   extern int (*mbedtls_fprintf)( FILE *stream, const char *format, ... );
                +  #endif

                        -------

                   #if defined(MBEDTLS_PLATFORM_PRINTF_ALT)
                +  #if defined(LIBMBED_WIN32) && !defined(LIBMBEDCRYPTO_SOURCE) //stdlib, linkage
                +  #include "crypto_globals.h"
                +  #define mbedtls_printf get_mbedtls_printf()
                +  #else
                   extern int (*mbedtls_printf)( const char * format, ... );
                +  #endif

                        -------

                   #if defined(MBEDTLS_PLATFORM_SNPRINTF_ALT)
                +  #if defined(LIBMBED_WIN32) && !defined(LIBMBEDCRYPTO_SOURCE) //stdlib, linkage
                +  #include "crypto_globals.h"
                +  #define mbedtls_snprintf get_mbedtls_snprintf()
                +  #else
                   extern int (*mbedtls_snprintf)( char * s, size_t n, const char * format, ... );
                +  #endif

                        -------

                   #if defined(MBEDTLS_PLATFORM_VSNPRINTF_ALT)
                   #include <stdarg.h>
                +  #if defined(LIBMBED_WIN32) && !defined(LIBMBEDCRYPTO_SOURCE) //stdlib, linkage
                +  #include "crypto_globals.h"
                +  #define mbedtls_vsnprintf get_mbedtls_vsnprintf()
                +  #else
                   extern int (*mbedtls_vsnprintf)( char * s, size_t n, const char * format, va_list arg );
                +  #endif

             include\mbedtls\platform_util.h

                +  #if defined(LIBMBED_WIN32) && defined(LIBMBEDCRYPTO_SOURCE) //stdlib, linkage
                +  #include "crypto_globals.h"
                +  CRYPTO_MBEDAPI struct tm *mbedtls_platform_gmtime_r( const mbedtls_time_t *tt,
                +                                                       struct tm *tm_buf );
                +  #else
                   struct tm *mbedtls_platform_gmtime_r( const mbedtls_time_t *tt,
                                                         struct tm *tm_buf );
                +  #endif

             library\platform_util.c:

                +  #if defined(_MSC_VER) || defined(__WATCOMC__) //stdlib, linkage
                +  static void * stdlib_memset( void *buf, int value, size_t len)
                +  {
                +      return memset( buf, value, len);
                +  }
                +
                +  static void * (* const volatile memset_func)( void *, int, size_t ) = stdlib_memset;
                +  #else
                   static void * (* const volatile memset_func)( void *, int, size_t ) = memset;
                +  #endif

                        -----

                   #if defined(_WIN32) && !defined(EFIX64) && !defined(EFI32)
                +  #if defined(__WATCOMC__)
                +    return gmtime_s( tt, tm_buf );
                +  #else
                +    return( ( gmtime_s( tm_buf, tt ) == 0 ) ? tm_buf : NULL );
                +  #endif

            library\ssl_tls.c:

                   #if defined(MBEDTLS_SSL_TLS_C)
                +  #if defined(_WINDLL) && defined(LIBMBED_DYNAMIC) //stdlib, linkage
                +  #include "x509_globals.h"
                +  #endif

                        -----

                +  #if defined(_WINDLL) && defined(LIBMBED_DYNAMIC) //stdlib, linkage
                +  conf->cert_profile = get_mbedtls_x509_crt_profile_suiteb();
                +  #else
                   conf->cert_profile = &mbedtls_x509_crt_profile_suiteb;
                +  #endf

                        -----

                +  #if defined(_WINDLL) && defined(LIBMBED_DYNAMIC) //stdlib, linkage
                +  conf->cert_profile = get_mbedtls_x509_crt_profile_default();
                +  #else
                   conf->cert_profile = &mbedtls_x509_crt_profile_default;
                +  #endif

//end
