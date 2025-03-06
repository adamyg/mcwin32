#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_reparse_c,"$Id: w32_reparse.c,v 1.19 2025/03/06 16:59:46 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 directory support services
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
 *
 * Sourced originally from a public domain implementation and highly.
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include "win32_internal.h"
#include "win32_ioctl.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#include <assert.h>
#include <unistd.h>

#if !defined(SYMLINK_FLAG_RELATIVE)
#define SYMLINK_FLAG_RELATIVE   1
#endif


static void
replace_dir(char *buf, size_t maxlen, const char *original, const char *replacement)
{
    assert(buf && maxlen > 0);

    if (maxlen-- > 0) {                         /* reserve nul */
        const char *d1 = strrchr(original, '/'), *d2 = strrchr(original, '\\'),
            *d = (d1 > d2 ? d1 : d2);           /* last delimit or; if any */

                                                /* note: wont deal with parent directory references */
        if (d++) {                              /* include delimiter within result */
            const size_t dirlen =
                ((size_t)(d - original) < maxlen ? (size_t)(d - original) : maxlen);

            memcpy(buf, original, dirlen);
            strncpy(buf + dirlen, replacement, maxlen - dirlen);

        } else {
            strncpy(buf, replacement, maxlen);
        }
        buf[maxlen-1] = 0;                      /* nul terminate */
    }
}


static void
replace_wdir(wchar_t *buf, size_t maxlen, const wchar_t *original, const wchar_t *replacement)
{
    assert(buf && maxlen > 0);

    if (maxlen-- > 0) {                         /* reserve nul */
        const wchar_t *d1 = wcsrchr(original, '/'), *d2 = wcsrchr(original, '\\'),
            *d = (d1 > d2 ? d1 : d2);           /* last delimiter; if any */

                                                /* note: wont deal with parent directory references */
        if (d++) {                              /* include delimiter within result */
            const size_t dirlen =
                ((size_t)(d - original) < maxlen ? (size_t)(d - original) : maxlen);

            wmemcpy(buf, original, dirlen);
            wcsncpy(buf + dirlen, replacement, maxlen - dirlen);

        } else {
            wcsncpy(buf, replacement, maxlen);
        }
        buf[maxlen-1] = 0;                      /* nul terminate */
    }
}


static void
memxcpy(char *dst, const char *src, size_t len, size_t maxlen)
{
    if (len >= maxlen) len = maxlen - 1;        /* limit to upper limit; plus nul terminator */
    (void) memcpy(dst, src, len);
    dst[len]=0;
}


LIBW32_API int
w32_reparse_readA(const char *name, char *buf, size_t maxlen)
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    BYTE *reparseBuffer = NULL;
    DWORD dwret = 0;
    int ret = -1;

    assert(24 == sizeof(REPARSE_DATA_BUFFER));

    /*
     *  open resource
     */
    if ((handle = CreateFileA(name, 0,      /* open the file image */
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
            FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL)) == INVALID_HANDLE_VALUE) {
        return -1;
    }

    /*
     *  retrieve reparse details
     */
    reparseBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_REPARSE_SIZE);
    if (reparseBuffer && 
            DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT,
                NULL, 0, (LPVOID)reparseBuffer, MAX_REPARSE_SIZE, &dwret, NULL)) {

        const REPARSE_DATA_BUFFER *rdb = (const REPARSE_DATA_BUFFER*)reparseBuffer;

        if (IsReparseTagMicrosoft(rdb->ReparseTag)) {
            char resolved[1024] = { 0 };        /* resolved name */
            int length;

            switch (rdb->ReparseTag) {
            case IO_REPARSE_TAG_SYMLINK:
                //
                //  Symbolic links.
                //
                if ((length = rdb->SymbolicLinkReparseBuffer.SubstituteNameLength) >= 4) {
                    const size_t offset = rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
                    const wchar_t* symlink = rdb->SymbolicLinkReparseBuffer.PathBuffer + offset;
                    size_t len = wcstombs(resolved, symlink, sizeof(resolved) - 1);

                    if (len != (size_t)-1) {
                        assert(len < sizeof(resolved));
                        resolved[len] = 0;

                        if (len >= 2 &&         /* eg: "C://link" */
                                ':' == resolved[1] && isalpha(resolved[0])) {
                            memxcpy(buf, resolved, len, maxlen);
                        } else if (0 == strncmp(resolved, "\\??\\", 4)) {
                                                /* eg: "C:/Users/All Users" */
                            memxcpy(buf, resolved + 4, len - 4, maxlen);
                        } else {                /* relative */
                            replace_dir(buf, maxlen, name, resolved);
                        }
                        ret = 0;
                    }
                }
                break;

            case IO_REPARSE_TAG_MOUNT_POINT:
                //
                //  Mount points and junctions.
                //
                if ((length = rdb->MountPointReparseBuffer.SubstituteNameLength) > 0) {
                    const size_t offset = rdb->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
                    const wchar_t* mount = rdb->MountPointReparseBuffer.PathBuffer + offset;
                    size_t len = wcstombs(resolved, mount, sizeof(resolved) - 1);

                    if (len != (size_t)-1) {
                        assert(len < sizeof(resolved));
                        resolved[len] = 0;

                        if (0 == strncmp(resolved, "\\??\\", 4)) {
                            /* absolute. */
                            if (0 == strncmp(resolved, "\\??\\Volume{", 11)) {
                                                /* mount, resolve volume. */
                                wchar_t volume[1024], pathNames[1024 * 4];
                                DWORD pathLen = 0;

                                wcscpy(volume, L"\\\\?\\");
                                wcscpy(volume + 4, mount + 4);

                                if (GetVolumePathNamesForVolumeNameW(volume, pathNames, _countof(pathNames), &pathLen) && pathLen > 0) {
                                    if ((size_t)-1 != (len = wcstombs(resolved, pathNames, _countof(resolved)-1))) {
                                        memxcpy(buf, resolved, len, maxlen);
                                        ret = 0;
                                    }
                                }
                            } else {            /* junction, remove leading "\??\" */
                                memxcpy(buf, resolved + 4, len - 4, maxlen);
                                ret = 0;
                            }
                        } else {                /* relative. */
                            replace_dir(buf, maxlen, name, resolved);
                            ret = 0;
                        }
                    }
                }
                break;
            }
        }
    }

    HeapFree(GetProcessHeap(), 0, reparseBuffer);
    CloseHandle(handle);
    return ret;
}



