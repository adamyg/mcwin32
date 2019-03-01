#ifndef LIBW32_WIN32_IOCTL_H_INCLUDED
#define LIBW32_WIN32_IOCTL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_ioctl_h,"$Id: win32_ioctl.h,v 1.2 2018/10/18 22:39:47 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 ioctl support.
 *
 * Copyright (c) 2018, Adam Young.
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

#pragma comment(lib, "Advapi32.lib")

#include "win32_include.h"                      /* <windows.h> */

#if !defined(__WATCOMC__)
#undef DEFINE_GUIDS
#include <winioctl.h>                           /* DeviceIoControls */
#endif

#if defined(HAVE_NTIFS_H)
#include <ntifs.h>
#endif

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

#define MAX_REPARSE_SIZE    (512+(16*1024))     /* Header + 16k */

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

#endif /*LIBW32_WIN32_IOCTL_H_INCLUDED*/
