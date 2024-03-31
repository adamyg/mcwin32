
    Source: mbed TLS 3.5.2

        Makefile.in:

                o CRYPTO_SOURCES, updated
                        + aesce.c
                        + ecp_curves_new.c
                        - hash_info.c
                        u psa_crypto_driver_wrappers_no_static.c
                        + psa_crypto_ffdh.c
                        + sha3.c

                o X509_SOURCES, cloned
                        + x509write.c

                o TLS_SOURCES, cloned

                o Externals
                        + owc\aseni.obj [OpenWatcom only]
                        + Bcrypt.lib

                Source: /mbedtls-3.5.2/library/CMakefile.txt

        Definitions:

                libmbedcrypto.def
                        + mbedtls_asn1_write_algorithm_identifier_ext
                        - library\bignum_mod.c
                        - library\bignum_mod_raw.c
                        - library\constant_time.c
                        - library\hash_info.c
                        * library\oid.c [updated]
                        * library\platform_util.c
                            + mbedtls_zeroize_and_free
                            + mbedtls_ms_time
                        * library\sha3.c
                            + mbedtls_sha3_init
                            + mbedtls_sha3_free
                            + mbedtls_sha3_clone
                            + mbedtls_sha3_starts
                            + mbedtls_sha3_update
                            + mbedtls_sha3_finish
                            + mbedtls_sha3
                            + mbedtls_sha3_self_test

                libmbedtls.def

                libmbedx509.def

        Compiler tweaks:

            library\aesni.h

                + #if defined(__WATCOMC__)
                + #define MBEDTLS_AESNI_DECL __cdecl
                + #endif
                + #if !defined(MBEDTLS_AESNI_DECL)
                + #define MBEDTLS_AESNI_DECL
                + #endif

                - int mbedtls_aesni_has_support(unsigned int what);
                + int MBEDTLS_AESNI_DECL mbedtls_aesni_has_support(unsigned int what);

                - int mbedtls_aesni_crypt_ecb(mbedtls_aes_context *ctx,
                + int MBEDTLS_AESNI_DECL mbedtls_aesni_crypt_ecb(mbedtls_aes_context *ctx,

                - void mbedtls_aesni_inverse_key(unsigned char *invkey,
                + void MBEDTLS_AESNI_DECL mbedtls_aesni_inverse_key(unsigned char *invkey,

                - int mbedtls_aesni_setkey_enc(unsigned char *rk,
                + int MBEDTLS_AESNI_DECL mbedtls_aesni_setkey_enc(unsigned char *rk,

            library\aesni.c

                + #if defined(__MINGW32__)
                +       static int info[4] = { 0, 0, 0, 0 };
                + #else
                        static unsigned info[4] = { 0, 0, 0, 0 };
                + #endif
                - #if defined(_MSC_VER)
                + #if defined(_MSC_VER) || defined(__MINGW32__)

            library\psa_crypto.c (19 occurances)

                -  psa_key_attributes_t attributes = {
                -          .core = slot->attr
                -  };
                +  psa_key_attributes_t attributes = {0}; //OWC
                +  attributes.core = slot->attr;

              and:
                -  psa_key_attributes_t attributes = {
                -          .core = private_key->attr
                -  };
                +  psa_key_attributes_t attributes = {0}; //OWC
                +  attributes.core = private_key->attr;


            library\ssl_tls12_server.c(948):

                - MBEDTLS_SSL_DEBUG_MSG( 2, ( "=> parse client hello" ) );

                + MBEDTLS_SSL_DEBUG_MSG( 2, ( "=> parse client hello" ) );
                        read_record_header:;


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
                +  static void * (* const volatile memset_func)( void *, int, size_t ) = stdlib_memset;
                +  #elif ...
                   static void * (* const volatile memset_func)( void *, int, size_t ) = memset;

                        -------

                -  #elif defined(__STDC_LIB_EXT1__)
                +  #elif defined(__STDC_LIB_EXT1__) && !defined(__WATCOMC__)
                           memset_s(buf, len, 0, len);
                -  #elif defined(_WIN32)
                +  #elif defined(_WIN32) && !defined(__WATCOMC__)

             library\ssl_tls.c:

                   #if defined(MBEDTLS_SSL_TLS_C)
                +  #if defined(_WINDLL) && defined(LIBMBED_DYNAMIC) //stdlib, linkage
                +  #include "x509_globals.h"
                +  #endif

                        -------

                +  #if defined(_WINDLL) && defined(LIBMBED_DYNAMIC) //stdlib, linkage
                +  conf->cert_profile = get_mbedtls_x509_crt_profile_suiteb();
                +  #else
                   conf->cert_profile = &mbedtls_x509_crt_profile_suiteb;
                +  #endf

                        -------

                +  #if defined(_WINDLL) && defined(LIBMBED_DYNAMIC) //stdlib, linkage
                +  conf->cert_profile = get_mbedtls_x509_crt_profile_default();
                +  #else
                   conf->cert_profile = &mbedtls_x509_crt_profile_default;
                +  #endif

             library\x509_create.c:

                + #if defined(__WATCOMC__)
                + mbedtls_asn1_buf oid = {0};
                + #else
                  mbedtls_asn1_buf oid = { .p = NULL, .len = 0, .tag = MBEDTLS_ASN1_NULL };
                + #endif

                + #if defined(__WATCOMC__)
                + oid.tag = MBEDTLS_ASN1_NULL;
                + #endif

//end
