#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_grp_c,"$Id: w32_grp.c,v 1.6 2018/10/12 00:52:03 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 pwd() implementation
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
#include <stdlib.h>
#include <unistd.h>
#include <grp.h>

static void                 fillin(void);

static struct group         grp;
static int                  counter;

/*
//  NAME
//      getgrgid - get group database entry for a group ID
//
//  SYNOPSIS
//      #include <grp.h>
//
//      struct group *getgrgid(gid_t gid);
//
//  DESCRIPTION
//      The getgrgid() function shall search the group database for an entry with a
//      matching gid.
//
//      The getgrgid() function need not be reentrant. A function that is not required to
//      be reentrant is not required to be thread-safe.
//
//
//  RETURN VALUE
//      Upon successful completion, getgrgid() shall return a pointer to a struct group
//      with the structure defined in <grp.h> with a matching entry if one is found. The
//      getgrgid() function shall return a null pointer if either the requested entry was
//      not found, or an error occurred. On error, errno shall be set to indicate the error.
//
//      The return value may point to a static area which is overwritten by a subsequent
//      call to getgrent(), getgrgid(), or getgrnam().
//
//  ERRORS
//      The getgrgid() and getgrgid_r() functions may fail if:
//
//      [EIO]
//          An I/O error has occurred.
//      [EINTR]
//          A signal was caught during getgrgid().
//      [EMFILE]
//          {OPEN_MAX} file descriptors are currently open in the calling process.
//      [ENFILE]
//          The maximum allowable number of files is currently open in the system.
*/
LIBW32_API struct group *
getgrgid(int gid)
{
    fillin();
    if (gid != grp.gr_gid) {
        return NULL;
    }
    return &grp;
}


/*
//  NAME
//      getgrnam - search group database for a name
//
//  SYNOPSIS
//      #include <grp.h>
//      struct group *getgrnam(const char *name);
//
//  DESCRIPTION
//      The getgrnam() function shall search the group database for an entry with a
//      matching name.
//
//      The getgrnam() function need not be reentrant. A function that is not required to
//      be reentrant is not required to be thread-safe.
//
//  RETURN VALUE
//      The getgrnam() function shall return a pointer to a struct group with the structure
//      defined in <grp.h> with a matching entry if one is found. The getgrnam() function
//      shall return a null pointer if either the requested entry was not found, or an
//      error occurred. On error, errno shall be set to indicate the error.
//
//      The return value may point to a static area which is overwritten by a subsequent
//      call to getgrent(), getgrgid(), or getgrnam().
//
//  ERRORS
//      The getgrnam() and getgrnam_r() functions may fail if:
//
//      [EIO]
//          An I/O error has occurred.
//      [EINTR]
//          A signal was caught during getgrnam().
//      [EMFILE]
//          {OPEN_MAX} file descriptors are currently open in the calling process.
//      [ENFILE]
//          The maximum allowable number of files is currently open in the system.
*/
LIBW32_API struct group *
getgrnam(const char * n)
{
    fillin();
    if (strcmp(n, grp.gr_name) != 0) {
        return NULL;
    }
    return &grp;
}


/*
//  NAME
//      endgrent, getgrent, setgrent - group database entry functions
//
//  SYNOPSIS
//      #include <grp.h>
//
//      void endgrent(void);
//      struct group *getgrent(void);
//      void setgrent(void); [Option End]
//
//  DESCRIPTION
//      The getgrent() function shall return a pointer to a structure containing the
//      broken-out fields of an entry in the group database. When first called, getgrent()
//      shall return a pointer to a group structure containing the first entry in the group
//      database. Thereafter, it shall return a pointer to a group structure containing the
//      next group structure in the group database, so successive calls may be used to
//      search the entire database.
//
//      An implementation that provides extended security controls may impose further
//      implementation-defined restrictions on accessing the group database. In particular,
//      the system may deny the existence of some or all of the group database entries
//      associated with groups other than those groups associated with the caller and may
//      omit users other than the caller from the list of members of groups in database
//      entries that are returned.
//
//      The setgrent() function shall rewind the group database to allow repeated searches.
//
//      The endgrent() function may be called to close the group database when processing
//      is complete.
//
//      These functions need not be reentrant. A function that is not required to be
//      reentrant is not required to be thread-safe.
//
//  RETURN VALUE
//      When first called, getgrent() shall return a pointer to the first group structure
//      in the group database. Upon subsequent calls it shall return the next group
//      structure in the group database. The getgrent() function shall return a null
//      pointer on end-of-file or an error and errno may be set to indicate the error.
//
//      The return value may point to a static area which is overwritten by a subsequent
//      call to getgrgid(), getgrnam(), or getgrent().
//
//  ERRORS
//      The getgrent() function may fail if:
//
//      [EINTR]
//          A signal was caught during the operation.
//      [EIO]
//          An I/O error has occurred.
//      [EMFILE]
//          {OPEN_MAX} file descriptors are currently open in the calling process.
//      [ENFILE]
//          The maximum allowable number of files is currently open in the system.
*/
LIBW32_API void
setgrent(void)
{
    counter = 0;
}


LIBW32_API struct group *
getgrent(void)
{
    if (counter++ == 0) {
        fillin();
        return &grp;
    }
    return NULL;
}


LIBW32_API void
endgrent(void)
{
    counter = 1;
}


static void
fillin(void)
{
    grp.gr_name     = "user";
    grp.gr_passwd   = "*";
    grp.gr_gid      = w32_getgid();
}


/*
//  NAME
//      getgroups
//
//  SYNOPSIS
//      #include <unistd.h>
//
//      int getgroups(int gidsetsize, gid_t grouplist[]);
//
//  DESCRIPTION
//      The getgroups() function shall fill in the array grouplist with the current
//      supplementary group IDs of the calling process. It is implementation-defined
//      whether getgroups() also returns the effective group ID in the grouplist array.
//
//      The gidsetsize argument specifies the number of elements in the array grouplist.
//      The actual number of group IDs stored in the array shall be returned. The values of
//      array entries with indices greater than or equal to the value returned are undefined.
//
//      If gidsetsize is 0, getgroups() shall return the number of group IDs that it would
//      otherwise return without modifying the array pointed to by grouplist.
//
//      If the effective group ID of the process is returned with the supplementary group
//      IDs, the value returned shall always be greater than or equal to one and less than
//      or equal to the value of {NGROUPS_MAX}+1.
//
//  RETURN VALUE
//      Upon successful completion, the number of supplementary group IDs shall be
//      returned. A return value of -1 indicates failure and errno shall be set to indicate
//      the error.
//
//  ERRORS
//      The getgroups() function shall fail if:
//
//      [EINVAL]
//          The gidsetsize argument is non-zero and less than the number of group IDs that
//          would have been returned.
*/
LIBW32_API int
getgroups(int gidsetsize, gid_t grouplist[])
{
    if (gidsetsize >= 1) {
        if (grouplist) {
            grouplist[0] = 42;
            return 1;
        }
    }
    errno = EINVAL;
    return -1;
}

/*end*/

