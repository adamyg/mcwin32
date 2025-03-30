#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_io_c, "$Id: w32_io.c,v 1.38 2025/03/30 17:16:02 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 system io functionality
 *
 *  stat, lstat, fstat, readlink, symlink, open
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

#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x600)
#undef _WIN32_WINNT                             /* Vista+ features; FILE_INFO_BY_HANDLE_CLASS */
#define _WIN32_WINNT 0x0600
#endif

#if defined(__MINGW32__)
#undef  _WIN32_VER
#define _WIN32_VER _WIN32_WINNT
#endif

#if !defined(_LARGEFILE64_SOURCE)
#define _LARGEFILE64_SOURCE
#endif

#include <assert.h>

#include "win32_internal.h"
#include "win32_ioctl.h"

#include "win32_misc.h"
#include "win32_io.h"

#if defined(__WATCOMC__)
#include <shellapi.h>                           /* SHSTDAPI */
#endif
#include <shlobj.h>                             /* shell interface */
#include <shlwapi.h>                            /* wide-char support */
#include <accctrl.h>
#include <aclapi.h>
#include <sddl.h>
#define PSAPI_VERSION       1                   /* GetMappedFileName and psapi.dll */
#include <psapi.h>

#include <sys/cdefs.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#if defined(_DEBUG)
#include <grp.h>
#endif
#include <time.h>
#include <ctype.h>
#include <unistd.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")

#if defined(_MSC_VER)
#pragma warning(disable : 4244) // conversion from 'xxx' to 'xxx', possible loss of data
#pragma warning(disable : 4312) // type cast' : conversion from 'xxx' to 'xxx' of greater size
#endif

#if defined(_MSC_VER)
#define NLINK_T short
#else
#define NLINK_T int
#endif

typedef DWORD (WINAPI *GetFinalPathNameByHandleW_t)(
                        HANDLE hFile, LPWSTR lpszFilePath, DWORD length, DWORD dwFlags);

typedef DWORD (WINAPI *GetFinalPathNameByHandleA_t)(
                        HANDLE hFile, LPSTR lpszFilePath, DWORD length, DWORD dwFlags);

typedef DWORD (WINAPI *GetFileInformationByHandleEx_t)(
                        HANDLE handle, FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInformation, DWORD dwBufferSize);

typedef BOOL  (WINAPI *CreateSymbolicLinkW_t)(
                        LPCWSTR lpSymlinkFileName, LPCWSTR lpTargetFileName, DWORD dwFlags);

typedef BOOL  (WINAPI *CreateSymbolicLinkA_t)(
                        LPCSTR lpSymlinkFileName, LPCSTR lpTargetFileName, DWORD dwFlags);

typedef BOOL  (WINAPI *GetVolumeInformationByHandleW_t)(
                        HANDLE hFile, LPWSTR lpVolumeNameBuffer, DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber,
                            LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, LPWSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize);

enum StatTye { SHSTAT = sizeof(struct stat), SHSTAT64 = sizeof(struct stat64) };
struct StatHandle {
    enum StatTye type;
    void *buf;
};

static int                  W32StatA(const char *path, struct StatHandle *sb);
static int                  W32StatW(const wchar_t *path, struct StatHandle *sb);
static int                  W32StatLinkA(const char *path, struct StatHandle *sb);
static int                  W32StatLinkW(const wchar_t *path, struct StatHandle *sb);
static int                  W32StatHandle(int fildes, struct StatHandle *sb);
static void                 W32StatPipe(HANDLE handle, DWORD ftype, struct StatHandle *sb);

static DWORD                my_GetFinalPathNameByHandleW(HANDLE handle, LPWSTR name, int length);
static DWORD WINAPI         my_GetFinalPathNameByHandleWImp(HANDLE handle, LPWSTR name, DWORD length, DWORD dwFlags);

static DWORD                my_GetFinalPathNameByHandleA(HANDLE handle, LPSTR name, int length);
static DWORD WINAPI         my_GetFinalPathNameByHandleAImp(HANDLE handle, LPSTR name, DWORD length, DWORD dwFlags);

static DWORD                my_GetFileInformationByHandleEx(HANDLE handle, FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInformation, DWORD dwBufferSize);
static DWORD WINAPI         my_GetFileInformationByHandleExImp(HANDLE handle, FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInformation, DWORD dwBufferSize);

static BOOL                 my_CreateSymbolicLinkW(LPCWSTR lpSymlinkFileName, LPCWSTR lpTargetFileName, DWORD dwFlags);
static BOOL WINAPI          my_CreateSymbolicLinkWImp(LPCWSTR lpSymlinkFileName, LPCWSTR lpTargetFileName, DWORD dwFlags);

static BOOL                 my_CreateSymbolicLinkA(const char *lpSymlinkFileName, const char *lpTargetFileName, DWORD dwFlags);
static BOOL WINAPI          my_CreateSymbolicLinkAImp(const char *lpSymlinkFileName, const char *lpTargetFileName, DWORD dwFlags);

static BOOL                 my_GetVolumeInformationByHandle(HANDLE handle, DWORD *serialno, DWORD *flags);
static BOOL WINAPI          my_GetVolumeInformationByHandleImp(HANDLE, LPWSTR, DWORD, LPDWORD, LPDWORD, LPDWORD, LPWSTR, DWORD);

static BOOL                 IsShortcutA(const char *name);
static BOOL                 IsShortcutW(const wchar_t *name);

static void                 StatZero(struct StatHandle *sb);
static void                 StatAttributes(struct StatHandle *sb, mode_t mode, const DWORD dwAttr, const wchar_t *name, const char *magic);
static void                 StatOwner(struct StatHandle *sb, const DWORD dwAttributes, HANDLE handle);
static void                 StatTimes(struct StatHandle *sb, const FILETIME *ftCreationTime, const FILETIME *ftLastAccessTime, const FILETIME *ftLastWriteTime);
static void                 StatSize(struct StatHandle *sb, const DWORD nFileSizeLow, const DWORD nFileSizeHigh);
static void                 StatDevice(struct StatHandle *sb, const DWORD dwVolumeSerialNumber, const DWORD nFileIndexLow, const DWORD nFileIndexHigh, const wchar_t *name);
static void                 StatLinks(struct StatHandle* sb, NLINK_T nlink);
static time_t               ConvertTime(const FILETIME *ft);

static int                  IsScriptMagic(const char *magic);
static BOOL                 IsExecA(const char *name, const char *magic);
static BOOL                 IsExecW(const wchar_t *name, const char *magic);
static const char *         HasExtensionA(const char *name);
static const wchar_t *      HasExtensionW(const wchar_t *name);
static BOOL                 IsExtensionA(const char *name, const char *ext);
static BOOL                 IsExtensionW(const wchar_t *name, const char *ext);

static int                  ReadlinkA(const char *path, const char **suffixes, char *buf, size_t maxlen);
static int                  ReadlinkW(const wchar_t *path, const char **suffixes, wchar_t *buf, size_t maxlen);
static int                  ReadShortcutA(const char *name, char *buf, size_t maxlen);
static int                  ReadShortcutW(const wchar_t *name, wchar_t *buf, size_t maxlen);

static int                  CreateShortcutA(const char *link, const char *name, const char *working, const char *desc);
static int                  CreateShortcutW(const wchar_t *link, const wchar_t *name, const wchar_t *working, const wchar_t *desc);

static int                  W32StatAFile(const char *name, struct StatHandle *sb);
static int                  W32StatWFile(const wchar_t *name, struct StatHandle *sb);

static BOOL                 W32StatByNameA(const char *name, struct StatHandle *sb);
static BOOL                 W32StatByNameW(const wchar_t *name, struct StatHandle *sb);

static BOOL                 W32StatCommon(HANDLE handle, const WIN32_FIND_DATAW *fb, struct StatHandle *sb, const wchar_t *fullname, size_t namelen);

static int                  x_utf8filenames = 0;

static const char *         suffixes_null[] = {
    "", NULL
    };
static const char *         suffixes_default[] = {
    "", ".lnk", NULL
    };


/*
//  NAME
//      w32_utf8filenames_enable - enable UTF8 filenames.
//
//  SYNOPSIS
//      #include <unistd.h>
//
//      int w32_utf8filenames_enable(void);
//      int w32_utf8filenames_state(void);
//
//  RETURN VALUE
//      w32_utf8filenames_enable() returns the previous state, whereas w32_utf8filenames_state()
//      returns the current UTF8 filename handling status.
*/
LIBW32_API int
w32_utf8filenames_enable (void)
{
    const int previous = x_utf8filenames;
    x_utf8filenames = 1;
    return previous;
}


LIBW32_API int
w32_utf8filenames_disable (void)
{
    const int previous = x_utf8filenames;
    x_utf8filenames = 0;
    return previous;
}


LIBW32_API int
w32_utf8filenames_set (int state)
{
    const int previous = x_utf8filenames;
    x_utf8filenames = state;
    return previous;
}


LIBW32_API int
w32_utf8filenames_state (void)
{
    return x_utf8filenames;
}


/*
//  NAME
//      handle conversion
//
//  SYNOPSIS
//      int w32_HTOI(HANDLE handle)
//      HANDLE w32_ITOH(int fd)
//
//  NOTES:
//
//      MSDN - Interprocess Communication Between 32-bit and 64-bit Applications
//
//          64-bit versions of Windows use 32-bit handles for interoperability.
//          When sharing a handle between 32-bit and 64-bit applications, only the lower 32 bits are significant,
//          so it is safe to truncate the handle (when passing it from 64-bit to 32-bit) or sign-extend the handle (when passing it from 32-bit to 64-bit).
//          Handles that can be shared include handles to user objects such as windows (HWND), handles to GDI objects such as pens and brushes (HBRUSH and HPEN),
//          and handles to named objects such as mutex's, semaphores, and file handles.
//
//  RETURN VALUE
//      Converted handle.
*/

int
w32_htof(HANDLE handle)
{
#if defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#endif
#if defined(_WIN32) && (_MSC_VER >= 1700)
    // note: sage to convert HANDLES 64 to 32; only lower 32-bits are used.
    assert((0xffffffff00000000LLU & (uint64_t)handle) == 0 || handle == INVALID_HANDLE_VALUE);
#pragma warning(disable:4311)
#endif
    if (INVALID_HANDLE_VALUE == handle) return -1;
    return (int)handle;
#if defined(__MINGW64__)
#pragma GCC diagnostic pop
#endif
}


HANDLE
w32_ftoh(int fd)
{
#if defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif
    if (-1 == fd) return INVALID_HANDLE_VALUE;
    return (HANDLE)fd;
#if defined(__MINGW64__)
#pragma GCC diagnostic pop
#endif
}


HANDLE
w32_osfhandle(int fildes)
{
    HANDLE handle = INVALID_HANDLE_VALUE;

    if (fildes >= 0 && fildes < WIN32_FILDES_MAX) {
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && defined(_DEBUG)
        if (fildes < _getmaxstdio())            // avoid run-time exceptions
#endif
            handle = (HANDLE) _get_osfhandle(fildes);
    }
    return handle;
}


/*
//  NAME
//      stat - get file status
//
//  SYNOPSIS
//      #include <sys/stat.h>
//
//      int stat(const char *restrict path, struct stat *restrict buf);
//
//  DESCRIPTION
//
//      The stat() function shall obtain information about the named file and write it to
//      the area pointed to by the 'buf' argument. The 'path' argument points to a pathname
//      naming a file. Read, write, or execute permission of the named file is not
//      required. An implementation that provides additional or alternate file access
//      control mechanisms may, under implementation-defined conditions, cause stat() to
//      fail. In particular, the system may deny the existence of the file specified by path.
//
//      If the named file is a symbolic link, the stat() function shall continue pathname
//      resolution using the contents of the symbolic link, and shall return information
//      pertaining to the resulting file if the file exists.
//
//      The 'buf' argument is a pointer to a stat structure, as defined in the <sys/stat.h>
//      header, into which information is placed concerning the file.
//
//      The stat() function shall update any time-related fields (as described in the Base
//      Definitions volume of IEEE Std 1003.1-2001, Section 4.7, File Times Update), before
//      writing into the stat structure.
//
//      Unless otherwise specified, the structure members st_mode, st_ino, st_dev, st_uid,
//      st_gid, st_atime, st_ctime, and st_mtime shall have meaningful values for all file
//      types defined in this volume of IEEE Std 1003.1-2001. The value of the member
//      st_nlink shall be set to the number of links to the file.
//
//  RETURN VALUE
//      Upon successful completion, 0 shall be returned. Otherwise, -1 shall be returned
//      and errno set to indicate the error.
//
//  ERRORS
//
//      The stat() function shall fail if:
//
//      [EACCES]
//          Search permission is denied for a component of the path prefix.
//
//      [EIO]
//          An error occurred while reading from the file system.
//
//      [ELOOP]
//          A loop exists in symbolic links encountered during resolution of the path
//          argument.
//
//      [ENAMETOOLONG]
//          The length of the path argument exceeds {PATH_MAX} or a pathname component is
//          longer than {NAME_MAX}.
//
//      [ENOENT]
//          A component of path does not name an existing file or path is an empty string.
//
//      [ENOTDIR]
//          A component of the path prefix is not a directory.
//
//      [EOVERFLOW]
//          The file size in bytes or the number of blocks allocated to the file or the
//          file serial number cannot be represented correctly in the structure pointed to
//          by buf.
//
//      The stat() function may fail if:
//
//      [ELOOP]
//          More than {SYMLOOP_MAX} symbolic links were encountered during resolution of
//          the path argument.
//
//      [ENAMETOOLONG]
//          As a result of encountering a symbolic link in resolution of the path argument,
//          the length of the substituted pathname string exceeded {PATH_MAX}.
//
//      [EOVERFLOW]
//          A value to be stored would overflow one of the members of the stat structure.
*/

LIBW32_API int
w32_stat(const char *path, struct stat *sb)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (path && sb) {
            wchar_t wpath[WIN32_PATH_MAX];

            if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
                return w32_statW(wpath, sb);
            }
            return -1;
        }
    }
#endif  //UTF8FILENAMES

    return w32_statA(path, sb);
}


LIBW32_API int
w32_stat64(const char *path, struct stat64 *sb)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (path && sb) {
            wchar_t wpath[WIN32_PATH_MAX];

            if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
                return w32_stat64W(wpath, sb);
            }
            return -1;
        }
    }
#endif  //UTF8FILENAMES

    return w32_stat64A(path, sb);
}


LIBW32_API int
w32_statA(const char *path, struct stat *sb)
{
    struct StatHandle sbh = { SHSTAT, sb };
    return W32StatA(path, &sbh);
}


LIBW32_API int
w32_stat64A(const char *path, struct stat64 *sb)
{
    struct StatHandle sbh = { SHSTAT64, sb };
    return W32StatA(path, &sbh);
}


LIBW32_API int
w32_statW(const wchar_t *path, struct stat *sb)
{
    struct StatHandle sbh = { SHSTAT, sb };
    return W32StatW(path, &sbh);
}


LIBW32_API int
w32_stat64W(const wchar_t *path, struct stat64 *sb)
{
    struct StatHandle sbh = { SHSTAT64, sb };
    return W32StatW(path, &sbh);
}


static int
W32StatA(const char *path, struct StatHandle *sb)
{
    char symbuf[WIN32_PATH_MAX] = { 0 };
    int ret = 0;

    if (NULL == path || NULL == sb) {
        ret = -EFAULT;

    } else {
        StatZero(sb);
        if ((ret = ReadlinkA(path, (void*)-1, symbuf, sizeof(symbuf))) > 0) {
            path = symbuf;
        }
    }

    if (ret < 0 || (ret = W32StatAFile(path, sb)) < 0) {
        if (-ENOTDIR == ret) {                  // component error
            if (path != symbuf &&               // expand embedded shortcut
                    w32_expandlinkA(path, symbuf, _countof(symbuf), SHORTCUT_COMPONENT)) {
                if ((ret = W32StatAFile(symbuf, sb)) >= 0) {
                    return ret;
                }
            }
        }
        errno = -ret;
        return -1;
    }
    return 0;
}


