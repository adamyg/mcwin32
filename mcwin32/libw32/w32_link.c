/* -*- mode: c; indent-width: 4; -*- */
/* 
 * win32 link() system calls.
 *
 * Copyright (c) 2007, 2012, Adam Young.
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
#include <unistd.h>

int __w32_link_backup = FALSE;

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
//      The link() function shall create a new link (directory entry) for the existing file, 
//      path1.
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
int
w32_link(const char *to, const char *from)
{
    HINSTANCE hinst;
    FARPROC createHardLink;
    int ret = 0;

    if (!to || !from) {
        ret = -EINVAL;

    } else if (0 == (hinst = LoadLibrary("Kernel32")) ||
                    0 == (createHardLink = GetProcAddress(hinst, "CreateHardLinkA"))) {

        if (__w32_link_backup) {                /* backup fallback option */
            HANDLE fh;
	    int wlen;
	    WCHAR wpath[MAX_PATH];
            WIN32_STREAM_ID wsi = {0};
            void *ctx = NULL;
            DWORD cnt;

            if (INVALID_HANDLE_VALUE ==         /* source image */
                    (fh = CreateFile(from, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_POSIX_SEMANTICS, NULL))) {
                w32_errno_set();
                CloseHandle(fh);
                return -1;
            }
            wlen = mbstowcs(wpath, to, MAX_PATH) * sizeof(WCHAR);
            wsi.dwStreamId = BACKUP_LINK;
            wsi.dwStreamAttributes = 0;
            wsi.dwStreamNameSize = 0;
            wsi.Size.QuadPart = wlen;
            if (! BackupWrite(fh, (LPBYTE) &wsi, 
                        offsetof(WIN32_STREAM_ID, cStreamName), &cnt, FALSE, FALSE, &ctx) ||
                    offsetof(WIN32_STREAM_ID, cStreamName) != cnt ||
                    ! BackupWrite(fh, (LPBYTE) wpath, wlen, &cnt, FALSE, FALSE, &ctx)) {
                w32_errno_set();
                CloseHandle(fh);
                return -1;
            }                                   /* free resources */
            BackupWrite(fh, NULL, 0, &cnt, TRUE, FALSE, &ctx);
            CloseHandle(fh);
            return 0;
        }
        ret = -EIO;

    } else if (! createHardLink(to, from, NULL)) {
        ret = -w32_errno_cnv(GetLastError());
    }

    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}
/*end*/

