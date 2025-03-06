#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_setrlimit_c,"$Id: w32_setrlimit.c,v 1.6 2025/03/06 16:59:47 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 setrlimit() system calls
 *
 * Copyright (c) 2020 - 2025, Adam Young.
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

#include "win32_internal.h"

#include <sys/resource.h>
#include <stdio.h>
#include <unistd.h>

int
setrlimit(int resource, const struct rlimit *rlp)
{
    if (NULL == rlp) {
        errno = EINVAL;
    } else if (rlp->rlim_cur > rlp->rlim_max) {
        errno = EINVAL;
    } else {
        switch (resource) {
        case RLIMIT_NOFILE: {
                int newmax, ret = 0;
                if (rlp->rlim_max > WIN32_FILDES_DEF) {
#if defined(__WATCOMC__)
                    if (_grow_handles(rlp->rlim_max) < rlp->rlim_max) {
#else
                    if (-1 == (newmax = _setmaxstdio((int)rlp->rlim_max))) {
#endif
                        errno = EINVAL;
                        ret = -1;
                    }
                    w32_fdregister((int)rlp->rlim_max);
                }
                return ret;
            }
        }
        errno = ENOSYS;
    }
    return -1;
}

/*end*/
