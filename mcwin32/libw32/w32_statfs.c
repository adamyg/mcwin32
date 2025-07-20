#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_statfs_c,"$Id: w32_statfs.c,v 1.30 2025/07/20 17:26:06 cvsuser Exp $")

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
#include "win32_misc.h"
#include "win32_io.h"

#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/mount.h>
#include <assert.h>

#pragma comment(lib, "Mpr.lib")

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
fstatfs(int fildes, struct statfs *sb)
{
    HANDLE handle = 0;

    if (sb == NULL) {
        errno = EFAULT;

    } else if ((handle = w32_osfhandle(fildes)) == INVALID_HANDLE_VALUE) {
        errno = EBADF;                          // socket or invalid file-descriptor

    } else {
        wchar_t fullname[WIN32_PATH_MAX];
        size_t namelen;

        fullname[0] = 0;
        if ((namelen = w32_GetFinalPathNameByHandleW(handle, fullname, _countof(fullname))) == 0) {
            errno = ENOENT;
        } else {
            return statfsW(fullname, sb);
        }
    }
    return -1;
}


int
statfsA(const char *path, struct statfs *sb)
{
    char   drive[4] = { "X:\\" }, volName[MNAMELEN], fsName[MFSNAMELEN];
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

    if (0 == memcmp(path, "\\\\?\\", 4)) {
        path += 4;                              /* UNC prefix "\\?\" */
    }

    if (path[1] == ':') {
        drive[0] = path[0];                     /* X:\ */
        path = drive;
    }

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
    case DRIVE_UNKNOWN:
    case DRIVE_NO_ROOT_DIR:
    default:
        strncpy(sb->f_mntfromname, "Unknown", MNAMELEN);
        break;
    }

    sb->f_bsize = 1024;                        /* block size */

    if (query_free)
    {
        DWORD SectorsPerCluster, BytesPerSector, FreeClusters, Clusters;

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
    wchar_t drive[4] = { L"X:\\" }, volName[MNAMELEN], fsName[MFSNAMELEN];
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

    if (0 == wmemcmp(path, L"\\\\?\\", 4)) {
        path += 4;                              /* UNC prefix "\\?\" */
    }

    if (path[1] == ':') {
        drive[0] = path[0];                     /* X:\ */
        path = drive;
    }

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
        query_free = ready;
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
        query_free = ready;
        break;
    case DRIVE_UNKNOWN:
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
statvfs(const char* path, struct statvfs *vfs)
{
    struct statfs sb;

    if (NULL == path || NULL == vfs) {
        errno = EFAULT;
        return -1;
    }

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


int
statvfsA(const char *path, struct statvfs *vfs)
{
    struct statfs sb = {0};

    if (NULL == path || NULL == vfs) {
        errno = EFAULT;
        return -1;
    }

    if (statfsA(path, &sb) != 0) {
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


int
statvfsW(const wchar_t *path, struct statvfs* vfs)
{
    struct statfs sb;

    if (NULL == path || NULL == vfs) {
        errno = EFAULT;
        return -1;
    }

    if (statfsW(path, &sb) != 0) {
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


int
fstatvfs(int fildes, struct statvfs *vfs)
{
    HANDLE handle = 0;

    if (NULL == vfs) {
        errno = EFAULT;

    } else if ((handle = w32_osfhandle(fildes)) == INVALID_HANDLE_VALUE) {
        errno = EBADF;                          // socket or invalid file-descriptor

    } else {
        wchar_t fullname[WIN32_PATH_MAX];
        size_t namelen;

        fullname[0] = 0;
        if ((namelen = w32_GetFinalPathNameByHandleW(handle, fullname, _countof(fullname))) == 0) {
            errno = ENOENT;
        } else {
            return statvfsW(fullname, vfs);
        }
    }
    return -1;
}


/*
//  NAME
//      getmntinfo -- get information about mounted file systems
//
//  SYNOPSIS
//      #include <sys/param.h>
//      #include <sys/ucred.h>
//      #include <sys/mount.h>
//
//      int getmntinfo(struct statfs **mntbufp, int flags);
//
//  DESCRIPTION
//      The getmntinfo() function returns an array of statfs structures describing each currently mounted file system
//
//      The getmntinfo() function passes its flags argument transparently to getfsstat(2).
//
//  RETURN VALUES
//      On successful completion, getmntinfo() returns a count of the number of elements in the array.The pointer to
//      the array is stored into mntbufp.
//
//      If an error occurs, zero is returned and the external variable errno is set to indicate the error. Although the
//      pointer mntbufp will be unmodified, any information previously returned by getmntinfo() will be lost.
//
//  ERRORS
//      The getmntinfo() function may fail and set errno for any of the errors specified for the library routines
//      getfsstat(2) or malloc(3).
//
//  SEE ALSO
//      getfsstat(2), mount(2), stat(2), statfs(2), mount(8)
//
//  HISTORY
//      The getmntinfo() function first appeared in 4.4BSD.
//
//  BUGS
//      The getmntinfo() function writes the array of structures to an internal static object and returns a pointer
//      to that object. Subsequent calls to getmntinfo() will modify the same object.
//
//      The memory allocated by getmntinfo() cannot be free'd by the application.
*/

struct StatBlock {
    uint32_t references;
    uint32_t count;
    uint32_t alloced;
    uint32_t mask;
    struct statfs result[1];
};

enum Status { NCUninit = 0, NCIdle, NCRunning };

struct NetworkConnections {
    CRITICAL_SECTION lock;
    enum Status status;
    unsigned ndrives;
    time_t timestamp;
    struct StatBlock *sb;
};

static struct StatBlock *enum_mounts(int flags);
static void enum_volumes(struct StatBlock *sb);
static void enum_connections(struct StatBlock *sb);

static void NetworkEnum(unsigned ndrives);
static void NetworkCached(struct StatBlock *sb);
static DWORD WINAPI NetworkEnumThread(LPVOID lpParam);

static struct StatBlock *sbcreate(size_t count);
static struct statfs *sballoc(struct StatBlock *sb);

static struct StatBlock *sbacquire(void);
static void sbrelease(struct StatBlock *sb);

static uint32_t drive_mask(int drive);

static DWORD x_getmntinfo = (DWORD)-1;          // TLS instance.
static struct NetworkConnections x_nestat;      // enumeration status


int
getfsstat(struct statfs *buf, long bufsize, int mode)
{
    struct StatBlock *sb = NULL;
    int mnts = -1;                              // result.

    if (MNT_WAIT != mode && MNT_NOWAIT != mode) {
        errno = EINVAL;

    } else if (buf && bufsize < 0) {
        errno = EINVAL;

    } else if (NULL != (sb = enum_mounts(mode))) {

        mnts = 0;

        if (sb->count) {
            uint32_t count = sb->count;

            if (buf) {
                // The buffer is filled with an array of statfs structures,
                // return one for each mounted file system up to the byte
                // count specified by bufsize.

                while (count-- && bufsize >= (long)sizeof(struct statfs)) {
                    memcpy(buf + mnts, sb->result + mnts, sizeof(struct statfs)); // copy element
                    bufsize -= sizeof(struct statfs);
                    ++mnts;
                }

                if (bufsize >= (long)sizeof(struct statfs)) { // zero trailing element
                    memset(buf + mnts, 0, sizeof(struct statfs));
                }

            } else {
                // If buf is NULL, returns the mounted count.
                mnts = (int)count;
            }
        }

        sbrelease(sb);
    }
    return mnts;
}


int
getmntinfo(struct statfs **psb, int flags)
{
    struct StatBlock *previous, *sb;

    if (! psb) {                                // invalid
        errno = EINVAL;
        return -1;

    } else if (MNT_WAIT != flags && MNT_NOWAIT != flags) {
        errno = EINVAL;
        *psb = NULL;
        return -1;

    } else if (NULL == (sb = enum_mounts(flags))) {
        *psb = NULL;
        return -1;
    }

    assert(x_getmntinfo != (DWORD)-1);
    previous = TlsGetValue(x_getmntinfo);
    TlsSetValue(x_getmntinfo, sb);
    sbrelease(previous);                        // release previous result

    if (sb->count) {
        *psb = sb->result;
    }
    return (int) sb->count;
}


static struct StatBlock *
enum_mounts(int flags)
{
    wchar_t szDrivesAvail[32 * 4], *cursor;
    struct StatBlock *sb;
    uint32_t ndrives = 0;

    (void) GetLogicalDriveStringsW(_countof(szDrivesAvail), szDrivesAvail);
    for (cursor = szDrivesAvail; *cursor; cursor += 4) {
        ++ndrives;                              // A:\<nul>B:\<nul>C:\<nul>...<nul>
    }

    if (flags == MNT_NOWAIT) {
        NetworkEnum(ndrives);                   // trigger background enumeration
    }

    if (NULL == (sb = sbcreate(ndrives))) {
        return NULL;
    }

    if (ndrives) {
        enum_volumes(sb);                       // by volumes

        if (flags == MNT_NOWAIT) {
            NetworkCached(sb);                  // cached connections
        } else {
            enum_connections(sb);               // by connection
        }
    }

    if (ndrives) {                              // by drives
        for (cursor = szDrivesAvail; *cursor; cursor += 4) {
            const uint32_t mask = drive_mask(cursor[0]);

            if (mask == 0x01 || mask == 0x02) {
                continue;                       // skip floppies/removable
            }

            if ((mask & sb->mask) == 0) {       // not enumerated
                struct statfs *sf;

                if (NULL != (sf = sballoc(sb))) {
                    (void) memset(sf, 0, sizeof(*sf));
                    sf->f_type = MOUNT_PC;
                    sf->f_mntonname[0] = (char) cursor[0];
                    sf->f_mntonname[1] = ':';
                    sf->f_mntonname[2] = '/';
                    strncpy(sf->f_fstypename, "unknown", MFSNAMELEN);
                    strncpy(sf->f_mntfromname, "Networked", MNAMELEN);
                    sf->f_bsize = 1024;
                    ++sb->count;
                }
            }
        }
    }
    return sb;
}


static void
enum_volumes(struct StatBlock *sb)
{
    WCHAR   volume[WIN32_PATH_MAX] = {0};
    HANDLE  handle;
    BOOL    ret;

    errno = 0;

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

                if (names[0]) {                 // associated path(s); if mounted
                    PWCHAR cursor, end;

                    for (cursor = names, end = cursor + count; cursor < end && *cursor; ++cursor) {
                        const unsigned len = (unsigned)wcslen(cursor);
                        struct statfs *sf;

                        if (NULL != (sf = sballoc(sb))) {
                            if (0 == statfsW(cursor, sf)) {
                                if (sf->f_mntonname[1] == ':') {
                                    sb->mask |= drive_mask(sf->f_mntonname[0]);
                                }
                                ++sb->count;
                            }
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
}


static void
enum_connections(struct StatBlock *sb)
{
    DWORD cbBuffer = 16384;                     // buffer size
    DWORD cEntries = (DWORD)-1;                 // enumerate all possible entries
    LPNETRESOURCEW lpnrLocal = NULL;            // pointer to enumerated structures
    DWORD dwResultEnum, i;
    HANDLE hEnum = NULL;

    // Enumerate all currently connected resources.
    if (WNetOpenEnumW(RESOURCE_CONNECTED, RESOURCETYPE_DISK, 0, NULL, &hEnum) != NO_ERROR) {
        return;
    }

    if (NULL == (lpnrLocal = (LPNETRESOURCEW) GlobalAlloc(GPTR, cbBuffer))) {
        (void) WNetCloseEnum(hEnum);
        return;
    }

    do {
        ZeroMemory(lpnrLocal, cbBuffer);
        dwResultEnum = WNetEnumResourceW(hEnum, &cEntries, lpnrLocal, &cbBuffer);
        if (dwResultEnum == NO_ERROR) {
            for (i = 0; i < cEntries; ++i) {
                const LPNETRESOURCEW netResource = lpnrLocal + i;

                if (netResource->dwType == RESOURCETYPE_DISK && netResource->lpLocalName) {
                    const wchar_t disk = netResource->lpLocalName[0];

                    if (disk && netResource->lpLocalName[1] == ':') {
                        const uint32_t mask = drive_mask(disk);

                        if ((mask & sb->mask) == 0) {
                            wchar_t drive[4] = { L"X:\\" };
                            struct statfs *sf;

                            drive[0] = disk;
                            if (NULL != (sf = sballoc(sb))) {
                                if (0 == statfsW(drive, sf)) {
                                    sb->mask |= mask;
                                    ++sb->count;
                                }
                            }
                        }
                    }
                }
           }

        } else if (dwResultEnum != ERROR_NO_MORE_ITEMS) {
            break;
        }

    } while (dwResultEnum != ERROR_NO_MORE_ITEMS);

    GlobalFree((HGLOBAL)lpnrLocal);
    (void) WNetCloseEnum(hEnum);
}


static void
NetworkEnum(unsigned ndrives)
{
    BOOL trigger = FALSE;

    // verify cache age/state
    assert(x_nestat.status != NCUninit);
    if (x_nestat.status == NCIdle) {
        const time_t now = time(NULL);

        EnterCriticalSection(&x_nestat.lock);   // --- network enum lock
        if (x_nestat.status == NCIdle) {
            if (ndrives != x_nestat.ndrives ||  // drive change or stale result (60 seconds)
                    now >= (x_nestat.timestamp + 60)) {
                x_nestat.status = NCRunning;
                trigger = TRUE;
            }
        }
        LeaveCriticalSection(&x_nestat.lock);   // --- network enum release
    }

    // enumeration worker
    if (trigger) {
        DWORD dwThreadId = 0;
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif
#if defined(_MSC_VER)
#pragma warning(suppress:28125)                 // CreateThread() try/catch
#pragma warning(disable:4312)                   // type cast
#endif
        HANDLE thread =
            CreateThread(NULL, 0, NetworkEnumThread, (void *)(ndrives), 0, &dwThreadId);
        if (thread != NULL) {
            CloseHandle(thread); // detach
        } else { // thread failure
            EnterCriticalSection(&x_nestat.lock);
            x_nestat.status = NCIdle;
            LeaveCriticalSection(&x_nestat.lock);
        }
    }
}


static void
NetworkCached(struct StatBlock *sb)
{
    struct StatBlock *active;

    if (NULL != (active = sbacquire())) {
        const struct statfs *nsf, *nend;

        nsf = active->result;
        nend = nsf + active->count;

        for (; nsf != nend; ++nsf) {
            const wchar_t disk = nsf->f_mntonname[0];

            if (disk && nsf->f_mntonname[1] == ':') {
                const uint32_t mask = drive_mask(disk);

                if ((mask & sb->mask) == 0) {   // import
                    struct statfs *sf;

                    if (NULL != (sf = sballoc(sb))) {
                        *sf = *nsf;             // re-statfs() local connections?
                        sb->mask |= mask;
                        ++sb->count;
                    }
                }
            }
        }

        sbrelease(active);
    }
}


static DWORD WINAPI
NetworkEnumThread(LPVOID lpParam)
{
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#endif
#if defined(_MSC_VER)
#pragma warning(disable:4311)                   // type cast
#endif
    const unsigned ndrives = (unsigned)(lpParam);
    struct StatBlock *previous = NULL, *sb;
    const time_t then = time(NULL);

    // enumerate network connections
    if (NULL != (sb = sbcreate(ndrives))) {
        enum_connections(sb);
        sb->references = 1;
    }

    // publish results
    EnterCriticalSection(&x_nestat.lock);       // --- network enum lock

    x_nestat.ndrives = ndrives;                 // drive count
    x_nestat.timestamp = then;                  // creation time-stamp
    previous = x_nestat.sb;                     // previous result
    x_nestat.sb = sb;                           // update
    x_nestat.status = NCIdle;

    LeaveCriticalSection(&x_nestat.lock);       // --- network enum release

    // cleanup
    sbrelease(previous);

    return 0;
}


static struct StatBlock *
sbcreate(size_t count)
{
    if (count) {
        const size_t sbbytes = (sizeof(struct statfs) * count);
        struct StatBlock *sb;

        if (NULL != (sb =                       // preliminary block
                (struct StatBlock *) malloc(sizeof(struct StatBlock) + sbbytes))) {
            memset(sb, 0, sizeof(*sb));
            sb->references = 0;
            sb->alloced = (uint32_t)count;
            return sb;
        }
    }
    return NULL;
}


static struct statfs *
sballoc(struct StatBlock *sb)
{
    assert(sb->references == 0);                // single-ownership

    if (sb->count >= sb->alloced) {
        const size_t sbbytes = (sizeof(struct statfs) * (sb->alloced + 8));
        struct StatBlock *sbrealloc;

        if (NULL == (sbrealloc =
                (struct StatBlock *) realloc(sb, sizeof(struct StatBlock) + sbbytes))) {
            return NULL;                        // no-memory or overflow
        }

        sbrealloc->alloced += 16;
        sb = sbrealloc;
    }
    return (sb->result + sb->count);
}


static struct StatBlock *
sbacquire(void)
{
    struct StatBlock *sb;

    EnterCriticalSection(&x_nestat.lock);       // --- network enum lock

    if (NULL != (sb = x_nestat.sb)) {
        assert(sb->references);                 // shared-ownership
        ++sb->references;
    }

    LeaveCriticalSection(&x_nestat.lock);       // --- network enum release
    return sb;
}


static void
sbrelease(struct StatBlock *sb)
{
    if (sb) {
        if (sb->references) {
            uint32_t references;                // --- network enum reference

            EnterCriticalSection(&x_nestat.lock);
            references = --sb->references;
            LeaveCriticalSection(&x_nestat.lock);

            if (references) {
                return;
            }
        }
        free((void*)sb);
    }
}


static uint32_t
drive_mask(int drive)
{
    if (drive >= 'A' && drive <= 'Z') {
        return 1U << (drive - 'A');

    } else if (drive >= 'a' && drive <= 'z') {
        return 1U << (drive - 'a');
    }

    return 0;
}


/*
 *  Run-time initialization function.
 */

#define CRTMODULE statfs                        // our module name

#include "win32_crtinit.h"

static void
crtinit(void)
{
    if (x_getmntinfo == (DWORD)-1) {
        x_getmntinfo = TlsAlloc();              // allocate TLS index
    }

    if (x_nestat.status == NCUninit) {
#if defined(_MSC_VER)
#pragma warning(suppress:28125)                 // InitializeCriticalSection() try/catch
#endif
        (void) InitializeCriticalSectionAndSpinCount(&x_nestat.lock, 0x400);
        x_nestat.status = NCIdle;
    }
}

/*end*/
