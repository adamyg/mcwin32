#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_dir_c, "$Id: w32_dir.c,v 1.17 2021/05/23 10:23:11 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 directory system calls.
 *
 *      mkdir, rmdir, chdir
 *
 * Copyright (c) 2007, 2012 - 2021 Adam Young.
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

#include "win32_internal.h"
#include "win32_io.h"

#include <sys/stat.h>
#include <ctype.h>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#include <assert.h>
#include <unistd.h>

#if defined(_MSC_VER)
#pragma warning(disable : 4244) // conversion from 'xxx' to 'xxx', possible loss of data
#pragma warning(disable : 4312) // type cast' : conversion from 'xxx' to 'xxx' of greater size
#endif

#include "win32_direct.h"

int                     x_w32_cwdn = 0;         /* current/last working drive number, A=1 etc */
const char *            x_w32_cwdd[26] = {0};   /* current working directory, per drive */
const char *            x_w32_vfscwd = NULL;    /* virtual UNC path, if any */

static int              set_root_directoryA(const char *path);
static int              set_root_directoryW(const wchar_t *path);
static int              set_vfs_directoryA(const char *path);
static int              set_vfs_directoryW(const wchar_t *path);
static void             cache_directory(void);


/*
//  NAME
//
//      mkdir - make a directory
//
//  SYNOPSIS
//
//      #include <sys/stat.h>
//
//      int mkdir(const char *path, mode_t mode);
//
//  DESCRIPTION
//
//      The mkdir() function shall create a new directory with name path. The file
//      permission bits of the new directory shall be initialized from mode. These file
//      permission bits of the mode argument shall be modified by the process' file
//      creation mask.
//
//      When bits in mode other than the file permission bits are set, the meaning of these
//      additional bits is implementation-defined.
//
//      The directory's user ID shall be set to the process' effective user ID. The
//      directory's group ID shall be set to the group ID of the parent directory or to the
//      effective group ID of the process. Implementations shall provide a way to
//      initialize the directory's group ID to the group ID of the parent directory.
//      Implementations may, but need not, provide an implementation-defined way to
//      initialize the directory's group ID to the effective group ID of the calling process.
//
//      The newly created directory shall be an empty directory.
//
//      If path names a symbolic link, mkdir() shall fail and set errno to [EEXIST].
//
//      Upon successful completion, mkdir() shall mark for update the st_atime, st_ctime,
//      and st_mtime fields of the directory. Also, the st_ctime and st_mtime fields of the
//      directory that contains the new entry shall be marked for update.
//
//  RETURN VALUE
//
//      Upon successful completion, mkdir() shall return 0. Otherwise, -1 shall be returned,
//      no directory shall be created, and errno shall be set to indicate the error.
//
//  ERRORS
//
//      The mkdir() function shall fail if:
//
//      [EACCES]
//          Search permission is denied on a component of the path prefix, or write
//          permission is denied on the parent directory of the directory to be created.
//      [EEXIST]
//          The named file exists.
//      [ELOOP]
//          A loop exists in symbolic links encountered during resolution of the path argument.
//      [EMLINK]
//          The link count of the parent directory would exceed {LINK_MAX}.
//      [ENAMETOOLONG]
//          The length of the path argument exceeds {PATH_MAX} or a pathname component is
//          longer than {NAME_MAX}.
//      [ENOENT]
//          A component of the path prefix specified by path does not name an existing
//          directory or path is an empty string.
//      [ENOSPC]
//          The file system does not contain enough space to hold the contents of the new
//          directory or to extend the parent directory of the new directory.
//      [ENOTDIR]
//          A component of the path prefix is not a directory.
//      [EROFS]
//          The parent directory resides on a read-only file system.
//
//      The mkdir() function may fail if:
//
//      [ELOOP]
//          More than {SYMLOOP_MAX} symbolic links were encountered during resolution of the path argument.
//      [ENAMETOOLONG]
//          As a result of encountering a symbolic link in resolution of the path argument,
//          the length of the substituted pathname string exceeded {PATH_MAX}.
*/
LIBW32_API int
w32_mkdir(const char *path, int mode)
{
#if defined(UTF8FILENAMES)
    wchar_t wpath[WIN32_PATH_MAX];

    if (NULL == path) {
        errno = EFAULT;
        return -1;
    }

    if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
        return w32_mkdirW(wpath, mode);
    }

    return -1;

