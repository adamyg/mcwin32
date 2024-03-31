/*
 * Copyright (c) 2010 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "libcompat.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

//  NAME
//
//      strndup - duplicate a specific number of bytes from a string.
//
//  SYNOPSIS
//
//      #include <strings.h>
//
//      char *strndup(const char *s, size_t size);
//
//  DESCRIPTION
//
//      The strndup() function shall be equivalent to the strdup() function, duplicating the 
//      provided s in a new block of memory allocated as if by using malloc(), with the exception
//      being that strndup() copies at most size plus one bytes into the newly allocated memory,
//      terminating the new string with a NUL character. If the length of s is larger than size,
//      only size bytes shall be duplicated. If size is larger than the length of s, all bytes 
//      in 's' shall be copied into the new memory buffer, including the terminating NUL character.
//      The newly created string shall always be properly terminated.
//
//  RETURN VALUE
//
//      Upon successful completion, the strndup() function shall return a pointer to the newly
//      allocated memory containing the duplicated string. Otherwise, it shall return a null
//      pointer and set errno to indicate the error.
//
//  ERRORS
//
//      These functions shall fail if:
//
//          [ENOMEM] -  Storage space available is insufficient.
//

#if !defined(HAVE_STRNDUP)
char *
strndup(const char *str, size_t maxlen)
{
	char *copy;
	size_t len;

	len = strnlen(str, maxlen);
	copy = malloc(len + 1);
	if (copy != NULL) {
		(void)memcpy(copy, str, len);
		copy[len] = '\0';
	}

	return copy;
}

#else
extern void __stdlibrary_has_strndup(void);

void
__stdlibrary_has_strndup(void)
{
}

#endif
