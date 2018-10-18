#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_progname_c,"$Id: w32_progname.c,v 1.3 2018/10/12 00:52:04 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 set/getprogname
 *
 * Copyright (c) 2016 - 2018, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
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
#include <ctype.h>
#include <unistd.h>

static const char *progname = NULL;

LIBW32_API void
setprogname(const char *name)
{
    char *p;
    if ((p = strrchr(name, '/')) || (p = strrchr(name, '\\'))) {
        name = p + 1; //consume leading path.
    }
    free((char *)progname);
    progname = _strdup(name); //clone buffer.
    if ((p = strrchr(progname, '.')) &&
            (0 == stricmp(p, ".exe") || 0 == stricmp(p, ".com"))) {
        *p = 0; //consume trailing exe/com extension.
    }
    for (p = (char *)progname; *p; ++p) { //hide case issues.
        *p = tolower(*p);
    }
}

LIBW32_API const char *
getprogname(void)
{
    if (NULL == progname) {
        char t_buffer[1024];
        DWORD buflen;
        if ((buflen = GetModuleFileNameA(NULL, t_buffer, sizeof(t_buffer)-1)) > 0) {
            t_buffer[buflen] = 0;
            setprogname(t_buffer);
        }
    }
    return (progname ? progname : "program");
}

/*end*/