#else

    return w32_mkdirA(path, mode);

#endif  //UTF8FILENAMES
}


LIBW32_API int
w32_mkdirA(const char *path, int mode)
{
    (void) mode;
    if (! CreateDirectoryA(path, NULL)) {
        return w32_errno_set();
    }
    return 0;
}


LIBW32_API int
w32_mkdirW(const wchar_t *path, int mode)
{
    (void) mode;
    if (! CreateDirectoryW(path, NULL)) {
        return w32_errno_set();
    }
    return 0;
}


/*
//  NAME
//
//      chdir - change working directory
//
//  SYNOPSIS
//
//      #include <<unistd.h>
//
//      int chdir(const char *path);
//
//  DESCRIPTION
//
//      The chdir() function shall cause the directory named by the pathname pointed to
//      by the path argument to become the current working directory; that is, the
//      starting point for path searches for pathnames not beginning with '/'.
//
//  RETURN VALUE
//
//      Upon successful completion, 0 shall be returned. Otherwise, -1 shall be returned,
//      the current working directory shall remain unchanged, and errno shall be set to
//      indicate the error.
//
//  ERRORS
//
//      The chdir() function shall fail if:
//
//      [EACCES]
//          Search permission is denied for any component of the pathname.
//
//      [ELOOP]
//          A loop exists in symbolic links encountered during resolution of the path argument.
//
//      [ENAMETOOLONG]
//          The length of the path argument exceeds {PATH_MAX} or a pathname component is
//          longer than { NAME_MAX}.
//
//      [ENOENT]
//          A component of path does not name an existing directory or path is an empty
//          string.
//
//      [ENOTDIR]
//          A component of the pathname is not a directory.
//
//      The chdir() function may fail if:
//
//      [ELOOP]
//          More than { SYMLOOP_MAX} symbolic links were encountered during resolution of
//          the path argument.
//
//      [ENAMETOOLONG]
//          As a result of encountering a symbolic link in resolution of the path argument,
//          the length of the substituted pathname string exceeded { PATH_MAX}.
*/

LIBW32_API int
w32_chdir(const char *path)
{
#if defined(UTF8FILENAMES)
    wchar_t wpath[WIN32_PATH_MAX];

    if (NULL == path) {
        errno = EFAULT;
        return -1;
    }

    if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
        return w32_chdirW(wpath);
    }

    return -1;

#else

    return w32_chdirA(path);

#endif  //UTF8FILENAMES
}


LIBW32_API int
w32_chdirA(const char *path)
{
    BOOL success, isunc = FALSE;
    int root;

    if (NULL == path || !*path) {
        errno = EINVAL;
        return -1;
    }

    if ((root = set_root_directoryA(path)) >= -1) {
        return root;
    }

    success = SetCurrentDirectoryA(path);
    if (! success) {                            // possible shortcut.
        char lnkbuf[WIN32_PATH_MAX];

        w32_errno_set();
        if (w32_lnkexpandA(path, lnkbuf, _countof(lnkbuf), SHORTCUT_TRAILING|SHORTCUT_COMPONENT)) {
            success = SetCurrentDirectoryA(lnkbuf);
            if (! success) {
                w32_errno_set();
            } else {
                isunc = w32_unc_validA(lnkbuf);
            }
        }
    } else {
        isunc = w32_unc_validA(path);
    }

    if (! success) {
        return set_vfs_directoryA(path);
    }

    set_vfs_directoryA(NULL);
    if (! isunc) cache_directory();

    return 0;
}


LIBW32_API int
w32_chdirW(const wchar_t *path)
{
    BOOL success, isunc = FALSE;
    int root;

    if (NULL == path || !*path) {
        errno = EINVAL;
        return -1;
    }

    if ((root = set_root_directoryW(path)) >= -1) {
        return root;
    }

    success = SetCurrentDirectoryW(path);
    if (! success) {                            // possible shortcut.
        wchar_t lnkbuf[WIN32_PATH_MAX];

        w32_errno_set();
        if (w32_lnkexpandW(path, lnkbuf, _countof(lnkbuf), SHORTCUT_TRAILING|SHORTCUT_COMPONENT)) {
            success = SetCurrentDirectoryW(lnkbuf);
            if (! success) {
                w32_errno_set();
            } else {
                isunc = w32_unc_validW(lnkbuf);
            }
        }
    } else {
        isunc = w32_unc_validW(path);
    }

    if (! success) {
        return set_vfs_directoryW(path);
    }

    set_vfs_directoryW(NULL);
    if (! isunc) cache_directory();

    return 0;
}


