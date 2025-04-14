//
//  compat_isblank
//

#if !defined(LIBCOMPAT_SOURCE)
#define LIBCOMPAT_SOURCE
#endif

#include "libcompat.h"

#if !defined(HAVE_ISBLANK) || defined(NEED_ISBLANK)

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
    SYNOPSIS
        int
        isblank(int ch)

    DESCRIPTION
        The isblank() function shall test whether c is a character of class blank in the program's
        current locale; see the Base Definitions volume of IEEE Std 1003.1-2001, Chapter 7, Locale.

        The c argument is a type int, the value of which the application shall ensure is a character
        representable as an unsigned char or equal to the value of the macro EOF. If the argument
        has any other value, the behavior is undefined.

    RETURNS
        The isblank() function shall return non-zero if c is a <blank>; otherwise, it shall return 0.
*/

int
isblank(int ch)
{
	return (' ' == ch || '\t' == ch);
}

#else
extern void __stdlibrary_has_isblank(void);

void
__stdlibrary_has_isblank(void)
{
}

#endif
