#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_pipe_c,"$Id: w32_pipe.c,v 1.5 2025/03/06 16:59:46 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 pipe() system calls,
 *
 * Copyright (c) 2018 - 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501                     /* enable xp+ features */
#endif

#include "win32_internal.h"
#include "win32_misc.h"
#include <unistd.h>

LIBW32_API int
w32_pipe(int fildes[2])
{
    HANDLE hReadPipe = 0, hWritePipe = 0;
    const BOOL ret = CreatePipe(&hReadPipe, &hWritePipe, NULL, 0);

    if (ret) {
        fildes[0] = _open_osfhandle((OSFHANDLE)hReadPipe, O_NOINHERIT);
        if (fildes[0] < 0) {
            CloseHandle(hReadPipe), CloseHandle(hWritePipe);
            return -1;
        }
        fildes[1] = _open_osfhandle((OSFHANDLE)hWritePipe, O_NOINHERIT);
        if (fildes[1] < 0) {
            _close(fildes[0]), CloseHandle(hWritePipe);
            return -1;
        }
        return 0;
    }
    w32_errno_set();
    return -1;
}

/*end*/