static int
W32StatW(const wchar_t *path, struct StatHandle *sb)
{
    wchar_t symbuf[WIN32_PATH_MAX] = { 0 };
    int ret = 0;

    if (NULL == path || NULL == sb) {
        ret = -EFAULT;

    } else {
        StatZero(sb);
        if ((ret = ReadlinkW(path, (void*)-1, symbuf, _countof(symbuf))) > 0) {
            path = symbuf;
        }
    }

    if (ret < 0 || (ret = W32StatWFile(path, sb)) < 0) {
        if (-ENOTDIR == ret) {                  // component error
            if (path != symbuf &&               // expand embedded shortcut
                    w32_expandlinkW(path, symbuf, _countof(symbuf), SHORTCUT_COMPONENT)) {
                if ((ret = W32StatWFile(symbuf, sb)) >= 0) {
                    return ret;
                }
            }
        }
        errno = -ret;
        return -1;
    }
    return 0;
}


/*
//  NAME
//      lstat - get symbolic link status
//
//  SYNOPSIS
//
//      #include <sys/stat.h>
//
//      int lstat(const char *restrict path, struct stat *restrict buf);
//
//  DESCRIPTION
//      The lstat() function shall be equivalent to stat(), except when 'path' refers to a
//      symbolic link. In that case lstat() shall return information about the link, while
//      stat() shall return information about the file the link references.
//
//      For symbolic links, the 'st_mode' member shall contain meaningful information when
//      used with the file type macros, and the 'st_size' member shall contain the length of
//      the pathname contained in the symbolic link. File mode bits and the contents of the
//      remaining members of the stat structure are unspecified. The value returned in the
//      st_size member is the length of the contents of the symbolic link, and does not
//      count any trailing null.
//
//  RETURN VALUE
//      Upon successful completion, lstat() shall return 0. Otherwise, it shall return -1
//      and set errno to indicate the error.
//
//  ERRORS
//
//      The lstat() function shall fail if:
//
//      [EACCES]
//          A component of the path prefix denies search permission.
//
//      [EIO]
//          An error occurred while reading from the file system.
//
//      [ELOOP]
//          A loop exists in symbolic links encountered during resolution of the path
//          argument.
//
//      [ENAMETOOLONG]
//          The length of a pathname exceeds {PATH_MAX} or a pathname component is longer
//          than {NAME_MAX}.
//
//      [ENOTDIR]
//          A component of the path prefix is not a directory.
//
//      [ENOENT]
//          A component of path does not name an existing file or path is an empty string.
//
//      [EOVERFLOW]
//          The file size in bytes or the number of blocks allocated to the file or the
//          file serial number cannot be represented correctly in the structure pointed to
//          by buf.
//
//      The lstat() function may fail if:
//
//      [ELOOP]
//          More than {SYMLOOP_MAX} symbolic links were encountered during resolution of
//          the path argument.
//
//      [ENAMETOOLONG]
//          As a result of encountering a symbolic link in resolution of the path argument,
//          the length of the substituted pathname string exceeded {PATH_MAX}.
//
//      [EOVERFLOW]
//          One of the members is too large to store into the structure pointed to by the
//          buf argument.
*/

LIBW32_API int
w32_lstat(const char *path, struct stat *sb)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (path && sb) {
            wchar_t wpath[WIN32_PATH_MAX];

            if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
                return w32_lstatW(wpath, sb);
            }
            return -1;
        }
    }
#endif  //UTF8FILENAMES

    return w32_lstatA(path, sb);
}


LIBW32_API int
w32_lstat64(const char *path, struct stat64 *sb)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (path && sb) {
            wchar_t wpath[WIN32_PATH_MAX];

            if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
                return w32_lstat64W(wpath, sb);
            }
            return -1;
        }
    }
#endif  //UTF8FILENAMES

    return w32_lstat64A(path, sb);
}


LIBW32_API int
w32_lstatA(const char *path, struct stat *sb)
{
    struct StatHandle sbh = { SHSTAT, sb };
    return W32StatLinkA(path, &sbh);
}


LIBW32_API int
w32_lstat64A(const char *path, struct stat64 *sb)
{
    struct StatHandle sbh = { SHSTAT64, sb };
    return W32StatLinkA(path, &sbh);
}


LIBW32_API int
w32_lstatW(const wchar_t *path, struct stat *sb)
{
    struct StatHandle sbh = { SHSTAT, sb };
    return W32StatLinkW(path, &sbh);
}


LIBW32_API int
w32_lstat64W(const wchar_t *path, struct stat64 *sb)
{
    struct StatHandle sbh = { SHSTAT64, sb };
    return W32StatLinkW(path, &sbh);
}


static int
W32StatLinkA(const char *path, struct StatHandle *sb)
{
    int ret = 0;

    if (path == NULL || sb == NULL) {
        ret = -EFAULT;
    } else {
        StatZero(sb);
    }

    if (ret < 0 || (ret = W32StatAFile(path, sb)) < 0) {
        if (-ENOTDIR == ret) {                  // component error.
            char lnkbuf[WIN32_PATH_MAX];
                                                // expand embedded shortcut
            if (w32_expandlinkA(path, lnkbuf, _countof(lnkbuf), SHORTCUT_COMPONENT)) {
                if ((ret = W32StatAFile(lnkbuf, sb)) >= 0) {
                    return ret;
                }
            }
        }
        errno = -ret;
        return -1;
    }
    return 0;
}


static int
W32StatLinkW(const wchar_t *path, struct StatHandle *sb)
{
    int ret = 0;

    if (path == NULL || sb == NULL) {
        ret = -EFAULT;
    } else {
        StatZero(sb);
    }

    if (ret < 0 || (ret = W32StatWFile(path, sb)) < 0) {
        if (-ENOTDIR == ret) {                  // component error.
            wchar_t lnkbuf[WIN32_PATH_MAX];
                                                // expand embedded shortcut
            if (w32_expandlinkW(path, lnkbuf, _countof(lnkbuf), SHORTCUT_COMPONENT)) {
                if ((ret = W32StatWFile(lnkbuf, sb)) >= 0) {
                    return ret;
                }
            }
        }
        errno = -ret;
        return -1;
    }
    return 0;
}


/*
//  NAME
//      fstat - get file status
//
//  SYNOPSIS
//      #include <sys/stat.h>
//
//      int fstat(int fildes, struct stat *buf);
//
//  DESCRIPTION
//      The fstat() function shall obtain information about an open file associated with
//      the file descriptor 'fildes', and shall write it to the area pointed to by 'buf'.
//
//      If 'fildes' references a shared memory object, the implementation shall update in the
//      stat structure pointed to by the 'buf' argument only the st_uid, st_gid, st_size, and
//      st_mode fields, and only the S_IRUSR, S_IWUSR, S_IRGRP, S_IWGRP, S_IROTH, and
//      S_IWOTH file permission bits need be valid. The implementation may update other
//      fields and flags.
//
//      If 'fildes' references a typed memory object, the implementation shall update in the
//      stat structure pointed to by the 'buf' argument only the st_uid, st_gid, st_size, and
//      st_mode fields, and only the S_IRUSR, S_IWUSR, S_IRGRP, S_IWGRP, S_IROTH, and
//      S_IWOTH file permission bits need be valid. The implementation may update other
//      fields and flags.
//
//      The 'buf' argument is a pointer to a stat structure, as defined in <sys/stat.h>, into
//      which information is placed concerning the file.
//
//      The structure members st_mode, st_ino, st_dev, st_uid, st_gid, st_atime, st_ctime,
//      and st_mtime shall have meaningful values for all other file types defined in this
//      volume of IEEE Std 1003.1-2001. The value of the member st_nlink shall be set to
//      the number of links to the file.
//
//      An implementation that provides additional or alternative file access control
//      mechanisms may, under implementation-defined conditions, cause fstat() to fail.
//
//      The fstat() function shall update any time-related fields as described in the Base
//      Definitions volume of IEEE Std 1003.1-2001, Section 4.7, File Times Update, before
//      writing into the stat structure.
//
//  RETURN VALUE
//      Upon successful completion, 0 shall be returned. Otherwise, -1 shall be returned
//      and errno set to indicate the error.
//
//  ERRORS
//      The fstat() function shall fail if:
//
//      [EBADF]
//          The fildes argument is not a valid file descriptor.
//
//      [EIO]
//          An I/O error occurred while reading from the file system.
//
//      [EOVERFLOW]
//          The file size in bytes or the number of blocks allocated to the file or the
//          file serial number cannot be represented correctly in the structure pointed to
//          by buf.
//
//      The fstat() function may fail if:
//
//      [EOVERFLOW]
//          One of the values is too large to store into the structure pointed to by
//          the buf argument.
*/

LIBW32_API int
w32_fstat(int fildes, struct stat *sb)
{
    struct StatHandle sbh = {SHSTAT, sb};
    return W32StatHandle(fildes, &sbh);
}


LIBW32_API int
w32_fstat64(int fildes, struct stat64 *sb)
{
    struct StatHandle sbh = {SHSTAT64, sb};
    return W32StatHandle(fildes, &sbh);
}


LIBW32_API int
w32_fstatA(int fildes, struct stat *sb)
{
    struct StatHandle sbh = {SHSTAT, sb};
    return W32StatHandle(fildes, &sbh);
}


LIBW32_API int
w32_fstat64A(int fildes, struct stat64 *sb)
{
    struct StatHandle sbh = {SHSTAT64, sb};
    return W32StatHandle(fildes, &sbh);
}


LIBW32_API int
w32_fstatW(int fildes, struct stat *sb)
{
    struct StatHandle sbh = {SHSTAT, sb};
    return W32StatHandle(fildes, &sbh);
}


LIBW32_API int
w32_fstat64W(int fildes, struct stat64 *sb)
{
    struct StatHandle sbh = {SHSTAT64, sb};
    return W32StatHandle(fildes, &sbh);
}


static int
W32StatHandle(int fildes, struct StatHandle *sb)
{
    HANDLE handle;
    int ret = 0;

    if (NULL == sb) {
        ret = -EFAULT;

    } else {
        StatZero(sb);

        if (fildes < 0) {
            ret = -EBADF;

        } else if ((handle = w32_osfhandle(fildes)) == INVALID_HANDLE_VALUE) {
                                                // socket, a named pipe, or an anonymous pipe.
            handle = w32_ftoh(fildes);
            if (FILE_TYPE_PIPE == GetFileType(handle)) {
                W32StatPipe(handle, FILE_TYPE_PIPE, sb);
            } else {
                ret = -EBADF;
            }

        } else {
            const DWORD ftype = GetFileType(handle);

            switch (ftype) {
            case FILE_TYPE_DISK: {              // disk file.
                    wchar_t fullname[WIN32_PATH_MAX];
                    size_t namelen;

                    fullname[0] = 0;
                    namelen = my_GetFinalPathNameByHandleW(handle, fullname, _countof(fullname));
                    if (! W32StatCommon(handle, NULL, sb, fullname, namelen)) {
                        ret = -EIO;
                    }
                }
                break;

            case FILE_TYPE_CHAR:                // character file, typically an LPT device or a console.
            case FILE_TYPE_PIPE:                // socket, a named pipe, or an anonymous pipe.
                W32StatPipe(handle, ftype, sb);
                break;

            case FILE_TYPE_REMOTE:
            case FILE_TYPE_UNKNOWN:             // others
            default:
                ret = -EBADF;
                break;
            }
        }
    }

    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}


static void
W32StatPipe(HANDLE handle, DWORD ftype, struct StatHandle *sb)
{
    mode_t mode = 0;
    off_t size = 0;

    // Attributes
    mode |= S_IRUSR | S_IRGRP | S_IROTH;
    if (FILE_TYPE_PIPE == ftype) {
        if (GetNamedPipeInfo(handle, NULL, NULL, NULL, NULL)) {
            DWORD bytesAvail = 0;

            mode |= S_IFIFO;
            if (PeekNamedPipe(handle, NULL, 0, NULL, &bytesAvail, NULL)) {
                size = (off_t)bytesAvail;
            }
        } else {
            mode |= S_IFSOCK;
        }
    } else {
        mode |= S_IFCHR;
    }

    // Assign results
    switch (sb->type) {
    case SHSTAT: {
            struct stat *esb = ((struct stat *) sb->buf);
            esb->st_mode = mode;
            esb->st_size = size;
            esb->st_dev  = 1;
            esb->st_rdev = 1;
        }
        break;
    case SHSTAT64: {
            struct stat64 *esb = ((struct stat64 *) sb->buf);
            esb->st_mode = mode;
            esb->st_size = size;
            esb->st_dev  = 1;
            esb->st_rdev = 1;
        }
        break;
    default:
        assert(0);
        break;
    }
}


//
//  my_GetFinalPathNameByHandleW ---
//      GetFinalPathNameByHandleW dynamic binding.
//

