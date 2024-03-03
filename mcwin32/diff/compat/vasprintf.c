//
//  compat_vasprintf
//

#include "libcompat.h"

#if !defined(HAVE_VASPRINTF)

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#ifndef VA_COPY
# if defined(HAVE_VA_COPY) || defined(va_copy)
        /* ISO C99 and later */
#define VA_COPY(__dst, __src)   va_copy(__dst, __src)
# elif defined(HAVE___VA_COPY) || defined(__va_copy)
        /* gnu */
#define VA_COPY(__dst, __src)   __va_copy(__dst, __src)
# elif defined(__WATCOMC__)
        /* Older Watcom implementations */
#define VA_COPY(__dst, __src)   memcpy((__dst), (__src), sizeof (va_list))
# else
#define VA_COPY(__dst, __src)   (__dst) = (__src)
# endif
#endif  /*VA_COPY*/


/*
    SYNOPSIS
        int
        asprintf(char **ret, const char *format, ...);

        int
        vasprintf(char **ret, const char *format, va_list ap);

    DESCRIPTION
        The printf() family of functions ...

        The functions asprintf() and vasprintf() are analogs of sprintf(3) and vsprintf(3), 
        except that they allocate a string large enough to hold the output including the 
        terminating null byte, and return a pointer to it via the first argument. 
        This pointer should be passed to free(3) to release the allocated storage when it is
        no longer needed.

    RETURNS
        When successful, these functions return the number of bytes printed, just like sprintf(3).
        If sufficient space cannot be allocated, asprintf() and vasprintf() will return -1 and set ret to be a NULL pointer.

    COMPAT
        These functions are GNU extensions, not in C or POSIX.
        They are also available under *BSD. The FreeBSD implementation sets strp to NULL on error.
*/

int
vasprintf(char **str, const char *fmt, va_list ap)
{
	va_list tap;
	char *buf = NULL;
	int osize, size;

	if (NULL == str || NULL == fmt) {
		errno = EINVAL;
		return -1;
	}

	VA_COPY(tap, ap);
	osize = vsnprintf(NULL, 0, fmt, tap);
	if (osize < 0 ||
		    (NULL == (buf = (char *)malloc(osize + 16)))) {
		size = -1;
	} else {
		size = vsprintf(buf, fmt, ap);
		assert(size == osize);
	}
	*str = buf;
	va_end(tap);

	return size;
}


#else
extern void __stdlibrary_has_vasprintf(void);

void
__stdlibrary_has_vasprintf(void)
{
}

#endif