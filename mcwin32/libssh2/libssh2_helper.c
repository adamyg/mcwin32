//$Id: libssh2_helper.c,v 1.1 2020/05/01 14:33:15 cvsuser Exp $
//
//  libssh2 support
//

#define  LIBSSH2_LIBRARY
#include <libssh2_helper.h>


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
    return "mbedtls";
    
#elif defined(LIBSSH2_WINCNG)
    return "wincng";

#else
#error Unknown engine configuration
#endif
}
 
//end