static DWORD
my_GetFinalPathNameByHandleW(HANDLE handle, LPWSTR path, int length)
{
    static GetFinalPathNameByHandleW_t x_GetFinalPathNameByHandleW = NULL;

    if (NULL == x_GetFinalPathNameByHandleW) {
        HINSTANCE hinst;                        // Vista+

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
        if (0 == (hinst = LoadLibraryA("Kernel32")) ||
                0 == (x_GetFinalPathNameByHandleW =
                            (GetFinalPathNameByHandleW_t)GetProcAddress(hinst, "GetFinalPathNameByHandleW"))) {
                                                // XP+
            x_GetFinalPathNameByHandleW = my_GetFinalPathNameByHandleWImp;
            if (hinst) FreeLibrary(hinst);
        }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif
    }

#ifndef FILE_NAME_NORMALIZED
#define FILE_NAME_NORMALIZED 0
#define VOLUME_NAME_DOS 0
#endif

    return x_GetFinalPathNameByHandleW(handle, path, length, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
}


static DWORD WINAPI
my_GetFinalPathNameByHandleWImp(HANDLE handle, LPWSTR path, DWORD length, DWORD flags)
{                                               // Determine underlying file-name, XP+
    HANDLE map;
    DWORD ret;

    __CUNUSED(flags)

    if (0 == GetFileSize(handle, &ret) && 0 == ret) {
        return 0;                               // Cannot map a file with a length of zero
    }

    ret = 0;

    if (0 != (map = CreateFileMappingW(handle, NULL, PAGE_READONLY, 0, 1, NULL))) {
        LPVOID pmem = MapViewOfFile(map, FILE_MAP_READ, 0, 0, 1);

        if (pmem) {                             // XP+
            if (GetMappedFileNameW(GetCurrentProcess(), pmem, path, length)) {
                //
                //  Translate path with device name to drive letters, for example:
                //
                //      \Device\Volume4\<path>
                //      => F:\<path>
                //
                wchar_t t_drives[512] = {0};    // 27*4 ...

                if (GetLogicalDriveStringsW(_countof(t_drives) - 1, t_drives)) {

                    BOOL found = FALSE;
                    const wchar_t *p = t_drives;
                    wchar_t t_name[WIN32_PATH_MAX];
                    wchar_t t_drive[3] = L" :";

                    do {                        // Look up each device name
                        t_drive[0] = *p;

                        if (QueryDosDeviceW(t_drive, t_name, _countof(t_name) - 1)) {
                            const size_t namelen = wcslen(t_name);

                            if (namelen < WIN32_PATH_MAX) {
                                found = (0 == _wcsnicmp(path, t_name, namelen) && path[namelen] == '\\');

                                if (found) {    // Reconstruct path, replacing device with drive
                                    wmemmove(path + 3, path + namelen, wcslen(path + namelen) + 1);
                                    path[0] = *p;
                                    path[1] = ':';
                                    path[2] = '\\';
                                    ret = 1;
                                    break;
                                }
                            }
                        }

                        while (*p++);           // Go to the next NULL character.

                    } while (!found && *p);     // end of string
                }
            }
            (void) UnmapViewOfFile(pmem);
        }
        (void) CloseHandle(map);
    }
    return ret;
}


//
//  my_GetFinalPathNameByHandleA ---
//      GetFinalPathNameByHandleA dynamic binding.
//

static DWORD
my_GetFinalPathNameByHandleA(HANDLE handle, char *path, int length)
{
    static GetFinalPathNameByHandleA_t x_GetFinalPathNameByHandleA = NULL;

    if (NULL == x_GetFinalPathNameByHandleA) {
        HINSTANCE hinst;                        // Vista+

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
        if (0 == (hinst = LoadLibraryA("Kernel32")) ||
                0 == (x_GetFinalPathNameByHandleA =
                          (GetFinalPathNameByHandleA_t)GetProcAddress(hinst, "GetFinalPathNameByHandleA"))) {
                                                // XP+
            x_GetFinalPathNameByHandleA = my_GetFinalPathNameByHandleAImp;
            if (hinst) FreeLibrary(hinst);
        }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif
    }

#ifndef FILE_NAME_NORMALIZED
#define FILE_NAME_NORMALIZED 0
#define VOLUME_NAME_DOS 0
#endif

    return x_GetFinalPathNameByHandleA(handle, path, length, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
}


static DWORD WINAPI
my_GetFinalPathNameByHandleAImp(HANDLE handle, LPSTR path, DWORD length, DWORD flags)
{                                               // Determine underlying file-name, XP+
    HANDLE map;
    DWORD ret;

    __CUNUSED(flags)

    if (0 == GetFileSize(handle, &ret) && 0 == ret) {
        return 0;                               // Cannot map a file with a length of zero
    }

    ret = 0;

    if (0 != (map = CreateFileMappingA(handle, NULL, PAGE_READONLY, 0, 1, NULL))) {
        LPVOID pmem = MapViewOfFile(map, FILE_MAP_READ, 0, 0, 1);

        if (pmem) {                             // XP+
            if (GetMappedFileNameA(GetCurrentProcess(), pmem, path, length)) {
                //
                //  Translate path with device name to drive letters, for example:
                //
                //      \Device\Volume4\<path>
                //      => F:\<path>
                //
                char t_drives[512] = {0};       // 27*4 ...

                if (GetLogicalDriveStringsA(sizeof(t_drives) - 1, t_drives)) {

                    BOOL found = FALSE;
                    const char *p = t_drives;
                    char t_name[WIN32_PATH_MAX];
                    char t_drive[3] = " :";

                    do {                        // Look up each device name
                        t_drive[0] = *p;

                        if (QueryDosDeviceA(t_drive, t_name, sizeof(t_name) - 1)) {
                            const size_t namelen = strlen(t_name);

                            if (namelen < WIN32_PATH_MAX) {
                                found = (0 == _strnicmp(path, t_name, namelen) && path[namelen] == '\\');

                                if (found) {    // Reconstruct path, replacing device with drive
                                    memmove(path + 2, path + namelen, strlen(path + namelen) + 1);
                                    path[0] = *p;
                                    path[1] = ':';
                                    path[2] = '\\';
                                    ret = 1;
                                    break;
                                }
                            }
                        }

                        while (*p++);           // Go to the next NULL character.

                    } while (!found && *p);     // end of string
                }
            }
            (void) UnmapViewOfFile(pmem);
        }
        (void) CloseHandle(map);
    }
    return ret;
}


//
//  my_GetFileInformationByHandleEx ---
//      GetFileInformationByHandle dynamic binding.
//

static DWORD
my_GetFileInformationByHandleEx(HANDLE handle, FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInformation, DWORD dwBufferSize)
{
    static GetFileInformationByHandleEx_t x_GetFileInformationByHandleEx = NULL;

    if (NULL == x_GetFileInformationByHandleEx) {
        HINSTANCE hinst;

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
        if (0 == (hinst = LoadLibraryA("Kernel32")) ||
                0 == (x_GetFileInformationByHandleEx =
                            (GetFileInformationByHandleEx_t)GetProcAddress(hinst, "GetFileInformationByHandleEx"))) {
            // Kernel32.lib; FileExtd.lib on Windows Server 2003 and Windows XP
            x_GetFileInformationByHandleEx = my_GetFileInformationByHandleExImp;
            if (hinst) FreeLibrary(hinst);
        }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif
    }

    return x_GetFileInformationByHandleEx(handle, FileInformationClass, lpFileInformation, dwBufferSize);
}


static DWORD WINAPI
my_GetFileInformationByHandleExImp(HANDLE handle, FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInformation, DWORD dwBufferSize)
{
    __CUNUSED(handle)
    __CUNUSED(FileInformationClass)
    __CUNUSED(lpFileInformation)
    __CUNUSED(dwBufferSize)

    SetLastError(ERROR_NOT_SUPPORTED);          // not implemented
    return 0;
}


//
//  my_GetVolumeInformationByHandle ---
//      my_GetVolumeInformationByHandle dynamic binding.
//
//      Retrieves information about the file system and volume associated with the specified file.
//      see: https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getvolumeinformationbyhandlew
//
//  Arguments:
//      serialno
//          Returns the volume serial number that the operating system assigns when a hard disk is formatted.
//
//      flags
//          Returns the flags associated with the specified file system.
//
//  Return Value:
//      If all the requested information is retrieved, the return value is nonzero TRUEl otherwise FALSE.
//
static BOOL
my_GetVolumeInformationByHandle(HANDLE handle, DWORD* serialno, DWORD* flags)
{
    static GetVolumeInformationByHandleW_t x_GetVolumeInformationByHandleW = NULL;

    if (NULL == x_GetVolumeInformationByHandleW) {
        HINSTANCE hinst;                        // Vista+

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
        if (0 == (hinst = LoadLibraryA("Kernel32")) ||
            0 == (x_GetVolumeInformationByHandleW =
                (GetVolumeInformationByHandleW_t)GetProcAddress(hinst, "GetVolumeInformationByHandleW"))) {
            // XP+
            x_GetVolumeInformationByHandleW = my_GetVolumeInformationByHandleImp;
            if (hinst) FreeLibrary(hinst);
        }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif
    }

    return x_GetVolumeInformationByHandleW(handle, NULL, 0, serialno, NULL, flags, NULL, 0);
}


static BOOL WINAPI
my_GetVolumeInformationByHandleImp(HANDLE hFile,
        LPWSTR lpVolumeNameBuffer, DWORD nVolumeNameSize,
        LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, LPWSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize)
{
    __CUNUSED(hFile)
        __CUNUSED(lpVolumeNameBuffer)
        __CUNUSED(nVolumeNameSize)
        __CUNUSED(lpVolumeSerialNumber)
        __CUNUSED(lpMaximumComponentLength)
        __CUNUSED(lpFileSystemFlags)
        __CUNUSED(lpFileSystemNameBuffer)
        __CUNUSED(nFileSystemNameSize)

        SetLastError(ERROR_NOT_SUPPORTED);          // not implemented
    return FALSE;
}

/*
//  NAME
//      readlink - read the contents of a symbolic link
//
//  SYNOPSIS
//
//      #include <unistd.h>
//
//      ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize);
//
//  DESCRIPTION
//      The readlink() function shall place the contents of the symbolic link referred to
//      by path in the buffer buf which has size bufsize. If the number of bytes in the
//      symbolic link is less than bufsize, the contents of the remainder of buf are
//      unspecified. If the buf argument is not large enough to contain the link content,
//      the first bufsize bytes shall be placed in buf.
//
//      If the value of bufsize is greater than {SSIZE_MAX}, the result is
//      implementation-defined.
//
//  RETURN VALUE
//      Upon successful completion, readlink() shall return the count of bytes placed in
//      the buffer. Otherwise, it shall return a value of -1, leave the buffer unchanged,
//      and set errno to indicate the error.
//
//  ERRORS
//
//      The readlink() function shall fail if:
//
//      [EACCES]
//          Search permission is denied for a component of the path prefix of path.
//
//      [EINVAL]
//          The path argument names a file that is not a symbolic link.
//
//      [EIO]
//          An I/O error occurred while reading from the file system.
//
//      [ELOOP]
//          A loop exists in symbolic links encountered during resolution of the path
//          argument.
//
//      [ENAMETOOLONG]
//          The length of the path argument exceeds {PATH_MAX} or a pathname component is
//          longer than {NAME_MAX}.
//
//      [ENOENT]
//          A component of path does not name an existing file or path is an empty string.
//
//      [ENOTDIR]
//          A component of the path prefix is not a directory.
//
//      The readlink() function may fail if:
//
//      [EACCES]
//          Read permission is denied for the directory.
//
//      [ELOOP]
//          More than {SYMLOOP_MAX} symbolic links were encountered during resolution of
//          the path argument.
//
//      [ENAMETOOLONG]
//          As a result of encountering a symbolic link in resolution of the path argument,
//          the length of the substituted pathname string exceeded {PATH_MAX}.
//
//  NOTES
//      Portable applications should not assume that the returned contents of the symbolic
//      link are null-terminated.
*/

LIBW32_API int
w32_readlink(const char *path, char *buf, size_t maxlen)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (path && buf) {
            wchar_t wpath[WIN32_PATH_MAX];

            if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
                if (w32_readlinkW(wpath, wpath, _countof(wpath)) > 0) {
                    return w32_wc2utf(wpath, buf, maxlen);
                }
            }
            return -1;
        }
    }
#endif  //UTF8FILENAMES

    return w32_readlinkA(path, buf, maxlen);
}


LIBW32_API int
w32_readlinkW(const wchar_t *path, wchar_t *buf, size_t maxlen)
{
    int ret = 0;

    if (path == NULL || buf == NULL) {
        ret = -EFAULT;
    } else if (0 == (ret = ReadlinkW(path, (void *)-1, buf, maxlen))) {
        ret = -EINVAL;                          /* not a symlink */
    }
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return ret;
}


LIBW32_API int
w32_readlinkA(const char *path, char *buf, size_t maxlen)
{
    int ret = 0;

    if (path == NULL || buf == NULL) {
        ret = -EFAULT;
    } else if (0 == (ret = ReadlinkA(path, (void *)-1, buf, maxlen))) {
        ret = -EINVAL;                          /* not a symlink */
    }
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return ret;
}


/*
//  NAME
//      symlink - make a symbolic link to a file
//
//  SYNOPSIS
//
//      #include <unistd.h>
//
//      int symlink(const char *path1, const char *path2);
//
//  DESCRIPTION
//      The symlink() function shall create a symbolic link called path2 that contains the
//      string pointed to by path1 ( path2 is the name of the symbolic link created, path1
//      is the string contained in the symbolic link).
//
//      The string pointed to by path1 shall be treated only as a character string and
//      shall not be validated as a pathname.
//
//      If the symlink() function fails for any reason other than [EIO], any file named by
//      path2 shall be unaffected.
//
//  RETURN VALUE
//      Upon successful completion, symlink() shall return 0; otherwise, it shall return -1
//      and set errno to indicate the error.
//
//  ERRORS
//      The symlink() function shall fail if:
//
//      [EACCES]
//          Write permission is denied in the directory where the symbolic link is being
//          created, or search permission is denied for a component of the path prefix of
//          path2.
//
//      [EEXIST]
//          The path2 argument names an existing file or symbolic link.
//
//      [EIO]
//          An I/O error occurs while reading from or writing to the file system.
//
//      [ELOOP]
//          A loop exists in symbolic links encountered during resolution of the path2
//          argument.
//
//      [ENAMETOOLONG]
//          The length of the path2 argument exceeds {PATH_MAX} or a pathname component is
//          longer than {NAME_MAX} or the length of the path1 argument is longer than
//          {SYMLINK_MAX}.
//
//      [ENOENT]
//          A component of path2 does not name an existing file or path2 is an empty string.
//
//      [ENOSPC]
//          The directory in which the entry for the new symbolic link is being placed
//          cannot be extended because no space is left on the file system containing the
//          directory, or the new symbolic link cannot be created because no space is left
//          on the file system which shall contain the link, or the file system is out of
//          file-allocation resources.
//
//      [ENOTDIR]
//          A component of the path prefix of path2 is not a directory.
//
//      [EROFS]
//          The new symbolic link would reside on a read-only file system.
//
//      The symlink() function may fail if:
//
//      [ELOOP]
//          More than {SYMLOOP_MAX} symbolic links were encountered during resolution of
//          the path2 argument.
//
//      [ENAMETOOLONG]
//          As a result of encountering a symbolic link in resolution of the path2 argument,
//          the length of the substituted pathname string exceeded {PATH_MAX} bytes
//          (including the terminating null byte), or the length of the string pointed to
//          by path1 exceeded {SYMLINK_MAX}.
*/

LIBW32_API int
w32_symlink(const char *name1, const char *name2)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (name1 && name2) {
            wchar_t wname1[WIN32_PATH_MAX], wname2[WIN32_PATH_MAX];

            if (w32_utf2wc(name1, wname1, _countof(wname1)) > 0 &&
                w32_utf2wc(name2, wname2, _countof(wname2)) > 0) {
                return w32_symlinkW(wname1, wname2);
            }
            return -1;
        }
    }
#endif  //UTF8FILENAMES

    return w32_symlinkA(name1, name2);
}


