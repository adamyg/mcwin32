#ifndef LIBW32_SYS_FILE_H_INCLUDED
#define LIBW32_SYS_FILE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_file_h,"$Id: file.h,v 1.3 2023/11/06 15:06:06 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 sys/file.h
 *
 * Copyright (c) 2020 - 2023, Adam Young.
 * All rights reserved.
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

#define LOCK_SH             0x01
#define LOCK_EX             0x02
#define LOCK_NB             0x04
#define LOCK_UN             0x08

__BEGIN_DECLS

LIBW32_API int              w32_flock(int fd, int operation);

__END_DECLS

#endif /*LIBW32_SYS_FILE_H_INCLUDED*/
