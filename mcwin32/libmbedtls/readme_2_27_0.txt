    Source: mbed TLS 2.27.0

	Makefile.in:

		TLS_SOURCES=
		+ ssl_msg.c
		+ ssl_tls13_keys.c

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

	win32\libmbedcrypto.def:

		; library\cipher.c
		+ mbedtls_cipher_auth_encrypt_ext
		+ mbedtls_cipher_auth_decrypt_ext

		; library\ans1write.c
		+ mbedtls_asn1_write_named_bitstring

		; library\oid.c
		+ mbedtls_oid_get_certificate_policies

	win32\libmbedx509.def:

--end--
