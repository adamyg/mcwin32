/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 statfs()/statvfs() system calls.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained 
 * online at http://www.opengroup.org/unix/online.html.
 * ==end==
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0501              /* enable xp+ features */
#endif
#include "win32_internal.h"

#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/mount.h>

/*
 *  statfs() system call
 */
int
statfs(const char *path, struct statfs *sb)
{
    char    volName[MNAMELEN], fsName[MFSNAMELEN];
    DWORD   SectorsPerCluster, BytesPerSector, FreeClusters, Clusters;
    DWORD   MaximumComponentLength, FileSystemFlags;

    (void) memset( sb, 0, sizeof(*sb) );

    sb->f_bsize = 1024;                         /* block size */

    if (GetDiskFreeSpace( path,
            &SectorsPerCluster, &BytesPerSector, &FreeClusters, &Clusters )) {
        /* KBytes available */
        sb->f_bavail = (unsigned int)
            (((__int64)SectorsPerCluster * BytesPerSector * FreeClusters) / 1024);

        /* KBytes total */
        sb->f_blocks = (unsigned int)
            (((__int64)SectorsPerCluster * BytesPerSector * Clusters) / 1024);

        /* inodes */
        sb->f_ffree = FreeClusters/10;
        sb->f_files = Clusters/10;
    }

    strncpy(sb->f_mntonname, path, MNAMELEN);   /* mount point */
    w32_dos2unix(sb->f_mntonname);

    switch (GetDriveType(path)) {               /* device */
    case DRIVE_REMOVABLE:
        strncpy(sb->f_mntfromname, "Removable", MNAMELEN);
        break;
    case DRIVE_FIXED:
        strncpy(sb->f_mntfromname, "Hard Disk", MNAMELEN);
        break;
    case DRIVE_REMOTE:
        strncpy(sb->f_mntfromname, "Networked", MNAMELEN);
        break;
    case DRIVE_CDROM:
        strncpy(sb->f_mntfromname, "CD-ROM", MNAMELEN);
        break;
    case DRIVE_RAMDISK:
        strncpy(sb->f_mntfromname, "RAM disk", MNAMELEN);
        break;
    default:
        strncpy(sb->f_mntfromname, "Unknown", MNAMELEN);
        break;
    }

    sb->f_type = MOUNT_PC;

    strncat(sb->f_fstypename, "unknown", MFSNAMELEN);

    if (GetVolumeInformation( path,
                volName, MNAMELEN,              /* VolumeName and size */
                NULL, &MaximumComponentLength, &FileSystemFlags,
                fsName, MNAMELEN ))             /* filesystem type */
    {
        /*
         *  FileSystem type/NTFS, FAT etc
         */
        if (fsName[0]) {
            strncpy(sb->f_fstypename, fsName, MFSNAMELEN);
        }
    }
    return (0);
}


/*
 *  statvfs() system call
 */
int
statvfs( const char *path, struct statvfs *vfs )
{
    struct statfs sb;

    if (statfs( path, &sb ) != 0) {
        return -1;
    }   
    vfs->f_bsize  = sb.f_bsize;
    vfs->f_frsize = sb.f_bsize;
    vfs->f_blocks = sb.f_blocks;
    vfs->f_bfree  = sb.f_bfree;
    vfs->f_bavail = sb.f_bavail;
    vfs->f_files  = sb.f_files;
    vfs->f_ffree  = sb.f_ffree;
    vfs->f_favail = sb.f_ffree;
    return 0;
}


/*
 *  getmntinfo() system call
 */
int
getmntinfo(struct statfs **psb, int entries)
{
    struct statfs *sb;
    char szDrivesAvail[32*4], *p;
    int cnt;

    *psb = NULL;

    GetLogicalDriveStrings(sizeof(szDrivesAvail), szDrivesAvail);
    for (cnt = 0, p = szDrivesAvail; *p; p += 4) {
        ++cnt;
    }

    if (cnt > entries) {
        errno = EINVAL;
        cnt = -1;

    } else if (cnt > 0) {
        if ((sb = calloc( sizeof(struct statfs), cnt )) == NULL)  {
            cnt = -1;
        } else {
            for (cnt = 0, p = szDrivesAvail; *p; p += 4) {
                if (*p == 'A' || *p == 'a' ||
                    *p == 'B' || *p == 'b') {
                    continue;                   // skip floppies
                }
                if (statfs(p, sb + cnt) == 0) {
                    ++cnt;
                }
            }

            if (cnt == 0) {
                free(sb);
            } else {
                *psb = sb;
            }
        }
    }
    return cnt;
}



