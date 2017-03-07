/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 chown() system calls.
 *
 * Copyright (c) 2007, 2012 - 2017 Adam Young.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0501              /* enable xp+ features */
#endif

#include "win32_internal.h"
#include <unistd.h>


/*
//  NAME
//      chown - change owner and group of a file
//  
//  SYNOPSIS
//      #include <unistd.h>
//  
//      int chown(const char *path, uid_t owner, gid_t group);
//  
//  DESCRIPTION
//      The chown() function shall change the user and group ownership of a file.
//  
//      The path argument points to a pathname naming a file. The user ID and group ID of
//      the named file shall be set to the numeric values contained in owner and group, 
//      respectively.
//  
//      Only processes with an effective user ID equal to the user ID of the file or with
//      appropriate privileges may change the ownership of a file. If
//      _POSIX_CHOWN_RESTRICTED is in effect for path:
//  
//          Changing the user ID is restricted to processes with appropriate privileges.
//  
//          Changing the group ID is permitted to a process with an effective user ID equal
//          to the user ID of the file, but without appropriate privileges, if and only if
//          owner is equal to the file's user ID or ( uid_t)-1 and group is equal either to
//          the calling process' effective group ID or to one of its supplementary group IDs.
//  
//      If the specified file is a regular file, one or more of the S_IXUSR, S_IXGRP, or
//      S_IXOTH bits of the file mode are set, and the process does not have appropriate
//      privileges, the set-user-ID (S_ISUID) and set-group-ID (S_ISGID) bits of the file
//      mode shall be cleared upon successful return from chown(). If the specified file is
//      a regular file, one or more of the S_IXUSR, S_IXGRP, or S_IXOTH bits of the file
//      mode are set, and the process has appropriate privileges, it is
//      implementation-defined whether the set-user-ID and set-group-ID bits are altered.
//      If the chown() function is successfully invoked on a file that is not a regular
//      file and one or more of the S_IXUSR, S_IXGRP, or S_IXOTH bits of the file mode are
//      set, the set-user-ID and set-group-ID bits may be cleared.
//  
//      If owner or group is specified as ( uid_t)-1 or ( gid_t)-1, respectively, the
//      corresponding ID of the file shall not be changed. If both owner and group are -1, 
//      the times need not be updated.
//  
//      Upon successful completion, chown() shall mark for update the st_ctime field of the
//      file.
//  
//  RETURN VALUE
//  
//      Upon successful completion, 0 shall be returned; otherwise, -1 shall be returned
//      and errno set to indicate the error. If -1 is returned, no changes are made in the
//      user ID and group ID of the file.
//  
//  ERRORS
//  
//      The chown() function shall fail if:
//  
//      [EACCES]
//          Search permission is denied on a component of the path prefix.
//  
//      [ELOOP]
//          A loop exists in symbolic links encountered during resolution of the path
//          argument.
//  
//      [ENAMETOOLONG]
//          The length of the path argument exceeds {PATH_MAX} or a pathname component is
//          longer than {NAME_MAX}.
//  
//      [ENOTDIR]
//          A component of the path prefix is not a directory.
//  
//      [ENOENT]
//          A component of path does not name an existing file or path is an empty string.
//  
//      [EPERM]
//          The effective user ID does not match the owner of the file, or the calling
//          process does not have appropriate privileges and _POSIX_CHOWN_RESTRICTED
//          indicates that such privilege is required.
//  
//      [EROFS]
//          The named file resides on a read-only file system.
//  
//      The chown() function may fail if:
//  
//      [EIO]
//          An I/O error occurred while reading or writing to the file system.
//  
//      [EINTR]
//          The chown() function was interrupted by a signal which was caught.
//  
//      [EINVAL]
//          The owner or group ID supplied is not a value supported by the implementation.
//  
//      [ELOOP]
//          More than {SYMLOOP_MAX} symbolic links were encountered during resolution of
//          the path argument.
//  
//      [ENAMETOOLONG]
//          As a result of encountering a symbolic link in resolution of the path argument, 
//          the length of the substituted pathname string exceeded {PATH_MAX}.
//  
*/
int
chown(const char *fname, uid_t uid, gid_t gid)
{
    __PUNUSED(fname);
    __PUNUSED(uid);
    __PUNUSED(gid);
    return 0;
}
