//
//  compat_bcopy
//

#include "libcompat.h"

#include <stdlib.h>
#include <string.h>
#if defined(HAVE_STRINGS_H)
#include <strings.h>
#endif

//  NAME
//
//      bcopy - memory operations (LEGACY)
//
//  SYNOPSIS
//
//      #include <strings.h>
//
//      void bcopy(const void *s1, void *s2, size_t n);
//
//  DESCRIPTION
//
//      The bcopy() function shall copy n bytes from the area pointed to by s1 to the area pointed to by s2.
//
//      The bytes are copied correctly even if the area pointed to by s1 overlaps the area pointed to by s2.
//
//  RETURN VALUE
//
//      The bcopy() function shall not return a value.
//
//  ERRORS
//
//      No errors are defined.
//

#if !defined(HAVE_BCOPY)
void
bcopy(const void *s1, void *s2, size_t n)
{
	(void) memmove(s2, s1, n);
}

#else
extern void __stdlibrary_has_bcopy(void);

void
__stdlibrary_has_bcopy(void)
{
}

#endif
