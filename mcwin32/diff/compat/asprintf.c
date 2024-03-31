//
//  compat_asprintf
//

#include "libcompat.h"

#if !defined(HAVE_ASPRINTF)

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
*/

#if !defined(HAVE_VASPRINTF)
extern int vasprintf(char **str, const char *fmt, va_list ap);
#endif

int
asprintf(char **str, const char *fmt, ...)
{
	va_list ap;
	int size;

	va_start(ap, fmt);
	size = vasprintf(str, fmt, ap);
	va_end(ap);

	return size;
}


#else
extern void __stdlibrary_has_asprintf(void);

void
__stdlibrary_has_asprintf(void)
{
}

#endif