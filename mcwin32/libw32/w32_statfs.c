#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_statfs_c,"$Id: w32_statfs.c,v 1.23 2025/05/15 18:20:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 statfs()/statvfs() and getmntinfo() system calls.
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
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
#define _WIN32_WINNT 0x0501                     /* enable xp+ features */
#endif

#include "win32_internal.h"

#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/mount.h>
#include <assert.h>


/*
//  NAME
//      fstatvfs, statvfs - get file system information
//
//  SYNOPSIS
//      #include <sys/statvfs.h>
//
//      int fstatvfs(int fildes, struct statvfs *buf);
//      int statvfs(const char *restrict path, struct statvfs *restrict buf);
//
//  DESCRIPTION
//      The fstatvfs() function shall obtain information about the file system containing the file referenced by fildes.
//
//      The statvfs() function shall obtain information about the file system containing the file named by path.
//
//      For both functions, the buf argument is a pointer to a statvfs structure that shall be filled.
//      Read, write, or execute permission of the named file is not required.
//
//      The following flags can be returned in the f_flag member :
//
//          ST_RDONLY       Read - only file system.
//          ST_NOSUID       Setuid / setgid bits ignored by exec.
//
//      It is unspecified whether all members of the statvfs structure have meaningful values on all file systems.
//
//  RETURN VALUE
//      Upon successful completion, statvfs() shall return 0. Otherwise, it shall return -1 and set errno to indicate the error.
//
//  ERRORS
//      The fstatvfs() and statvfs() functions shall fail if:
//
//          [EIO]           An I / O error occurred while reading the file system.
//          [EINTR]         A signal was caught during execution of the function.
//          [EOVERFLOW]     One of the values to be returned cannot be represented correctly in the structure pointed to by buf.
//
//      The fstatvfs() function shall fail if:
//
//          [EBADF]         The fildes argument is not an open file descriptor.
//
//      The statvfs() function shall fail if :
//
//          [EACCES]        Search permission is denied on a component of the path prefix.
//          [ELOOP]         A loop exists in symbolic links encountered during resolution of the path argument.
//          [ENAMETOOLONG]  The length of a pathname exceeds{ PATH_MAX } or a pathname component is longer than{ NAME_MAX }.
//          [ENOENT]        A component of path does not name an existing file or path is an empty string.
//          [ENOTDIR]       A component of the path prefix of path is not a directory.
//
//      The statvfs() function may fail if:
//
//          [ELOOP]         More than <YMLOOP_MAX>symbolic links were encountered during resolution of the path argument.
//          [ENAMETOOLONG]  Pathname resolution of a symbolic link produced an intermediate result whose length exceeds{ PATH_MAX }.
*/

int
statfs(const char *path, struct statfs *buf)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (path && buf) {
            wchar_t wpath[WIN32_PATH_MAX];

            if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
                return statfsW(wpath, buf);
            }

            return -1;
        }
    }
#endif  //UTF8FILENAMES

    return statfsA(path, buf);
}


