#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_progname_c,"$Id: w32_progname.c,v 1.9 2022/03/16 13:47:00 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 set/getprogname
 *
 * Copyright (c) 2016 - 2022, Adam Young.
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
#include <ctype.h>
#include <unistd.h>

static const char *progname = NULL;
static const wchar_t *wprogname = NULL;


LIBW32_API void
setprogname(const char *name)
{
    const char *p1 = strrchr(name, '\\'),
        *p2 = strrchr(name, '/');
    char *p;

    if (p1 || p2) { //last component.
        name = (p1 > p2 ? p1 : p2) + 1;  //consume leading path.
    }

    free((void *)progname);
    progname = WIN32_STRDUP(name); //clone buffer.

    if (NULL != (p = strrchr(progname, '.')) &&
            (0 == stricmp(p, ".exe") || 0 == stricmp(p, ".com"))) {
        *p = 0; //consume trailing exe/com extension.
    }

    for (p = (char *)progname; *p; ++p) { //hide case issues.
        *p = (char)tolower(*p);
    }
}


LIBW32_API void
setprognameW(const wchar_t *name)
{
    const wchar_t *p1 = wcsrchr(name, '\\'),
        *p2 = wcsrchr(name, '/');
    wchar_t *p;

    if (p1 || p2) { //last component.
        name = (p1 > p2 ? p1 : p2) + 1; //consume leading path.
    }

    free((void *)wprogname);
    wprogname = WIN32_STRDUPW(name); //clone buffer.

    if (NULL != (p = wcsrchr(wprogname, '.')) &&
            (0 == _wcsicmp(p, L".exe") || 0 == _wcsicmp(p, L".com"))) {
        *p = 0; //consume trailing exe/com extension.
    }

    for (p = (wchar_t *)wprogname; *p; ++p) { //hide case issues.
        if (*p < 0x7f) *p = tolower((char)*p);
    }
}


LIBW32_API const char *
getprogname(void)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (NULL == progname) {
            char path[1024];
            const wchar_t *wpath;
            if (NULL != (wpath = getprognameW())) {
                w32_wc2utf(wpath, path, sizeof(path));
                setprogname(path);
            }
        }
        return (progname ? progname : "program");
    }
#endif  //UTF8FILENAMES

    return getprognameA();
}


LIBW32_API const char *
getprognameA(void)
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


LIBW32_API const wchar_t *
getprognameW(void)
{
    if (NULL == wprogname) {
        wchar_t t_buffer[1024];
        DWORD buflen;
        if ((buflen = GetModuleFileNameW(NULL, t_buffer, _countof(t_buffer)-1)) > 0) {
            t_buffer[buflen] = 0;
            setprognameW(t_buffer);
        }
    }
    return (wprogname ? wprogname : L"program");
}

/*end*/
