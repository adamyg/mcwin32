//$Id: x509_globals.c,v 1.4 2023/09/12 16:48:17 cvsuser Exp $
//
//  libmetlx509 support
//

#include "x509_globals.h"


MBEDAPI const mbedtls_x509_crt_profile *
get_mbedtls_x509_crt_profile_default(void) {
    return &mbedtls_x509_crt_profile_default;
}


MBEDAPI const mbedtls_x509_crt_profile *
get_mbedtls_x509_crt_profile_next(void) {
    return &mbedtls_x509_crt_profile_next;
}


MBEDAPI const mbedtls_x509_crt_profile *
get_mbedtls_x509_crt_profile_suiteb(void) {
    return &mbedtls_x509_crt_profile_suiteb;
}

//end

