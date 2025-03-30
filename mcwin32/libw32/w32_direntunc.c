#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_direntunc_c,"$Id: w32_direntunc.c,v 1.16 2025/03/30 17:16:02 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 unc directory access services ...
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT            0x0501          /* reparse */
#endif

#define _DIRENT_SOURCE
#include "win32_internal.h"
#include <unistd.h>

#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Mpr.lib")

#include <lm.h>                                 /* NetEnum... */
#include <winnetwk.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "win32_direct.h"

static void unc_errno(void);


/////////////////////////////////////////////////////////////////////////////////////////
//  SMB shares
//
//  Theses functions applies only to Server Message Block (SMB) shares.
//  For other types of shares, such as Distributed File System (DFS) or WebDAV shares,
//  use Windows Networking (WNet) functions, which support all types of shares.

int
w32_unc_iterateA(const char *servername, unc_push_t push, void *data)
{
    wchar_t wservername[256], *param = NULL;

    if (servername && servername[0]) {
        if (-1 == w32_utf2wc(servername, wservername, _countof(wservername))) {
            wservername[0] = 0;
        }
        param = wservername;
    }

    return w32_unc_iterateW(param, push, data);
}


int
w32_unc_iterateW(const wchar_t *servername, unc_push_t push, void *data)
{
    SHARE_INFO_502 *buffer = NULL;
    NET_API_STATUS res = 0;
    int ret = 0;

    assert(NULL != push);
    assert(NULL != data);
    do {
        DWORD entries = (DWORD)-1, tr = 0;
        DWORD resume_handle = 0;

        if (servername && !*servername) {       // DNS or NetBIOS name
            servername = NULL;                  // NULL == localserver
        }

        res = NetShareEnum((wchar_t *)servername, 502, (LPBYTE *)&buffer, MAX_PREFERRED_LENGTH, &entries, &tr, &resume_handle);
        if (ERROR_SUCCESS == res || ERROR_MORE_DATA == res) {
            const SHARE_INFO_502 *ent;
            unsigned count = 0;
            DWORD e;

            // build directory ..
            for (e = 0, ent = buffer; e < entries; ++e, ++ent) {
                if (STYPE_DISKTREE == ent->shi502_type) {
                    const wchar_t *filename = ent->shi502_netname;

                    if ('p' == filename[0]) {   // prnproc$ or print$
                        if (0 == wcscmp(filename, L"prnproc$") ||
                                0 == wcscmp(filename, L"print$")) {
                            continue;
                        }
                    }

                    if ((1 == ++count &&        // implied
                            -1 == (ret = push(data, L".."))) ||
                            -1 == (ret = push(data, filename))) {
                        break;
                    }
                }
            }
            NetApiBufferFree(buffer);
        }
    } while (ERROR_MORE_DATA == res);

    return ret;
}


/////////////////////////////////////////////////////////////////////////////////////////
//  General UNC support

DIR *
w32_unc_opendirA(const char *dirname)
{
    NETRESOURCEA nrw = {0};
    HANDLE handle = INVALID_HANDLE_VALUE;
    DWORD result;
    DIR *dp;

    nrw.dwScope = RESOURCE_GLOBALNET;
    nrw.dwType  = RESOURCETYPE_DISK;
    nrw.dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
    nrw.dwUsage = RESOURCEUSAGE_CONTAINER;
    nrw.lpRemoteName = (char *)dirname;

    result = WNetOpenEnumA(RESOURCE_GLOBALNET, RESOURCETYPE_DISK, RESOURCEUSAGE_CONNECTABLE, &nrw, &handle);
    if (NO_ERROR != result) {
        errno = ENOTDIR;
        return (DIR *)NULL;
    }

    if (NULL == (dp = w32_dir_alloc())) {
        (void) WNetCloseEnum(handle);
        return (DIR *)NULL;
    }

    dp->dd_handle = handle;
    dp->dd_magic = DIR_UMAGIC;
    return dp;
}