int
statfsA(const char *path, struct statfs *sb)
{
    char   volName[MNAMELEN], fsName[MFSNAMELEN];
    DWORD  MaximumComponentLength, FileSystemFlags;
    BOOL   ready = FALSE, query_free = FALSE;
    size_t mnamelen;
    EMODEINIT()

    if (NULL == path || NULL == sb) {
        errno = EFAULT;
        return -1;
    }

    (void) memset(sb, 0, sizeof(*sb));

    EMODESUPPRESS()

    strncpy(sb->f_mntonname, path, MNAMELEN-1); /* mount point */
    w32_dos2unixA(sb->f_mntonname);
    if ((mnamelen = strlen(sb->f_mntonname)) > 3) {
        if (sb->f_mntonname[mnamelen - 1] == '/') {
            sb->f_mntonname[mnamelen - 1] = 0;
                // remove trailing delimiter on mount-points.
        }
    }

    sb->f_type = MOUNT_PC;                      /* TODO */

    strncpy(sb->f_fstypename, "unknown", MFSNAMELEN);
    if (GetVolumeInformationA(path,
            volName, MNAMELEN,                  /* VolumeName and size */
            NULL, &MaximumComponentLength, &FileSystemFlags, fsName, MFSNAMELEN)) /* filesystem type */
    {                                           /* FileSystem type/NTFS, FAT etc */
        if (fsName[0]) {
            strncpy(sb->f_fstypename, fsName, MFSNAMELEN);
        }
        ready = TRUE;
    }

    switch (GetDriveTypeA(path)) {              /* device */
    case DRIVE_REMOVABLE:
        strncpy(sb->f_mntfromname, "Removable", MNAMELEN);
        query_free = ready;
        break;
    case DRIVE_FIXED:
        strncpy(sb->f_mntfromname, "Hard Disk", MNAMELEN);
        query_free = TRUE;
        break;
    case DRIVE_REMOTE:
        strncpy(sb->f_mntfromname, "Networked", MNAMELEN);
        if (0 == strcmp(sb->f_fstypename, "9P"))
            query_free = ready;                 /* WSL2 */
        break;
    case DRIVE_CDROM:
        strncpy(sb->f_mntfromname, "CD-ROM", MNAMELEN);
        query_free = ready;
        break;
    case DRIVE_RAMDISK:
        strncpy(sb->f_mntfromname, "RAM disk", MNAMELEN);
        query_free = TRUE;
        break;
    case DRIVE_NO_ROOT_DIR:
    default:
        strncpy(sb->f_mntfromname, "Unknown", MNAMELEN);
        break;
    }

    sb->f_bsize = 1024;                         /* block size */

    if (query_free)
    {
        DWORD  SectorsPerCluster, BytesPerSector, FreeClusters, Clusters;

        if (GetDiskFreeSpaceA(path, &SectorsPerCluster, &BytesPerSector, &FreeClusters, &Clusters)) {
            /* KBytes available */
            sb->f_bavail = (unsigned int)
                (((__int64)SectorsPerCluster * BytesPerSector * FreeClusters) / 1024);

            /* KBytes total */
            sb->f_blocks = (unsigned int)
                (((__int64)SectorsPerCluster * BytesPerSector * Clusters) / 1024);

            /* inodes */
            sb->f_ffree = FreeClusters / 10;
            sb->f_files = Clusters / 10;
        }
    }

    EMODERESTORE()

    return 0;
}


int
statfsW(const wchar_t *path, struct statfs *sb)
{
    wchar_t volName[MNAMELEN], fsName[MFSNAMELEN];
    DWORD   MaximumComponentLength, FileSystemFlags;
    BOOL    ready = FALSE, query_free = FALSE;
    size_t  mnamelen;
    EMODEINIT()

    if (NULL == path || NULL == sb) {
        errno = EFAULT;
        return -1;
    }

    (void) memset(sb, 0, sizeof(*sb));

    EMODESUPPRESS()

    w32_wc2utf(path, sb->f_mntonname, sizeof(sb->f_mntonname));
    w32_dos2unixA(sb->f_mntonname);
    if ((mnamelen = strlen(sb->f_mntonname)) > 3) {
        if (sb->f_mntonname[mnamelen - 1] == '/') {
            sb->f_mntonname[mnamelen - 1] = 0;
                // remove trailing delimiter on mount-points.
        }
    }

    sb->f_type = MOUNT_PC;                      /* TODO */

    strncpy(sb->f_fstypename, "unknown", MFSNAMELEN);
    if (GetVolumeInformationW(path,
            volName, MNAMELEN,                  /* VolumeName and size */
            NULL, &MaximumComponentLength, &FileSystemFlags, fsName, MFSNAMELEN)) /* file system type */
    {                                           /* FileSystem type/NTFS, FAT etc */
        if (fsName[0]) {
            w32_wc2utf(fsName, sb->f_fstypename, sizeof(sb->f_fstypename));
        }
        ready = TRUE;
    }

    switch (GetDriveTypeW(path)) {              /* device */
    case DRIVE_REMOVABLE:
        strncpy(sb->f_mntfromname, "Removable", MNAMELEN);
        query_free = ready;
        break;
    case DRIVE_FIXED:
        strncpy(sb->f_mntfromname, "Hard Disk", MNAMELEN);
        query_free = TRUE;
        break;
    case DRIVE_REMOTE:
        strncpy(sb->f_mntfromname, "Networked", MNAMELEN);
        if (0 == strcmp(sb->f_fstypename, "9P"))
            query_free = ready;                 /* WSL2 */
        break;
    case DRIVE_CDROM:
        strncpy(sb->f_mntfromname, "CD-ROM", MNAMELEN);
        query_free = ready;
        break;
    case DRIVE_RAMDISK:
        strncpy(sb->f_mntfromname, "RAM disk", MNAMELEN);
        query_free = TRUE;
        break;
    case DRIVE_NO_ROOT_DIR:
    default:
        strncpy(sb->f_mntfromname, "Unknown", MNAMELEN);
        break;
    }

    sb->f_bsize = 1024;                         /* block size */

    if (query_free) {
        DWORD SectorsPerCluster = 0, BytesPerSector = 0, FreeClusters = 0, Clusters = 0;

        if (GetDiskFreeSpaceW(path, &SectorsPerCluster, &BytesPerSector, &FreeClusters, &Clusters)) {
            /* KBytes available */
            sb->f_bavail = (unsigned int)
                (((__int64)SectorsPerCluster * BytesPerSector * FreeClusters) / 1024);

            /* KBytes total */
            sb->f_blocks = (unsigned int)
                (((__int64)SectorsPerCluster * BytesPerSector * Clusters) / 1024);

            /* inodes */
            sb->f_ffree = FreeClusters / 10;
            sb->f_files = Clusters / 10;
        }
    }

    EMODERESTORE()

    return 0;
}


