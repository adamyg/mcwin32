//$Id: libssh2_helper.c,v 1.3 2025/02/15 18:27:32 cvsuser Exp $
//
//  libssh2 support
//

#define  LIBSSH2_LIBRARY
#include <libssh2_helper.h>

#include <mbedtls/version.h>

#if defined(_MSC_VER) || defined(__WATCOMC__)
#pragma comment(lib, "BCrypt.lib") // BCryptGenRandom(), +3.5.0
#endif


LIBSSH2_API int
libssh2_helper_trace(void)
{
#if defined(LIBSSH2DEBUG)
    return 1;
#else
    return 0;
#endif
}


LIBSSH2_API const char *
libssh2_helper_engine(void)
{
#if defined(LIBSSH2_OPENSSL)
    return "openssl";
    
#elif defined(LIBSSH2_MBEDTLS)
    static char string[32];
    if (string[0] == 0)
        mbedtls_version_get_string_full(string); //18 bytes
    return (string[0] ? string : "mbedtls");
    
#elif defined(LIBSSH2_WINCNG)
    return "wincng";

#else
#error Unknown engine configuration
#endif
}
 
//end

