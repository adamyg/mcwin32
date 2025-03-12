#ifndef LIBW32_WIN32_IOCTL_H_INCLUDED
#define LIBW32_WIN32_IOCTL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_ioctl_h,"$Id: win32_ioctl.h,v 1.9 2025/03/06 16:59:47 cvsuser Exp $")
__CPRAGMA_ONCE
              
/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 ioctl support.
 *
 * Copyright (c) 2018 - 2025, Adam Young.
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
 */

#pragma comment(lib, "Advapi32.lib")

#include "win32_include.h"                      /* <windows.h> */

#if defined(__WATCOMC__)
#undef HAVE_NTDEF_H
#undef HAVE_NTIFS_H

#else
#undef DEFINE_GUIDS
#include <winioctl.h>                           /* DeviceIoControls */

#if defined(_MSC_VER)
#undef HAVE_NTDEF_H
#undef HAVE_NTIFS_H

#elif defined(__MINGW64_VERSION_MAJOR) || defined(HAVE_NTDEF_H)
#include <ntdef.h>
#endif

#endif //WATCOMC

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

#define MAX_REPARSE_SIZE (512+(16*1024))        /* Header + 16k */

#if defined(__WATCOMC__)                        /* WC19 SDK has incorrect definitions */
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
#ifndef IO_REPARSE_TAG_LX_SYMLINK
#define IO_REPARSE_TAG_LX_SYMLINK (0xA000001DL)
#endif

/*
// https://github.com/libyal/libfsntfs/blob/main/documentation/New%20Technologies%20File%20System%20(NTFS).asciidoc
// https://devblogs.microsoft.com/commandline/whats-new-for-wsl-in-windows-10-version-1903/
// https://blog.trailofbits.com/2024/02/12/why-windows-cant-follow-wsl-symlinks/
//
// LinuX Sub-System (LXSS) Symbolic Links
// Note: these are not accessible from a window client; see above.
//
typedef struct _REPARSE_LX_SYMLINK_BUFFER {
    DWORD ReparseTag;
    WORD  ReparseDataLength;
    WORD  Reserved;
    struct {
        DWORD FileType;         // Take member name with a grain of salt. Value is apparently always 2 for symlinks.
        char  PathBuffer[1];    // POSIX path as given to symlink(2).
                //
                //  Path is not \0 terminated, Length is ReparseDataLength - sizeof (FileType).
                //  Always UTF-8, Chars given in incompatible codesets, e.g. umlauts in ISO-8859-x, are converted to the Unicode. rEPLACEMENT CHARACTER 0xfffd
                //
    } LxSymlinkReparseBuffer;
} REPARSE_LX_SYMLINK_BUFFER, *PREPARSE_LX_SYMLINK_BUFFER;
*/

#endif /*LIBW32_WIN32_IOCTL_H_INCLUDED*/