LIBW32_API int
w32_symlinkW(const wchar_t* name1, const wchar_t* name2)
{
    int ret = -1;

    if (name1 == NULL || name2 == NULL) {
        errno = EFAULT;

    } else if (!*name1 || !*name2) {
        errno = ENOENT;

    } else if (wcslen(name1) > WIN32_PATH_MAX || wcslen(name2) > WIN32_PATH_MAX) {
        errno = ENAMETOOLONG;

    } else if (GetFileAttributesW(name2) != INVALID_FILE_ATTRIBUTES /*0xffffffff*/) {
        errno = EEXIST;

    } else {
        ret = 0;
        if (IsShortcutW(name2)) {               // possible shortcut (xxx.lnk)
            if (!CreateShortcutW(name2, name1, NULL, name1)) {
                errno = EIO;
                ret = -1;
            }

        } else {                                // otherwise symlink (vista+).
#ifndef SYMBOLIC_LINK_FLAG_DIRECTORY
#define SYMBOLIC_LINK_FLAG_DIRECTORY 0x01
#endif
#ifndef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
#define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE 0x2
    //https://docs.microsoft.com/en-us/windows/uwp/get-started/enable-your-device-for-development
#endif

            DWORD attrs1, flag =                // Note: Generally only available under an Admin account
                (((attrs1 = GetFileAttributesW(name1)) != INVALID_FILE_ATTRIBUTES &&
                    (attrs1 & FILE_ATTRIBUTE_DIRECTORY)) ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);
            if (! my_CreateSymbolicLinkW(/*target-link*/name2, /*existing*/name1, flag)) {
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
    }

    return ret;
}


LIBW32_API int
w32_symlinkA(const char *name1, const char *name2)
{
    int ret = -1;

    if (name1 == NULL || name2 == NULL) {
        errno = EFAULT;

    } else if (!*name1 || !*name2) {
        errno = ENOENT;

    } else if (strlen(name1) > WIN32_PATH_MAX || strlen(name2) > WIN32_PATH_MAX) {
        errno = ENAMETOOLONG;

    } else if (GetFileAttributesA(name2) != INVALID_FILE_ATTRIBUTES /*0xffffffff*/) {
        errno = EEXIST;

    } else {
        ret = 0;
        if (IsShortcutA(name2)) {               // possible shortcut (xxx.lnk)
            if (! CreateShortcutA(name2, name1, NULL, name1)) {
                errno = EIO;
                ret = -1;
            }

        } else {                                // otherwise symlink (vista+).
#ifndef SYMBOLIC_LINK_FLAG_DIRECTORY
#define SYMBOLIC_LINK_FLAG_DIRECTORY 0x01
#endif
#ifndef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
#define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE 0x2
    //https://docs.microsoft.com/en-us/windows/uwp/get-started/enable-your-device-for-development
#endif

            DWORD attrs1, flag =                // Note: Generally only available under an Admin account
                    (((attrs1 = GetFileAttributesA(name1)) != INVALID_FILE_ATTRIBUTES &&
                            (attrs1 & FILE_ATTRIBUTE_DIRECTORY)) ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);
            if (! my_CreateSymbolicLinkA(/*target-link*/name2, /*existing*/name1, flag)) {
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
    }

    return ret;
}


static BOOL
my_CreateSymbolicLinkW(LPCWSTR lpSymlinkFileName, LPCWSTR lpTargetFileName, DWORD dwFlags)
{
    static CreateSymbolicLinkW_t x_CreateSymbolicLinkW = NULL;

    if (NULL == x_CreateSymbolicLinkW) {
        HINSTANCE hinst;                        // Vista+

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
        if (0 == (hinst = LoadLibraryA("Kernel32")) ||
                0 == (x_CreateSymbolicLinkW =
                        (CreateSymbolicLinkW_t)GetProcAddress(hinst, "CreateSymbolicLinkW"))) {
                                                // XP+
            x_CreateSymbolicLinkW = my_CreateSymbolicLinkWImp;
            if (hinst) FreeLibrary(hinst);
        }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif
    }
    return x_CreateSymbolicLinkW(lpSymlinkFileName, lpTargetFileName, dwFlags);
}


static BOOL WINAPI
my_CreateSymbolicLinkWImp(LPCWSTR lpSymlinkFileName, LPCWSTR lpTargetFileName, DWORD dwFlags)
{
    __CUNUSED(lpSymlinkFileName)
    __CUNUSED(lpTargetFileName)
    __CUNUSED(dwFlags)

    SetLastError(ERROR_NOT_SUPPORTED);          // not implemented
    return FALSE;
}


static BOOL
my_CreateSymbolicLinkA(const char *lpSymlinkFileName, const char *lpTargetFileName, DWORD dwFlags)
{
    static CreateSymbolicLinkA_t x_CreateSymbolicLinkA = NULL;

    if (NULL == x_CreateSymbolicLinkA) {
        HINSTANCE hinst;                        // Vista+

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
        if (0 == (hinst = LoadLibraryA("Kernel32")) ||
                0 == (x_CreateSymbolicLinkA =
                        (CreateSymbolicLinkA_t)GetProcAddress(hinst, "CreateSymbolicLinkA"))) {
                                                // XP+
            x_CreateSymbolicLinkA = my_CreateSymbolicLinkAImp;
            if (hinst) FreeLibrary(hinst);
        }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif
    }
    return x_CreateSymbolicLinkA(lpSymlinkFileName, lpTargetFileName, dwFlags);
}


static BOOL WINAPI
my_CreateSymbolicLinkAImp(const char *lpSymlinkFileName, const char *lpTargetFileName, DWORD dwFlags)
{
    __CUNUSED(lpSymlinkFileName)
    __CUNUSED(lpTargetFileName)
    __CUNUSED(dwFlags)

    SetLastError(ERROR_NOT_SUPPORTED);          // not implemented
    return FALSE;
}


static BOOL
IsShortcutA(const char *name)
{
    const size_t len = strlen(name);
    const char *cursor;

    for (cursor = name + len; --cursor >= name;) {
        if (*cursor == '.') {                   // extension
            return (*++cursor && 0 == w32_iostricmp(cursor, "lnk"));
        }
        if (*cursor == '/' || *cursor == '\\') {
            break;                              // delimiter
        }
    }
    return FALSE;
}


static BOOL
IsShortcutW(const wchar_t *name)
{
    const size_t len = wcslen(name);
    const wchar_t *cursor;

    for (cursor = name + len; --cursor >= name;) {
        if (*cursor == '.') {                   // extension
            return (*++cursor && 0 == w32_iowstricmp(cursor, "lnk"));
        }
        if (*cursor == '/' || *cursor == '\\') {
            break;                              // delimiter
        }
    }
    return FALSE;
}


/*
//  NAME
//      open - open a file
//
//  SYNOPSIS
//
//      #include <sys/stat.h>
//      #include <fcntl.h>
//
//      int open(const char *path, int oflag, ... );
//
//  DESCRIPTION
//      The open() function shall establish the connection between a file and a file
//      descriptor. It shall create an open file description that refers to a file and a
//      file descriptor that refers to that open file description. The file descriptor is
//      used by other I/O functions to refer to that file. The path argument points to a
//      pathname naming the file.
//
//      The open() function shall return a file descriptor for the named file that is the
//      lowest file descriptor not currently open for that process. The open file
//      description is new, and therefore the file descriptor shall not share it with any
//      other process in the system. The FD_CLOEXEC file descriptor flag associated with
//      the new file descriptor shall be cleared.
//
//      The file offset used to mark the current position within the file shall be set to
//      the beginning of the file.
//
//      The file status flags and file access modes of the open file description shall be
//      set according to the value of 'oflag'.
//
//      Values for 'oflag' are constructed by a bitwise-inclusive OR of flags from the
//      following list, defined in <fcntl.h>. Applications shall specify exactly one of the
//      first three values (file access modes) below in the value of 'oflag':
//
//          O_RDONLY
//              Open for reading only.
//
//          O_WRONLY
//              Open for writing only.
//
//          O_RDWR
//              Open for reading and writing. The result is undefined if this flag is
//              applied to a FIFO.
//
//      Any combination of the following may be used:
//
//          O_APPEND
//              If set, the file offset shall be set to the end of the file prior to each
//              write.
//
//          O_CREAT
//              If the file exists, this flag has no effect except as noted under O_EXCL
//              below. Otherwise, the file shall be created; the user ID of the file shall
//              be set to the effective user ID of the process; the group ID of the file
//              shall be set to the group ID of the file's parent directory or to the
//              effective group ID of the process; and the access permission bits (see
//              <sys/stat.h>) of the file mode shall be set to the value of the third
//              argument taken as type mode_t modified as follows: a bitwise AND is
//              performed on the file-mode bits and the corresponding bits in the
//              complement of the process' file mode creation mask. Thus, all bits in the
//              file mode whose corresponding bit in the file mode creation mask is set are
//              cleared. When bits other than the file permission bits are set, the effect
//              is unspecified. The third argument does not affect whether the file is open
//              for reading, writing, or for both. Implementations shall provide a way to
//              initialize the file's group ID to the group ID of the parent directory.
//              Implementations may, but need not, provide an implementation-defined way to
//              initialize the file's group ID to the effective group ID of the calling
//              process.
//
//          O_DSYNC
//              Write I/O operations on the file descriptor shall complete as defined by
//              synchronized I/O data integrity completion. [Option End]
//
//          O_EXCL
//              If O_CREAT and O_EXCL are set, open() shall fail if the file exists. The
//              check for the existence of the file and the creation of the file if it does
//              not exist shall be atomic with respect to other threads executing open()
//              naming the same filename in the same directory with O_EXCL and O_CREAT set.
//              If O_EXCL and O_CREAT are set, and path names a symbolic link, open() shall
//              fail and set errno to [EEXIST], regardless of the contents of the symbolic
//              link. If O_EXCL is set and O_CREAT is not set, the result is undefined.
//
//          O_NOCTTY
//              If set and path identifies a terminal device, open() shall not cause the
//              terminal device to become the controlling terminal for the process.
//
//          O_NONBLOCK
//              When opening a FIFO with O_RDONLY or O_WRONLY set:
//
//                  If O_NONBLOCK is set, an open() for reading-only shall return without
//                  delay. An open() for writing-only shall return an error if no process
//                  currently has the file open for reading.
//
//                  If O_NONBLOCK is clear, an open() for reading-only shall block the
//                  calling thread until a thread opens the file for writing. An open() for
//                  writing-only shall block the calling thread until a thread opens the
//                  file for reading.
//
//              When opening a block special or character special file that supports
//              non-blocking opens:
//
//                  If O_NONBLOCK is set, the open() function shall return without blocking
//                  for the device to be ready or available. Subsequent behavior of the
//                  device is device-specific.
//
//                  If O_NONBLOCK is clear, the open() function shall block the calling
//                  thread until the device is ready or available before returning.
//
//              Otherwise, the behavior of O_NONBLOCK is unspecified.
//
//          O_RSYNC
//              Read I/O operations on the file descriptor shall complete at the same level
//              of integrity as specified by the O_DSYNC and O_SYNC flags. If both O_DSYNC
//              and O_RSYNC are set in oflag, all I/O operations on the file descriptor
//              shall complete as defined by synchronized I/O data integrity completion. If
//              both O_SYNC and O_RSYNC are set in flags, all I/O operations on the file
//              descriptor shall complete as defined by synchronized I/O file integrity
//              completion. [Option End]
//
//          O_SYNC
//              Write I/O operations on the file descriptor shall complete as defined by
//              synchronized I/O file integrity completion. [Option End]
//
//          O_TRUNC
//              If the file exists and is a regular file, and the file is successfully
//              opened O_RDWR or O_WRONLY, its length shall be truncated to 0, and the
//              mode and owner shall be unchanged. It shall have no effect on FIFO
//              special files or terminal device files. Its effect on other file types
//              is implementation-defined. The result of using O_TRUNC with O_RDONLY is
//              undefined.
//
//      If O_CREAT is set and the file did not previously exist, upon successful completion,
//      open() shall mark for update the st_atime, st_ctime, and st_mtime fields of the
//      file and the st_ctime and st_mtime fields of the parent directory.
//
//      If O_TRUNC is set and the file did previously exist, upon successful completion,
//      open() shall mark for update the st_ctime and st_mtime fields of the file.
//
//      [Optional] If both the O_SYNC and O_DSYNC flags are set, the effect is as if only the O_SYNC
//      flag was set.
//
//      [Optional] If path refers to a STREAMS file, 'oflag' may be constructed from O_NONBLOCK OR'ed
//      with either O_RDONLY, O_WRONLY, or O_RDWR. Other flag values are not applicable to
//      STREAMS devices and shall have no effect on them. The value O_NONBLOCK affects the
//      operation of STREAMS drivers and certain functions applied to file descriptors
//      associated with STREAMS files. For STREAMS drivers, the implementation of
//      O_NONBLOCK is device-specific.
//
//      [Optional If path names the master side of a pseudo-terminal device, then it is unspecified
//      whether open() locks the slave side so that it cannot be opened. Conforming
//      applications shall call unlockpt() before opening the slave side.
//
//      The largest value that can be represented correctly in an object of type off_t
//      shall be established as the offset maximum in the open file description.
//
//  RETURN VALUE
//
//      Upon successful completion, the function shall open the file and return a
//      non-negative integer representing the lowest numbered unused file descriptor.
//      Otherwise, -1 shall be returned and errno set to indicate the error. No files shall
//      be created or modified if the function returns -1.
//
//  ERRORS
//
//    The open() function shall fail if:
//
//    [EACCES]
//        Search permission is denied on a component of the path prefix, or the file
//        exists and the permissions specified by oflag are denied, or the file does not
//        exist and write permission is denied for the parent directory of the file to be
//        created, or O_TRUNC is specified and write permission is denied.
//
//    [EEXIST]
//        O_CREAT and O_EXCL are set, and the named file exists.
//
//    [EINTR]
//        A signal was caught during open().
//
//    [EINVAL]
//        The implementation does not support synchronized I/O for this file. [Option End]
//
//    [EIO]
//        The path argument names a STREAMS file and a hangup or error occurred during
//        the open(). [Option End]
//
//    [EISDIR]
//        The named file is a directory and oflag includes O_WRONLY or O_RDWR.
//
//    [ELOOP]
//        A loop exists in symbolic links encountered during resolution of the path argument.
//
//    [EMFILE]
//        {OPEN_MAX} file descriptors are currently open in the calling process.
//
//    [ENAMETOOLONG]
//        The length of the path argument exceeds {PATH_MAX} or a pathname component is
//        longer than {NAME_MAX}.
//
//    [ENFILE]
//        The maximum allowable number of files is currently open in the system.
//
//    [ENOENT]
//        O_CREAT is not set and the named file does not exist; or O_CREAT is set and
//        either the path prefix does not exist or the path argument points to an empty
//        string.
//
//    [ENOSR]
//        [XSR] [Option Start] The path argument names a STREAMS-based file and the
//        system is unable to allocate a STREAM. [Option End]
//
//    [ENOSPC]
//        The directory or file system that would contain the new file cannot be expanded,
//        the file does not exist, and O_CREAT is specified.
//
//    [ENOTDIR]
//        A component of the path prefix is not a directory.
//
//    [ENXIO]
//        O_NONBLOCK is set, the named file is a FIFO, O_WRONLY is set, and no process
//        has the file open for reading.
//
//    [ENXIO]
//        The named file is a character special or block special file, and the device
//        associated with this special file does not exist.
//
//    [EOVERFLOW]
//        The named file is a regular file and the size of the file cannot be represented
//        correctly in an object of type off_t.
//
//    [EROFS]
//        The named file resides on a read-only file system and either O_WRONLY, O_RDWR,
//        O_CREAT (if the file does not exist), or O_TRUNC is set in the oflag argument.
//
//    The open() function may fail if:
//
//    [EAGAIN]
//        The path argument names the slave side of a pseudo-terminal device that is
//        locked. [Option End]
//
//    [EINVAL]
//        The value of the oflag argument is not valid.
//
//    [ELOOP]
//        More than {SYMLOOP_MAX} symbolic links were encountered during resolution of
//        the path argument.
//
//    [ENAMETOOLONG]
//        As a result of encountering a symbolic link in resolution of the path argument,
//        the length of the substituted pathname string exceeded {PATH_MAX}.
//
//    [ENOMEM]
//        The path argument names a STREAMS file and the system is unable to allocate
//        resources. [Option End]
//
//    [ETXTBSY]
//        The file is a pure procedure (shared text) file that is being executed and
//        oflag is O_WRONLY or O_RDWR.
//
*/

LIBW32_API int
w32_open(const char *path, int oflag, ...)
{
    int mode = 0;

    if (O_CREAT & oflag) {
        va_list ap;
        va_start(ap, oflag);
        mode = va_arg(ap, int);
        va_end(ap);
    }

#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (path) {
            wchar_t wpath[WIN32_PATH_MAX];

            if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
                return w32_openW(wpath, oflag, mode);
            }
            return -1;
        }
    }
#endif  //UTF8FILENAMES

    return w32_openA(path, oflag, mode);
}


LIBW32_API int
w32_openW(const wchar_t *path, int oflag, int mode)
{
    wchar_t symbuf[WIN32_PATH_MAX];
    const wchar_t *expath;
    int fd, ret = 0;

    if (NULL == path) {
        errno = EFAULT;
        return -1;
    }

    if (!*path) {
        errno = ENOENT;
        return -1;
    }

    // specials
    if (0 == w32_iowstricmp(path, "/dev/null")) {
        path = L"NUL";                          // redirect

    } else if ((ret = ReadlinkW(path, (void *)-1, symbuf, _countof(symbuf))) < 0) {
        /*
         *  If O_CREAT create the file if it does not exist, in which case the
         *  file is created with mode mode as described in chmod(2) and modified
         *  by the process' umask value (see umask(2)).
         */
        if ((oflag & O_CREAT) && (ret == -ENOTDIR || ret == -ENOENT)) {
            ret = 0;
        }

    } else if (ret > 0) {
        /*
         *  If O_NOFOLLOW and pathname is a symbolic link, then the open fails with ELOOP.
         */
#if defined(O_NOFOLLOW) && (O_NOFOLLOW)         // extension
        if (oflag & O_NOFOLLOW) {
            ret = -ELOOP;
        }
#endif

        /*
         *  If O_EXCL is set and the last component of the pathname is a symbolic link,
         *  open() will fail even if the symbolic link points to a non-existent name.
         */
        if ((oflag & (O_CREAT|O_EXCL)) == (O_CREAT|O_EXCL)) {
            oflag &= ~(O_CREAT|O_EXCL);         // link must exist
        }

        path = symbuf;
    }

    if (ret < 0) {
        errno = -ret;
        return -1;
    }

    // open
#undef _wopen
    if (NULL != (expath = w32_extendedpathW(path))) {
        path = expath;                          // abs-path to expanded
    }

    fd = _wopen(path, oflag, mode);

    free((void*)expath);                        // release temporary

    return w32_fdregister(fd);
}


LIBW32_API int
w32_openA(const char *path, int oflag, int mode)
{
    char symbuf[WIN32_PATH_MAX];
    const char *expath;
    int fd, ret = 0;

    if (NULL == path) {
        errno = EFAULT;
        return -1;
    }

    if (!*path) {
        errno = ENOENT;
        return -1;
    }

    // specials
    if (0 == w32_iostricmp(path, "/dev/null")) {
        path = "NUL";                           // redirect

    } else if ((ret = ReadlinkA(path, (void *)-1, symbuf, sizeof(symbuf))) < 0) {
        /*
         *  If O_CREAT create the file if it does not exist, in which case the
         *  file is created with mode mode as described in chmod(2) and modified
         *  by the process' umask value (see umask(2)).
         */
        if ((oflag & O_CREAT) && (ret == -ENOTDIR || ret == -ENOENT)) {
            ret = 0;
        }

    } else if (ret > 0) {
        /*
         *  If O_NOFOLLOW and pathname is a symbolic link, then the open fails with ELOOP.
         */
#if defined(O_NOFOLLOW) && (O_NOFOLLOW)         // extension
        if (oflag & O_NOFOLLOW) {
            ret = -ELOOP;
        }
#endif

        /*
         *  If O_EXCL is set and the last component of the pathname is a symbolic link,
         *  open() will fail even if the symbolic link points to a non-existent name.
         */
        if ((oflag & (O_CREAT|O_EXCL)) == (O_CREAT|O_EXCL)) {
            oflag &= ~(O_CREAT|O_EXCL);         // link must exist
        }
        path = symbuf;
    }

    if (ret < 0) {
        errno = -ret;
        return -1;
    }

    // open
#undef _open
    if (NULL != (expath = w32_extendedpathA(path))) {
        path = expath;                          // abs-path to expanded
    }

    fd = _open(path, oflag, mode);

    free((void*)expath);                        // release temporary

    return w32_fdregister(fd);
}


/*
//  NAME
//      fopen - open a stream
//
//  SYNOPSIS
//
//      #include <stdio.h>
//
//      FILE *fopen(const char *path, const char *mode);
//
//  DESCRIPTION
//
//      The fopen() function shall open the file whose pathname is the string pointed to by filename,
//      and associates a stream with it.
//
//      The mode argument points to a string. If the string is one of the following,
//      the file shall be opened in the indicated mode. Otherwise, the behavior is undefined.
//
//        "r" or "rb"
//          - Open file for reading.
//
//        "w" or "wb"
//          - Truncate to zero length or create file for writing.
//
//        "a" or "ab"
//           - Append; open or create file for writing at end-of-file.
//
//        "r+" or "rb+" or "r+b"
//           - Open file for update (reading and writing).
//
//        "w+" or "wb+" or "w+b"
//           - Truncate to zero length or create file for update.
//
//        "a+" or "ab+" or "a+b"
//           - Append; open or create file for update, writing at end-of-file.
//
//      Opening a file with read mode (r as the first character in the mode argument) shall fail
//      if the file does not exist or cannot be read.
//
//      Opening a file with append mode (a as the first character in the mode argument) shall
//      cause all subsequent writes to the file to be forced to the then current end-of-file,
//      regardless of intervening calls to fseek().
//
//      When a file is opened with update mode ( '+' as the second or third character in the mode
//      argument), both input and output may be performed on the associated stream. However, the
//      application shall ensure that output is not directly followed by input without an
//      intervening call to fflush() or to a file positioning function (fseek(), fsetpos(), or
//      rewind()), and input is not directly followed by output without an intervening call to a
//      file positioning function, unless the input operation encounters end-of-file.
//
//      When opened, a stream is fully buffered if and only if it can be determined not to refer
//      to an interactive device. The error and end-of-file indicators for the stream shall be
//      cleared.
//
//      If mode is "w", "wb", "a", "ab", "w+", "wb+", "w+b", "a+", "ab+", or "a+b", and the file
//      did not previously exist, upon successful completion, the fopen() function shall mark for
//      update the 'st_atime', 'st_ctime', and 'st_mtime' fields of the file and the 'st_ctime'
//      and 'st_mtime' fields of the parent directory.
//
//      If mode is "w", "wb", "w+", "wb+", or "w+b", and the file did previously exist, upon successful
//      completion, fopen() shall mark for update the 'st_ctime' and 'st_mtime' fields of the file.
//      The fopen() function shall allocate a file descriptor as open() does.
//
//      After a successful call to the fopen() function, the orientation of the stream shall be
//      cleared, the encoding rule shall be cleared, and the associated mbstate_t object shall be
//      set to describe an initial conversion state.
//
//      The largest value that can be represented correctly in an object of type off_t shall be
//      established as the offset maximum in the open file description.
//
//  RETURN VALUE
//
//      Upon successful completion, fopen() shall return a pointer to the object controlling the stream.
//      Otherwise, a null pointer shall be returned, and errno shall be set indicate the error.
//
//  ERRORS
//
*/

FILE *
w32_fopen(const char *path, const char *mode)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (path && mode) {
            wchar_t wpath[WIN32_PATH_MAX], wmode[32];

            if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
                if (w32_utf2wc(mode, wmode, _countof(wmode)) > 0) {
                    return w32_fopenW(wpath, wmode);
                }
            }
            return NULL;
        }
    }
