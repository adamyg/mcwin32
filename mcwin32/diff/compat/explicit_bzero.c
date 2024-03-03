/*
 * compat_explicit_bzero - don't let the compiler optimize away bzero
 *
 * Public domain.
 * Written by Ted Unangst
 * OPENBSD ORIGINAL: lib/libc/string/explicit_bzero.c
 */
 
#include "libcompat.h"

#include <stdlib.h>
#include <string.h>
#if defined(HAVE_STRINGS_H)
#include <strings.h>
#endif

#if defined(HAVE_SECUREZEROMEMORY) && (defined(WIN32) || defined(_WIN32))
#define WINDOWS_MEAN_AND_LEAN
#include <Windows.h>
#endif


//  NAME
//
//      explicit_bzero - memory operations (LEGACY)
//
//  SYNOPSIS
//
//      #include <strings.h>
//
//      void explicit_bzero(void *s, size_t n);
//
//  DESCRIPTION
//
//      The explicit_bzero() variant behaves like bzero(), but will not be removed by
//      a compiler's dead store optimization pass, making it useful for clearing
//      sensitive memory such as a password.
//
//  RETURN VALUE
//
//      The explicit_bzero() function shall not return a value.
//
//  ERRORS
//
//      No errors are defined.
//

#if defined(HAVE_MEMSET_S)

void
explicit_bzero(void *p, size_t n)
{
	if (n == 0)
		return;
	(void)memset_s(p, n, 0, n);
}


#elif defined(HAVE_SECUREZEROMEMORY)

void
explicit_bzero(void *p, size_t n)
{
	if (n == 0)
		return;
	(void)SecureZeroMemory(p, n);
}


#else /*HAVE_MEMSET_S*/

#if !defined(HAVE_BZERO)
/*
 *  See bzero.c
 */
extern void bzero(void *s, size_t len);
#endif

/*
 *  Indirect bzero through a volatile pointer to hopefully avoid
 *  dead-store optimisation eliminating the call.
 */
static void (* volatile xbzero)(void *, size_t) = bzero;

void
explicit_bzero(void *p, size_t n)
{
	if (n == 0)
		return;
	/*
	 * clang -fsanitize=memory needs to intercept memset-like functions
	 * to correctly detect memory initialisation. Make sure one is called
	 * directly since our indirection trick above sucessfully confuses it.
	 */
#if defined(__has_feature)
#if __has_feature(memory_sanitizer)
	memset(p, 0, n);
#endif
#endif

	xbzero(p, n);
}

#endif /* HAVE_MEMSET_S */