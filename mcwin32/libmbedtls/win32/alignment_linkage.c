//$Id: alignment_linkage.c,v 1.1 2023/09/12 16:48:17 cvsuser Exp $
//
//  alignment support
//
//  3.4.1
//  Provide external definitions of some inline functions so that the compiler has the option to not inline them
//
//  Source: library/platform_util.c
//

#if defined(__MINGW32__)

#include "mbedtls/build_info.h"

#if (MBEDTLS_VERSION_NUMBER >= 0x03040100)

#include "../library/common.h"

#include "mbedtls/platform_util.h"

extern inline void mbedtls_xor(unsigned char *r,
                               const unsigned char *a,
                               const unsigned char *b,
                               size_t n);

extern inline uint16_t mbedtls_get_unaligned_uint16(const void *p);

extern inline void mbedtls_put_unaligned_uint16(void *p, uint16_t x);

extern inline uint32_t mbedtls_get_unaligned_uint32(const void *p);

extern inline void mbedtls_put_unaligned_uint32(void *p, uint32_t x);

extern inline uint64_t mbedtls_get_unaligned_uint64(const void *p);

extern inline void mbedtls_put_unaligned_uint64(void *p, uint64_t x);

#endif //MBEDTLS_VERSION_NUMBER

#endif //__MINGW32__

//end

