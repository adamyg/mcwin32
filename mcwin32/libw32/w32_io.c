#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_io_c, "$Id: w32_io.c,v 1.13 2018/10/18 22:39:26 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 system io functionality
 *
 *      stat, lstat, fstat, readlink, symlink, open
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

#include <assert.h>

#include "win32_internal.h"
#include "win32_ioctl.h"

#include "win32_misc.h"
#include "win32_io.h"

#if defined(__WATCOMC__)
#include <shellapi.h>                           /* SHSTDAPI */
#endif
#include <shlobj.h>                             /* shell interface */
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

typedef DWORD (WINAPI *GetFinalPathNameByHandleA_t)(
                        HANDLE hFile, LPSTR lpszFilePath, DWORD length, DWORD dwFlags);

typedef BOOL  (WINAPI *CreateSymbolicLinkA_t)(
                        LPCSTR lpSymlinkFileName, LPCSTR lpTargetFileName, DWORD  dwFlags);

typedef BOOL  (WINAPI *GetVolumeInformationByHandleW_t)(
                        HANDLE hFile, LPWSTR lpVolumeNameBuffer, DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber,
                            LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, LPWSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize);

static DWORD                my_GetFinalPathNameByHandle(HANDLE handle, char *name, int length);
static DWORD WINAPI         my_GetFinalPathNameByHandleImp(HANDLE handle, LPSTR name, DWORD length, DWORD dwFlags);

static BOOL                 my_CreateSymbolicLink(const char *lpSymlinkFileName, const char *lpTargetFileName, DWORD dwFlags);
static BOOL WINAPI          my_CreateSymbolicLinkImp(LPCSTR lpSymlinkFileName, LPCSTR lpTargetFileName, DWORD dwFlags);
static BOOL                 isshortcut(const char *name);

static void                 ApplyAttributes(struct stat *sb, const DWORD dwAttr, const char *name, const char *magic);
static void                 ApplyTimes(struct stat *sb, const FILETIME *ftCreationTime,
                                            const FILETIME *ftLastAccessTime, const FILETIME *ftLastWriteTime);
static void                 ApplySize(struct stat *sb, const DWORD nFileSizeLow, const DWORD nFileSizeHigh);
static time_t               ConvertTime(const FILETIME *ft);

static BOOL                 IsExec(const char *name, const char *magic);
static const char *         HasExtension(const char *name);
static BOOL                 IsExtension(const char *name, const char *ext);

static int                  Readlink(const char *path, const char **suffixes, char *buf, int maxlen);
static int                  ReadShortcut(const char *name, char *buf, int maxlen);
static int                  CreateShortcut(const char *link, const char *name, const char *working, const char *desc);
static int                  Stat(const char *name, struct stat *sb);

static BOOL                 my_GetVolumeInformationByHandle(HANDLE handle, DWORD *serialno, DWORD *flags);
static BOOL WINAPI          my_GetVolumeInformationByHandleImp(HANDLE, LPWSTR, DWORD, LPDWORD, LPDWORD, LPDWORD, LPWSTR, DWORD);

static const char *         suffixes_null[] = {
    "", NULL
    };
