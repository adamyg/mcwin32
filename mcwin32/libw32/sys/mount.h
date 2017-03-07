#ifndef WIN32_SYS_MOUNT_H
#define WIN32_SYS_MOUNT_H

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 mount() implementation
 *
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

#include <sys/cdefs.h>

#define MOUNT_UFS   0
#define MOUNT_NFS   1
#define MOUNT_PC    2
#define MOUNT_MFS   3
#define MOUNT_LO    4
#define MOUNT_TFS   5
#define MOUNT_TMP   6

#define MNT_WAIT    0
#define MNT_NOWAIT  1

__BEGIN_DECLS

struct statfs;

LIBW32_API int		getmntinfo(struct statfs **mntbufp, int flags);

__END_DECLS

#endif

