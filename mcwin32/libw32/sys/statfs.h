#ifndef LIBW32_SYS_STATFS_H_INCLUDED
#define LIBW32_SYS_STATFS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_statfs_h,"$Id: statfs.h,v 1.9 2023/01/31 17:14:13 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 [f]statfs implementation
 *
 * Copyright (c) 2012 - 2023, Adam Young.
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

#include <sys/cdefs.h>          /* __BEGIN/__END/.. */
#include <sys/utypes.h>         /* fsblkcnt_t and fsfilcnt_t */
#include <sys/mount.h>
#include <wchar.h>

#define FS_MAGIC    0x11954                     /* Taken from HP-UX */

#define MFSNAMELEN  16                          /* length of fs type name, including null */
#define MNAMELEN    90                          /* length of buffer for returned name */

typedef struct fsid {                           /* file system id type */
    unsigned long   val[2]; 
} fsid_t;

struct statfs {
    unsigned long   f_bsize;                    /* fundamental file system block size */
    unsigned long   f_iosize;                   /* optimal transfer block size */
    fsblkcnt_t      f_blocks;                   /* total data blocks in file system */
    fsblkcnt_t      f_bfree;                    /* free blocks in fs */
    fsblkcnt_t      f_bavail;                   /* free blocks avail to non-superuser */
    fsfilcnt_t      f_files;                    /* total file nodes in file system */
    fsfilcnt_t      f_ffree;                    /* free file nodes in fs */
    fsid_t          f_fsid;                     /* file system id */
    uid_t           f_owner;                    /* user that mounted the filesystem */
    int             f_type;                     /* type of filesystem (see below) */
    int             f_flags;                    /* copy of mount flags */
    long            f_spare[2];                 /* spare for later */
    char            f_fstypename[MFSNAMELEN];   /* fs type name */
    char            f_mntonname[MNAMELEN];      /* directory on which mounted */
    char            f_mntfromname[MNAMELEN];    /* mounted filesystem */
};

__BEGIN_DECLS

LIBW32_API int      statfs(const char *path, struct statfs *buf);
LIBW32_API int      statfsA(const char *path, struct statfs *buf);
LIBW32_API int      statfsW(const wchar_t *path, struct statfs *buf);
LIBW32_API int      fstatfs(int fd, struct statfs *buf);

__END_DECLS

#endif  /*LIBW32_SYS_STATFS_H_INCLUDED*/
