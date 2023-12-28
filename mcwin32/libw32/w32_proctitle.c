#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_proctitle_c,"$Id: w32_proctitle.c,v 1.3 2023/11/06 15:07:42 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 setproctitle
 *
 * Copyright (c) 2020 - 2023, Adam Young.
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

static void setproctitle_impl(const char *fmt, va_list ap);


/*
    NAME
        setproctitle -- set process title

    SYNOPSIS
        #include <sys/types.h>
        #include <unistd.h>

        void
        setproctitle(const char *fmt, ...);

        void
        setproctitle_fast(const char *fmt, ...);

    DESCRIPTION
        The setproctitle() library routine sets the process title that appears on
        the ps(1) command.  The setproctitle_fast() variant is optimized for high
        frequency updates, but may make the ps(1) command slightly slower by not
        updating the kernel cache of the program arguments.

        The title is set from the executable's name, followed by the result of a
        printf(3) style expansion of the arguments as specified by the fmt argument.
        If the fmt argument begins with a "-" character, the executable's
        name is skipped.

        If fmt is NULL, the process title is restored.

    EXAMPLES
        To set the title on a daemon to indicate its activity:

            setproctitle("talking to %s", inet_ntoa(addr));

    SEE ALSO
        ps(1), w(1), kvm(3), kvm_getargv(3), printf(3)

    STANDARDS
        The setproctitle() function is implicitly non-standard.  Other methods of
        causing the ps(1) command line to change, including copying over the
        argv[0] string are also implicitly non-portable.  It is preferable to use
        an operating system supplied setproctitle() if present.

        Unfortunately, it is possible that there are other calling conventions to
        other versions of setproctitle(), although none have been found by the
        author as yet.  This is believed to be the predominant convention.

        It is thought that the implementation is compatible with other systems,
        including NetBSD and BSD/OS.

    HISTORY
        The setproctitle() function first appeared in FreeBSD 2.2.  The
        setproctitle_fast() function first appeared in FreeBSD 12.  Other operat-
        ing systems have similar functions.

    AUTHORS
        Peter Wemm <peter@FreeBSD.org> stole the idea from the Sendmail 8.7.3
        source code by Eric Allman <eric@sendmail.org>.

    BUGS
        Never pass a string with user-supplied data as a format without using
        `%s'.  An attacker can put format specifiers in the string to mangle your
        stack, leading to a possible security hole.  This holds true even if the
        string was built using a function like snprintf(), as the resulting
        string may still contain user-supplied conversion specifiers for later
        interpolation by setproctitle().

        Always use the proper secure idiom:

            setproctitle("%s", string);

*/

LIBW32_API void
setproctitle(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    setproctitle_impl(fmt, ap);
    va_end(ap);
}


LIBW32_API void
setproctitle_fast(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    setproctitle_impl(fmt, ap);
    va_end(ap);
}


static void
setproctitle_impl(const char *fmt, va_list ap)
{
    static WCHAR *saved_console_title, 
        *saved_window_title;

    if (NULL == fmt) {
        if (saved_console_title) { //restore console
            SetConsoleTitleW(saved_console_title);
        } else if (saved_window_title) { //restore window
            HWND hWnd = GetActiveWindow();
            if (hWnd) {
                SetWindowTextW(hWnd, saved_window_title);            
            }
        }

    } else {
        char n_title[2 * 1024];

        if (vsnprintf(n_title, _countof(n_title), fmt, ap) > 0) {
            HWND hWnd = 0;

            if (! saved_console_title && ! saved_window_title) { //original titles
                WCHAR t_title[2 * 1024];
                DWORD ret;

                //console
                if (0 != (ret = GetConsoleTitleW(t_title, _countof(t_title)))) {
                    const size_t sz = (ret + 1) * sizeof(WCHAR);
                        //If the function succeeds, the return value is the length of the console window's title, in characters.
                        //Buffer receives a null-terminated string containing the title.
                        //If the buffer is too small to store the title, the function stores as many characters of the title as will fit in the buffer, ending with a null terminator.
                        //
                    if (NULL != (saved_console_title = (WCHAR *)malloc(sz))) {
                        memcpy(saved_console_title, t_title, sz);
                    }
                }

                // window
                if (0 != (hWnd = GetActiveWindow())) {
                    if (0 != (ret = GetWindowTextLength(hWnd))) {
                        const size_t sz = (ret + 1) * sizeof(WCHAR);
                        if (NULL != (saved_window_title = (WCHAR *)calloc(sz, 1))) {
                            if (! GetWindowTextW(hWnd, saved_window_title, (int)sz)) {
                                free(saved_window_title);
                                saved_window_title = NULL;
                            }
                        }
                    }
                }
            }

            if (! SetConsoleTitleA(n_title)) {
                if (!hWnd) hWnd = GetActiveWindow();
                if (hWnd) {
                    SetWindowTextA(hWnd, n_title);      
                }
            }
        }
    }
}

/*end*/
