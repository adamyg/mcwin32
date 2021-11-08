//$Id: crypto_globals.c,v 1.4 2021/11/08 13:13:02 cvsuser Exp $
//
//  libmbedcrypto support -
//      retrieve the dynamic fprintf/snprintf/printf implementations (if required)
//

#include "crypto_globals.h"

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
#endif


#if defined(MBEDTLS_PLATFORM_SNPRINTF_ALT)
CRYPTO_MBEDAPI mbedtls_vsnprintf_t
get_mbedtls_vsnprintf(void) {
#undef mbedtls_vsnprintf   
    return mbedtls_vsnprintf;
}
#endif

//end

