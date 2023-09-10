
    Source: libssh2 1.11.0, code changes

        Makefile.in:

                + agent_win$(O)
                + mbedtls$(O)

        win32\

                + libssh2_config.h

        src\libssh2_priv.h:

                  #include <time.h>
                + #include <limits.h> //UINT_MAX

        src\mbedtls.h:

              520: int
                   _libssh2_mbedtls_rsa_sha1_verify(libssh2_rsa_ctx *rsa,
                                                    const unsigned char *sig,
                -                                   unsigned long sig_len,
                +                                   size_t /*unsigned long*/ sig_len,
                                                    const unsigned char *m,
                -                                   unsigned long m_len);
                +                                   size_t /*unsigned long*/ m_len);

              539:
                   int
                   _libssh2_mbedtls_rsa_sha2_verify(libssh2_rsa_ctx * rsactx,
                                                    size_t hash_len,
                                                    const unsigned char *sig,
                                                    unsigned long sig_len,
                -                                   const unsigned char *m, unsigned long m_len);
                +                                   const unsigned char *m, size_t /*unsigned long*/ m_len);

        src\mbedtls.c:

              502:                  
                   int
                   _libssh2_mbedtls_rsa_sha2_verify(libssh2_rsa_ctx * rsactx,
                                                    size_t hash_len,
                                                    const unsigned char *sig,
                -                                   unsigned long sig_len,
                +                                   size_t /*unsigned long*/ sig_len,
                -                                   const unsigned char *m, unsigned long m_len)
                +                                   const unsigned char *m, size_t /*unsigned long*/ m_len)

              553:
                   int
                   _libssh2_mbedtls_rsa_sha1_verify(libssh2_rsa_ctx * rsactx,
                                                    const unsigned char *sig,
                -                                   unsigned long sig_len,
                +                                   size_t /*unsigned long*/ sig_len,
                -                                    const unsigned char *m, unsigned long m_len)
                +                                   const unsigned char *m, size_t /*unsigned long*/ m_len)

        src\misc.h:

                97:
                -   #ifdef WIN32
                +   #if defined(WIN32) && !defined(__WATCOMC__)

        src\misc.c:
        
                773:
                    #ifdef LIBSSH2_MEMZERO
                +   #if defined(__WATCOMC__)
                +   static void *safememset(void *a, int b, size_t c) 
                +   {
                +       return memset(a, b, c);
                +   }
                +   static void * (__watcall * const volatile memset_libssh)(void *, int, size_t) = safememset;
                +   #else
                    static void * (* const volatile memset_libssh)(void *, int, size_t) = memset;
                +   #endif
        

                  #ifdef WIN32
                + #if defined(__WATCOMC__)
                +  static void *safememset(void *a, int b, size_t c) {
                +      return memset(a, b, c);
                +  }
                +  static void * (__watcall * const volatile memset_libssh)(void *, int, size_t) =
                +      safememset;
                + #else
                  static void * (__cdecl * const volatile memset_libssh)(void *, int, size_t) =
                       memset;
                + #endif
                  #else
                  static void * (* const volatile memset_libssh)(void *, int, size_t) = memset;
                  #endif

                            ---------
