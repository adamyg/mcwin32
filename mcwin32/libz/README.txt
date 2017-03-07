
        Source: http://www.zlib.net/

        o zlib 1.2.8

           patch

            _   #if defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
            +   #if (defined(MSDOS) && !defined(WIN32)) || (defined(WINDOWS) && !defined(WIN32))

        o zlib 1.2.11

           patch

                #if defined(WIN32) && !defined(__CYGWIN__)
            +   #  undef OS_CODE
