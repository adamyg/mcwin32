/* -*- mode: c; tabs: 4 -*- */
#ifndef WIN32_SYS_UTSNAME_H_INCLUDED
#define WIN32_SYS_UTSNAME_H_INCLUDED

/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 2012 - 2017, Adam Young.
 * All rights reserved.
 *
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

#endif  /*WIN32_SYS_UTSNAME_H_INCLUDED*/
