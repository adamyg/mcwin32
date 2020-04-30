
    Source: mbed TLS 2.16.6 (Apache)

	include\mbedtls\platform.h
		
		   #if defined(_WIN32) && !defined(LIBMBEDCRYPTO_SOURCE) //stdlib, linkage
		+  #include "crypto_globals.h"
		+  #define mbedtls_fprintf   get_mbedtls_fprintf()
		+  #else
		   extern int (*mbedtls_fprintf)( FILE *stream, const char *format, ... );
		+  #endif

			-------

		+  #if defined(_WIN32) && !defined(LIBMBEDCRYPTO_SOURCE) //stdlib, linkage
		+  #include "crypto_globals.h"
		+  #define mbedtls_printf get_mbedtls_printf()
		+  #else
		   extern int (*mbedtls_printf)( const char * format, ... );
		+  #endif

			-------

		+  #if defined(_WIN32) && !defined(LIBMBEDCRYPTO_SOURCE) //stdlib, linkage
		+  #include "crypto_globals.h"
		+  #define mbedtls_snprintf get_mbedtls_snprintf()
		+  #else
		   extern int (*mbedtls_snprintf)( char * s, size_t n, const char * format, ... );
		+  #endif


	include\mbedtls\platform_util.h

		+  #if defined(_WIN32) //stdlib, linkage
		+  #include "crypto_globals.h"
		+  CRYPTO_MBEDAPI struct tm *mbedtls_platform_gmtime_r( const mbedtls_time_t *tt,
		+                                                       struct tm *tm_buf );
		+  #else
		   struct tm *mbedtls_platform_gmtime_r( const mbedtls_time_t *tt,
		                                         struct tm *tm_buf );
		+  #endif


	library\platform_util.c

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
		+    return( gmtime_s( tt, &tm_buf ));
		+  #else
		+    return( gmtime_s( &tm_buf, tt ));
		+  #endif


	library\ssl_tls.c

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


	win32\libmbedcrypto.def

		   ; library\asn1write.c

		+  mbedtls_asn1_write_tagged_string

			-----

		   ; library\pk.c

		+ mbedtls_pk_verify_restartable
		+ mbedtls_pk_sign_restartable

			-----

		   ; library\ecdh.c

		+ mbedtls_ecdh_setup


	win32\libmbedx509.def

		   ; library\x509.c

		+  mbedtls_x509_crt_verify_restartable_

			-----

		   ; library\x509_crt.c

		+  mbedtls_x509_crt_verify_restartable


--end--