DIR *
w32_unc_opendirW(const wchar_t *dirname)
{
    NETRESOURCEW nrw = {0};
    HANDLE handle = INVALID_HANDLE_VALUE;
    DWORD result;
    DIR *dp;

    nrw.dwScope = RESOURCE_GLOBALNET;
    nrw.dwType = RESOURCETYPE_DISK;
    nrw.dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
    nrw.dwUsage = RESOURCEUSAGE_CONTAINER;
    nrw.lpRemoteName = (wchar_t *)dirname;

    result = WNetOpenEnumW(RESOURCE_GLOBALNET, RESOURCETYPE_DISK, RESOURCEUSAGE_CONNECTABLE, &nrw, &handle);
    if (NO_ERROR != result) {
        unc_errno();
        return (DIR *)NULL;
    }

    if (NULL == (dp = w32_dir_alloc())) {
        (void) WNetCloseEnum(handle);
        return (DIR *)NULL;
    }

    dp->dd_handle = handle;
    dp->dd_magic = DIR_UMAGIC;
    return dp;
}


struct dirent *
w32_unc_readdirA(DIR *dp)
{
    DWORD bufsize = 4 * 1024;
    DWORD result, count = 0;
    void *buffer;
    char *cursor;

    if (NULL == dp || NULL == dp->dd_handle) {
        errno = EBADF;
        return NULL;
    }
    assert(DIR_UMAGIC == dp->dd_magic);

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:6255)
#endif
    if (NULL == (buffer = alloca(bufsize))) return NULL;
    result = WNetEnumResourceA(dp->dd_handle, &count, buffer, &bufsize);
    if (NO_ERROR != result) {
        unc_errno();
        return NULL;
    }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    cursor = ((LPNETRESOURCEA) buffer)->lpRemoteName;
    if (cursor[0] && cursor[1]) {

        cursor += 2;                            // "//"
        while (*cursor && !IS_PATH_SEP(*cursor)) {
            ++cursor;
        }

        if (IS_PATH_SEP(*cursor)) {             // filename component
            struct dirent *dpent = (struct dirent *)dp->dd_buf;
            size_t namlen = strlen(cursor);

            if (namlen >= sizeof(dpent->d_name))
                namlen = sizeof(dpent->d_name) - 1;
            dpent->d_namlen = (unsigned short)namlen;
            memcpy(dpent->d_name, cursor, namlen + 1 /*nul*/);
            dpent->d_reclen = sizeof(struct dirent);
            return dpent;
        }
    }
    return NULL;
}


struct dirent *
w32_unc_readdirW(DIR *dp)
{
    DWORD bufsize = 4 * 1024;
    DWORD result, count = 0;
    void *buffer;
    wchar_t *cursor;

    if (NULL == dp || NULL == dp->dd_handle) {
        errno = EBADF;
        return NULL;
    }
    assert(DIR_UMAGIC == dp->dd_magic);

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:6255)
#endif
    if (NULL == (buffer = alloca(bufsize))) return NULL;
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    result = WNetEnumResourceW(dp->dd_handle, &count, buffer, &bufsize);
    if (NO_ERROR != result) {
        unc_errno();
        return NULL;
    }

    cursor = ((LPNETRESOURCEW) buffer)->lpRemoteName;
    if (cursor[0] && cursor[1]) {

        cursor += 2;                            // "//"
        while (*cursor && !IS_PATH_SEP(*cursor)) {
            ++cursor;
        }

        if (IS_PATH_SEP(*cursor)) {             // filename component
            struct dirent *dpent = (struct dirent *)dp->dd_buf;
            char filename[sizeof(dpent->d_name)];
            int namlen;

            if ((namlen = w32_wc2utf(cursor, filename, _countof(filename))) < 0) {
                namlen = 0;                     // shouldn't occur
            }
            dpent->d_namlen = namlen;
            memcpy(dpent->d_name, filename, namlen + 1 /*nul*/);
            dpent->d_reclen = sizeof(struct dirent);
            return dpent;
        }
    }
    return NULL;
}