int
statvfs(const char *path, struct statvfs *vfs)
{
    struct statfs sb = {0};

    if (statfs(path, &sb) != 0) {
        return -1;
    }

    (void) memset(vfs, 0, sizeof(*vfs));
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
//  NAME
//      getmntinfo getmntinfo64 -- get information about mounted file systems
//
//  SYNOPSIS
//      #include <sys/param.h>
//      #include <sys/ucred.h>
//      #include <sys/mount.h>
//
//      int getmntinfo(struct statfs **mntbufp, int flags);
//
//      DESCRIPTION
//          The getmntinfo() function returns an array of statfs structures describing each currently mounted file system
//          The getmntinfo() function passes its flags argument transparently to getfsstat(2).
//
//      RETURN VALUES
//          On successful completion, getmntinfo() returns a count of the number of elements in the array.The pointer to
//          the array is stored into mntbufp.
//
//          If an error occurs, zero is returned and the external variable errno is set to indicate the error. Although the
//          pointer mntbufp will be unmodified, any information previously returned by getmntinfo() will be lost.
//
//      ERRORS
//          The getmntinfo() function may fail and set errno for any of the errors specified for the library routines
//          getfsstat(2) or malloc(3).
//
//      SEE ALSO
//          getfsstat(2), mount(2), stat(2), statfs(2), mount(8)
//
//      HISTORY
//          /The getmntinfo() function first appeared in 4.4BSD.
//
//      BUGS
//          The getmntinfo() function writes the array of structures to an internal static object and returns a pointer to that object.
//          Subsequent calls to getmntinfo() will modify the same object.
//
//          The memory allocated by getmntinfo() cannot be free'd by the application.
*/

static unsigned drive_mask(char drive);
static struct statfs *enum_volumes(struct statfs *result, long resultsize, int *mnts, unsigned *drives);


int
getfsstat(struct statfs *buf, long bufsize, int mode)
{
    struct statfs *sb;
    int mnts = -1;                              // result.

    if (MNT_WAIT != mode && MNT_NOWAIT != mode) {
        errno = EINVAL;
    } else {
        if (NULL != (sb = enum_volumes(buf, buf ? bufsize : 0, &mnts, NULL))) {
            if (NULL == buf) {
                free((void *)sb);               // release temporary; only returning the count.
            }
        }
    }
    return mnts;
}


int
getmntinfo(struct statfs **psb, int flags)
{
    static struct statfs *x_getmntinfo = NULL;  // global instance

    char szDrivesAvail[32 * 4], *p;
    unsigned sbdrives = 0;
    struct statfs *sb;
    int ndrives, cnt = -1;

    if (! psb) {                                // invalid
        errno = EINVAL;
        return -1;

    } else if (MNT_WAIT != flags && MNT_NOWAIT != flags) {
        errno = EINVAL;
        return -1;
    }

    *psb = NULL;

    if (x_getmntinfo) {                         // release previous result
        free((void*) x_getmntinfo);
        x_getmntinfo = NULL;
    }

    (void) GetLogicalDriveStringsA(sizeof(szDrivesAvail), szDrivesAvail);
    for (ndrives = 0, p = szDrivesAvail; *p; p += 4) {
        ++ndrives;                              // A:\<nul>B:\<nul>C:\<nul>...<nul>
    }

    if (ndrives > 0) {                          // by volumes
        int t_cnt = -1;

        if (NULL != (sb = enum_volumes(NULL, ndrives, &t_cnt, &sbdrives))) {
            cnt = t_cnt;
        }
    }

    if (ndrives > 0) {                          // by drives / network-mappings
        if (NULL == sb && 
                NULL == (sb = (struct statfs *)calloc(ndrives, sizeof(struct statfs)))) {
            cnt = -1;

        } else {
            EMODEINIT()
            EMODESUPPRESS()

            for (p = szDrivesAvail; *p && cnt < ndrives; p += 4) {
                const unsigned mask = drive_mask(p[0]);

                if (mask == 0x01 || mask == 0x02) {
                //  if (DRIVE_REMOVABLE == GetDriveTypeA(p)) {
                        continue;               // skip floppies/removable
                //  }
                }

                if ((mask & sbdrives) == 0) {   // not by volume
                    if (0 == statfs(p, sb + cnt)) {
                        sbdrives |= mask;
                        ++cnt;
                    }
                }
            }

            EMODERESTORE();

            if (0 == cnt) {
                free((void *)sb);
            } else {
                x_getmntinfo = sb;
                *psb = sb;
            }
        }
    }
    return cnt;
}


static unsigned
drive_mask(char drive)
{
    if (drive >= 'A' && drive <= 'Z') {
        return 1 << (drive - 'A');
    } else if (drive >= 'a' && drive <= 'z') {
        return 1 << (drive - 'a');
    }
    return 0;
}


static struct statfs *
enum_volumes(struct statfs *result, long resultsize, int *mnts, unsigned *drives)
{
    unsigned sballoc = (result ? resultsize / sizeof(struct statfs) : 0);
    unsigned sbsize = 16, sbcnt = 0, sbdrives = 0;
    struct statfs *sb = result;

    WCHAR   volume[WIN32_PATH_MAX] = {0};
    HANDLE  handle;
    BOOL    ret;

    errno = 0;

    if (result == NULL && (resultsize > (long)sbsize)) {
        sbsize = resultsize;                    // allocation size
    }

    if (INVALID_HANDLE_VALUE != (handle = FindFirstVolumeW(volume, _countof(volume)))) {
        DWORD names_size = 1024 + 1;
        PWCHAR names = NULL;

        do {
            //
            //  query volume(s).
            if (0 == memcmp(volume, L"\\\\?\\", 4 * sizeof(WCHAR))) {
                DWORD rc, count = 0;

                for (;;) {
                    if (NULL == names &&
                            NULL == (names = (PWCHAR)calloc(names_size, sizeof(WCHAR)))) {
                        sballoc = -1;
                        goto error;             // allocation error.
                    }

                    count = 0;
                    names[0] = 0;

                    if (GetVolumePathNamesForVolumeNameW(volume, names, names_size, &count)) {
                        break;                  // success
                    }

                    if ((rc = GetLastError()) != ERROR_MORE_DATA) {
                        break;
                    }

                    assert(count > names_size);
                    names_size = count;
                    free((void*)names);         // additional storage required.
                    names = NULL;
                }

                if (names[0]) {                 // associated path(s)
                    PWCHAR cursor, end;

                    for (cursor = names, end = cursor + count; cursor < end && *cursor; ++cursor) {
                        const unsigned len = (unsigned)wcslen(cursor);
                        struct statfs *csb;

                        if (sbcnt >= sballoc) {
                            struct statfs *t_sb =
                                    (NULL == result ? realloc(sb, (sballoc += sbsize) * sizeof(*sb)) : NULL);
                            if (NULL == t_sb) {
                                sballoc = -1;
                                goto error;     // no-memory or overflow
                            }
                            sb = t_sb;
                        }

                        csb = sb + sbcnt;
                        if (0 == statfsW(cursor, csb)) {
                            if (csb->f_mntonname[1] == ':') {
                                sbdrives |= drive_mask(csb->f_mntonname[0]);
                            }
                            ++sbcnt;
                        }
                        cursor += len;
                    }
                }
            }

            //
            //  next volume.
            ret = FindNextVolumeW(handle, volume, _countof(volume));
            if (! ret) {
                const DWORD lasterr = GetLastError();
                if (lasterr != ERROR_NO_MORE_FILES) {
                    errno = EIO;
                }
                break;
            }
        } while (1);

    error:;
        FindVolumeClose(handle);
        free((void*)names);
    }

    if (sbcnt <= sballoc) {
        if (mnts) *mnts = sbcnt;
        if (drives) *drives = sbdrives;
        return sb;
    }
    if (NULL == result) free((void *)sb);
    return NULL;
}

/*end*/
