#ifndef LIBW32_SYS_UTSNAME_H_INCLUDED
#define LIBW32_SYS_UTSNAME_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_utsname_h,"$Id: utsname.h,v 1.8 2025/03/06 16:59:48 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 2012 - 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ==end==
 */

#define _UTSNAME_LENGTH 64

struct utsname {
    char sysname[_UTSNAME_LENGTH];
    char nodename[_UTSNAME_LENGTH];
    char release[_UTSNAME_LENGTH];
    char version[_UTSNAME_LENGTH];
    char machine[_UTSNAME_LENGTH];
};

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */

__BEGIN_DECLS
LIBW32_API int          uname(struct utsname *buf);
__END_DECLS

#endif  /*LIBW32_SYS_UTSNAME_H_INCLUDED*/
