/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 pwd(2) implementation
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

#include "win32_internal.h"
#include <win32_child.h>			/* gethome */
#include <pwd.h>
#include <unistd.h>

static void                 fillin(void);

static struct               passwd pw;
static int                  counter;


/*
//  NAME
//      endpwent, getpwent, setpwent - user database functions
//  
//  SYNOPSIS
//  
//      #include <pwd.h>
//  
//      void endpwent(void);
//      struct passwd *getpwent(void);
//      void setpwent(void); [Option End]
//  
//  DESCRIPTION
//  
//      These functions shall retrieve information about users.
//  
//      The getpwent() function shall return a pointer to a structure containing the
//      broken-out fields of an entry in the user database. Each entry in the user database
//      contains a passwd structure. When first called, getpwent() shall return a pointer
//      to a passwd structure containing the first entry in the user database. Thereafter, 
//      it shall return a pointer to a passwd structure containing the next entry in the
//      user database. Successive calls can be used to search the entire user database.
//  
//      If an end-of-file or an error is encountered on reading, getpwent() shall return a
//      null pointer.
//  
//      An implementation that provides extended security controls may impose further
//      implementation-defined restrictions on accessing the user database. In particular, 
//      the system may deny the existence of some or all of the user database entries
//      associated with users other than the caller.
//  
//      The setpwent() function effectively rewinds the user database to allow repeated
//      searches.
//  
//      The endpwent() function may be called to close the user database when processing is
//      complete.
//  
//      These functions need not be reentrant. A function that is not required to be
//      reentrant is not required to be thread-safe.
//  
//  RETURN VALUE
//  
//      The getpwent() function shall return a null pointer on end-of-file or error.
//  
//  ERRORS
//  
//      The getpwent(), setpwent(), and endpwent() functions may fail if:
//  
//      [EIO]
//          An I/O error has occurred.
//  
//      In addition, getpwent() and setpwent() may fail if:
//  
//      [EMFILE]
//          {OPEN_MAX} file descriptors are currently open in the calling process.
//  
//      [ENFILE]
//          The maximum allowable number of files is currently open in the system.
//  
//      The return value may point to a static area which is overwritten by a subsequent
//      call to getpwuid(), getpwnam(), or getpwent().
*/
struct passwd *
getpwent(void)
{
    if (counter == 0) {
        counter++;
        fillin();
        return &pw;
    }
    return NULL;
}


/*
//  NAME
//      getpwuid, getpwuid_r - search user database for a user ID
//  
//  SYNOPSIS
//  
//      #include <pwd.h>
//  
//      struct passwd *getpwuid(uid_t uid);
//      int getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer,
//              size_t bufsize, struct passwd **result); [Option End]
//  
//  DESCRIPTION
//      The getpwuid() function shall search the user database for an entry with a matching
//      uid.
//  
//      The getpwuid() function need not be reentrant. A function that is not required to
//      be reentrant is not required to be thread-safe.
//  
//      Applications wishing to check for error situations should set errno to 0 before
//      calling getpwuid(). If getpwuid() returns a null pointer and errno is set to
//      non-zero, an error occurred.
//  
//      The getpwuid_r() function shall update the passwd structure pointed to by pwd and
//      store a pointer to that structure at the location pointed to by result. The
//      structure shall contain an entry from the user database with a matching uid.
//      Storage referenced by the structure is allocated from the memory provided with the
//      buffer parameter, which is bufsize bytes in size. The maximum size needed for this
//      buffer can be determined with the {_SC_GETPW_R_SIZE_MAX} sysconf() parameter. A
//      NULL pointer shall be returned at the location pointed to by result on error or if
//      the requested entry is not found. [Option End]
//  
//  RETURN VALUE
//  
//      The getpwuid() function shall return a pointer to a struct passwd with the
//      structure as defined in <pwd.h> with a matching entry if found. A null pointer
//      shall be returned if the requested entry is not found, or an error occurs. On error, 
//      errno shall be set to indicate the error.
//  
//      The return value may point to a static area which is overwritten by a subsequent
//      call to getpwent(), getpwnam(), or getpwuid().
//  
//      If successful, the getpwuid_r() function shall return zero; otherwise, an error
//      number shall be returned to indicate the error. [Option End]
//  
//  ERRORS
//  
//      The getpwuid() and getpwuid_r() functions may fail if:
//  
//      [EIO]
//          An I/O error has occurred.
//  
//      [EINTR]
//          A signal was caught during getpwuid().
//  
//      [EMFILE]
//          {OPEN_MAX} file descriptors are currently open in the calling process.
//  
//      [ENFILE]
//          The maximum allowable number of files is currently open in the system.
//  
//      The getpwuid_r() function may fail if:
//  
//      [ERANGE]
//          Insufficient storage was supplied via buffer and bufsize to contain the data to
//          be referenced by the resulting passwd structure. [Option End]
*/
struct passwd *
getpwuid(int uid)
{
    fillin();
    if (pw.pw_uid == uid) return &pw;
    return NULL;
}


