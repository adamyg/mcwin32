#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_link_c, "$Id: w32_link.c,v 1.14 2023/09/17 13:04:58 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 link() system calls.
 *
 * Copyright (c) 2007, 2012 - 2023 Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 * or (at your option) any later version.
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

#include "win32_internal.h"
#include "win32_io.h"
#include <unistd.h>

typedef BOOL(WINAPI *CreateHardLinkW_t)(
        LPCWSTR lpFileName, LPCWSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

typedef BOOL(WINAPI *CreateHardLinkA_t)(
        LPCSTR lpFileName, LPCSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

static BOOL                 my_CreateHardLinkW(const wchar_t *lpLinkFileName, const wchar_t *lpTargetFileName);
static BOOL WINAPI          my_CreateHardLinkImpW(LPCWSTR lpFileName, LPCWSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

static BOOL                 my_CreateHardLinkA(const char *lpLinkFileName, const char *lpTargetFileName);
static BOOL WINAPI          my_CreateHardLinkImpA(LPCSTR lpFileName, LPCSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

int                         __w32_link_backup = FALSE;


/*
//  NAME
//      link - link to a file
//
//  SYNOPSIS
//      #include <<unistd.h>
//
//      int link(const char *path1, const char *path2);
//
//  DESCRIPTION
//      The link() function shall create a new link (directory entry) for the existing file, path1.
//
//      The path1 argument points to a pathname naming an existing file. The path2 argument
//      points to a pathname naming the new directory entry to be created. The link()
//      function shall atomically create a new link for the existing file and the link
//      count of the file shall be incremented by one.
//
//      If path1 names a directory, link() shall fail unless the process has appropriate
//      privileges and the implementation supports using link() on directories.
//
//      Upon successful completion, link() shall mark for update the st_ctime field of the
//      file. Also, the st_ctime and st_mtime fields of the directory that contains the new
//      entry shall be marked for update.
//
//      If link() fails, no link shall be created and the link count of the file shall
//      remain unchanged.
//
//      The implementation may require that the calling process has permission to access
//      the existing file.
//
//  RETURN VALUE
//      Upon successful completion, 0 shall be returned. Otherwise, -1 shall be returned
//      and errno set to indicate the error.
//
//  ERRORS
//      The link() function shall fail if:
//
//      [EACCES]
//          A component of either path prefix denies search permission, or the requested
//          link requires writing in a directory that denies write permission, or the
//          calling process does not have permission to access the existing file and this
//          is required by the implementation.
//      [EEXIST]
//          The path2 argument resolves to an existing file or refers to a symbolic link.
//      [ELOOP]
//          A loop exists in symbolic links encountered during resolution of the path1 or
//          path2 argument.
//      [EMLINK]
//          The number of links to the file named by path1 would exceed {LINK_MAX}.
//      [ENAMETOOLONG]
//          The length of the path1 or path2 argument exceeds { PATH_MAX} or a pathname
//          component is longer than { NAME_MAX}.
//      [ENOENT]
//          A component of either path prefix does not exist; the file named by path1 does
//          not exist; or path1 or path2 points to an empty string.
//      [ENOSPC]
//          The directory to contain the link cannot be extended.
//      [ENOTDIR]
//          A component of either path prefix is not a directory.
//      [EPERM]
//          The file named by path1 is a directory and either the calling process does not
//          have appropriate privileges or the implementation prohibits using link() on
//          directories.
//      [EROFS]
//          The requested link requires writing in a directory on a read-only file system.
//      [EXDEV]
//          The link named by path2 and the file named by path1 are on different file
//          systems and the implementation does not support links between file systems.
//      [EXDEV]
//          path1 refers to a named STREAM. [Option End]
//
//      The link() function may fail if:
//
//      [ELOOP]
//          More than { SYMLOOP_MAX} symbolic links were encountered during resolution of
//          the path1 or path2 argument.
//      [ENAMETOOLONG]
//          As a result of encountering a symbolic link in resolution of the path1 or path2
//          argument, the length of the substituted pathname string exceeded { PATH_MAX}.
*/

LIBW32_API int
w32_link(const char *path1, const char *path2)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t wpath1[WIN32_PATH_MAX], wpath2[WIN32_PATH_MAX];

        if (NULL == path1 || NULL == path2) {
            errno = EFAULT;
            return -1;
        }

        if (w32_utf2wc(path1, wpath1, _countof(wpath1)) > 0 &&
                w32_utf2wc(path2, wpath2, _countof(wpath2)) > 0) {
            return w32_linkW(wpath1, wpath2);
        }

        return -1;
    }
#endif  //UTF8FILENAMES

    return w32_linkA(path1, path2);
}


LIBW32_API int
w32_linkW(const wchar_t *path1, const wchar_t *path2)
{
    int ret = -1;

    if (!path1 || !path2) {
        errno = EFAULT;

    } else if (!*path1 || !*path2) {
        errno = ENOENT;

    } else if (wcslen(path1) > MAX_PATH || wcslen(path2) > MAX_PATH) {
        errno = ENAMETOOLONG;

    } else if (GetFileAttributesW(path2) != INVALID_FILE_ATTRIBUTES /*0xffffffff*/) {
        errno = EEXIST;

    } else {    // Note: Generally only available under an Admin account
        ret = 0;
        if (! my_CreateHardLinkW(/*target-link*/ path2, /*existing*/ path1)) {
            switch (GetLastError()) {
            case ERROR_ACCESS_DENIED:
            case ERROR_SHARING_VIOLATION:
            case ERROR_PRIVILEGE_NOT_HELD:
                errno = EACCES;  break;
            case ERROR_FILE_NOT_FOUND:
                errno = ENOENT;  break;
            case ERROR_PATH_NOT_FOUND:
            case ERROR_INVALID_DRIVE:
                errno = ENOTDIR; break;
            case ERROR_WRITE_PROTECT:
                errno = EROFS;   break;
            default:
                w32_errno_set();
                break;
            }
            ret = -1;
        }
    }
    return ret;
}


LIBW32_API int
w32_linkA(const char *path1, const char *path2)
{
    int ret = -1;

    if (!path1 || !path2) {
        errno = EFAULT;

    } else if (!*path1 || !*path2) {
        errno = ENOENT;

    } else if (strlen(path1) > MAX_PATH || strlen(path2) > MAX_PATH) {
        errno = ENAMETOOLONG;

    } else if (GetFileAttributesA(path2) != INVALID_FILE_ATTRIBUTES /*0xffffffff*/) {
        errno = EEXIST;

    } else {    // Note: Generally only available under an Admin account
        ret = 0;
        if (! my_CreateHardLinkA(/*target-link*/ path2, /*existing*/ path1)) {
            switch (GetLastError()) {
            case ERROR_ACCESS_DENIED:
            case ERROR_SHARING_VIOLATION:
            case ERROR_PRIVILEGE_NOT_HELD:
                errno = EACCES;  break;
            case ERROR_FILE_NOT_FOUND:
                errno = ENOENT;  break;
            case ERROR_PATH_NOT_FOUND:
            case ERROR_INVALID_DRIVE:
                errno = ENOTDIR; break;
            case ERROR_WRITE_PROTECT:
                errno = EROFS;   break;
            default:
                w32_errno_set();
                break;
            }
            ret = -1;
        }
    }
    return ret;
}


static BOOL
my_CreateHardLinkW(LPCWSTR lpFileName, LPCWSTR lpExistingFileName)
{
    static CreateHardLinkW_t x_CreateHardLinkW = NULL;

    if (NULL == x_CreateHardLinkW) {
        HINSTANCE hinst;                        // Vista+

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
        if (0 == (hinst = LoadLibraryA("Kernel32")) ||
                0 == (x_CreateHardLinkW =
                        (CreateHardLinkW_t)GetProcAddress(hinst, "CreateHardLinkW"))) {
                                                // XP+
            x_CreateHardLinkW = my_CreateHardLinkImpW;
            if (hinst) FreeLibrary(hinst);
        }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif
    }
    return x_CreateHardLinkW(lpFileName, lpExistingFileName, NULL);
}


static BOOL WINAPI
my_CreateHardLinkImpW(LPCWSTR lpFileName, LPCWSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    __PUNUSED(lpSecurityAttributes)

    if (!__w32_link_backup) {                   /* backup fallback option */
        SetLastError(ERROR_NOT_SUPPORTED);      // not implemented
        return FALSE;

    } else {
        WIN32_STREAM_ID wsi = { 0 };
        void *ctx = NULL;
        HANDLE handle;
        int wlen;
        DWORD cnt;

        if (INVALID_HANDLE_VALUE ==             /* source image */
                    (handle = CreateFileW(lpExistingFileName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_POSIX_SEMANTICS, NULL))) {
            w32_errno_set();
            CloseHandle(handle);
            return FALSE;
        }

        wlen = wcslen(lpFileName);
        wsi.dwStreamId = BACKUP_LINK;
        wsi.dwStreamAttributes = 0;
        wsi.dwStreamNameSize = 0;
        wsi.Size.QuadPart = wlen;

        if (!BackupWrite(handle, (LPBYTE)&wsi, offsetof(WIN32_STREAM_ID, cStreamName), &cnt, FALSE, FALSE, &ctx) ||
                offsetof(WIN32_STREAM_ID, cStreamName) != cnt ||
                !BackupWrite(handle, (LPBYTE)lpFileName, wlen, &cnt, FALSE, FALSE, &ctx)) {
            w32_errno_set();
            CloseHandle(handle);
            return FALSE;
        }

        BackupWrite(handle, NULL, 0, &cnt, TRUE, FALSE, &ctx);
        CloseHandle(handle);

        return TRUE;
    }
}


static BOOL
my_CreateHardLinkA(LPCSTR lpFileName, LPCSTR lpExistingFileName)
{
    static CreateHardLinkA_t x_CreateHardLinkA = NULL;

    if (NULL == x_CreateHardLinkA) {
        HINSTANCE hinst;                        // Vista+

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
        if (0 == (hinst = LoadLibraryA("Kernel32")) ||
                0 == (x_CreateHardLinkA =
                        (CreateHardLinkA_t)GetProcAddress(hinst, "CreateHardLinkA"))) {
                                                // XP+
            x_CreateHardLinkA = my_CreateHardLinkImpA;
            if (hinst) FreeLibrary(hinst);
        }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif
    }
    return x_CreateHardLinkA(lpFileName, lpExistingFileName, NULL);
}


static BOOL WINAPI
my_CreateHardLinkImpA(LPCSTR lpFileName, LPCSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    __PUNUSED(lpSecurityAttributes)

    if (!__w32_link_backup) {                   /* backup fallback option */
        SetLastError(ERROR_NOT_SUPPORTED);      // not implemented
        return FALSE;

    } else {
        WCHAR wpath[MAX_PATH] = { 0 };
        WIN32_STREAM_ID wsi = { 0 };
        void *ctx = NULL;
        HANDLE handle;
        int wlen;
        DWORD cnt;

        if (INVALID_HANDLE_VALUE ==             /* source image */
                    (handle = CreateFileA(lpExistingFileName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_POSIX_SEMANTICS, NULL))) {
            w32_errno_set();
            CloseHandle(handle);
            return FALSE;
        }

        wlen = mbstowcs(wpath, lpFileName, MAX_PATH) * sizeof(WCHAR);
        wsi.dwStreamId = BACKUP_LINK;
        wsi.dwStreamAttributes = 0;
        wsi.dwStreamNameSize = 0;
        wsi.Size.QuadPart = wlen;

        if (!BackupWrite(handle, (LPBYTE)&wsi, offsetof(WIN32_STREAM_ID, cStreamName), &cnt, FALSE, FALSE, &ctx) ||
                offsetof(WIN32_STREAM_ID, cStreamName) != cnt ||
                !BackupWrite(handle, (LPBYTE)wpath, wlen, &cnt, FALSE, FALSE, &ctx)) {
            w32_errno_set();
            CloseHandle(handle);
            return FALSE;
        }

        BackupWrite(handle, NULL, 0, &cnt, TRUE, FALSE, &ctx);
        CloseHandle(handle);

        return TRUE;
    }
}

/*end*/
