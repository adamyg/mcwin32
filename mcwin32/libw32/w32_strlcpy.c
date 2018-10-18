#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_strlcpy_c,"$Id: w32_strlcpy.c,v 1.7 2018/10/12 00:52:04 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
*
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * The Midnight Commander is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ==end==
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 *
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

#include "w32config.h"

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
*/

#if defined(_MSC_VER)

#include <sys/types.h>
#include <string.h>
#include <unistd.h>

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
LIBW32_API size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);		/* count does not include NUL */
}

#else
extern void __stdlibrary_has_strlcpy(void);

void
__stdlibrary_has_strlcpy(void)
{
}
#endif

/*end*/