static int
set_root_directoryA(const char *path)
{
    while (*path) {
        if (! IS_PATH_SEP(*path)) {
            return -2;                          // not root
        }
        ++path;
    }

    /*
     *  chdir("/") behaviour is context specific, meaning goto root of current drive,
     *  mount point or current UNC. Normalisze this behaviour to root of current/last drive.
     *  Also see realpath() and related opendir() usage.
     */
    {   char path[4];
        int driveno = w32_getdrive();

        if (driveno <= 0) driveno = w32_getlastdrive();
        if (driveno <= 0) driveno = w32_getsystemdrive();
        if (driveno > 0) {
            path[0] = driveno + ('A' - 1);
            path[1] = ':';
            path[2] = PATH_SEP;
            path[3] = 0;
            return w32_chdirA(path);
        }
    }

    errno = ENOTDIR;
    return -1;
}


static int
set_root_directoryW(const wchar_t *path)
{
    while (*path) {
        if (! IS_PATH_SEP(*path)) {
            return -2;                          // not root
        }
        ++path;
    }

    /*
     *  Generic chdir("/") behaviour is context specific, meaning goto root of current drive,
     *  mount point or current UNC. Normalisze this behaviour to root of current/last drive.
     *  Also see realpath() and related opendir() usage.
     */
    {   wchar_t path[4];
        int driveno = w32_getdrive();

        if (driveno <= 0) driveno = w32_getlastdrive();
        if (driveno <= 0) driveno = w32_getsystemdrive();
        if (driveno > 0) {
            path[0] = driveno + ('A' - 1);
            path[1] = ':';
            path[2] = PATH_SEP;
            path[3] = 0;
            return w32_chdirW(path);
        }
    }

    errno = ENOTDIR;
    return -1;
}


static int
set_vfs_directoryA(const char *path)
{
    free((void *)x_w32_vfscwd);
    x_w32_vfscwd = NULL;

    if (path) {
        int serverlen;

        if ((serverlen = w32_unc_validA(path)) > 0) {

            serverlen += 4;                     // delimiters
            if (NULL != (x_w32_vfscwd = malloc(serverlen))) {
                char *cursor = (char *)x_w32_vfscwd;
                int i;

                path += 2;                      // "//" or "\\"
                *cursor++ = '/'; *cursor++ = '/';
                for (i = serverlen - 4; i > 0; --i) {
                    *cursor++ = toupper((unsigned char)*path++);
                }
                *cursor++ = '/';
                *cursor = 0;
                assert(cursor <= (x_w32_vfscwd + serverlen));
                return 0;
            }
        }
    }
    return -1;
}


static int
set_vfs_directoryW(const wchar_t *path)
{
    free((void *)x_w32_vfscwd);
    x_w32_vfscwd = NULL;

    if (path) {
        int serverlen;

        if ((serverlen = w32_unc_validW(path)) > 0) {

            serverlen += 4;                     // delimiters
            if (NULL != (x_w32_vfscwd = malloc(serverlen))) {
                char *cursor = (char *)x_w32_vfscwd;
                int i;

                path += 2;                      // "//" or "\\"
                // Valid characters for hostnames are ASCII(7), letters from a to z,
                // the digits from 0 to 9, and the hyphen (-).
                *cursor++ = '/'; *cursor++ = '/';
                for (i = serverlen - 4; i > 0; --i) {
                    *cursor++ = toupper((unsigned char)*path++);
                }
                *cursor++ = '/';
                *cursor = 0;
                assert(cursor <= (x_w32_vfscwd + serverlen));
                return 1;
            }
        }
    }
    return -1;
}


