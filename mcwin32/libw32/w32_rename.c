#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_rename_c,"$Id: w32_rename.c,v 1.3 2021/05/07 17:52:56 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 rename() system calls.
 *
 * Copyright (c) 2020 - 2021 Adam Young.
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
#include "win32_misc.h"

#include <stdio.h>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#include <unistd.h>


/*
//  NAME
//
//      rename - rename a file
//
//  SYNOPSIS
//
//      #include <stdio.h>
//
//      int rename(const char *old, const char *new);
//
//  DESCRIPTION
//
//      The rename() function shall change the name of a file. The old argument points to
//      the pathname of the file to be renamed. The new argument points to the new pathname
//      of the file.
//
//      If either the old or new argument names a symbolic link, rename() shall operate on
//      the symbolic link itself, and shall not resolve the last component of the argument.
//      If the old argument and the new argument resolve to the same existing file,
//      rename() shall return successfully and perform no other action.
//
//      If the old argument points to the pathname of a file that is not a directory, the
//      new argument shall not point to the pathname of a directory. If the link named by
//      the new argument exists, it shall be removed and old renamed to new. In this case,
//      a link named new shall remain visible to other processes throughout the renaming
//      operation and refer either to the file referred to by new or old before the
//      operation began. Write access permission is required for both the directory
//      containing old and the directory containing new.
//
//      If the old argument points to the pathname of a directory, the new argument shall
//      not point to the pathname of a file that is not a directory. If the directory named
//      by the new argument exists, it shall be removed and old renamed to new. In this
//      case, a link named new shall exist throughout the renaming operation and shall
//      refer either to the directory referred to by new or old before the operation began.
//      If new names an existing directory, it shall be required to be an empty directory.
//
//      If the old argument points to a pathname of a symbolic link, the symbolic link
//      shall be renamed. If the new argument points to a pathname of a symbolic link, the
//      symbolic link shall be removed.
//
//      The new pathname shall not contain a path prefix that names old. Write access
//      permission is required for the directory containing old and the directory
//      containing new. If the old argument points to the pathname of a directory, write
//      access permission may be required for the directory named by old, and, if it exists,
//      the directory named by new.
//
//      If the link named by the new argument exists and the file's link count becomes 0
//      when it is removed and no process has the file open, the space occupied by the file
//      shall be freed and the file shall no longer be accessible. If one or more processes
//      have the file open when the last link is removed, the link shall be removed before
//      rename() returns, but the removal of the file contents shall be postponed until all
//      references to the file are closed.
//
//      Upon successful completion, rename() shall mark for update the st_ctime and
//      st_mtime fields of the parent directory of each file.
//
//  RETURN VALUE
//
//      Upon successful completion, rename() shall return 0; otherwise, -1 shall be returned,
//
*/

LIBW32_API int
w32_rename(const char *ofile, const char *nfile)
{
#if defined(UTF8FILENAMES)
    wchar_t wofile[WIN32_PATH_MAX], wnfile[WIN32_PATH_MAX];
    int mode = 0;

    if (NULL == ofile || NULL == nfile) {
        errno = EFAULT;
        return -1;
    }

    if (w32_utf2wc(ofile, wofile, _countof(wofile)) > 0) {
        if (w32_utf2wc(nfile, wnfile, _countof(wnfile)) > 0) {
            return w32_renameW(wofile, wnfile);
        }
    }

    return -1;

#else

    return w32_renameA(ofile, nfile);

#endif  //UTF8FILENAMES
}


LIBW32_API int
w32_renameA(const char *ofile, const char *nfile)
{
#undef rename
    return rename(ofile, nfile);
}


LIBW32_API int
w32_renameW(const wchar_t *ofile, const wchar_t *nfile)
{
    return _wrename(ofile, nfile);
}

/*end*/
