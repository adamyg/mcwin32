//$Id: x509_globals.c,v 1.6 2025/06/12 18:02:33 cvsuser Exp $
//
//  libmetlx509 support
//

#if defined(HAVE_X509_INET_XTOX)
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x600)
#undef  WINVER
#undef  _WIN32_WINNT
#define WINVER 0x600
#define _WIN32_WINNT 0x600
#endif
#endif

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


/**
 *  inet_pton - convert IPv4 and IPv6 addresses from text to binary form.
 *  inet_ntop - convert IPv4 and IPv6 addresses from binary to text.
 */

#if defined(HAVE_X509_INET_XTOX)

#if !defined(_WINSOCK_DEPRECATED_NO_WARNINGS)
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#endif

#include <WinSock2.h>
#include <Ws2ipdef.h>

int
/*mbedtls_*/ x509_inet_pton(int af, const char *src, void *dst)
{
    int rc = -1, srclen = (src ? strlen(src) : 0);

    if (af == AF_INET6) {
        struct sockaddr_in6 sockaddr = {0};

        rc = WSAStringToAddressA((char *)src, AF_INET6, NULL, (struct sockaddr *)&sockaddr, &srclen);
        memcpy(dst, &sockaddr.sin6_addr.s6_addr, sizeof(sockaddr.sin6_addr.s6_addr));

    } else {
        struct sockaddr_in sockaddr = {0};

        rc = WSAStringToAddressA((char *)src, AF_INET, NULL, (struct sockaddr *)&sockaddr, &srclen);
        memcpy(dst, &sockaddr.sin_addr.s_addr, sizeof(sockaddr.sin_addr.s_addr));
    }
    return rc;
}


const char *
/*mbedtls_*/ x509_inet_ntop(int af, const void *src, char *dst, size_t /*socklen_t*/ size)
{
    struct sockaddr_storage ss = {0};
    unsigned long s = (unsigned long)size;

    ss.ss_family = af;
    switch (af) {
    case AF_INET:
        ((struct sockaddr_in *)&ss)->sin_addr = *(struct in_addr *)src;
        break;
    case AF_INET6:
        ((struct sockaddr_in6 *)&ss)->sin6_addr = *(struct in6_addr *)src;
        break;
    default:
        return NULL;
    }
    return (WSAAddressToStringA((struct sockaddr *)&ss, sizeof(ss), NULL, dst, &s) == 0) ? dst : NULL;
}

#endif //HAVE_X509_INET_XTOX

//end

