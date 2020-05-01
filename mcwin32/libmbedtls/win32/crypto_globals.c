//$Id: crypto_globals.c,v 1.3 2020/05/01 14:33:00 cvsuser Exp $
//
//  libmbedcrypto support -
//      retrieve the dynamic fprintf/snprintf/printf implementations (if required)
//

#include "crypto_globals.h"

#if defined(MBEDTLS_PLATFORM_FPRINTF_ALT)
mbedtls_fprintf_t
get_mbedtls_fprintf(void) {
#undef mbedtls_fprintf
    return mbedtls_fprintf;
}
#endif


#if defined(MBEDTLS_PLATFORM_SNPRINTF_ALT)
mbedtls_snprintf_t
get_mbedtls_snprintf(void) {
#undef mbedtls_snprintf   
    return mbedtls_snprintf;
}
#endif


#if defined(MBEDTLS_PLATFORM_PRINTF_ALT)
mbedtls_printf_t
get_mbedtls_printf(void) {
#undef mbedtls_printf
    return mbedtls_printf;
}
#endif

//end

