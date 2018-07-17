/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 directory system calls.
 *
 *      mkdir, rmdir, chdir
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
 */

#include "win32_internal.h"
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>

#if defined(_MSC_VER)
#pragma warning(disable : 4244) // conversion from 'xxx' to 'xxx', possible loss of data
#pragma warning(disable : 4312) // type cast' : conversion from 'xxx' to 'xxx' of greater size
#endif
const char *            x_w32_cwdd[26];         /* current working directory, per drive */

const char *            x_w32_vfscwd = NULL;    /* virtual UNC path, if any */


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
int
w32_mkdir(const char *path, int mode)
{
    (void) mode;
    if (! CreateDirectoryA(path, NULL)) {
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
int
w32_chdir(const char *path)
{
    char t_path[1024];

    if (! SetCurrentDirectoryA(path)) {
        int serverlen = w32_root_unc(path);

        if (serverlen > 0) {                    // UNC root path (//servername/)
	    free((void *)x_w32_vfscwd);
	    if (NULL != (x_w32_vfscwd = malloc(serverlen + 4))) {
                char *cursor = (char *)x_w32_vfscwd;

                path += 2;
                *cursor++ = '/';
                *cursor++ = '/';
                while (serverlen-- > 0) {
                    *cursor++ = toupper((unsigned char)*path++);
                }
                *cursor++ = '/';
                *cursor = 0;
                return 0;
            }
        }
        return w32_errno_set();
    }

    free((void *)x_w32_vfscwd), x_w32_vfscwd = NULL;

    if (!(('/' == path[0] && '/' == path[1]) || // UNC paths
                ('\\' == path[0] && '\\' == path[1]))) {

        if (w32_getcwd(t_path, sizeof(t_path))) {
            if (isalpha((unsigned char)t_path[0]) && ':' == t_path[1]) {
                const unsigned nDrive =
                        toupper(t_path[0]) - 'A';
                char env_var[4] = { "=X:" };

                /*
                *   Cache drive specific directory
                */
                free((char *)x_w32_cwdd[nDrive]);
                x_w32_cwdd[nDrive] = WIN32_STRDUP(t_path);

                /*
                *   Update the environment (=)
                *       This is required to support the MSVCRT runtime logic based on
                *       the current-directory-on-drive environment variables. Function
                *       like (fullpath, spawn, etc) *may* need them to be set.
                *
                *   If associated with a 'drive', the current directory should
                *   have the form of the example below:
                *       
                *       C:\Program and Settings\users\
                *
                *   so that the environment variable should be of the form:
                *
                *       =C:=C:\Program and Settings\users\
                */
                env_var[1] = toupper(t_path[0]);
                w32_unix2dos(t_path);
                (void) SetEnvironmentVariableA(env_var, t_path);
            }
        }
    }
    return 0;
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
int
w32_rmdir(const char *path)
{
    if (! RemoveDirectoryA(path)) {
        return w32_errno_set();
    }
    return 0;
}


/*
 *  w32_root_unc ---
 *      determine if the specific path is a UNC root (i.e. //servername[/])
 */
int
w32_root_unc(const char *path)
{
    if (('/' == path[0] && '/' == path[1]) ||   // UNC prefix?
            ('\\' == path[0] && '\\' == path[1])) {

        const char *slash = w32_strslash(path + 2);

        if (NULL == slash || 0 == slash[1]) {

            const size_t serverlen =            // servername length
                    (slash ? (slash - (path + 2)) : strlen(path + 2));

            if (serverlen > 0) {

                char computerName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
                DWORD computerSz = sizeof(computerName);

                                                // local server ?
                if (GetComputerNameA(computerName, &computerSz)) {
                    if (serverlen == computerSz &&
                            0 == _strnicmp(path + 2, computerName, serverlen)) {
                        return (int)serverlen;
                    }
                }
            }
        }
    }
    return 0;
}
/*end*/
