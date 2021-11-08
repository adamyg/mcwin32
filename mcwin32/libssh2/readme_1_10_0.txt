
    Source: libssh2 1.10.0, code changes

        Makefile.in:

                + agent_win$(O)


        src\libssh2_priv.h:

                  #include <time.h>
                + #include <limits.h>   //UINT_MAX


        src\misc.c:

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
