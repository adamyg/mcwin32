//$Id: x509_globals.h,v 1.2 2020/05/01 14:33:00 cvsuser Exp $
//
//  libmetlx509 support
//

#if defined(_WIN32)
#   if defined(LIBMBED_DYNAMIC)
#       if defined(LIBMBED_STATIC)
#           error LIBMBED_DYNAMIC and LIBMBED_STATIC defined
#       endif
#       if defined(LIBMBEDX509_SOURCE)
#           define MBEDAPI __declspec(dllexport)
#       else
#           define MBEDAPI __declspec(dllimport)
#       endif
#   else
#       if !defined(LIBMBED_STATIC)
#           define LIBMBED_STATIC
#       endif
#       define MBEDAPI extern
#   endif
#else
#   define MBEDAPI extern
#endif

#include "mbedtls/x509_crt.h"

MBEDAPI const mbedtls_x509_crt_profile * get_mbedtls_x509_crt_profile_default(void);
MBEDAPI const mbedtls_x509_crt_profile * get_mbedtls_x509_crt_profile_next(void);
MBEDAPI const mbedtls_x509_crt_profile * get_mbedtls_x509_crt_profile_suiteb(void);
