/* -*- mode: c; indent-width: 4; -*- */
/*
* win32 directory access services ...
*
*      opendir, closedir, readdir, seekdir, rewindir, telldir
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
*
* Sourced originally from a public domain implementation and highly.
*/

#ifndef _WIN32_WINNT
#define _WIN32_WINNT            0x0501
#endif

#include "win32_internal.h"
#include <unistd.h>

#pragma comment(lib, "Advapi32.lib")
#include <winioctl.h>                           /* DeviceIoControls */
#if defined(HAVE_NTIFS_H)
#include <ntifs.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

#if !defined(HAVE_NTIFS_H) && !defined(__MINGW32__)
typedef struct _REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG  Flags;
            WCHAR  PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR  PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR DataBuffer[1];
        } GenericReparseBuffer;
    };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;
#endif  //HAVE_NTIFS_H

#ifdef  __WATCOMC__                             /* WC19 SDK has incorrect definitions */
#undef  IsReparseTagMicrosoft
#undef  IO_REPARSE_TAG_SYMLINK
#undef  IO_REPARSE_TAG_MOUNT_POINT
#endif

#ifndef IsReparseTagMicrosoft
#define IsReparseTagMicrosoft(_tag) ((_tag) & 0x80000000UL)
#endif
#ifndef IO_REPARSE_TAG_MOUNT_POINT
#define IO_REPARSE_TAG_MOUNT_POINT (0xA0000003L)
#endif
#ifndef IO_REPARSE_TAG_SYMLINK
#define IO_REPARSE_TAG_SYMLINK (0xA000000CL)
#endif


static void
replace_dir(char *buf, int maxlen, const char *original, const char *replacement)
{
    assert(buf && maxlen > 0);

    if (maxlen-- > 0) {                         /* reserve nul */
        const char *d1 = strrchr(original, '/'), *d2 = strrchr(original, '\\'),
            *d = (d1 > d2 ? d1 : d2);           /* last delimitor; if any */

                                                /* note: wont deal with parent directory references */
        if (d++) {                              /* include delimitor within result */
            const int dirlen =
                ((d - original) < maxlen ? (d - original) : maxlen);

            memcpy(buf, original, dirlen);
            strncpy(buf + dirlen, replacement, maxlen - dirlen);

        }
        else {
            strncpy(buf, replacement, maxlen);
        }
        buf[maxlen] = 0;                        /* nul terminate */
    }
}


int
w32_reparse_read(const char *name, char *buf, int maxlen)
{
#define MAX_REPARSE_SIZE        (512+(16*1024)) /* Header + 16k */
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    BYTE reparseBuffer[MAX_REPARSE_SIZE] = { 0 };
    PREPARSE_DATA_BUFFER rdb = (PREPARSE_DATA_BUFFER)reparseBuffer;
    DWORD returnedLength;
    int ret = -1;

    assert(24 == sizeof(REPARSE_DATA_BUFFER));

    if ((fileHandle = CreateFile(name, 0,       /* open the file image */
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL)) == INVALID_HANDLE_VALUE) {
        return -1;
    }

    /* retrieve reparse details */
    if (DeviceIoControl(fileHandle, FSCTL_GET_REPARSE_POINT,
        NULL, 0, rdb, sizeof(reparseBuffer), &returnedLength, NULL)) {
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

                    if (len >= sizeof(resolved)) len = sizeof(resolved);
                    if (len >= (size_t)maxlen) len = maxlen;

                    if (len >= 2 &&             /* absolute or relative */
                            ':' == resolved[1] && isalpha(resolved[0])) {
                        (void)memcpy(buf, resolved, len);
                        buf[len] = 0;
                    } else {
                        resolved[len] = 0;
                        replace_dir(buf, maxlen, name, resolved);
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
                    size_t len = wcstombs(resolved, mount, sizeof(resolved) - 1);

                    if (len >= sizeof(resolved)) len = sizeof(resolved);
                    if (len >= (size_t) maxlen) len = maxlen;

                    if (0 == strncmp(resolved, "\\??\\", 4)) { /* absolute or relative */
                        (void)memcpy(buf, resolved + 4, len -= 4);
                        buf[len] = 0;
                    } else {
                        resolved[len] = 0;
                        replace_dir(buf, maxlen, name, resolved);
                    }
                    ret = 0;
                }
                break;
            }
        }
    }

    CloseHandle(fileHandle);
    return ret;
}

/*end*/