static const char *         suffixes_default[] = {
    "", ".lnk", NULL
    };


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
//      the area pointed to by the buf argument. The path argument points to a pathname
//      naming a file. Read, write, or execute permission of the named file is not
//      required. An implementation that provides additional or alternate file access
//      control mechanisms may, under implementation-defined conditions, cause stat() to
//      fail. In particular, the system may deny the existence of the file specified by path.
//
//      If the named file is a symbolic link, the stat() function shall continue pathname
//      resolution using the contents of the symbolic link, and shall return information
//      pertaining to the resulting file if the file exists.
//
//      The buf argument is a pointer to a stat structure, as defined in the <sys/stat.h>
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
    char symbuf[WIN32_PATH_MAX];
    int ret = 0;

    if (NULL == path || NULL == sb) {
        ret = -EFAULT;

    } else {
        (void) memset(sb, 0, sizeof(struct stat));
        if ((ret = Readlink(path, (void *)-1, symbuf, sizeof(symbuf))) > 0) {
            path = symbuf;
        }
    }
    if (ret < 0 || (ret = Stat(path, sb)) < 0) {
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
//      The lstat() function shall be equivalent to stat(), except when path refers to a
//      symbolic link. In that case lstat() shall return information about the link, while
//      stat() shall return information about the file the link references.
//
//      For symbolic links, the st_mode member shall contain meaningful information when
//      used with the file type macros, and the st_size member shall contain the length of
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
    int ret = 0;

    if (path == NULL || sb == NULL) {
        ret = -EFAULT;
    }

    if (ret < 0 || (ret = Stat(path, sb)) < 0) {
        errno = -ret;
        return (-1);
    }
    return (0);
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
//      the file descriptor fildes, and shall write it to the area pointed to by buf.
//
//      If fildes references a shared memory object, the implementation shall update in the
//      stat structure pointed to by the buf argument only the st_uid, st_gid, st_size, and
//      st_mode fields, and only the S_IRUSR, S_IWUSR, S_IRGRP, S_IWGRP, S_IROTH, and
//      S_IWOTH file permission bits need be valid. The implementation may update other
//      fields and flags. [Optional]
//
//      If fildes references a typed memory object, the implementation shall update in the
//      stat structure pointed to by the buf argument only the st_uid, st_gid, st_size, and
//      st_mode fields, and only the S_IRUSR, S_IWUSR, S_IRGRP, S_IWGRP, S_IROTH, and
//      S_IWOTH file permission bits need be valid. The implementation may update other
//      fields and flags. [Optional]
//
//      The buf argument is a pointer to a stat structure, as defined in <sys/stat.h>, into
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
w32_fstat(int fd, struct stat *sb)
{
    HANDLE handle;
    int ret = 0;

    if (NULL == sb) {
        ret = -EFAULT;

    } else {
        memset(sb, 0, sizeof(struct stat));

        if (fd < 0) {
            ret = -EBADF;

        } else if ((handle = ((HANDLE) _get_osfhandle(fd))) == INVALID_HANDLE_VALUE) {
                                                // socket, a named pipe, or an anonymous pipe.
            if (fd > WIN32_FILDES_MAX && FILE_TYPE_PIPE == GetFileType((HANDLE) fd)) {
                sb->st_mode |= S_IRUSR | S_IRGRP | S_IROTH;
                sb->st_mode |= S_IFIFO;
                sb->st_dev = sb->st_rdev = 1;

            } else {
                ret = -EBADF;
            }

        } else {
            const DWORD ftype = GetFileType(handle);

            switch (ftype) {
            case FILE_TYPE_DISK: {              // disk file.
                    BY_HANDLE_FILE_INFORMATION fi = {0};

                    if (GetFileInformationByHandle(handle, &fi)) {
                        char t_name[MAX_PATH], *name = NULL;

                        if (my_GetFinalPathNameByHandle(handle, t_name, sizeof(t_name))) {
                            name = t_name;      // resolved filename.
                        }
                        ApplyAttributes(sb, fi.dwFileAttributes, name, NULL);
                        ApplyTimes(sb, &fi.ftCreationTime, &fi.ftLastAccessTime, &fi.ftLastWriteTime);
                        ApplySize(sb, fi.nFileSizeLow, fi.nFileSizeHigh);
                        if (fi.nNumberOfLinks > 0) {
                            sb->st_nlink = (int)fi.nNumberOfLinks;
                        }
                    }
                }
                break;

            case FILE_TYPE_CHAR:                // character file, typically an LPT device or a console.
            case FILE_TYPE_PIPE:                // socket, a named pipe, or an anonymous pipe.
                sb->st_mode |= S_IRUSR | S_IRGRP | S_IROTH;
                if (FILE_TYPE_PIPE == ftype) {
                    sb->st_mode |= S_IFIFO;
                } else {
                    sb->st_mode |= S_IFCHR;
                }
                sb->st_dev = sb->st_rdev = 1;
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


static DWORD
my_GetFinalPathNameByHandle(HANDLE handle, char *path, int length)
{
    static GetFinalPathNameByHandleA_t x_GetFinalPathNameByHandleA = NULL;

    if (NULL == x_GetFinalPathNameByHandleA) {
        HINSTANCE hinst;                        // Vista+

        if (0 == (hinst = LoadLibraryA("Kernel32")) ||
                0 == (x_GetFinalPathNameByHandleA =
                            (GetFinalPathNameByHandleA_t)GetProcAddress(hinst, "GetFinalPathNameByHandleA"))) {
                                                // XP+
            x_GetFinalPathNameByHandleA = my_GetFinalPathNameByHandleImp;
            (void)FreeLibrary(hinst);
        }
    }

#ifndef FILE_NAME_NORMALIZED
#define FILE_NAME_NORMALIZED 0
#define VOLUME_NAME_DOS 0
#endif

    return x_GetFinalPathNameByHandleA(handle, path, length, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
}


static DWORD WINAPI
my_GetFinalPathNameByHandleImp(HANDLE handle, char *path, DWORD length, DWORD flags)
{                                               // Determine underlying file-name, XP+
    HANDLE map;
    DWORD ret;

    __CUNUSED(flags)

    if (0 == GetFileSize(handle, &ret) && 0 == ret) {
        return 0;                               // Cannot map a file with a length of zero
    }

    ret = 0;

    if (0 != (map = CreateFileMapping(handle, NULL, PAGE_READONLY, 0, 1, NULL))) {
        LPVOID pmem = MapViewOfFile(map, FILE_MAP_READ, 0, 0, 1);

        if (pmem) {                             // XP+
            if (GetMappedFileNameA(GetCurrentProcess(), pmem, path, length)) {
                //
                //  Translate path with device name to drive letters, for example:
                //
                //      \Device\Volume4\<path>
                //
                //      => F:\<path>
                //
                char t_drives[512] = {0};       // 27*4 ...

                if (GetLogicalDriveStringsA(sizeof(t_drives) - 1, t_drives)) {

                    BOOL found = FALSE;
                    const char *p = t_drives;
                    char t_name[MAX_PATH];
                    char t_drive[3] = " :";

                    do {                        // Look up each device name
                        t_drive[0] = *p;

                        if (QueryDosDeviceA(t_drive, t_name, sizeof(t_name) - 1)) {
                            const size_t namelen = strlen(t_name);

                            if (namelen < MAX_PATH) {
                                found = (0 == _strnicmp(path, t_name, namelen) &&
                                                path[namelen] == '\\');

                                if (found) {
                                    //
                                    //  Reconstruct path, replacing device path with DOS path
                                    //
                                    char t_path[MAX_PATH];
                                    size_t len;

                                    len = _snprintf(t_path, sizeof(t_path), "%s%s", t_drive, path + namelen);
                                    t_path[sizeof(t_path) - 1] = 0;
                                    memcpy(path, (const char *)t_path, (len < length ? len + 1 : length));
                                    path[length - 1] = 0;
                                    ret = 1;
                                }
                            }
                        }

                        while (*p++);           // Go to the next NULL character.

                    } while (!found && *p);     // end of string
                }
                ret = 1;
            }
            (void) UnmapViewOfFile(pmem);
        }
        (void) CloseHandle(map);
    }
    return ret;
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
//
//  NOTES:
//      Portable applications should not assume that the returned contents of
//      the symblic link are null-terminated.
*/
LIBW32_API int
w32_readlink(const char *path, char *buf, int maxlen)
{
    int ret = 0;

    if (path == NULL || buf == NULL) {
        ret = -EFAULT;

    } else if (0 == (ret = Readlink(path, (void *)-1, buf, maxlen))) {
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
    int ret = -1;

    if (name1 == NULL || name2 == NULL) {
        errno = EFAULT;

    } else if (!*name1 || !*name2) {
        errno = ENOENT;

    } else if (strlen(name1) > MAX_PATH || strlen(name2) > MAX_PATH) {
        errno = ENAMETOOLONG;

    } else if (GetFileAttributesA(name2) != INVALID_FILE_ATTRIBUTES /*0xffffffff*/) {
        errno = EEXIST;

    } else {
        ret = 0;
        if (isshortcut(name2)) {                // possible shortcut (xxx.lnk)
            if (! CreateShortcut(name2, name1, "", name1)) {
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
            if (! my_CreateSymbolicLink(/*target-link*/name2, /*existing*/name1, flag)) {
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
my_CreateSymbolicLink(const char *lpSymlinkFileName, const char *lpTargetFileName, DWORD dwFlags)
{
    static CreateSymbolicLinkA_t x_CreateSymbolicLinkA = NULL;

    if (NULL == x_CreateSymbolicLinkA) {
        HINSTANCE hinst;                        // Vista+

        if (0 == (hinst = LoadLibraryA("Kernel32")) ||
                0 == (x_CreateSymbolicLinkA =
                        (CreateSymbolicLinkA_t)GetProcAddress(hinst, "CreateSymbolicLinkA"))) {
                                                // XP+
            x_CreateSymbolicLinkA = my_CreateSymbolicLinkImp;
            (void) FreeLibrary(hinst);
        }
    }
    return x_CreateSymbolicLinkA(lpSymlinkFileName, lpTargetFileName, dwFlags);
}


static BOOL WINAPI
my_CreateSymbolicLinkImp(LPCSTR lpSymlinkFileName, LPCSTR lpTargetFileName, DWORD dwFlags)
{
    __CUNUSED(lpSymlinkFileName)
    __CUNUSED(lpTargetFileName)
    __CUNUSED(dwFlags)

    SetLastError(ERROR_NOT_SUPPORTED);          // not implemented
    return FALSE;
}


static BOOL
isshortcut(const char *name)
{
    const size_t len = strlen(name);
    const char *cursor;

    for (cursor = name + len; --cursor >= name;) {
        if (*cursor == '.') {                   // extension
            return (*++cursor && 0 == WIN32_STRICMP(cursor, "lnk"));
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
//      set according to the value of oflag.
//
//      Values for oflag are constructed by a bitwise-inclusive OR of flags from the
//      following list, defined in <fcntl.h>. Applications shall specify exactly one of the
//      first three values (file access modes) below in the value of oflag:
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
//      If both the O_SYNC and O_DSYNC flags are set, the effect is as if only the O_SYNC
//      flag was set. [Option End]
//
//      If path refers to a STREAMS file, oflag may be constructed from O_NONBLOCK OR'ed
//      with either O_RDONLY, O_WRONLY, or O_RDWR. Other flag values are not applicable to
//      STREAMS devices and shall have no effect on them. The value O_NONBLOCK affects the
//      operation of STREAMS drivers and certain functions applied to file descriptors
//      associated with STREAMS files. For STREAMS drivers, the implementation of
//      O_NONBLOCK is device-specific. [Option End]
//
//      If path names the master side of a pseudo-terminal device, then it is unspecified
//      whether open() locks the slave side so that it cannot be opened. Conforming
//      applications shall call unlockpt() before opening the slave side. [Option End]
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
    char symbuf[WIN32_PATH_MAX];
    int mode = 0, ret = 0;

    if (O_CREAT & oflag) {
        va_list ap;
        va_start(ap, oflag);
        mode = va_arg(ap, int);
        va_end(ap);
    }

    if (0 == WIN32_STRICMP(path, "/dev/null")) {
        /*
         *  Redirect ..
         */
        path = "NUL";

    } else if ((ret = Readlink(path, (void *)-1, symbuf, sizeof(symbuf))) < 0) {
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
            oflag &= ~(O_CREAT|O_EXCL);         // link must exist !
        }
        path = symbuf;
    }

    if (ret < 0) {
        errno = -ret;
        return -1;
    }

    // true open
#undef _open
    return w32_sockfd_limit(_open(path, oflag, mode));
}


/*
 *  Convert WIN attributes to thier Unix counterparts.
 */
static void
ApplyAttributes(struct stat *sb,
        const DWORD dwAttributes, const char *name, const char *magic)
{
    const char *p;
    char symbuf[WIN32_PATH_MAX];
    mode_t mode = 0;

    /*
     *  mode
     */

    /* S_IFxxx */
    if (NULL != (p = name) && p[0] && p[1] == ':') {
        p += 2;                                 /* remove drive */
    }

    if (p && (!p[0] || (ISSLASH(p[0]) && !p[1]))) {
        mode |= S_IFDIR|S_IEXEC;                /* handle root directory explicity */

    } else if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        mode |= S_IFDIR|S_IEXEC;                /* directory */

    } else if ((FILE_ATTRIBUTE_REPARSE_POINT & dwAttributes) ||
                   (name && Readlink(name, NULL, symbuf, sizeof(symbuf)) > 0)) {
        mode |= S_IFLNK;                        /* link */

    } else {
        mode |= S_IFREG;                        /* normal file */
    }

    /* rw */
    mode |= (dwAttributes & FILE_ATTRIBUTE_READONLY) ?
                S_IREAD : (S_IREAD|S_IWRITE);

    /* x */
    if (0 == (mode & S_IEXEC)) {
        if (name && IsExec(name, magic)) {
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

    /*
     *  apply
     */
    sb->st_mode = mode;
    if (sb->st_nlink <= 0) {                    /* assigned by caller? */
        sb->st_nlink = 1;
    }

    if (dwAttributes & FILE_ATTRIBUTE_SYSTEM) {
        sb->st_uid = sb->st_gid = 0;            /* root */
    } else {                                    /* current user */
        sb->st_uid = w32_getuid();
        sb->st_gid = w32_getgid();
    }
}


static void
ApplyTimes(struct stat *sb,
        const FILETIME *ftCreationTime, const FILETIME *ftLastAccessTime, const FILETIME *ftLastWriteTime)
{
    sb->st_mtime = ConvertTime(ftLastWriteTime);

    if (0 == (sb->st_atime = ConvertTime(ftLastAccessTime))) {
        sb->st_atime = sb->st_mtime;
    }

    if (0 == (sb->st_ctime = ConvertTime(ftCreationTime))) {
        sb->st_ctime = sb->st_mtime;
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



static void
ApplySize(struct stat *sb, const DWORD nFileSizeLow, const DWORD nFileSizeHigh)
{
    __CUNUSED(nFileSizeHigh)

#if (HAVE_STRUCT_STAT_ST_BLKSIZE)
    sb->st_blksize = 512;
#endif

    sb->st_size = nFileSizeLow;
/*TODO
 *  sb->st_size =
 *      (((__int64)nFileSizeHigh) << 32) + nFileSizeLow;
 */

#if (HAVE_STRUCT_STAT_ST_BLOCKS)
    if (0 == sb->st_size) {
        sb->st_blocks = 0;
    } else {
#if (HAVE_STRUCT_STAT_ST_BLKSIZE)
        blkcnt_t ioblocks = 1 + (sb->st_size - 1) / sb->st_blksize;
        blksize_t ioblock_size = 1 + (sb->st_blksize - 1) / 512;
        sb->st_blocks = ioblocks * ioblock_size;
#else
        sb->st_blocks = 1 + (sb->st_size - 1) / 512;
#endif /*HAVE_STRUCT_STAT_ST_BLKSIZE*/
    }
#endif /*HAVE_STRUCT_STAT_ST_BLOCKS*/
}


/*
 *  Is the file an executFileType
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
IsExec(const char *name, const char *magic)
{
    DWORD driveType;
    const char *dot;
    int idx = -1;

    if ((dot = HasExtension(name)) != NULL) {   /* check well-known extensions */
        for (idx = EXEC_ASSUME-1; idx >= 0; idx--)
            if (WIN32_STRICMP(dot, exec_assume[idx]) == 0) {
                return TRUE;
            }

        for (idx = EXEC_EXCLUDE-1; idx >= 0; idx--)
            if (WIN32_STRICMP(dot, exec_exclude[idx]) == 0) {
                break;
            }
        }

        if (magic) {                            /* #! */
            if (magic[0] == '#' && magic[1] == '!' && magic[2]) {
                 /*
                  * #! <path> [options]\n
                  */
                const char *exec = magic + 2;
                int isscript = 0, len = -1;

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
                return isscript;
            }
        }

    if (-1 == idx) {                            /* only local drives */
        if ((driveType = GetDriveTypeA(name)) == DRIVE_FIXED) {
            DWORD binaryType = 0;

            if (GetBinaryTypeA(name, &binaryType)) {
                /*
                switch(binaryType) {
                case SCS_32BIT_BINARY:          // 32-bit Windows-based application
                case SCS_64BIT_BINARY:          // 64-bit Windows-based application
                case SCS_DOS_BINARY:            // MS-DOS – based application
                case SCS_OS216_BINARY:          // 16-bit OS/2-based application
                case SCS_PIF_BINARY:            // PIF file that executes an MS-DOS based application
                case SCS_POSIX_BINARY:          // A POSIX based application
                case SCS_WOW_BINARY:            // A 16-bit Windows-based application
                }*/
                return TRUE;
            }
        }
    }
    return FALSE;
}


static const char *
HasExtension(const char *name)
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


static BOOL
IsExtension(const char *name, const char *ext)
{
    const char *dot;

    if (ext && (dot = HasExtension(name)) != NULL &&
            WIN32_STRICMP(dot, ext) == 0) {
        return TRUE;
    }
    return FALSE;
}


static int
Readlink(const char *path, const char **suffixes, char *buf, int maxlen)
{
    DWORD attrs;
    const char *suffix;
    int length;
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
        /* Concat suffix */
        if (length + (int)strlen(suffix) >= maxlen) {
            ret = -ENAMETOOLONG;
            continue;
        }
        strcpy(buf + length, suffix);           // concat suffix

        /* File attributes */
        if (0xffffffff == (attrs = GetFileAttributesA(buf))) {
            DWORD rc;

            if ((rc = GetLastError()) == ERROR_ACCESS_DENIED ||
                        rc == ERROR_SHARING_VIOLATION) {
                ret = -EACCES;                  // true error ???
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
                                                // possible mount-point.
                if (0 == w32_reparse_read(path, buf, maxlen)) {
                    ret = (int)strlen(buf);
                }
            }

        /* readparse point - symlink/mount-point */
        } else if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) {
            if ((ret = w32_reparse_read(path, buf, maxlen)) < 0) {
                ret = -EIO;
            } else {
                ret = (int)strlen(buf);
            }

        /* shortcut */
        } else if (attrs & FILE_ATTRIBUTE_OFFLINE) {
            ret = -EACCES;                      // wont be able to access

#define SHORTCUT_COOKIE         "L\0\0\0"       // shortcut magic

                                                // cygwin shortcut also syste/rdonly
#define CYGWIN_ATTRS            FILE_ATTRIBUTE_SYSTEM
#define CYGWIN_COOKIE           "!<symlink>"    // old style shortcut

        } else if (IsExtension(buf, ".lnk") ||
                        (attrs & (FILE_ATTRIBUTE_HIDDEN|CYGWIN_ATTRS)) == CYGWIN_ATTRS) {

            SECURITY_ATTRIBUTES sa;
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
                    if ((ret = ReadShortcut (buf, buf, maxlen)) < 0) {
                        ret = -EIO;
                    } else {
                        ret = (int)strlen(buf);
                    }

                // cygwin symlink (old style)
                } else if ((attrs & CYGWIN_ATTRS) && got == sizeof(cookie) &&
                                0 == memcmp(cookie, CYGWIN_COOKIE, sizeof(cookie))) {

                    if (! ReadFile(fh, buf, maxlen, &got, 0)) {
                        ret = -EIO;
                    } else {
                        char *end;

                        if ((end = (char *)memchr(buf, 0, got)) != NULL) {
                            ret = (int)(end - buf); // find the NUL terminator
                        } else {
                            ret = (int)got;
                        }
                        if (ret == 0) {
                            ret = -EIO;         // hmmm .. empty link specification
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


static const GUID   x_CLSID_ShellLink   =       // local copies; OWC linker crashes otherwise
    { 0x00021401, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
static const IID    x_IID_IShellLink    =
    { 0x000214EE, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
static const IID    x_IID_IPersistFile  =
    { 0x0000010B, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};


/*  Function:       ReadShortcut
 *      Fills the filename and path buffer with relevant information.
 *
 *  Parameters:
 *      name -          name of the link file passed into the function.
 *
 *      path -          the buffer that receives the file's path name.
 *
 *      maxlen -        max length of the 'path' buffer.
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
ReadShortcut(const char *name, char *buf, int maxlen)
{
    HRESULT hres = FALSE;
    WIN32_FIND_DATA wfd;
    IShellLink *pShLink;

    CoInitialize(NULL);

    hres = CoCreateInstance(&x_CLSID_ShellLink, NULL,
                CLSCTX_INPROC_SERVER, &x_IID_IShellLink, (LPVOID *)&pShLink);

    if (SUCCEEDED(hres)) {
        WORD wsz[ MAX_PATH ];
        IPersistFile *ppf;

        hres = pShLink->lpVtbl->QueryInterface(pShLink, &x_IID_IPersistFile, (LPVOID *)&ppf);
        if (SUCCEEDED(hres)) {
            MultiByteToWideChar(CP_ACP, 0, name, -1, wsz, sizeof(wsz));

            hres = ppf->lpVtbl->Load(ppf, wsz, STGM_READ);
            if (SUCCEEDED(hres)) {
/*              if (SUCCEEDED(hres)) {
 *                  hres = pShLink->lpVtbl->Resolve(
 *                              pShLink, 0, SLR_NOUPDATE | SLR_ANY_MATCH | SLR_NO_UI);
 *              }
 */

                hres = pShLink->lpVtbl->GetPath(pShLink, buf, maxlen, &wfd, 0);
                if (!SUCCEEDED(hres) || buf[0] == '\0') {
                    /*
                    *  A document shortcut may only have a description ...
                    *  Also CYGWIN generates this style of link.
                    */
                    hres = pShLink->lpVtbl->GetDescription(pShLink, buf, maxlen);
                    if (buf[0] == '\0')
                        hres = !S_OK;
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
CreateShortcut(const char *link, const char *name, const char *working, const char *desc)
{
    IShellLink *pShLink;
    HRESULT hres;

    CoInitialize(NULL);

    // Get a pointer to the IShellLink interface.
    hres = CoCreateInstance(&x_CLSID_ShellLink, NULL,
                CLSCTX_INPROC_SERVER, &x_IID_IShellLink, (PVOID *) &pShLink);

    if (SUCCEEDED(hres)) {
        IPersistFile* ppf;
        WORD wsz[ MAX_PATH ];

        // Set the path to the shortcut target and add the
        // description.
    //  if (flags & MAXIMIZED)
    //      pShLink->lpVtbl->SetShowCmd(pShLink, SW_SHOW);
        if (name)
            pShLink->lpVtbl->SetPath(pShLink, (LPCSTR) name);
        if (working)
            pShLink->lpVtbl->SetWorkingDirectory(pShLink, (LPCSTR) working);
        if (desc)
            pShLink->lpVtbl->SetDescription(pShLink, (LPCSTR) desc);
    //  if (icon)
    //      pShLink->lpVtbl->SetIconLocation(pShLink, (LPCSTR) icon);

        // Query IShellLink for the IPersistFile interface for saving the
        // shortcut in persistent storage.
        hres = pShLink->lpVtbl->QueryInterface(pShLink, &x_IID_IPersistFile, (PVOID *) &ppf);

        if (SUCCEEDED(hres)) {
            // Ensure that the string is ANSI.
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR) link, -1, wsz, MAX_PATH);

            // Save the link by calling IPersistFile::Save.
            hres = ppf->lpVtbl->Save(ppf, wsz, TRUE);
            ppf->lpVtbl->Release(ppf);
        }

        pShLink->lpVtbl->Release(pShLink);
    }

    CoUninitialize();

    return (SUCCEEDED(hres) ? TRUE : FALSE);
}


/*
 *  Stat() system call
 */
static int
Stat(const char *name, struct stat *sb)
{
    char fullname[WIN32_PATH_MAX] = {0}, *pfname = NULL;
    int flength, ret = -1;
    BOOL domagic = 0;

    if (name == NULL || sb == NULL) {
        ret = -EFAULT;                          /* basic checks */

    } else if (name[0] == '\0' ||
                    (name[1] == ':' && name[2] == '\0')) {
        ret = -ENOENT;                          /* missing directory ??? */

    } else if (strchr(name, '?') || strchr(name, '*')) {
        ret = -ENOENT;                          /* wildcards -- break FindFirstFile() */

    } else if (0 == (flength = GetFullPathNameA(name, sizeof(fullname), fullname, &pfname))) {
        ret = -ENOENT;                          /* parse error */

    } else if (flength >= (int)sizeof(fullname)) {
        ret = -ENAMETOOLONG;                    /* buffer overflow */

    } else {
        HANDLE h = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATAA fb = {0};
        int root = FALSE, drive = 0;

        /*
         *  determine the drive .. used as st_dev
         */
        if (! ISSLASH(fullname[0])) {           /* A=1 .. */
            drive = toupper(fullname[0]) - 'A' + 1;
        }

        /*
         *  retrieve the file details
         */
        memset(sb, 0, sizeof(struct stat));

        if ((h = FindFirstFileA(fullname, &fb)) == INVALID_HANDLE_VALUE) {

            if (((fullname[0] && fullname[1] == ':' &&
                        ISSLASH(fullname[2]) && fullname[3] == '\0')) &&
                    GetDriveTypeA(fullname) > 1) {
                                                /* "x:\" */
                /*
                 *  root directories
                 */
                root = 1;
                ret = 0;

            } else if (ISSLASH(fullname[0]) && ISSLASH(fullname[1])) {
                /*
                 *  root UNC (//servername/xxx)
                 */
                const char *slash = w32_strslash(fullname + 2);
                const char *nextslash = w32_strslash(slash ? slash+1 : NULL);

                ret = -ENOENT;
                if (NULL != slash &&
                        (NULL == nextslash || 0 == nextslash[1])) {
                    root = 2;
                    ret = 0;
                }

            } else {
                /*
                 *  Determine cause
                 */
                DWORD rc = GetLastError(), attrs;

                if (0xffffffff == (attrs = GetFileAttributesA(fullname))) {
                    // Decode error.
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
                    // Junction enountered (e.g C:/Users/Default User --> Default),
                    //  FindFirstFileA() behaviour is by design to stop applications recursively accessing symlinks.
                    fb.dwFileAttributes = attrs;
                    ret = 0;
                } else {
                    // Other conditions.
                    ret = -EIO;
                }
            }

        } else {
            (void) FindClose(h);                /* release find session */
            ret = 0;
        }

        /*
         *  assign results
         */
#if defined(DO_FILEMAGIC)                       /* verify file magic */
        domagic = (HasExtension(fullname) == NULL);
#endif

        while (0 == ret) {
            char magic[1024] = {0};
            BOOL dirsymlink = FALSE;
            DWORD count = 0;
            HANDLE handle;

            if (INVALID_HANDLE_VALUE != (handle =
                    CreateFileA(fullname, (domagic ? GENERIC_READ : 0), 0, NULL, OPEN_EXISTING,
                        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_READONLY, NULL))) {
                //
                //  file information
                //      FILE_FLAG_BACKUP_SEMANTICS      permit directories to be open'ed
                BY_HANDLE_FILE_INFORMATION fi = {0};

                if (GetFileInformationByHandle(handle, &fi)) {
                    if (fi.nNumberOfLinks > 0) {
#if defined(_MSC_VER)                           /* XXX/NOTE */
                        sb->st_nlink = (short)fi.nNumberOfLinks;
#else
                        sb->st_nlink = fi.nNumberOfLinks;
#endif
                    }
                    fb.nFileSizeHigh    = fi.nFileSizeHigh;
                    fb.nFileSizeLow     = fi.nFileSizeLow;
                    fb.ftCreationTime   = fi.ftCreationTime;
                    fb.ftLastAccessTime = fi.ftLastAccessTime;
                    fb.ftLastWriteTime  = fi.ftLastWriteTime;
                    sb->st_ino = w32_ino_gen(fi.nFileIndexLow, fi.nFileIndexHigh);
                        //Note: Generate an inode from nFileIndexHigh and nFileIndexLow, yet warning
                        //  The identifier that is stored in the nFileIndexHigh and nFileIndexLow members is called the file ID.
                        //  Support for file IDs is file system - specific.File IDs are not guaranteed to be unique over time,
                        //  because file systems are free to reuse them.In some cases, the file ID for a file can change over time.
                }

                //
                //  directory symlinks
                //  see: https://docs.microsoft.com/en-us/windows/desktop/FileIO/reparse-point-tags
                if ((fb.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                            (fb.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
                                                /* retrieve reparse details */
                    if (IO_REPARSE_TAG_SYMLINK == fb.dwReserved0) {
                        dirsymlink = TRUE;

                    } else if (IO_REPARSE_TAG_MOUNT_POINT == fb.dwReserved0 || 0 == fb.dwReserved0) {
                        BYTE reparseBuffer[MAX_REPARSE_SIZE];
                            /* XXX: warning: owc crash if = {0} under full optimisation */
                        PREPARSE_DATA_BUFFER rdb = (PREPARSE_DATA_BUFFER)reparseBuffer;
                        DWORD returnedLength = 0;
                        int ret = -1;

                        memset(reparseBuffer, 0, sizeof(MAX_REPARSE_SIZE));

                        if (DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT,
                                NULL, 0, rdb, sizeof(reparseBuffer), &returnedLength, NULL)) {
                            if (IsReparseTagMicrosoft(rdb->ReparseTag)) {
                                switch (rdb->ReparseTag) {
                                case IO_REPARSE_TAG_SYMLINK:
                                    dirsymlink = TRUE;
                                        /*XXX: unexpected */
                                    break;
                                case IO_REPARSE_TAG_MOUNT_POINT:
                                    if (rdb->MountPointReparseBuffer.SubstituteNameLength > 0) {
                                        const size_t offset = rdb->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
                                        const wchar_t* mount = rdb->MountPointReparseBuffer.PathBuffer + offset;

                                        if (0 == memcmp(mount, L"\\??\\", 4 * sizeof(wchar_t)) &&
                                                0 != memcmp(mount, L"\\??\\Volume{", 11 * sizeof(wchar_t))) {
                                            dirsymlink = TRUE;
                                                /* not a volume mount point -- hard-link */
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }

                //
                //  extended file-type
#if defined(DO_FILEMAGIC)
                if (domagic) {                  /* read file magic; regular files only */
                     /* performed on files without an extension to determine whether an exec script */
                     if (0 == (fb.dwFileAttributes &
                                (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_TEMPORARY |
                                 FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_OFFLINE | FILE_ATTRIBUTE_ENCRYPTED))) {
                        (void) ReadFile(handle, magic, sizeof(magic)-1, &count, NULL);
                     }
                }
#endif //DO_MAGIC
                magic[count] = 0;               /* null terminate magic buffer */

                //
                //  dev emulation
                {   DWORD serialno = 0, flags = 0;
                    if (my_GetVolumeInformationByHandle(handle, &serialno, &flags)) {
                        sb->st_dev = serialno;
                    }
                }

                //
                //  inode emulation
                if (0 == sb->st_ino) {
                    sb->st_ino = w32_ino_hash(fullname);
                }

                CloseHandle(handle);

            } else {
#if defined(DO_FILEMAGIC)
                if (domagic) {
                   domagic = 0;
                   continue;                    /* retry without read; eg directories */
                }
#endif //DO_MAGIC

                if (root) {
                    ret = -ENOENT;
                } else if (0 == (sb->st_ino = w32_ino_file(fullname))) {
                    sb->st_ino = w32_ino_hash(fullname);
                }
            }

            if (root) {
                fb.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
            }
            ApplyAttributes(sb, fb.dwFileAttributes, fullname, magic);
            if (dirsymlink) {
                // NOTE: Directory reparse-points are hidden, as these are closer to hard-links
                //          other link types are reclassed as symlinks.
                sb->st_mode &= ~S_IFDIR;        /* can only be one type */
                sb->st_mode |= S_IFLNK;
            }

            ApplyTimes(sb, &fb.ftCreationTime, &fb.ftLastAccessTime, &fb.ftLastWriteTime);
            ApplySize(sb, fb.nFileSizeLow, fb.nFileSizeHigh);

            //  st_dev
            //      This field describes the device on which this file resides.
            //
            //  st_rdev
            //      This field describes the device that this file (inode) represents.
            //
            //  st_ino
            //      This field contains the file's inode number.
            //
            sb->st_rdev = (dev_t)(drive - 1);   /* A=0 ... */

            if (0 == sb->st_dev) {
                sb->st_dev = sb->st_rdev;
                    //XXX: This wont work for reparse-points, hence it is not possible to test in call cases
                    //  as to whether a parent/child directory represent a mount-point or are cross-device.
            }

            break;                              // done
        }
    }
    return ret;
}


//  Retrieves information about the file system and volume associated with the specified file.
//  see: https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getvolumeinformationbyhandlew
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
my_GetVolumeInformationByHandle(HANDLE handle, DWORD *serialno, DWORD *flags)
{
    static GetVolumeInformationByHandleW_t x_GetVolumeInformationByHandleW = NULL;

    if (NULL == x_GetVolumeInformationByHandleW) {
        HINSTANCE hinst;                        // Vista+

        if (0 == (hinst = LoadLibraryA("Kernel32")) ||
                0 == (x_GetVolumeInformationByHandleW =
                        (GetVolumeInformationByHandleW_t)GetProcAddress(hinst, "GetVolumeInformationByHandleW"))) {
                                                // XP+
            x_GetVolumeInformationByHandleW = my_GetVolumeInformationByHandleImp;
            (void) FreeLibrary(hinst);
        }
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

/*end*/
