#ifndef LIBW32_GRP_H_INCLUDED
#define LIBW32_GRP_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <grp.h> implementation
 *
 * Copyright (c) 2007, 2012 - 2018 Adam Young.
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

#include <sys/cdefs.h>

__BEGIN_DECLS

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

__END_DECLS

#endif /*LIBW32_GRP_H_INCLUDED*/
