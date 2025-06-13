//$Id: x509_globals.h,v 1.3 2025/06/12 18:02:33 cvsuser Exp $
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

#if defined(HAVE_X509_INET_XTOX)
int x509_inet_pton(int af, const char *src, void *dst);
const char *x509_inet_ntop(int af, const void *src, char *dst, size_t /*socklen_t*/ size);
#endif

//end
