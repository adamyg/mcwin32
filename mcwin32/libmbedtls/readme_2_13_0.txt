
	Source: mbed TLS 2.13.0 (Apache)

	include\libmbedtls\platform.h

	     -  extern int (*mbedtls_fprintf)( FILE *stream, const char *format, ... );

	        #if defined(_WIN32) && !defined(LIBMBEDCRYPTO_SOURCE)
	     +  #include "crypto_globals.h"
	     +  #define mbedtls_fprintf   get_mbedtls_fprintf()
	     +  #else
	     +  extern int (*mbedtls_fprintf)( FILE *stream, const char *format, ... );
	     +  #endif

			-------

	     -  extern int (*mbedtls_printf)( const char * format, ... );

	     +  #if defined(_WIN32) && !defined(LIBMBEDCRYPTO_SOURCE)
	     +  #include "crypto_globals.h"
	     +  #define mbedtls_printf    get_mbedtls_printf()
	     +  #else
	     +  extern int (*mbedtls_printf)( const char * format, ... );
	     +  #endif

			-------

	     -  extern int (*mbedtls_snprintf)( char * s, size_t n, const char * format, ... );

	     +  #if defined(_WIN32) && !defined(LIBMBEDCRYPTO_SOURCE)
	     +  #include "crypto_globals.h"
	     +  #define mbedtls_snprintf  get_mbedtls_snprintf()
	     +  #else
	     +  extern int (*mbedtls_snprintf)( char * s, size_t n, const char * format, ... );
	     +  #endif

	include\libmbedtls\platform_util.h

	     //stdlib linkage
	     -  static void * (* const volatile memset_func)( void *, int, size_t ) = memset;

	     +  static void * stdlib_memset( void *buf, int value, size_t len)
	     +  {
	     +      return memset( buf, value, len);
	     +  }
	     +  static void * (* const volatile memset_func)( void *, int, size_t ) = stdlib_memset;

	include\libmbedtls\ssl_tls.h

		#if defined(MBEDTLS_X509_CRT_PARSE_C)
		#include "mbedtls/oid.h"
		#endif

	     +  #if defined(_WIN32)
	     +  #include "x509_globals.h"
	     +  #endif

			-------

	     +  #if defined(_WIN32)
	     +  conf->cert_profile = get_mbedtls_x509_crt_profile_suiteb();
	     +  #else
	        conf->cert_profile = &mbedtls_x509_crt_profile_suiteb;
	     +  #endf

			-------

	    +   #if defined(_WIN32)
	    +   conf->cert_profile = get_mbedtls_x509_crt_profile_default();
	    +   #else
	        conf->cert_profile = &mbedtls_x509_crt_profile_default;
	    +   #endif

	library\x590.c

	     -  lt = gmtime_s( &tm_buf, &tt ) == 0 ? &tm_buf : NULL;
	     +  #if defined(__WATCOMC__)
	     +      lt = gmtime_s( (const tm *)&tt, &tm_buf );
	     +  #else
	     +      lt = (gmtime_s( &tm_buf, (const tm *)&tt ) == 0) ? &tm_buf : NULL;
	     +  #endif

	win32\libmbedcrypto.def

	        ; library\asn1write.c
	     +     mbedtls_asn1_write_tagged_string
     
    