int
w32_unc_closedir(DIR *dp)
{
    if (NULL == dp) {
        errno = EBADF;
        return -1;
    }
    if (dp->dd_handle && INVALID_HANDLE_VALUE != dp->dd_handle) {
        (void) WNetCloseEnum(dp->dd_handle);
        dp->dd_handle = INVALID_HANDLE_VALUE;
    }
    w32_dir_free(dp);
    return 0;
}



/*
 *  Determine if the specific path is a UNC path (i.e. //servername[/[components]])
 */

int
w32_unc_validA(const char *path)
{
    if (IS_PATH_SEP(path[0]) && (path[0] == path[1])) {
        const char *scan;                       // UNC prefix

        path += 2;                              // "//" or "\\"
        if (NULL == (scan = strpbrk(path, "*?|<>\"\\/"))
                || IS_PATH_SEP(scan[0])) {
            const size_t namelen =              // servername length
                    (scan ? (size_t)(scan - path) : strlen(path));

            if (namelen > 0) {
                return (int)namelen;
            }
        }
    }
    return 0;
}


int
w32_unc_validW(const wchar_t *path)
{
    if (IS_PATH_SEP(path[0]) && (path[0] == path[1])) {
        const wchar_t *scan;                    // UNC prefix

        path += 2;                              // "//" or "\\"
        if (NULL == (scan = wcspbrk(path, L"*?|<>\"\\/"))
                || IS_PATH_SEP(scan[0])) {
            const size_t namelen =              // servername length
                    (scan ? (size_t)(scan - path) : wcslen(path));

            if (namelen > 0) {
                return (int)namelen;
            }
        }
    }
    return 0;
}


/*
 *  Determine if the specific path is a UNC root (i.e. //servername[/])
 */

int
w32_unc_rootA(const char *path, int *length)
{
    int namelen;

    if ((namelen = w32_unc_validA(path)) > 0) {
        const char *end = path + 2 + namelen;

        if (0 == end[0] ||                      // "//servername[/]"
                (0 == end[1] && IS_PATH_SEP(end[0]))) {

            char computerName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
            DWORD computerSz = _countof(computerName);

            if (length) *length = namelen;
            if (GetComputerNameA(computerName, &computerSz)) {
                if ((DWORD)namelen == computerSz &&
                        0 == _strnicmp(path + 2, computerName, namelen)) {
                    return 2;                   // local server
                }
            }
            return 1;                           // remote
        }
    }
    return 0;
}


int
w32_unc_rootW(const wchar_t *path, int *length)
{
    int namelen;

    if ((namelen = w32_unc_validW(path)) > 0) {
        const wchar_t *end = path + 2 + namelen;

        if (0 == end[0] ||                      // "//servername[/]"
                (0 == end[1] && IS_PATH_SEP(end[0]))) {

            wchar_t computerName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
            DWORD computerSz = _countof(computerName);

            if (length) *length = namelen;
            if (GetComputerNameW(computerName, &computerSz)) {
                if ((DWORD)namelen == computerSz &&
                        0 == _wcsnicmp(path + 2, computerName, namelen)) {
                    return 2;                   // local server
                }
            }
            return 1;                           // remote
        }
    }
    return 0;
}


static void
unc_errno(void)
{
    int ret = EINVAL;
    switch (GetLastError()) {
    case ERROR_ACCESS_DENIED:
    case ERROR_SHARING_VIOLATION:
        ret = EACCES;  break;
    case ERROR_FILE_NOT_FOUND:
        ret = ENOENT;  break;
    case ERROR_PATH_NOT_FOUND:
    case ERROR_INVALID_DRIVE:
        ret = ENOTDIR; break;
    case ERROR_NOT_SUPPORTED:
        ret = ENOTSUP; break;
    default: break;
    }
    errno = ret;
}

/*end*/