static void
cache_directory()
{
    char t_cwd[WIN32_PATH_MAX] = {0};

    if (w32_getcwd(t_cwd, _countof(t_cwd))) {
        if (isalpha((unsigned char)t_cwd[0]) && ':' == t_cwd[1]) {
            const unsigned driveno = toupper(t_cwd[0]) - 'A';
            char env_var[4] = { "=X:" };

            /*
             *  Cache drive specific directory
             */
            free((char *)x_w32_cwdd[driveno]);
            x_w32_cwdd[driveno] = WIN32_STRDUP(t_cwd);
            x_w32_cwdn = driveno + 1;

            /*
             *  Update the environment (=)
             *      This is required to support the MSVCRT runtime logic based on the current-directory-on-drive
             *      environment variables. Function like (fullpath, spawn, etc) *may* need them to be set.
             *
             *  If associated with a 'drive', the current directory should have the form of the example below:
             *
             *       C:\Program and Settings\users\
             *
             *  so that the environment variable should be of the form:
             *
             *      =C:=C:\Program and Settings\users\
             */
            env_var[1] = toupper(t_cwd[0]);
            w32_unix2dos(t_cwd);
            (void) SetEnvironmentVariableA(env_var, t_cwd);
        }
    }
}


/*
//  NAME
//
//      rmdir - remove a directory
//
//  SYNOPSIS
//
//      #include <unistd.h>
//
//      int rmdir(const char *path);
//
//  DESCRIPTION
//
//      The rmdir() function shall remove a directory whose name is given by path. The
//      directory shall be removed only if it is an empty directory.
//
//      If the directory is the root directory or the current working directory of any
//      process, it is unspecified whether the function succeeds, or whether it shall
//      fail and set errno to [EBUSY].
//
//      If path names a symbolic link, then rmdir() shall fail and set errno to [ENOTDIR].
//
//      If the path argument refers to a path whose final component is either dot or
//      dot-dot, rmdir() shall fail.
//
//      If the directory's link count becomes 0 and no process has the directory open,
//      the space occupied by the directory shall be freed and the directory shall no
//      longer be accessible. If one or more processes have the directory open when the
//      last link is removed, the dot and dot-dot entries, if present, shall be removed
//      before rmdir() returns and no new entries may be created in the directory, but
//      the directory shall not be removed until all references to the directory are
//      closed.
//
//      If the directory is not an empty directory, rmdir() shall fail and set errno to
//      [EEXIST] or [ENOTEMPTY].
//
//      Upon successful completion, the rmdir() function shall mark for update the
//      st_ctime and st_mtime fields of the parent directory.
//
//  RETURN VALUE
//
//      Upon successful completion, the function rmdir() shall return 0. Otherwise, -1
//      shall be returned, and errno set to indicate the error. If -1 is returned, the
//      named directory shall not be changed.
//
//  ERRORS
//
//    The rmdir() function shall fail if:
//
//      [EACCES]
//          Search permission is denied on a component of the path prefix, or write
//          permission is denied on the parent directory of the directory to be removed.
//
//      [EBUSY]
//          The directory to be removed is currently in use by the system or some process
//          and the implementation considers this to be an error.
//
//      [EEXIST] or [ENOTEMPTY]
//          The path argument names a directory that is not an empty directory, or there
//          are hard links to the directory other than dot or a single entry in dot-dot.
//
//      [EINVAL]
//          The path argument contains a last component that is dot.
//
//      [EIO]
//          A physical I/O error has occurred.
//
//      [ELOOP]
//          A loop exists in symbolic links encountered during resolution of the path argument.
//
//      [ENAMETOOLONG]
//          The length of the path argument exceeds {PATH_MAX} or a pathname component
//          is longer than {NAME_MAX}.
//
//      [ENOENT]
//          A component of path does not name an existing file, or the path argument
//          names a nonexistent directory or points to an empty string.
//
//      [ENOTDIR]
//          A component of path is not a directory.
//
//      [EPERM] or [EACCES]
//          The S_ISVTX flag is set on the parent directory of the directory to be
//          removed and the caller is not the owner of the directory to be removed, nor
//          is the caller the owner of the parent directory, nor does the caller have the
//          appropriate privileges. [Option End]
//
//      [EROFS]
//          The directory entry to be removed resides on a read-only file system.
//
//    The rmdir() function may fail if:
//
//      [ELOOP]
//          More than {SYMLOOP_MAX} symbolic links were encountered during resolution of
//          the path argument.
//
//      [ENAMETOOLONG]
//          As a result of encountering a symbolic link in resolution of the path argument,
//          the length of the substituted pathname string exceeded { PATH_MAX}.
*/
LIBW32_API int
w32_rmdir(const char *path)
{
#if defined(UTF8FILENAMES)
    wchar_t wpath[WIN32_PATH_MAX];

    if (NULL == path) {
        errno = EFAULT;
        return -1;
    }

    if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
        return w32_rmdirW(wpath);
    }

    return -1;

