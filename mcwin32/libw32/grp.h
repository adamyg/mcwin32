#ifndef LIBW32_GRP_H_INCLUDED
#define LIBW32_GRP_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_grp_h,"$Id: grp.h,v 1.6 2021/05/16 14:40:44 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <grp.h> implementation
 *
 * Copyright (c) 2007, 2012 - 2021 Adam Young.
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
 */

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */
#include <sys/utypes.h>                         /* uid_t */

__BEGIN_DECLS

#if !defined(NGROUPS_MAX)
#define NGROUPS_MAX     32
#endif

struct group {
    const char *        gr_name;
    const char *        gr_passwd;
    int                 gr_gid;
    const char **       gr_mem;
};

LIBW32_API struct group *getgrent(void);
LIBW32_API struct group *getgrgid(int);
LIBW32_API struct group *getgrnam(const char *);
LIBW32_API void         setgrent(void);
LIBW32_API void         endgrent(void);

LIBW32_API int          getgroups(int gidsetsize, gid_t grouplist[]);
LIBW32_API int          setgroups(size_t size, const gid_t *gidset);

__END_DECLS

#endif /*LIBW32_GRP_H_INCLUDED*/
