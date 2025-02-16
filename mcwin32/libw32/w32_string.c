#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_string_c,"$Id: w32_string.c,v 1.13 2025/02/16 12:04:05 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 string functionality
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 * or (at your option) any later version.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#include "win32_internal.h"

#include <stddef.h>
#include <string.h>
#include <unistd.h>


#if defined(NEED_STRCASECMP)
LIBW32_API int
strcasecmp(const char *s1, const char *s2)
{
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    return _stricmp(s1, s2);
#else
    return stricmp(s1, s2);
#endif
}
#endif


#if defined(NEED_STRCASECMP)
LIBW32_API int
strncasecmp(const char *s1, const char *s2, size_t len)
{
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    return _strnicmp(s1, s2, len);
#else
    return strnicmp(s1, s2, len);
#endif
}
#endif


#if defined(NEED_STRNLEN)
LIBW32_API size_t
strnlen(const char *s, size_t maxlen)
{
    register const char *e;
    size_t n;

    for (e = s, n = 0; *e && n < maxlen; e++, n++)
        /**/;
    return n;
}
#endif

/*end*/