/*
//  NAME
//  
//      getpwnam, getpwnam_r - search user database for a name
//  
//  SYNOPSIS
//  
//      #include <pwd.h>
//  
//      struct passwd *getpwnam(const char *name);
//      int getpwnam_r(const char *name, struct passwd *pwd, char *buffer,
//              size_t bufsize, struct passwd **result); [Option End]
//  
//  DESCRIPTION
//  
//      The getpwnam() function shall search the user database for an entry with a matching
//      name.
//  
//      The getpwnam() function need not be reentrant. A function that is not required to
//      be reentrant is not required to be thread-safe.
//  
//      Applications wishing to check for error situations should set errno to 0 before
//      calling getpwnam(). If getpwnam() returns a null pointer and errno is non-zero, an
//      error occurred.
//  
//      The getpwnam_r() function shall update the passwd structure pointed to by pwd and
//      store a pointer to that structure at the location pointed to by result. The
//      structure shall contain an entry from the user database with a matching name.
//      Storage referenced by the structure is allocated from the memory provided with the
//      buffer parameter, which is bufsize bytes in size. The maximum size needed for this
//      buffer can be determined with the {_SC_GETPW_R_SIZE_MAX} sysconf() parameter. A
//      NULL pointer shall be returned at the location pointed to by result on error or if
//      the requested entry is not found. [Option End]
//  
//  RETURN VALUE
//  
//      The getpwnam() function shall return a pointer to a struct passwd with the
//      structure as defined in <pwd.h> with a matching entry if found. A null pointer
//      shall be returned if the requested entry is not found, or an error occurs. On error, 
//      errno shall be set to indicate the error.
//  
//      The return value may point to a static area which is overwritten by a subsequent
//      call to getpwent(), getpwnam(), or getpwuid().
//  
//      If successful, the getpwnam_r() function shall return zero; otherwise, an error
//      number shall be returned to indicate the error. [Option End]
//  
//  ERRORS
//  
//      The getpwnam() and getpwnam_r() functions may fail if:
//  
//      [EIO]
//          An I/O error has occurred.
//  
//      [EINTR]
//          A signal was caught during getpwnam().
//  
//      [EMFILE]
//          {OPEN_MAX} file descriptors are currently open in the calling process.
//  
//      [ENFILE]
//          The maximum allowable number of files is currently open in the system.
//  
//      The getpwnam_r() function may fail if:
//  
//      [ERANGE]
//          Insufficient storage was supplied via buffer and bufsize to contain the data to
//          be referenced by the resulting passwd structure. [Option End]
*/
struct passwd *
getpwnam(const char *name)
{
    fillin();
    if (strcmp(pw.pw_name, name) == 0) {
        return &pw;
    }
    return NULL;
}


void
setpwent(void)
{
    counter = 0;
}


void
endpwent(void)
{
    counter = 0;
}


static void
fillin(void)
{
    pw.pw_name      = getlogin();
    pw.pw_passwd    = "*";
    pw.pw_uid       = w32_getuid();
    pw.pw_gid       = w32_getgid();
    pw.pw_age       = "";
    pw.pw_comment   = "";
    pw.pw_gecos     = "pc User";
    pw.pw_dir       = w32_gethome();
    pw.pw_shell     = w32_getshell();
    pw.pw_audid     = -1;
    pw.pw_audflg    = -1;
}