#endif  //UTF8FILENAMES

    return w32_fopenA(path, mode);
}


FILE *
w32_fopenA(const char *path, const char *mode)
{
    char symbuf[WIN32_PATH_MAX];
    FILE *file = NULL;

    if (NULL == path || NULL == mode) {
        errno = EFAULT;
        return NULL;
    }

    if (0 == w32_iostricmp(path, "/dev/null")) {
        path = "NUL";                           /* Redirect */

    } else if (ReadlinkA(path, (void*)-1, symbuf, _countof(symbuf)) >= 0) {
        path = symbuf;                          /* Follow link */
    }

    if (NULL != (file = fopen(path, mode))) {
        w32_fdregister(_fileno(file));
    }

    return file;
}


FILE *
w32_fopenW(const wchar_t *path, const wchar_t *mode)
{
    wchar_t symbuf[WIN32_PATH_MAX];
    const wchar_t* expath;
    FILE *file = NULL;

    if (NULL == path || NULL == mode) {
        errno = EFAULT;
        return NULL;
    }

    if (0 == w32_iowstricmp(path, "/dev/null")) {
        path = L"NUL";                          /* Redirect */

    } else if (ReadlinkW(path, (void*)-1, symbuf, _countof(symbuf)) >= 0) {
        path = symbuf;                          /* Follow link */
    }

    if (NULL != (expath = w32_extendedpathW(path))) {
        path = expath;                          /* abs-path to expanded */
    }

    if (NULL != (file = _wfopen(path, mode))) {
        w32_fdregister(_fileno(file));
    }

    free((void*)expath);                        /* release temporary */

    return file;
}


//
//  StatZero ---
//      Zero a stat structure.
//
static void
StatZero(struct StatHandle *sb)
{
    switch (sb->type) {
    case SHSTAT:
        memset(sb->buf, 0, sizeof(struct stat));
        break;
    case SHSTAT64:
        memset(sb->buf, 0, sizeof(struct stat64));
        break;
    default:
        assert(0);
        break;
    }
}


//
//  StatAttributes ---
//      Convert WIN attributes to their Unix counterparts.
//
static void
StatAttributes(struct StatHandle *sb, mode_t mode,
        const DWORD dwAttributes, const wchar_t *name, const char *magic)
{
    const wchar_t *p = name;
    wchar_t symbuf[WIN32_PATH_MAX];

    /*
     *  mode, S_IFxxx
     */
    if (p) {
        if (0 == wmemcmp(p, L"\\\\?\\", 4)) {
            if (0 == wmemcmp(p, L"\\\\?\\UNC\\", 8)) {
                p += 4 + 3;                     /* Network UNC prefix, "\\?\UNC" */
            } else {
                p += 4;                         /* UNC prefix; consume */
            }
        }

        if (p[0] && p[1] == ':') {
            p += 2;                             /* consume drive */
        }
    }

    /* type */
    if (0 == mode) {
        if (p && (!p[0] || (ISSLASH(p[0]) && !p[1]))) {
            mode |= S_IFDIR|S_IEXEC;            /* handle root directory explicitly */

        } else if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            mode |= S_IFDIR|S_IEXEC;            /* directory */

        } else if ((FILE_ATTRIBUTE_REPARSE_POINT & dwAttributes) ||
                       (name && ReadlinkW(name, NULL, symbuf, _countof(symbuf)) > 0)) {
            mode |= S_IFLNK;                    /* link */

        } else {
            mode |= S_IFREG;                    /* normal file */
        }
    }

    /* rw */
    mode |= (dwAttributes & FILE_ATTRIBUTE_READONLY) ?
                S_IREAD : (S_IREAD|S_IWRITE);

    /* x */
    if (0 == (mode & S_IEXEC)) {
        if (name && IsExecW(name, magic)) {
            mode |= S_IEXEC;                    /* known exec type */
        }
    }

    /* group/other */
    if (0 == (dwAttributes & FILE_ATTRIBUTE_SYSTEM)) {
        mode |= (mode & 0700) >> 3;             /* group */
        if (0 == (dwAttributes & FILE_ATTRIBUTE_HIDDEN)) {
            mode |= (mode & 0700) >> 6;         /* other */
        }
    }

    /* assign */
    switch (sb->type) {
    case SHSTAT: {
            struct stat *esb = ((struct stat *)sb->buf);
            esb->st_mode = mode;
            if (esb->st_nlink <= 0) {           /* assigned by caller? */
                esb->st_nlink = 1;
            }
        }
        break;
    case SHSTAT64: {
            struct stat64 *esb = ((struct stat64 *)sb->buf);
            esb->st_mode = mode;
            if (esb->st_nlink <= 0) {           /* assigned by caller? */
                esb->st_nlink = 1;
            }
        }
        break;
    default:
        assert(0);
        break;
    }
}


static unsigned
RID(PSID sid)
{
    // Example: S-1-5-32-544
    // Returns the last component, 544.
    const int subAuthorities = *GetSidSubAuthorityCount(sid);
    if (subAuthorities >= 1) {                  // last sub-authority value.
        return *GetSidSubAuthority(sid, subAuthorities - 1);
            // Last component should be the user's relative identifier (RID).
            // It uniquely defines this user account to SAM within the domain.
    }
    return 0;
}


