//
//  compat_bzero
//

#include "libcompat.h"

#include <stdlib.h>
#include <string.h>
#if defined(HAVE_STRINGS_H)
#include <strings.h>
#endif

//  NAME
//
//      bzero - memory operations (LEGACY)
//
//  SYNOPSIS
//
//      #include <strings.h>
//
//      void bzero(void *s, size_t n);
//
//  DESCRIPTION
//
//      The bzero() function shall place n zero-valued bytes in the area pointed to by s.
//
//  RETURN VALUE
//
//      The bzero() function shall not return a value.
//
//  ERRORS
//
//      No errors are defined.
//

#if !defined(HAVE_BZERO)
void
bzero(void *s, size_t len)
{
	(void) memset (s, '\0', len);
}

#else
extern void __stdlibrary_has_bzero(void);

void
__stdlibrary_has_bzero(void)
{
}

#endif