LIBW32_API int
w32_reparse_readW(const wchar_t *name, wchar_t *buf, size_t maxlen)
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    BYTE *reparseBuffer = NULL;
    DWORD dwret = 0;
    int ret = -1;

    assert(24 == sizeof(REPARSE_DATA_BUFFER));

    /*
     *  open resource
     */
    if ((handle = CreateFileW(name, 0, /* open the file image */
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
            FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL)) == INVALID_HANDLE_VALUE) {
        return -1;
    }

    /*
     *  retrieve reparse details
     */
    reparseBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_REPARSE_SIZE);
    if (reparseBuffer && 
            DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT,
                NULL, 0, (LPVOID)reparseBuffer, MAX_REPARSE_SIZE, &dwret, NULL)) {

        const REPARSE_DATA_BUFFER *rdb = (const REPARSE_DATA_BUFFER *)reparseBuffer;

        if (IsReparseTagMicrosoft(rdb->ReparseTag)) {
            int length;

            switch (rdb->ReparseTag) {
            case IO_REPARSE_TAG_SYMLINK:
                //
                //  Symbolic links.
                //
                if ((length = rdb->SymbolicLinkReparseBuffer.SubstituteNameLength) >= 4) {
                    const size_t offset = rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
                    const wchar_t* symlink = rdb->SymbolicLinkReparseBuffer.PathBuffer + offset;

                    if (SYMLINK_FLAG_RELATIVE & rdb->SymbolicLinkReparseBuffer.Flags) {
                        replace_wdir(buf, maxlen, name, symlink);

                    } else if (0 == wcsncmp(symlink, L"\\??\\", 4)) {
                                                /* junction, remove leading "\??\", eg: "C:/Users/All Users" */
                        wcsncpy_s(buf, maxlen, symlink + 4, maxlen);

                    } else {
                        wcsncpy_s(buf, maxlen, symlink, maxlen);
                    }
                    ret = 0;
                }
                break;

            case IO_REPARSE_TAG_MOUNT_POINT:
                //
                //  Mount points and junctions.
                //
                if ((length = rdb->MountPointReparseBuffer.SubstituteNameLength) > 0) {
                    const size_t offset = rdb->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
                    const wchar_t* mount = rdb->MountPointReparseBuffer.PathBuffer + offset;

                    ret = 0;
                    if (SYMLINK_FLAG_RELATIVE & rdb->SymbolicLinkReparseBuffer.Flags) {
                        replace_wdir(buf, maxlen, name, mount);

                    } else if (0 == wcsncmp(mount, L"\\??\\", 4)) {
                        if (0 == wcsncmp(mount, L"\\??\\Volume{", 11)) {
                                                /* mount, resolve volume. */
                            wchar_t volume[1024], pathNames[1024 * 4];
                            DWORD pathLen = 0;

                            ret = -1;
                            wcscpy(volume, L"\\\\?\\");
                            wcscpy(volume + 4, mount + 4);
                            if (GetVolumePathNamesForVolumeNameW(volume, pathNames, _countof(pathNames), &pathLen) && pathLen > 0) {
                                wcsncpy_s(buf, maxlen, pathNames, maxlen);
                                ret = 0;
                            }
                        } else {                /* junction, remove leading "\??\" */
                            wcsncpy_s(buf, maxlen, mount + 4, maxlen);
                        }

                    } else {
                        wcsncpy_s(buf, maxlen, mount, maxlen);
                    }
                }
                break;
            }
        }
    }

    HeapFree(GetProcessHeap(), 0, reparseBuffer);
    CloseHandle(handle);
    return ret;
}

/*end*/
