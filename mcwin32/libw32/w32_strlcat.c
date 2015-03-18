/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
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
 *
 *      $OpenBSD: strlcat.c,v 1.13 2005/08/08 08:05:37 espie Exp$
 */

/*
//  NAME
//       strlcpy, strlcat - size-bounded string copying and concatenation
//  
//  SYNOPSIS
//       #include <string.h>
//  
//       size_t
//       strlcpy(char *dst, const char *src, size_t dstsize);
//  
//       size_t
//       strlcat(char *dst, const char *src, size_t dstsize);
//  
//  DESCRIPTION
//       The strlcpy() and strlcat() functions copy and concatenate strings with
//       the same input parameters and output result as snprintf(3).  They are
//       designed to be safer, more consistent, and less error prone replacements
//       for the easily misused functions strncpy(3) and strncat(3).
//  
//       strlcpy() and strlcat() take the full size of the destination buffer and
//       guarantee NUL-termination if there is room.  Note that room for the NUL
//       should be included in dstsize.
//  
//       strlcpy() copies up to dstsize - 1 characters from the string src to dst,
//       NUL-terminating the result if dstsize is not 0.
//  
//       strlcat() appends string src to the end of dst.  It will append at most
//       dstsize - strlen(dst) - 1 characters.  It will then NUL-terminate, unless
//       dstsize is 0 or the original dst string was longer than dstsize (in
//       practice this should not happen as it means that either dstsize is
//       incorrect or that dst is not a proper string).
//  
//  RETURN VALUES
//       Besides quibbles over the return type (size_t versus int) and signal
//       handler safety (snprintf(3) is not entirely safe on some systems), the
//       following two are equivalent:
//  
//             n = strlcpy(dst, src, len);
//             n = snprintf(dst, len, "%s", src);
//  
//       Like snprintf(3), the strlcpy() and strlcat() functions return the total
//       length of the string they tried to create.  For strlcpy() that means the
//       length of src.  For strlcat() that means the initial length of dst plus
//       the length of src.
//  
//       If the return value is >= dstsize, the output string has been truncated.
//       It is the caller's responsibility to handle this.
//  
//  EXAMPLES
//       The following code fragment illustrates the simple case:
//  
//             char *s, *p, buf[BUFSIZ];
//  
//             ...
//  
//             (void)strlcpy(buf, s, sizeof(buf));
//             (void)strlcat(buf, p, sizeof(buf));
//  
//       To detect truncation, perhaps while building a pathname, something like
//       the following might be used:
//  
//             char *dir, *file, pname[MAXPATHLEN];
//  
//             ...
//  
//             if (strlcpy(pname, dir, sizeof(pname)) >= sizeof(pname))
//                     goto toolong;
//             if (strlcat(pname, file, sizeof(pname)) >= sizeof(pname))
//                     goto toolong;
//  
//       Since it is known how many characters were copied the first time, things
//       can be sped up a bit by using a copy instead of an append:
//  
//             char *dir, *file, pname[MAXPATHLEN];
//             size_t n;
//  
//             ...
//  
//             n = strlcpy(pname, dir, sizeof(pname));
//             if (n >= sizeof(pname))
//                     goto toolong;
//             if (strlcpy(pname + n, file, sizeof(pname) - n) >= sizeof(pname) - n)
//                     goto toolong;
//  
//       However, one may question the validity of such optimizations, as they
//       defeat the whole purpose of strlcpy() and strlcat().  As a matter of
//       fact, the first version of this manual page got it wrong.
//  
//  SEE ALSO
//       snprintf(3), strncat(3), strncpy(3), wcslcpy(3)
//  
//  HISTORY
//       strlcpy() and strlcat() first appeared in OpenBSD 2.4.
//  
//  AUTHORS
//       strlcpy() and strlcat() were created by Todd C. Miller
//       <Todd.Miller@courtesan.com>.
/*/

#if defined(_MSC_VER)

#include <sys/types.h>
#include <string.h>
#include <unistd.h>

/*
 *  Appends src to string dst of size siz (unlike strncat, siz is the
 *  full size of dst, not space left).  At most siz-1 characters
 *  will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 *  Returns strlen(src) + MIN(siz, strlen(initial dst)).
 *  If retval >= siz, truncation occurred.
 */
size_t
strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));	/* count does not include NUL */
}


#else
void
__stdlibrary_has_strlcat(void)
{
}
#endif  /*_MSC_VER*/