#else

    return w32_rmdirA(path);

#endif  //UTF8FILENAMES
}


LIBW32_API int
w32_rmdirA(const char *path)
{
    if (! RemoveDirectoryA(path)) {
        return w32_errno_set();
    }
    return 0;
}


LIBW32_API int
w32_rmdirW(const wchar_t *path)
{
    if (! RemoveDirectoryW(path)) {
        return w32_errno_set();
    }
    return 0;
}


/*
 *  w32_lnkexpandA ---
 *      expand embedded shortcuts.
 */
LIBW32_API BOOL
w32_lnkexpandA(const char *name, char *buf, size_t buflen, unsigned flags)
{
    const size_t length = strlen(name);
    char *t_name;
    BOOL ret = 0;

    if (length > 4 && NULL != (t_name = calloc(sizeof(char), length + 1 /*nul*/))) {
        char *cursor, *end;
        int dots = 0;

        memcpy(t_name, name, length + 1 /*nul*/);

        for (cursor = t_name + length, end = cursor; --cursor >= t_name;) {
            if ('.' == *cursor) {                   // extension
                if (1 == ++dots) {                  // last/trailing
                    if (0 == IO_STRNICMP(cursor, ".lnk", 4) && (cursor + 4) == end) {
                        //
                        //  <shortcut>.lnk
                        //      - attempt expansion, allowing one within any given path.
                        const size_t trailing = length - (end - t_name);
                        const char term = *end;
                        int t_ret;

                        assert((0 == trailing && 0 == term) || (trailing && ('/' == term || '\\' == term)));

                        if (flags & (term ? SHORTCUT_COMPONENT : SHORTCUT_TRAILING)) {

                            *end = 0;               // remove trailing component.
                            if ((t_ret = w32_readlinkA(t_name, buf, buflen)) > 0) {
                                if (buflen > (t_ret + trailing)) {
                                    if (trailing) { // appending trailing component(s).
                                        *end = term, memcpy(buf + t_ret, end, trailing + 1 /*nul*/);
                                    }
                                    ret = 1;        // success.
                                }
                            }
                        }
                        break;  //done
                    }
                }

            } else if ('/' == *cursor || '\\' == *cursor) {
                end  = cursor;                      // new component.
                dots = 0;
            }
        }
        free((void *)t_name);
    }
    return ret;
}


/*
 *  w32_lnkexpandW ---
 *      expand embedded shortcuts.
 */
LIBW32_API BOOL
w32_lnkexpandW(const wchar_t *name, wchar_t *buf, size_t buflen, unsigned flags)
{
    const size_t length = wcslen(name);
    wchar_t *t_name;
    BOOL ret = 0;

    if (length > 4 && NULL != (t_name = calloc(sizeof(wchar_t), length + 1 /*nul*/))) {
        wchar_t *cursor, *end;
        int dots = 0;

        wmemcpy(t_name, name, length + 1 /*nul*/);

        for (cursor = t_name + length, end = cursor; --cursor >= t_name;) {
            if ('.' == *cursor) {                   // extension
                if (1 == ++dots) {                  // last/trailing
                    if (0 == IO_WSTRNICMP(cursor, ".lnk", 4) && (cursor + 4) == end) {
                        //
                        //  <shortcut>.lnk
                        //      - attempt expansion, allowing one within any given path.
                        const size_t trailing = length - (end - t_name);
                        const char term = *end;
                        int t_ret;

                        assert((0 == trailing && 0 == term) || (trailing && ('/' == term || '\\' == term)));

                        if (flags & (term ? SHORTCUT_COMPONENT : SHORTCUT_TRAILING)) {

                            *end = 0;               // remove trailing component.
                            if ((t_ret = w32_readlinkW(t_name, buf, buflen)) > 0) {
                                if (buflen > (t_ret + trailing)) {
                                    if (trailing) { // appending trailing component(s).
                                        *end = term, wmemcpy(buf + t_ret, end, trailing + 1 /*nul*/);
                                    }
                                    ret = 1;        // success.
                                }
                            }
                        }
                        break;  //done
                    }
                }

            } else if ('/' == *cursor || '\\' == *cursor) {
                end  = cursor;                      // new component.
                dots = 0;
            }
        }
        free((void *)t_name);
    }
    return ret;
}

/*end*/
