#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_sysconf_c,"$Id: w32_sysconf.c,v 1.2 2024/01/16 15:17:52 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 interface support
 *
 * Copyright (c) 2021 - 2022 Adam Young.
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

#include <sys/cdefs.h>

#include "win32_internal.h"

static int
LogicalProcessors(void)
{
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION *pi = NULL;
    DWORD len = 0, i;
    int count = 0;

    while (! GetLogicalProcessorInformation(pi, &len)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            if (NULL == (pi = malloc(len))) {
                return -1;
            }
        } else {
            errno = EINVAL;
            return -1;
        }
    }

    for (i = 0; i < len / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
        if (pi[i].Relationship == RelationProcessorCore) {
            count += hweight64(pi[i].ProcessorMask);
        }
    }

    free(pi);
    return count;
}


static int
PageSize(void)
{
    SYSTEM_INFO si = {0};
    GetSystemInfo(&si);
    return si.dwPageSize;
}



static int
PhysicalPages()
{
    MEMORYSTATUSEX ms = {0};
    ms.dwLength = sizeof(ms);
    if (GlobalMemoryStatusEx(&ms))
        return ms.ullTotalPhys / PageSize();
    return -1;
}


long
sysconf(int name)
{
    switch (name) {
//  case _SC_CLK_TCK:
//  case _SC_OPEN_MAX:
    case _SC_NPROCESSORS_ONLN:
        return NumLogicalProcessors();
//  case PAGE_SIZE:
//  case _SC_PAGE_SIZE:
    case _SC_PAGESIZE:
        return PageSize();
    case _SC_PHYS_PAGES:
        return PhysicalPages();
    default:
        break;
    }
    return -1;
}

//end
