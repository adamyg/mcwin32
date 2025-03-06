
    Source: libssh2 1.11.1, code changes

        makefile.in

                #1.11.1+
                CSOURCES+=\
                     cipher-chachapoly.c

        win32\

                + libssh2_config.h

        src\misc.h:

                47:
                -   #elif defined(_WIN32)
                +   #elif defined(_WIN32) && !defined(__WATCOMC__)

        src\misc.c:

                805:
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

                            ---------
