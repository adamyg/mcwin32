/*
   SYNOPSIS:
       #include "lib/global.h"

       char *strchr2(const char *s, int c1, int c2);
       char *strrchr2(const char *s, int c1, int c2);

   DESCRIPTION:
       The strchr2() function returns a pointer to the first occurrence of the
       character 'c1' or 'c2' in the string 's'.

       The strrchr2() function returns a pointer to the last occurrence of the
       character 'c1' or 'c2' in the string 's'.

       Here "character" means "byte" - these functions do not work with wide
       or multi-byte characters.

   RETURN VALUE:
       The strchr2() and strrchr2() functions return a pointer to the matched
       character or NULL if the character is not found.
*/

#include "config.h"

#include <string.h>

#include "lib/global.h"

char *
strchr2(const char *s, int c1, int c2)
{
	do {
		if ((unsigned)*s == (unsigned)c1 || (unsigned)*s == (unsigned)c2)
			return (char *)s;

	} while (*(++s) != 0);

	return NULL;
}


char *
strrchr2(const char *s, int c1, int c2)
{
	const char *e = s + strlen(s);

	for (;;) {
		if (--e < s)
			break;

		if ((unsigned)*e == (unsigned)c1 || (unsigned)*e == (unsigned)c2)
			return (char *)e;
	}

	return NULL;
}