static void
StatOwner(struct StatHandle *sb, const DWORD dwAttributes, HANDLE handle)
{
    short uid = 0, gid = 0;

    // defaults
    if ((FILE_ATTRIBUTE_SYSTEM & dwAttributes) || 0 == handle) {
        uid = 0;                                // root/system
        gid = 0;
    } else {                                    // current user (default)
        uid = (short) w32_getuid();
        gid = (short) w32_getgid();
    }

    // inquire
    if (handle && INVALID_HANDLE_VALUE != handle) {
        PSID owner = NULL, group = NULL;

        if (GetSecurityInfo(handle, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION,
                                &owner, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            uid = (short) RID(owner);
                // Note: Unfortunately st_uid/st_gid are short's resulting in RID truncation.

            if (GetSecurityInfo(handle, SE_FILE_OBJECT, GROUP_SECURITY_INFORMATION,
                                    NULL, &group, NULL, NULL, NULL) == ERROR_SUCCESS) {
                gid = (short) RID(group);
            } else {
                gid = uid;
            }
        }
    }

    // assign
    switch (sb->type) {
    case SHSTAT: {
            struct stat *esb = ((struct stat *)sb->buf);
            esb->st_uid = uid;
            esb->st_gid = gid;
        }
        break;
    case SHSTAT64: {
            struct stat64 *esb = ((struct stat64 *)sb->buf);
            esb->st_uid = uid;
            esb->st_gid = gid;
        }
        break;
    default:
        assert(0);
        break;
    }
}


static void
StatTimes(struct StatHandle *sb,
        const FILETIME *ftCreationTime, const FILETIME *ftLastAccessTime, const FILETIME *ftLastWriteTime)
{
    time_t atime, mtime, ctime;

    // convert
    mtime = ConvertTime(ftLastWriteTime);

    if (0 == (atime = ConvertTime(ftLastAccessTime))) {
        atime = mtime;
    }

    if (0 == (ctime = ConvertTime(ftCreationTime))) {
        ctime = mtime;
    }

    // assign
    switch (sb->type) {
    case SHSTAT: {
            struct stat *esb = ((struct stat *)sb->buf);
            esb->st_atime = atime;
            esb->st_mtime = mtime;
            esb->st_ctime = ctime;
        }
        break;
    case SHSTAT64: {
            struct stat64 *esb = ((struct stat64 *)sb->buf);
            esb->st_atime = atime;
            esb->st_mtime = mtime;
            esb->st_ctime = ctime;
        }
        break;
    default:
        assert(0);
        break;
    }
}


/*  Function:       ConvertTime
 *      Convert a FILETIME structure into a UTC time.
 *
 *  Notes:
 *      Not all file systems can record creation and last access time and not all file
 *      systems record them in the same manner. For example, on Windows NT FAT, create
 *      time has a resolution of 10 milliseconds, write time has a resolution of 2
 *      seconds, and access time has a resolution of 1 day (really, the access date).
 *      On NTFS, access time has a resolution of 1 hour. Therefore, the GetFileTime
 *      function may not return the same file time information set using the
 *      SetFileTime function. Furthermore, FAT records times on disk in local time.
 *      However, NTFS records times on disk in UTC, so it is not affected by changes in
 *      time zone or daylight saving time.
 *
 */
static time_t
ConvertTime(const FILETIME *ft)
{
    SYSTEMTIME SystemTime;
    FILETIME LocalFTime;
    struct tm tm = {0};

    if (! ft->dwLowDateTime && !ft->dwHighDateTime)  {
        return 0;                               /* time unknown */
    }

    if (! FileTimeToLocalFileTime(ft, &LocalFTime) ||
            ! FileTimeToSystemTime(&LocalFTime, &SystemTime)) {
        return -1;
    }

    tm.tm_year = SystemTime.wYear - 1900;       /* Year (current year minus 1900) */
    tm.tm_mon  = SystemTime.wMonth - 1;         /* Month (0  11; January = 0) */
    tm.tm_mday = SystemTime.wDay;               /* Day of month (1  31) */
    tm.tm_hour = SystemTime.wHour;              /* Hours after midnight (0  23) */
    tm.tm_min  = SystemTime.wMinute;            /* Minutes after hour (0  59) */
    tm.tm_sec  = SystemTime.wSecond;            /* Seconds after minute (0  59) */
    tm.tm_isdst = -1;

    return mktime(&tm);
}


#define BLKSIZ 512
#if (HAVE_STRUCT_STAT_ST_BLOCKS)
static blksize_t
blocks(off64_t size)
{
    blksize_t blocks = 0;
    if (size) {
        return (blksize_t)(1 + (size - 1) / BLKSIZ);
    }
    return 0;
}
#endif /*HAVE_STRUCT_STAT_ST_BLOCKS*/


static void
StatSize(struct StatHandle *sb, const DWORD nFileSizeLow, const DWORD nFileSizeHigh)
{
    // assign
    switch (sb->type) {
    case SHSTAT: {
            struct stat *esb = ((struct stat *)sb->buf);

            if (nFileSizeHigh) {
                if (sizeof(esb->st_size) >= sizeof(uint64_t)) {
                    esb->st_size = (((uint64_t)nFileSizeHigh) << 32) + nFileSizeLow;
                } else {
                    esb->st_size = 0xffffffff; // uint32_t
                }
            } else {
                esb->st_size = nFileSizeLow;
            }
#if (HAVE_STRUCT_STAT_ST_BLOCKS)
            esb->st_blocks = blocks(esb->st_size);
#endif
#if (HAVE_STRUCT_STAT_ST_BLKSIZE)
            esb->st_blksize = BLKSIZ;
#endif
        }
        break;
    case SHSTAT64: {
            struct stat64 *esb = ((struct stat64 *)sb->buf);

            esb->st_size = (((off64_t)nFileSizeHigh) << 32) + nFileSizeLow;
#if (HAVE_STRUCT_STAT_ST_BLOCKS)
            esb->st_blocks = blocks(esb->st_size);
#endif
#if (HAVE_STRUCT_STAT_ST_BLKSIZE)
            esb->st_blksize = BLKSIZ;
#endif
        }
        break;
    default:
        assert(0);
        break;
    }
}


static void
StatDevice(struct StatHandle *sb, const DWORD dwVolumeSerialNumber, const DWORD nFileIndexLow, const DWORD nFileIndexHigh, const wchar_t *name)
{
    unsigned short ino = 0;
    unsigned int rdev = 0, dev = 0;

    //  st_dev
    //      This field describes the device on which this file resides.
    //
    //  st_rdev
    //      This field describes the device that this file (INODE) represents.
    //
    //  st_ino
    //      This field contains the file's INODE number.
    //
    if (name) {
        const wchar_t *path = name;

        if (0 == wmemcmp(path, L"\\\\?\\", 4)) {
            path += 4;                          // UNC prefix
        }

        if (path[1] == ':' && iswalpha(path[0])) {
            const int drive = toupper((unsigned char)path[0]) - ('A' - 1);
            rdev = (dev_t)(drive);              // A=1 ...
        }
    }

    dev = (dev_t)dwVolumeSerialNumber;
    if (0 == dev) {
        dev = rdev;
            //Note:
            //  Wont function for reparse-points, hence it is not possible to test in call cases
            //  as to whether a parent/child directory represent a mount-point or are cross-device.
    }

    ino = w32_ino_gen(nFileIndexLow, nFileIndexHigh);
        //Note:
        //  Generate an INODE from nFileIndexHigh and nFileIndexLow, yet warning the identifier
        //  that is stored in the nFileIndexHigh and nFileIndexLow members is called the file ID.
        //
        //  Support for file IDs is file system - specific.File IDs are not guaranteed to be unique
        //  over time, because file systems are free to reuse them. In some cases, the file ID
        //  for a file can change over time.

    if (0 == ino && name) {
        ino = w32_ino_whash(name);
    }

    // assign
    switch (sb->type) {
    case SHSTAT: {
            struct stat *esb = ((struct stat *)sb->buf);
            esb->st_dev = dev;
            esb->st_ino = ino;
            esb->st_rdev = rdev;
        }
        break;
    case SHSTAT64: {
            struct stat64 *esb = ((struct stat64 *)sb->buf);
            esb->st_dev = dev;
            esb->st_ino = ino;
            esb->st_rdev = rdev;
        }
        break;
    default:
        assert(0);
        break;
    }
}


static void
StatLinks(struct StatHandle *sb, NLINK_T nlink)
{
    // assign
    switch (sb->type) {
    case SHSTAT: {
            struct stat *esb = ((struct stat *)sb->buf);
            esb->st_nlink = nlink;
        }
        break;
    case SHSTAT64: {
            struct stat64 *esb = ((struct stat64 *)sb->buf);
            esb->st_nlink = nlink;
        }
        break;
    default:
        assert(0);
        break;
    }
}


/*
 *  IsScriptMagic ---
 *      Determine is a well-known script magic.
 */

static int
IsScriptMagic(const char *magic)
{
    int isscript = -1;

    if (magic[0] == '#' && magic[1] == '!' && magic[2]) {
        /*
         *  #! <path> [options]\n
         */
        const char *exec = magic + 2;
        int len = -1;

        while (*exec && ' ' == *exec) ++exec;
        if (*exec == '/') {
            if (0 == strncmp(exec, "/bin/sh", len = (sizeof("/bin/sh")-1)))
                isscript = 1;
            else if (0 == strncmp(exec, "/bin/ash",  len = (sizeof("/bin/ash")-1)))
                isscript = 1;
            else if (0 == strncmp(exec, "/bin/csh",  len = (sizeof("/bin/csh")-1)))
                isscript = 1;
            else if (0 == strncmp(exec, "/bin/ksh",  len = (sizeof("/bin/ksh")-1)))
                isscript = 1;
            else if (0 == strncmp(exec, "/bin/zsh",  len = (sizeof("/bin/zsh")-1)))
                isscript = 1;
            else if (0 == strncmp(exec, "/bin/bash", len = (sizeof("/bin/bash")-1)))
                isscript = 1;
            else if (0 == strncmp(exec, "/bin/dash", len = (sizeof("/bin/dash")-1)))
                isscript = 1;
            else if (0 == strncmp(exec, "/bin/fish", len = (sizeof("/bin/fish")-1)))
                isscript = 1;
            else if (0 == strncmp(exec, "/bin/tcsh", len = (sizeof("/bin/tcsh")-1)))
                isscript = 1;
            else if (0 == strncmp(exec, "/bin/sed",  len = (sizeof("/bin/sed")-1)))
                isscript = 1;
            else if (0 == strncmp(exec, "/bin/awk",  len = (sizeof("/bin/awk")-1)))
                isscript = 1;
            else if (0 == strncmp(exec, "/usr/bin/perl", len = (sizeof("/usr/bin/perl")-1)))
                isscript = 1;
            else if (0 == strncmp(exec, "/usr/bin/python", len = (sizeof("/usr/bin/python")-1)))
                isscript = 1;
            if (isscript &&
                    exec[len] != ' ' && exec[len] != '\n' && exec[len] != '\r') {
                isscript = 0;           /* bad termination, ignore */
            }
        }
    }
    return isscript;
}


/*
 *  IsExecA ---
 *      Determine if the file a possible executable file-type.
 */

#define EXEC_ASSUME     \
    (sizeof(exec_assume)/sizeof(exec_assume[0]))

#define EXEC_EXCLUDE    \
    (sizeof(exec_exclude)/sizeof(exec_exclude[0]))

static const char *     exec_assume[]   = {
    ".exe", ".com", ".cmd", ".bat"
    };

static const char *     exec_exclude[]  = {
    ".o",   ".obj",                             /* objects */
    ".h",   ".hpp", ".inc",                     /* header files */
    ".c",   ".cc",  ".cpp", ".cs",              /* source files */
    ".a",   ".lib", ".dll",                     /* libraries */
                                                /* archives */
    ".zip", ".gz",  ".tar", ".tgz", ".bz2", ".rar",
    ".doc", ".txt",                             /* documents */
    ".hlp", ".chm",                             /* help */
    ".dat"                                      /* data files */
    };


static int
IsExecA(const char *name, const char *magic)
{
    DWORD driveType;
    const char *dot;
    int idx = -1;

    if ((dot = HasExtensionA(name)) != NULL) {  /* check well-known extensions */
        for (idx = EXEC_ASSUME-1; idx >= 0; idx--)
            if (w32_iostricmp(dot, exec_assume[idx]) == 0) {
                return TRUE;
            }

        for (idx = EXEC_EXCLUDE-1; idx >= 0; idx--)
            if (w32_iostricmp(dot, exec_exclude[idx]) == 0) {
                break;
            }
    }

    if (magic) {                                /* #! */
        int isscript;

        if ((isscript = IsScriptMagic(magic)) >= 0) {
            return isscript;
        }
    }

    if (-1 == idx) {                            /* only local drives */
        if ((driveType = GetDriveTypeA(name)) == DRIVE_FIXED) {
            DWORD binaryType = 0;

            if (GetBinaryTypeA(name, &binaryType)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}


/*
 *  IsExecW ---
 *      Determine if the file a possible executable file-type.
 */

static int
IsExecW(const wchar_t *name, const char *magic)
{
    DWORD driveType;
    const wchar_t *dot;
    int idx = -1;

    if ((dot = HasExtensionW(name)) != NULL) {  /* check well-known extensions */
        for (idx = EXEC_ASSUME-1; idx >= 0; idx--)
            if (w32_iowstricmp(dot, exec_assume[idx]) == 0) {
                return TRUE;
            }

        for (idx = EXEC_EXCLUDE-1; idx >= 0; idx--)
            if (w32_iowstricmp(dot, exec_exclude[idx]) == 0) {
                break;
            }
    }

    if (magic) {                                /* #! */
        int isscript;

        if ((isscript = IsScriptMagic(magic)) >= 0) {
            return isscript;
        }
    }

    if (-1 == idx) {                            /* only local drives */
        if ((driveType = GetDriveTypeW(name)) == DRIVE_FIXED) {
            DWORD binaryType = 0;

            if (GetBinaryTypeW(name, &binaryType)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}


static const char *
HasExtensionA(const char *name)
{
    const size_t len = strlen(name);
    const char *cursor;

    for (cursor = name + len; --cursor >= name;) {
        if (*cursor == '.')
            return cursor;                      /* extension */
        if (*cursor == '/' || *cursor == '\\')
            break;
    }
    return NULL;
}


static const wchar_t *
HasExtensionW(const wchar_t *name)
{
    const size_t len = wcslen(name);
    const wchar_t *cursor;

    for (cursor = name + len; --cursor >= name;) {
        if (*cursor == '.')
            return cursor;                      /* extension */
        if (*cursor == '/' || *cursor == '\\')
            break;
    }
    return NULL;
}


static BOOL
IsExtensionA(const char *name, const char *ext)
{
    const char *dot;

    if (ext && (dot = HasExtensionA(name)) != NULL &&
            w32_iostricmp(dot, ext) == 0) {
        return TRUE;
    }
    return FALSE;
}


static BOOL
IsExtensionW(const wchar_t *name, const char *ext)
{
    const wchar_t *dot;

    if (ext && (dot = HasExtensionW(name)) != NULL &&
            w32_iowstricmp(dot, ext) == 0) {
        return TRUE;
    }
    return FALSE;
}


//
//  ReadlinkA ---
//      Resolve a short-cut/symlink reference.
//

static int
ReadlinkA(const char *path, const char **suffixes, char *buf, size_t maxlen)
{
    DWORD attrs;
    const char *suffix;
    size_t length;
    int ret = -ENOENT;

    (void) strncpy( buf, path, maxlen );        // prime working buffer
    buf[ maxlen-1 ] = '\0';
    length = (int)strlen(buf);

    if (suffixes == (void *)NULL) {
        suffixes = suffixes_null;
    } else if (suffixes == (void *)-1) {
        suffixes = suffixes_default;
    }

    while ((suffix = *suffixes++) != NULL) {
        /* Concatenate suffix */
        if (length + strlen(suffix) >= maxlen) {
            ret = -ENAMETOOLONG;
            continue;
        }
        strcpy(buf + length, suffix);           // concatenate suffix

        /* File attributes */
        if (0xffffffff == (attrs = GetFileAttributesA(buf))) {
            const char *expath;                 // abs-path to expanded

            if (NULL != (expath = w32_extendedpathA(buf))) {
                attrs = GetFileAttributesA(expath);
                free((void*)expath);
            }
        }

        if (0xffffffff == attrs) {
            DWORD rc;

            if ((rc = GetLastError()) == ERROR_ACCESS_DENIED ||
                        rc == ERROR_SHARING_VIOLATION) {
                ret = -EACCES;
            } else if (rc == ERROR_PATH_NOT_FOUND) {
                ret = -ENOTDIR;
            } else if (rc == ERROR_FILE_NOT_FOUND) {
                ret = -ENOENT;
            } else {
                ret = -EIO;
            }
            continue;                           // next suffix
        }

        /* Parse attributes */
        if ((attrs & (FILE_ATTRIBUTE_DIRECTORY)) ||
#ifdef FILE_ATTRIBUTE_COMPRESSED
                    (attrs & (FILE_ATTRIBUTE_COMPRESSED)) ||
#endif
#ifdef FILE_ATTRIBUTE_DEVICE
                    (attrs & (FILE_ATTRIBUTE_DEVICE)) ||
#endif
#ifdef FILE_ATTRIBUTE_ENCRYPTED
                    (attrs & (FILE_ATTRIBUTE_ENCRYPTED))
#endif
            ) {
            ret = 0;                            // generally not a symlink
            if ((attrs & FILE_ATTRIBUTE_DIRECTORY) &&
                    (attrs & FILE_ATTRIBUTE_REPARSE_POINT)) {
                                                // possible mount-point
                if (0 == w32_reparse_readA(path, buf, maxlen)) {
                    ret = (int)strlen(buf);
                }
            }

        /* reparse point - symlink/mount-point */
        } else if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) {
            if ((ret = w32_reparse_readA(path, buf, maxlen)) >= 0) {
                ret = (int)strlen(buf);
            } else {
                ret = -EIO;
            }

        /* shortcut */
        } else if (attrs & FILE_ATTRIBUTE_OFFLINE) {
            ret = -EACCES;                      // wont be able to access

#define SHORTCUT_COOKIE         "L\0\0\0"       // shortcut magic

                                                // cygwin shortcut also system/read-only
#define CYGWIN_ATTRS            FILE_ATTRIBUTE_SYSTEM
#define CYGWIN_COOKIE           "!<symlink>"    // old style shortcut

        } else if (IsExtensionA(buf, ".lnk") ||
                        (attrs & (FILE_ATTRIBUTE_HIDDEN|CYGWIN_ATTRS)) == CYGWIN_ATTRS) {

            SECURITY_ATTRIBUTES sa = {0};
            char cookie[sizeof(CYGWIN_COOKIE)-1];
            HANDLE fh;
            DWORD got;

            sa.nLength = sizeof(sa);
            sa.lpSecurityDescriptor = NULL;
            sa.bInheritHandle = FALSE;

            if ((fh = CreateFileA(buf, GENERIC_READ, FILE_SHARE_READ,
                            &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE) {

                // read header
                if (! ReadFile(fh, cookie, sizeof (cookie), &got, 0)) {
                    ret = -EIO;

                // win32 shortcut (will also read cygwin shortcuts)
                } else if (got >= 4 && 0 == memcmp(cookie, SHORTCUT_COOKIE, 4)) {
                    if ((ret = ReadShortcutA(buf, buf, maxlen)) >= 0) {
                        ret = (int)strlen(buf);
                    } else {
                        ret = -EIO;
                    }

                // cygwin symlink (old style)
                } else if ((attrs & CYGWIN_ATTRS) && got == sizeof(cookie) &&
                                0 == memcmp(cookie, CYGWIN_COOKIE, sizeof(cookie))) {

                    if (! ReadFile(fh, buf, (DWORD)maxlen, &got, 0)) {
                        ret = -EIO;
                    } else {
                        char *end;

                        if ((end = (char *)memchr(buf, 0, got)) != NULL) {
                            ret = (int)(end - buf); // find the NUL terminator
                        } else {
                            ret = (int)got;
                        }
                        if (ret == 0) {
                            ret = -EIO;         // empty link specification
                        }
                    }
                } else {
                    ret = 0;                    // not a symlink
                }
                CloseHandle(fh);
            }

        } else {
            ret = 0;                            // not a symlink
        }
        break;
    }

    if (ret > 0) {
        w32_dos2unix(buf);
    }
    return ret;
}


//
//  ReadlinkW ---
//      Resolve a short-cut/symlink reference.
//

static int
ReadlinkW(const wchar_t *path, const char **suffixes, wchar_t *buf, size_t maxlen)
{
    DWORD attrs;
    const char *suffix;
    size_t length;
    int ret = -ENOENT;

    wcsncpy(buf, path, maxlen);                 // prime working buffer
    buf[ maxlen-1 ] = '\0';
    length = (int)wcslen(buf);

    if (suffixes == (void *)NULL) {
        suffixes = suffixes_null;
    } else if (suffixes == (void *)-1) {
        suffixes = suffixes_default;
    }

    while ((suffix = *suffixes++) != NULL) {
        /* Concatenate suffix */
        if (length + strlen(suffix) >= maxlen) {
            ret = -ENAMETOOLONG;
            continue;
        }

        {   wchar_t *cursor;
            for (cursor = buf + length;; ++cursor) {
                if (0 == (*cursor = (wchar_t)*suffix++)) {
                    break;
                }
            }
        }

        /* File attributes */
        if (0xffffffff == (attrs = GetFileAttributesW(buf))) {
            const wchar_t *expath;              // abs-path to expanded

            if (NULL != (expath = w32_extendedpathW(buf))) {
                attrs = GetFileAttributesW(expath);
                free((void*)expath);
            }
        }

        if (0xffffffff == attrs) {
            DWORD rc;

            if ((rc = GetLastError()) == ERROR_ACCESS_DENIED ||
                        rc == ERROR_SHARING_VIOLATION) {
                ret = -EACCES;
            } else if (rc == ERROR_PATH_NOT_FOUND) {
                ret = -ENOTDIR;
            } else if (rc == ERROR_FILE_NOT_FOUND) {
                ret = -ENOENT;
            } else {
                ret = -EIO;
            }
            continue;                           // next suffix
        }

        /* Parse attributes */
        if ((attrs & (FILE_ATTRIBUTE_DIRECTORY)) ||
#ifdef FILE_ATTRIBUTE_COMPRESSED
                    (attrs & (FILE_ATTRIBUTE_COMPRESSED)) ||
#endif
#ifdef FILE_ATTRIBUTE_DEVICE
                    (attrs & (FILE_ATTRIBUTE_DEVICE)) ||
#endif
#ifdef FILE_ATTRIBUTE_ENCRYPTED
                    (attrs & (FILE_ATTRIBUTE_ENCRYPTED))
#endif
            ) {
            ret = 0;                            // generally not a symlink
            if ((attrs & FILE_ATTRIBUTE_DIRECTORY) &&
                    (attrs & FILE_ATTRIBUTE_REPARSE_POINT)) {
                                                // possible mount-point
                if (0 == w32_reparse_readW(path, buf, maxlen)) {
                    ret = (int)wcslen(buf);
                }
            }

        /* reparse point - symlink/mount-point */
        } else if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) {
            if ((ret = w32_reparse_readW(path, buf, maxlen)) >= 0) {
                ret = (int) wcslen(buf);
            } else {
                ret = -EIO;
            }

        /* shortcut */
        } else if (attrs & FILE_ATTRIBUTE_OFFLINE) {
            ret = -EACCES;                      // wont be able to access

#define SHORTCUT_COOKIE         "L\0\0\0"       // shortcut magic

                                                // Cygwin shortcut also system/read-only
#define CYGWIN_ATTRS            FILE_ATTRIBUTE_SYSTEM
#define CYGWIN_COOKIE           "!<symlink>"    // old style shortcut

        } else if (IsExtensionW(buf, ".lnk") ||
                        (attrs & (FILE_ATTRIBUTE_HIDDEN|CYGWIN_ATTRS)) == CYGWIN_ATTRS) {

            SECURITY_ATTRIBUTES sa = {0};
            char cookie[sizeof(CYGWIN_COOKIE)-1];
            HANDLE fh;
            DWORD got;

            sa.nLength = sizeof(sa);
            sa.lpSecurityDescriptor = NULL;
            sa.bInheritHandle = FALSE;

            if ((fh = CreateFileW(buf, GENERIC_READ, FILE_SHARE_READ,
                            &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE) {

                // read header
                if (! ReadFile(fh, cookie, sizeof (cookie), &got, 0)) {
                    ret = -EIO;

                // win32 shortcut (will also read cygwin shortcuts)
                } else if (got >= 4 && 0 == memcmp(cookie, SHORTCUT_COOKIE, 4)) {
                    if ((ret = ReadShortcutW(buf, buf, maxlen)) >= 0) {
                        ret = (int)wcslen(buf);
                    } else {
                        ret = -EIO;
                    }

                // cygwin symlink (old style)
                } else if ((attrs & CYGWIN_ATTRS) && got == sizeof(cookie) &&
                                0 == memcmp(cookie, CYGWIN_COOKIE, sizeof(cookie))) {

                    if (! ReadFile(fh, buf, (DWORD)maxlen, &got, 0)) {
                        ret = -EIO;
                    } else {
                        wchar_t *end;

                        if ((end = (wchar_t *)wmemchr(buf, 0, got)) != NULL) {
                            ret = (int)(end - buf); // find the NUL terminator
                        } else {
                            ret = (int)(got / sizeof(wchar_t));
                        }
                        if (ret == 0) {
                            ret = -EIO;         // empty link specification
                        }
                    }
                } else {
                    ret = 0;                    // not a symlink
                }
                CloseHandle(fh);
            }

        } else {
            ret = 0;                            // not a symlink
        }
        break;
    }

    if (ret > 0) {
        w32_dos2unixW(buf);
    }
    return ret;
}


static const GUID   x_CLSID_ShellLink   =       // local copies; OWC linker crashes otherwise
    { 0x00021401, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

static const IID    x_IID_IShellLinkA   =
    { 0x000214EE, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
static const IID    x_IID_IShellLinkW   =
    { 0x000214F9, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

static const IID    x_IID_IPersistFile  =
    { 0x0000010B, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};


/*  Function:       ReadShortcut
 *      Fills the filename and path buffer with relevant information.
 *
 *  Parameters:
 *      name - name of the link file passed into the function.
 *
 *      path - buffer that receives the file's path name.
 *
 *      maxlen - max length of the 'path' buffer.
 *
 *  Notes:
 *      The shortcuts used in Microsoft Windows 95 provide applications and users a way
 *      to create shortcuts or links to objects in the shell's namespace. The
 *      IShellLink OLE Interface can be used to obtain the path and filename from the
 *      shortcut, among other things.
 *
 *      A shortcut allows the user or an application to access an object from anywhere
 *      in the namespace. Shortcuts to objects are stored as binary files. These files
 *      contain information such as the path to the object, working directory, the path
 *      of the icon used to display the object, the description string, and so on.
 *
 *      Given a shortcut, applications can use the IShellLink interface and its
 *      functions to obtain all the pertinent information about that object. The
 *      IShellLink interface supports functions such as GetPath(), GetDescription(),
 *      Resolve(), GetWorkingDirectory(), and so on.
 */

static int
ReadShortcutA(const char *name, char *buf, size_t maxlen)
{
    WIN32_FIND_DATA wfd = {0};
    IShellLinkA *pShLink;
    HRESULT hres = FALSE;

    (void) CoInitialize(NULL);

    hres = CoCreateInstance(&x_CLSID_ShellLink, NULL,
                CLSCTX_INPROC_SERVER, &x_IID_IShellLinkA, (LPVOID *)&pShLink);

    if (name != buf) buf[0] = 0;
    if (SUCCEEDED(hres)) {
        IPersistFile *ppf;

        hres = pShLink->lpVtbl->QueryInterface(pShLink, &x_IID_IPersistFile, (LPVOID *)&ppf);
        if (SUCCEEDED(hres)) {
            wchar_t wname[WIN32_PATH_MAX];

            w32_utf2wc(name, wname, _countof(wname));
            hres = ppf->lpVtbl->Load(ppf, wname, STGM_READ);
            if (SUCCEEDED(hres)) {
                /*
                 -  if (SUCCEEDED(hres)) {
                 -      hres = pShLink->lpVtbl->Resolve(
                 -                  pShLink, 0, SLR_NOUPDATE | SLR_ANY_MATCH | SLR_NO_UI);
                 -  }
                 */
                hres = pShLink->lpVtbl->GetPath(pShLink, buf, (int)maxlen, &wfd, 0);
                if (!SUCCEEDED(hres) || 0 == buf[0]) {
                    /*
                     *  A document shortcut may only have a description ...
                     *  Also CYGWIN generates this style of link.
                     */
                    hres = pShLink->lpVtbl->GetDescription(pShLink, buf, (int)maxlen);
                    if (SUCCEEDED(hres) && 0 == buf[0]) {
                        hres = E_FAIL;
                    }
                }
                ppf->lpVtbl->Release(ppf);
            }
        }
        pShLink->lpVtbl->Release(pShLink);
    }

    CoUninitialize();

    return (SUCCEEDED(hres) ? 0 : -1);
}


static int
ReadShortcutW(const wchar_t *name, wchar_t *buf, size_t maxlen)
{
    WIN32_FIND_DATAW wfd = {0};
    IShellLinkW *pShLink;
    HRESULT hres = FALSE;

    (void) CoInitialize(NULL);

    hres = CoCreateInstance(&x_CLSID_ShellLink, NULL,
                CLSCTX_INPROC_SERVER, &x_IID_IShellLinkW, (LPVOID *)&pShLink);

    if (name != buf) buf[0] = 0;
    if (SUCCEEDED(hres)) {
        IPersistFile *ppf;

        hres = pShLink->lpVtbl->QueryInterface(pShLink, &x_IID_IPersistFile, (LPVOID *)&ppf);
        if (SUCCEEDED(hres)) {
            hres = ppf->lpVtbl->Load(ppf, name, STGM_READ);
            if (SUCCEEDED(hres)) {
                hres = pShLink->lpVtbl->GetPath(pShLink, buf, maxlen, &wfd, 0);
                if (!SUCCEEDED(hres) || 0 == buf[0]) {
                    /*
                     *  A document shortcut may only have a description ...
                     *  Also CYGWIN generates this style of link.
                     */
                    hres = pShLink->lpVtbl->GetDescription(pShLink, buf, maxlen);
                    if (SUCCEEDED(hres) && 0 == buf[0]) {
                        hres = E_FAIL;
                    }
                }
                ppf->lpVtbl->Release(ppf);
            }
        }
        pShLink->lpVtbl->Release(pShLink);
    }

    CoUninitialize();

    return (SUCCEEDED(hres) ? 0 : -1);
}


static int
CreateShortcutA(const char *link, const char *name, const char *working, const char *desc)
{
    IShellLinkA *pShLink;
    HRESULT hres;

    (void) CoInitialize(NULL);

    // IShellLink interface.
    hres = CoCreateInstance(&x_CLSID_ShellLink, NULL,
                CLSCTX_INPROC_SERVER, &x_IID_IShellLinkA, (PVOID *) &pShLink);

    if (SUCCEEDED(hres)) {
        IPersistFile *ppf;

        // Attributes.
        if (name) {
            char resolved[WIN32_PATH_MAX];

            if (GetFullPathNameA(name, (DWORD)sizeof(resolved), resolved, 0)) {
                pShLink->lpVtbl->SetPath(pShLink, (LPCSTR)resolved);
            } else {
                pShLink->lpVtbl->SetPath(pShLink, (LPCSTR)name);
            }
        }

        if (working && *working) {
            pShLink->lpVtbl->SetWorkingDirectory(pShLink, (LPCSTR)working);
        }

        if (desc) {
            pShLink->lpVtbl->SetDescription(pShLink, (LPCSTR)desc);
        }

        // IPersistFile interface, for saving the shortcut in persistent storage.
        hres = pShLink->lpVtbl->QueryInterface(pShLink, &x_IID_IPersistFile, (PVOID *) &ppf);

        if (SUCCEEDED(hres)) {
            wchar_t wlink[WIN32_PATH_MAX];

            w32_utf2wc(link, wlink, _countof(wlink));
            hres = ppf->lpVtbl->Save(ppf, wlink, TRUE);
            ppf->lpVtbl->Release(ppf);
        }
        pShLink->lpVtbl->Release(pShLink);
    }

    CoUninitialize();

    return (SUCCEEDED(hres) ? TRUE : FALSE);
}


static int
CreateShortcutW(const wchar_t *link, const wchar_t *name, const wchar_t *working, const wchar_t *desc)
{
    IShellLinkW *pShLink;
    HRESULT hres;

    (void) CoInitialize(NULL);

    // IShellLink interface.
    hres = CoCreateInstance(&x_CLSID_ShellLink, NULL,
                CLSCTX_INPROC_SERVER, &x_IID_IShellLinkW, (PVOID*)&pShLink);

    if (SUCCEEDED(hres)) {
        IPersistFile *ppf;

        // Attributes.
        if (name) {
            wchar_t resolved[WIN32_PATH_MAX];

            if (GetFullPathNameW(name, (DWORD)_countof(resolved), resolved, 0)) {
                pShLink->lpVtbl->SetPath(pShLink, (LPCWSTR)resolved);
            } else {
                pShLink->lpVtbl->SetPath(pShLink, (LPCWSTR)name);
            }
        }

        if (working && *working) {
            pShLink->lpVtbl->SetWorkingDirectory(pShLink, (LPCWSTR)working);
        }

        if (desc) {
            pShLink->lpVtbl->SetDescription(pShLink, (LPCWSTR)desc);
        }

        // IPersistFile interface, for saving the shortcut in persistent storage.
        hres = pShLink->lpVtbl->QueryInterface(pShLink, &x_IID_IPersistFile, (PVOID*)&ppf);

        if (SUCCEEDED(hres)) {
            hres = ppf->lpVtbl->Save(ppf, link, TRUE);
            ppf->lpVtbl->Release(ppf);
        }
        pShLink->lpVtbl->Release(pShLink);
    }

    CoUninitialize();

    return (SUCCEEDED(hres) ? TRUE : FALSE);
}


//
//  W32StatAFile ---
//      stat() system call.
//

static int
W32StatAFile(const char *name, struct StatHandle *sb)
{
    union {
        char name[WIN32_PATH_MAX];
        wchar_t wname[WIN32_PATH_MAX];
    } resolved;
    char *pfname = NULL;
    int namelen, ret = -1;

    resolved.wname[0] = 0;

    if (name == NULL || sb == NULL) {
        ret = -EFAULT;                          /* basic checks */

    } else if (name[0] == '\0' ||
                (name[1] == ':' && name[2] == '\0')) {
        ret = -ENOENT;                          /* missing directory ??? */

    } else if (W32StatByNameA(name, sb)) {
        ret = 0;                                /* direct by handle interface */

    } else if (strchr(name, '?') || strchr(name, '*')) {
        ret = -ENOENT;                          /* wild cards -- break FindFirstFile() */

    } else if (0 == (namelen =
                GetFullPathNameA(name, sizeof(resolved.name), resolved.name, &pfname))) {
        ret = -ENOENT;                          /* parse error */

    } else if (namelen >= (int)sizeof(resolved.name)) {
        ret = -ENAMETOOLONG;                    /* buffer overflow */

    } else {
        /*
         *  Indirect method using FindFirstFileW.
         */
        const char *fullname = resolved.name;
        HANDLE find = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATAA fb = {0};

        StatZero(sb);

        if ((find = FindFirstFileA(fullname, &fb)) != INVALID_HANDLE_VALUE) {
            ret = 0;

        } else {                                /* "x:\" */
            if (((fullname[0] && fullname[1] == ':' &&
                        ISSLASH(fullname[2]) && fullname[3] == '\0')) &&
                    GetDriveTypeA(fullname) > 1) {
                /*
                 *  Root directories
                 */
                fb.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
                ret = 0;

            } else if (namelen >= 5 && ISSLASH(fullname[0]) && ISSLASH(fullname[1])) {
                /*
                 *  Root UNC (//server-name/mount)
                 */
                const char *slash = w32_strslash(fullname + 2);
                const char *nextslash = w32_strslash(slash ? slash + 1 : NULL);

                ret = -ENOENT;
                if (NULL != slash &&
                        (NULL == nextslash || 0 == nextslash[1])) {
                    fb.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
                    ret = 0;
                }

            } else {
                /*
                 *  Determine cause
                 */
                DWORD rc = GetLastError(), attrs = 0;

                if (0xffffffff == (attrs = GetFileAttributesA(fullname))) {
                   if ((rc = GetLastError()) == ERROR_ACCESS_DENIED ||
                            rc == ERROR_SHARING_VIOLATION) {
                        ret = -EACCES;
                    } else if (rc == ERROR_PATH_NOT_FOUND) {
                        ret = -ENOTDIR;
                    } else if (rc == ERROR_FILE_NOT_FOUND) {
                        ret = -ENOENT;
                    } else {
                        ret = -EIO;
                    }

                } else if (rc == ERROR_ACCESS_DENIED) {
                    // Junction encountered (e.g C:/Users/Default User --> Default),
                    // FindFirstFile() behavior is by design to stop applications recursively accessing symlinks.
                    fb.dwFileAttributes = attrs;
                    ret = 0;

                } else {
                    // Other conditions.
                    ret = -EIO;
                }
            }
        }

        /*
         *  Export results
         */
        if (0 == ret) {
            const wchar_t *wfullname = NULL;
            size_t wnamelen = 0;
            HANDLE file;

            file = CreateFileA(fullname, READ_CONTROL, 0, NULL, OPEN_EXISTING,
                        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_READONLY, NULL);
            if (INVALID_HANDLE_VALUE == file) {
                file = CreateFileA(fullname, 0, 0, NULL, OPEN_EXISTING,
                            FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_READONLY, NULL);
            }

            if (INVALID_HANDLE_VALUE != file) {
                wnamelen = my_GetFinalPathNameByHandleW(file, resolved.wname, _countof(resolved.wname));
                if (wnamelen) {
                    wfullname = resolved.wname;
                }
            }

            assert(offsetof(WIN32_FIND_DATAW, dwFileAttributes) == offsetof(WIN32_FIND_DATAA, dwFileAttributes));
            assert(offsetof(WIN32_FIND_DATAW, ftCreationTime) == offsetof(WIN32_FIND_DATAA, ftCreationTime));
            assert(offsetof(WIN32_FIND_DATAW, ftLastAccessTime) == offsetof(WIN32_FIND_DATAA, ftLastAccessTime));
            assert(offsetof(WIN32_FIND_DATAW, ftLastWriteTime) == offsetof(WIN32_FIND_DATAA, ftLastWriteTime));
            assert(offsetof(WIN32_FIND_DATAW, nFileSizeHigh) == offsetof(WIN32_FIND_DATAA, nFileSizeHigh));
            assert(offsetof(WIN32_FIND_DATAW, nFileSizeLow) == offsetof(WIN32_FIND_DATAA, nFileSizeLow));
            assert(offsetof(WIN32_FIND_DATAW, dwReserved0) == offsetof(WIN32_FIND_DATAA, dwReserved0));

            if (! W32StatCommon(file, (const WIN32_FIND_DATAW *)&fb, sb, wfullname, wnamelen)) {
                ret = -EIO;
            }

            if (INVALID_HANDLE_VALUE != file) {
                CloseHandle(file);              /* file session */
            }
        }

        if (INVALID_HANDLE_VALUE != find) {
            FindClose(find);                    /* find session */
        }
    }
    return ret;
}


//
//  W32StatWFile ---
//      stat() system call.
//

static int
W32StatWFile(const wchar_t *name, struct StatHandle *sb)
{
    wchar_t fullname[WIN32_PATH_MAX] = {0}, *pfname = NULL;
    size_t namelen;
    int ret = -1;

    if (name == NULL || sb == NULL) {
        ret = -EFAULT;                          /* basic checks */

    } else if (name[0] == '\0' ||
                (name[1] == ':' && name[2] == '\0')) {
        ret = -ENOENT;                          /* missing path */

    } else if (W32StatByNameW(name, sb)) {
        ret = 0;                                /* direct by handle interface */

    } else if (StrChrW(name, '?') || StrChrW(name, '*')) {
        ret = -ENOENT;                          /* wild cards -- break FindFirstFile() */

    } else if (0 == (namelen =
                GetFullPathNameW(name, _countof(fullname), fullname, &pfname))) {
        ret = -ENOENT;                          /* parse error */

    } else if (namelen >= _countof(fullname)) {
        ret = -ENAMETOOLONG;                    /* buffer overflow */

    } else {
        /*
         *  Indirect method using FindFirstFileW.
         */
        WIN32_FIND_DATAW fb = {0};
        HANDLE find;

        StatZero(sb);

        if ((find = FindFirstFileW(fullname, &fb)) != INVALID_HANDLE_VALUE) {
            ret = 0;

        } else {                                /* "x:\" */
            if (((fullname[0] && fullname[1] == ':' &&
                        ISSLASH(fullname[2]) && fullname[3] == '\0')) &&
                    GetDriveTypeW(fullname) > 1) {
                /*
                 *  Root directories
                 */
                fb.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
                ret = 0;

            } else if (namelen >= 5 && ISSLASH(fullname[0]) && ISSLASH(fullname[1])) {
                /*
                 *  Root UNC (//server-name/mount)
                 */
                const wchar_t *slash = w32_wcsslash(fullname + 2);
                const wchar_t *nextslash = w32_wcsslash(slash ? slash + 1 : NULL);

                ret = -ENOENT;
                if (NULL != slash &&
                        (NULL == nextslash || 0 == nextslash[1])) {
                    fb.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
                    ret = 0;
                }

            } else {
                /*
                 *  Determine cause
                 */
                DWORD rc = GetLastError(), attrs = 0;

                if (0xffffffff == (attrs = GetFileAttributesW(fullname))) {
                    if ((rc = GetLastError()) == ERROR_ACCESS_DENIED ||
                                rc == ERROR_SHARING_VIOLATION) {
                        ret = -EACCES;
                    } else if (rc == ERROR_PATH_NOT_FOUND) {
                        ret = -ENOTDIR;
                    } else if (rc == ERROR_FILE_NOT_FOUND) {
                        ret = -ENOENT;
                    } else {
                        ret = -EIO;
                    }

                } else if (rc == ERROR_ACCESS_DENIED) {
                    // Junction encountered (e.g C:/Users/Default User --> Default),
                    // FindFirstFile() behavior is by design to stop applications recursively accessing symbolic links.
                    fb.dwFileAttributes = attrs;
                    ret = 0;

                } else {
                    // Other conditions.
                    ret = -EIO;
                }
            }
        }

        /*
         *  Export results
         */
        if (0 == ret) {
            HANDLE file;

            file = CreateFileW(fullname, READ_CONTROL, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_READONLY, NULL);
            if (INVALID_HANDLE_VALUE == file) {
                file = CreateFileW(fullname, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                            FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_READONLY, NULL);
            }

            if (! W32StatCommon(file, &fb, sb, fullname, namelen)) {
                ret = -EIO;
            }

            if (INVALID_HANDLE_VALUE != file) {
                CloseHandle(file);              /* file session */
            }
        }

        if (INVALID_HANDLE_VALUE != find) {
            FindClose(find);                    /* find session */
        }
    }
    return ret;
}


//
//  W32StatByNameA ---
//      fstat() system call.
//

static BOOL
W32StatByNameA(const char *name, struct StatHandle *sb)
{
    const char *expath;
    HANDLE handle;

    if (NULL != (expath = w32_extendedpathA(name))) {
        name = expath;                          // abs-path to expanded
    }

    handle = CreateFileA(name, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                    FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_READONLY, NULL);

    free((void*)expath);                        // release temporary

    if (handle != INVALID_HANDLE_VALUE) {
        wchar_t fullname[WIN32_PATH_MAX];
        size_t namelen;

        fullname[0] = 0;
        namelen = my_GetFinalPathNameByHandleW(handle, fullname, _countof(fullname));
        if (W32StatCommon(handle, NULL, sb, fullname, namelen)) {
            return TRUE;
        }
    }

    return FALSE;
}


//
//  W32StatByNameW ---
//      fstat() system call.
//

static BOOL
W32StatByNameW(const wchar_t *name, struct StatHandle *sb)
{
    const wchar_t *expath;
    HANDLE handle;

    if (NULL != (expath = w32_extendedpathW(name))) {
        name = expath;                          // abs-path to expanded
    }

    handle = CreateFileW(name, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                    FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_READONLY, NULL);

    free((void *)expath);                       // release temporary

    if (handle != INVALID_HANDLE_VALUE) {
        wchar_t fullname[WIN32_PATH_MAX];
        size_t namelen;

        fullname[0] = 0;
        namelen = my_GetFinalPathNameByHandleW(handle, fullname, _countof(fullname));
        if (W32StatCommon(handle, NULL, sb, fullname, namelen)) {
            return TRUE;
        }
    }

    return FALSE;
}


//
//  W32StatCommon ---
//      Common stat()/fstat() functionality.
//

static BOOL
W32StatCommon(HANDLE handle, const WIN32_FIND_DATAW *fb, struct StatHandle *sb, const wchar_t *fullname, size_t namelen)
{
    BY_HANDLE_FILE_INFORMATION fi = {0};
    DWORD dwRootAttributes = 0;
    const wchar_t *path = fullname;
    mode_t mode = 0;
    char magic[512];

    // File Information
    if (handle) {                               // direct handle
        if (! GetFileInformationByHandle(handle, &fi) && NULL == fb) {
            return FALSE;
        }

    } else {                                    // FindFirstFile() sourced information
        if (NULL == fb) {
            return FALSE;
        }
    }

    if (fb && 0 == fi.dwFileAttributes) {       // import FindFirst information
        fi.dwFileAttributes = fb->dwFileAttributes;
        fi.nFileSizeHigh = fb->nFileSizeHigh;
        fi.nFileSizeLow = fb->nFileSizeLow;
        fi.ftCreationTime = fb->ftCreationTime;
        fi.ftLastAccessTime = fb->ftLastAccessTime;
        fi.ftLastWriteTime = fb->ftLastWriteTime;
        fi.nFileIndexLow = 0;
        fi.nFileIndexHigh = 0;
        fi.dwVolumeSerialNumber = 0;
    }

    // Name
    if (namelen && path) {
        BOOL isnetwork = FALSE;

        if (0 == wmemcmp(path, L"\\\\?\\", 4)) {
            if (0 == wmemcmp(path, L"\\\\?\\UNC\\", 8)) {
                isnetwork = TRUE;
                path += 4 + 3;                  // Network UNC prefix, "\\?\UNC"
            } else {
                path += 4;                      // UNC prefix "\\?\"
            }
        }

        if (path[0]) {
            if (path[1] == ':') {
                // Root directories
                if (ISSLASH(path[2]) && path[3] == '\0' &&
                        GetDriveTypeW(path) > 1) { // "x:\", root directories
                    dwRootAttributes = FILE_ATTRIBUTE_DIRECTORY;
                }

            } else if (ISSLASH(path[0]) && (isnetwork || ISSLASH(path[1]))) {
                // UNC root (/<server-name>/<path>)
                const wchar_t *slash = w32_wcsslash(path + 1);
                const wchar_t *nextslash = w32_wcsslash(slash ? slash + 1 : NULL);

                if (NULL != slash &&
                        (NULL == nextslash || 0 == nextslash[1])) {
                    dwRootAttributes = FILE_ATTRIBUTE_DIRECTORY;
                }
            }
        }
    }

    // Symbolic links
    if (fi.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        DWORD ReparseTag = 0;

        if (fb) {
            // FindFirstFile() sourced information
            // See: https://docs.microsoft.com/en-us/windows/desktop/FileIO/reparse-point-tags
            if (fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                ReparseTag = fb->dwReserved0;
            }

        } else {
            // Extended file attribute information
            FILE_ATTRIBUTE_TAG_INFO ai;

            if (my_GetFileInformationByHandleEx(handle, FileAttributeTagInfo, &ai, sizeof(ai))) {
                ReparseTag = ai.ReparseTag;
            }
        }

        if (IO_REPARSE_TAG_SYMLINK == ReparseTag) {
            mode = S_IFLNK;                     // Window symbolic link

#if defined(IO_REPARSE_TAG_LX_SYMLINK)
        } else if (IO_REPARSE_TAG_LX_SYMLINK == ReparseTag) {
            mode = S_IFLNK;                     // WSL symbolic link
                // Note: not accessible from a window's client.
#endif

#if defined(IO_REPARSE_TAG_AF_UNIX)
        } else if (IO_REPARSE_TAG_AF_UNIX == ReparseTag) {
            mode = S_IFSOCK;
#endif

        } else if (handle && (IO_REPARSE_TAG_MOUNT_POINT == ReparseTag || 0 == ReparseTag)) {
            BYTE *reparseBuffer;
            DWORD dwret = 0;

            reparseBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_REPARSE_SIZE);
            if (reparseBuffer &&
                    DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT,
                            NULL, 0, (LPVOID)reparseBuffer, MAX_REPARSE_SIZE, &dwret, NULL)) {
                const REPARSE_DATA_BUFFER *rdb = (const REPARSE_DATA_BUFFER *)reparseBuffer;

                if (IsReparseTagMicrosoft(rdb->ReparseTag)) {
                    switch (rdb->ReparseTag) {
                    case IO_REPARSE_TAG_SYMLINK:
#if defined(IO_REPARSE_TAG_LX_SYMLINK)
                    case IO_REPARSE_TAG_LX_SYMLINK:
#endif
                        mode = S_IFLNK;
                            // Note: unexpected
                        break;
#if defined(IO_REPARSE_TAG_AF_UNIX)
                    case IO_REPARSE_TAG_AF_UNIX:
                        mode = S_IFSOCK;
                        break;
#endif
                    case IO_REPARSE_TAG_MOUNT_POINT:
                        // NOTE:
                        // Directory reparse-points are hidden, as these are closer to hard-links
                        // other link types are re-classed as symlinks.
                        if (rdb->MountPointReparseBuffer.SubstituteNameLength > 0) {
                            const size_t offset = rdb->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
                            const wchar_t *mount = rdb->MountPointReparseBuffer.PathBuffer + offset;

                            if (0 == wmemcmp(mount, L"\\??\\", 4) &&
                                    0 != wmemcmp(mount, L"\\??\\Volume{", 11)) {
                                mode = S_IFLNK; /* not a volume mount point -- hard-link */
                            }
                        }
                        break;
                    }
                }
            }
            HeapFree(GetProcessHeap(), 0, reparseBuffer);
        }
    }

    // Extended file-type
    magic[0] = 0;
#if defined(DO_FILEMAGIC)
    if (HasExtension(fullname) == NULL) {       // read file magic; regular files only
        /*
         *  Performed on files without an extension to determine whether an exec script
         */
        DWORD count = 0;

        if (0 == (fb.dwFileAttributes &
                (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_TEMPORARY |
                    FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_OFFLINE | FILE_ATTRIBUTE_ENCRYPTED))) {
            (void) ReadFile(handle, magic, sizeof(magic) - 1, &count, NULL);
        }

        magic[count] = 0;                       // null terminate magic buffer
    }
#endif  //DO_FILEMAGIC

    // Apply results
    StatOwner(sb, fi.dwFileAttributes, handle);

    StatAttributes(sb, mode, fi.dwFileAttributes|dwRootAttributes, fullname, magic);
    if (mode == S_IFSOCK) {
        fi.nFileSizeLow = fi.nFileSizeHigh = 0;
    }

    StatTimes(sb, &fi.ftCreationTime, &fi.ftLastAccessTime, &fi.ftLastWriteTime);

    StatSize(sb, fi.nFileSizeLow, fi.nFileSizeHigh);

    StatDevice(sb, fi.dwVolumeSerialNumber, fi.nFileIndexLow, fi.nFileIndexHigh, fullname);

    if (fi.nNumberOfLinks > 0) {
         StatLinks(sb, (NLINK_T)fi.nNumberOfLinks);
    }

    return TRUE;
}


//
//  w32_iostricmp ---
//      Internal stricmp implementation.
//

int
w32_iostricmp(const char *s1, const char *s2)
{
    char a = 0, b = 0;

    do {
        a = *s1++; if (a < 0x7f) a = tolower(a);
        b = *s2++; if (b < 0x7f) b = tolower(b);
        if (a != b) {
            return a - b;
        }
    } while (a);
    return 0;
}


//
//  w32_iostrnicmp ---
//      Internal strnicmp implementation.
//

int
w32_iostrnicmp(const char *s1, const char *s2, int slen)
{
    char a = 0, b = 0;

    if (slen > 0) {
        do {
            a = *s1++; if (a < 0x7f) a = tolower(a);
            b = *s2++; if (b < 0x7f) b = tolower(b);
            if (a != b) {
                return a - b;
            }
            if (0 == --slen) {
                break;
            }
        } while (a);
    }
    return 0;
}


//
//  w32_iowstricmp ---
//      Internal stricmp implementation, for wide-char values.
//

int
w32_iowstricmp(const wchar_t *s1, const char *s2)
{
    wchar_t a = 0, b = 0;

    do {
        a = *s1++; if (a < 0x7f) a = tolower((char)a);
        b = *s2++; if (b < 0x7f) b = tolower((char)b);
        if (a != b) {
            return a - b;
        }
    } while (a);
    return 0;
}


//
//  w32_iowstrnicmp ---
//      Internal strnicmp implementation, for wide-char values.
//

int
w32_iowstrnicmp(const wchar_t *s1, const char *s2, int slen)
{
    wchar_t a = 0, b = 0;

    if (slen > 0) {
        do {
            a = *s1++; if (a < 0x7f) a = tolower((char)a);
            b = *s2++; if (b < 0x7f) b = tolower((char)b);
            if (a != b) {
                return a - b;
            }
            if (0 == --slen) {
                break;
            }
        } while (a);
    }
    return 0;
}

/*end*/
