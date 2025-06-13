
    Source: mbed TLS 3.6.2
    
        Notes:
        
                2017+ only: https://github.com/Mbed-TLS/mbedtls/issues/8770

        Makefile.in:

                o CRYPTO_SOURCES: additional
                o X509_SOURCES: unchanged
                o TLS_SOURCES: unchanged
    
        Compiler tweaks:

            library\platform.h: (DLL support)

                +   #if defined(_WIN32) && !defined(LIBMBED_STATIC)
                +   #if defined(LIBMBEDCRYPTO_SOURCE)
                +   __declspec(dllexport)           
                +   #else                           
                +   __declspec(dllimport)           
                +   #endif                          
                +   #endif
                    extern int (*mbedtls_fprintf)(FILE *stream, const char *format, ...);

                +   #if defined(_WIN32) && !defined(LIBMBED_STATIC)
                +   #if defined(LIBMBEDCRYPTO_SOURCE)
                +   __declspec(dllexport)           
                +   #else                           
                +   __declspec(dllimport)           
                +   #endif                          
                +   #endif
                    extern int (*mbedtls_printf)(const char *format, ...);
            
                +   #if defined(_WIN32) && !defined(LIBMBED_STATIC)
                +   #if defined(LIBMBEDCRYPTO_SOURCE)
                +   __declspec(dllexport)
                +   #else
                +   __declspec(dllimport)
                +   #endif
                    extern int (*mbedtls_snprintf)(char *s, size_t n, const char *format, ...);

                +   #if defined(_WIN32) && !defined(LIBMBED_STATIC)
                +   #if defined(LIBMBEDCRYPTO_SOURCE)
                +   __declspec(dllexport)
                +   #else
                +   __declspec(dllimport)
                +   #endif
                    extern int (*mbedtls_vsnprintf)(char *s, size_t n, const char *format, va_list arg);

                +   #if defined(_WIN32) && !defined(LIBMBED_STATIC)
                +   #if defined(LIBMBEDCRYPTO_SOURCE)
                +   __declspec(dllexport)
                +   #else
                +   __declspec(dllimport)
                +   #endif
                    extern void (*mbedtls_setbuf)(FILE *stream, char *buf);

                +   #if defined(_WIN32) && !defined(LIBMBED_STATIC)
                +   #if defined(LIBMBEDCRYPTO_SOURCE)
                +   __declspec(dllexport)
                +   #else
                +   __declspec(dllimport)
                +   #endif
                    extern void (*mbedtls_exit)(int status);

            library\psa_util_internal.h: (DLL support)

                    #if defined(MBEDTLS_USE_PSA_CRYPTO) || defined(MBEDTLS_SSL_PROTO_TLS1_3)
                +   #if defined(_WIN32) && !defined(LIBMBED_STATIC)
                +   #if defined(LIBMBEDCRYPTO_SOURCE)
                +   __declspec(dllexport)
                +   #else
                +   __declspec(dllimport)
                +   #endif
                +   #endif //WIN32

             library\platform_util.c:

                +  #if defined(__WATCOMC__)
                +  static void * owcmemset( void *buf, int value, size_t len) {
                +      return memset( buf, value, len);
                +  }
                +  static void * (* const volatile owc_memset_func)( void *, int, size_t ) = owcmemset;
                +  #endif

                        :

                +  #elif defined(__WATCOMC__)
                +       owc_memset_func(buf, 0, len);

            library\psa_crypto_core.h: (OWC support)

                +   #if defined(__WATCOMC__)
                +   #define PSA_CRYPTO_LOCAL_INPUT_INIT { NULL, 0 }
                +   #else
                    #define PSA_CRYPTO_LOCAL_INPUT_INIT ((psa_crypto_local_input_t) { NULL, 0 })
                +   #endif

                +   #if defined(__WATCOMC__)
                +   #define PSA_CRYPTO_LOCAL_OUTPUT_INIT { NULL, NULL, 0 }
                +   #else
                    #define PSA_CRYPTO_LOCAL_OUTPUT_INIT ((psa_crypto_local_output_t) { NULL, NULL, 0 })
                +   #endif
                
            library\psa_core.c: (OWC support)
                
                +   #if defined(__WATCOMC__)
                +       {  psa_crypto_local_input_t t_local_input = PSA_CRYPTO_LOCAL_INPUT_INIT;
                +          *local_input = t_local_input;
                +       }
                +   #else
                        *local_input = PSA_CRYPTO_LOCAL_INPUT_INIT;
                +   #endif

                +   #if defined(__WATCOMC__)
                +       {   psa_crypto_local_output_t t_local_output = PSA_CRYPTO_LOCAL_OUTPUT_INIT;
                +           *local_output = t_local_output;
                +       }
                +   #else
                        *local_output = PSA_CRYPTO_LOCAL_OUTPUT_INIT;
                +   #endif
                    
            library\aesni.h: (OWC support)

                  #define MBEDTLS_AESNI_HAVE_CODE 1 // via assembly
                + #elif defined(__WATCOMC__)
                + #define MBEDTLS_AESNI_HAVE_CODE 1 // via assembly: see win32\aesni.asm
                                                   
                  #if defined(MBEDTLS_AESNI_HAVE_CODE)
                + #if defined(__WATCOMC__)
                + #define MBEDTLS_AESNI_DECL __cdecl
                + #endif
                + #if !defined(MBEDTLS_AESNI_DECL)
                + #define MBEDTLS_AESNI_DECL
                + #endif

                  #if !defined(MBEDTLS_AES_USE_HARDWARE_ONLY)
                - int mbedtls_aesni_has_support(unsigned int what);
                + int MBEDTLS_AESNI_DECL mbedtls_aesni_has_support(unsigned int what);

                - int mbedtls_aesni_crypt_ecb(mbedtls_aes_context *ctx,
                + int MBEDTLS_AESNI_DECL mbedtls_aesni_crypt_ecb(mbedtls_aes_context *ctx,
                
                - void mbedtls_aesni_gcm_mult(unsigned char c[16],
                - void MBEDTLS_AESNI_DECL mbedtls_aesni_gcm_mult(unsigned char c[16],

                - void mbedtls_aesni_inverse_key(unsigned char *invkey,
                + void MBEDTLS_AESNI_DECL mbedtls_aesni_inverse_key(unsigned char *invkey,

                - int mbedtls_aesni_setkey_enc(unsigned char *rk,
                + int MBEDTLS_AESNI_DECL mbedtls_aesni_setkey_enc(unsigned char *rk,

            include\x509_crt.h: (DLL support)

                +   #if defined(_WIN32) && !defined(LIBMBED_STATIC)
                +   #if defined(LIBMBEDX509_SOURCE)
                +   __declspec(dllexport)
                +   #else
                +   __declspec(dllimport)
                +   #endif
                +   #endif
                    extern const mbedtls_x509_crt_profile mbedtls_x509_crt_profile_default;

                +   #if defined(_WIN32) && !defined(LIBMBED_STATIC) && !defined(LIBMBEDX509_SOURCE)
                +   __declspec(dllimport)
                +   #endif
                    extern const mbedtls_x509_crt_profile mbedtls_x509_crt_profile_next;

                +   #if defined(_WIN32) && !defined(LIBMBED_STATIC) && !defined(LIBMBEDX509_SOURCE)
                +   __declspec(dllimport)
                +   #endif
                    extern const mbedtls_x509_crt_profile mbedtls_x509_crt_profile_suiteb;

                +   #if defined(_WIN32) && !defined(LIBMBED_STATIC) && !defined(LIBMBEDX509_SOURCE)
                +   __declspec(dllimport)
                +   #endif
                    extern const mbedtls_x509_crt_profile mbedtls_x509_crt_profile_none;
                    
            library\x590_crt.c: (Legacy support)
                   
                -  #ifdef _MSC_VER
                +  #if defined(_MSC_VER) && _WIN32_WINNT >= 0x0600 //#ifdef _MSC_VER

            library\x509_create.c: (OWC support)

                +   #if !defined(__WATCOMC__)    
                    mbedtls_asn1_buf oid = {};
                +   #else
                    mbedtls_asn1_buf oid = { .p = NULL, .len = 0, .tag = MBEDTLS_ASN1_NULL };
                +   #endif
                        :
                +   #if defined(__WATCOMC__)    
                +   oid.tag = MBEDTLS_ASN1_NULL;
                +   #endif

//end

