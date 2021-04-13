#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_getdomainname_c,"$Id: w32_domainname.c,v 1.2 2021/04/13 15:49:34 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 getdomainname()
 *
 * Copyright (c) 2017, Adam Young.
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
#define _WIN32_WINNT        0x0501              /* enable xp+ features */
#endif

#include "win32_internal.h"
#include <DSRole.h>
#include <unistd.h>

#pragma comment(lib, "netapi32.lib")

/*
//  NAME
//      getdomainname, setdomainname - get/set domain name
//
//  SYNOPSIS
//      #include <unistd.h>
//
//      int getdomainname(char *name, size_t len);
//      int setdomainname(const char *name, size_t len);
//
//  DESCRIPTION
//      These functions are used to access or to change the domain name of the current
//      processor.
//
//  RETURN VALUE
//      On success, zero is returned. On error, -1 is returned, and errno is set
//      appropriately.
//
//  ERRORS
//      EINVAL  For getdomainname, name points to NULL or name is longer than len.
//      EPERM   For setdomainname, the caller was not the superuser.
//      EINVAL  For setdomainname, len was too long.
*/
LIBW32_API int
w32_getdomainname(char *name, size_t namelen)
{
#undef getdomainname
    int ret = -1;

    if (name == NULL || namelen < 2) {
        errno = EINVAL;

    } else {
        char t_name[256] = "";
        DWORD dwSize = sizeof(t_name);

        if (! GetComputerNameExA(ComputerNameDnsDomain, t_name, &dwSize)) {
            errno = EINVAL;
        } else {
            size_t t_namelen = strlen(t_name);
            if (t_namelen < namelen) t_namelen = namelen - 1;
            memcpy(name, t_name, t_namelen);
            name[t_namelen]=0;
            ret = 0;
        }
    }
    return ret;
}

/*end*/

