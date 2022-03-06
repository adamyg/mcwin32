#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_ino_c,"$Id: w32_ino.c,v 1.11 2022/02/17 16:04:59 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 ino implementation
 *
 * Copyright (c) 2007, 2012 - 2022 Adam Young.
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
#define _WIN32_WINNT        0x0501              /* enable xp+ features */
#endif

#include "win32_internal.h"
#include "win32_misc.h"
#include <ctype.h>
#include <unistd.h>


/*
 *  w32_ino_has ---
 *      Generate a file inode based on a simple hash of the specific file-name.
 */
LIBW32_API ino_t
w32_ino_hash(const char *name)
{
    const char *p = name;
    short hash = 0;
    char c;

    if (name[0] && name[1] == ':') {
        p += 2;                                 /* remove drive */
    }

    for (;*p; ++p) {
        if (ISSLASH(*p)) {                      /* convert slashes */
            c = '/';
        } else {
            c = (char)tolower(*p);
        }
        hash = (hash << 7) + hash + (ino_t)c;
    }
    return hash + (ino_t)(p - name);
}


/*
 *  w32_ino_has ---
 *      Generate a file inode based on a simple hash of the specific file-name.
 */
LIBW32_API ino_t
w32_ino_whash(const wchar_t *name)
{
    const wchar_t *p = name;
    short hash = 0;
    wchar_t c;

    if (name[0] && name[1] == ':') {
        p += 2;                                 /* remove drive */
    }

    for (;*p; ++p) {
        if (ISSLASH(*p)) {                      /* convert slashes */
            c = '/';
        } else {
            c = *p;
            if (c < 0x7f) c = (wchar_t)tolower((char)c);
        }
        hash = (hash << 7) + hash + (ino_t)c;
    }
    return hash + (ino_t)(p - name);
}


/*
 *  w32_ino_gen ---
 *      Generate a file-name inode based on specific file index.
 *
 *  Remarks:
 *
 *      The identifier (low and high parts) and the volume serial number uniquely
 *      identify a file on a single computer. To determine whether two open handles
 *      represent the same file, combine the identifier and the volume serial
 *      number for each file and compare them.
 *
 *      The identifier that is stored in the nFileIndexHigh and nFileIndexLow members
 *      is called the file ID. Support for file IDs is file system-specific. File IDs
 *      are not guaranteed to be unique over time, because file systems are free to
 *      reuse them. In some cases, the file ID for a file can change over time.
 *
 *      In the FAT file system, the file ID is generated from the first cluster of the
 *      containing directory and the byte offset within the directory of the entry for
 *      the file. Some defragmentation products change this byte offset. (Windows
 *      in-box defragmentation does not.) Thus, a FAT file ID can change over time.
 *      Renaming a file in the FAT file system can also change the file ID, but only if
 *      the new file name is longer than the old one.
 *
 *      In the NTFS file system, a file keeps the same file ID until it is deleted. You
 *      can replace one file with another file without changing the file ID by using
 *      the ReplaceFile function. However, the file ID of the replacement file, not the
 *      replaced file, is retained as the file ID of the resulting file.
 *
 */
LIBW32_API ino_t
w32_ino_gen(const DWORD fileIndexLow, const DWORD fileIndexHigh)
{
    //
    //  Linux-NTFS has some documentation about NTFS as well as some programs that can be
    //  used to investigate the MFT and which show the described behavior of the FileIndex.
    //
    //  For example, the docs say, and the programs confirm this, that the root directory
    //  of a volume always has a file reference number of 5, because that is its index in
    //  the MFT.
    //

#define UINT64MAKE(a,b)     ((DWORDLONG)(((DWORD)(a))|(((DWORDLONG)((DWORD)(b))) << 32)))

#define LODWORD(l)          ((DWORD)((uint64_t)(l)))
#define HIDWORD(l)          ((DWORD)(((uint64_t)(l) >> 32) & 0xFFFFFFFF))

#if defined(_MSVC)          /* st_ino, is *not* a ino_t type */
#define INOSIZE             (8 * sizeof(unsigned short))
#define SEQNUMSIZE          (16)
#else
#define INOSIZE             (8 * sizeof(ino_t))
#define SEQNUMSIZE          (16)
#endif

    const uint64_t ino64    =                   /* high + low */
            (uint64_t) UINT64MAKE (fileIndexLow, fileIndexHigh);

    const uint64_t fileid   =                   /* strip sequence number */
            ino64 & ((~(0ULL)) >> SEQNUMSIZE);

    return (ino_t)(((LODWORD(fileid)) ^ ((LODWORD(fileid)) >> INOSIZE))
                    ^ ( (HIDWORD(fileid)) ^ ((HIDWORD(fileid)) >> INOSIZE)));
}


/*
 *  w32_ino_hande ---
 *      Generate a file inode for the specified open file 'handle'.
 */
LIBW32_API ino_t
w32_ino_handle(HANDLE handle)
{
    BY_HANDLE_FILE_INFORMATION fi = {0};

    if (GetFileInformationByHandle(handle, &fi)) {
        return w32_ino_gen(fi.nFileIndexLow, fi.nFileIndexHigh);
    }
    return 0;
}


/*
 *  w32_ino_fildes ---
 *      Generate a file inode for the specified open osf file 'fildes'.
 */
LIBW32_API ino_t
w32_ino_fildes(int fildes)
{
    HANDLE handle;

    if (fildes < 0) {
        return 0;
    } else if (fildes >= WIN32_FILDES_MAX ||
                (handle = (HANDLE) _get_osfhandle(fildes)) == INVALID_HANDLE_VALUE) {
        return 0;
    }
    return w32_ino_handle(handle);
}


/*
 *  w32_ino_file ---
 *      Generate a file inode for the specified file 'path'.
 */
LIBW32_API ino_t
w32_ino_file(const char *path)
{
    HANDLE handle;

    if (NULL != path && *path &&
            INVALID_HANDLE_VALUE != (handle =
                CreateFileA(path, 0, 0, NULL, OPEN_EXISTING,
                            FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_READONLY, NULL))) {
        const ino_t ino = w32_ino_handle(handle);
        CloseHandle(handle);
        return ino;
    }
    return 0;
}


LIBW32_API ino_t
w32_ino_wfile(const wchar_t *path)
{
    HANDLE handle;

    if (NULL != path && *path &&
            INVALID_HANDLE_VALUE != (handle =
                CreateFileW(path, 0, 0, NULL, OPEN_EXISTING,
                            FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_READONLY, NULL))) {
        const ino_t ino = w32_ino_handle(handle);
        CloseHandle(handle);
        return ino;
    }
    return 0;
}

/*end*/
