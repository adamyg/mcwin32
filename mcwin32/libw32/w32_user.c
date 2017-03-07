/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 user identification functionality
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
#include <unistd.h>

#pragma comment(lib, "Advapi32.lib")


/*
//  NAME
//      getuid - get a real user ID
//
//  SYNOPSIS
//      #include <unistd.h>
//
//      uid_t getuid(void);
//
//  DESCRIPTION
//      The getuid() function shall return the real user ID of the calling process.
//
//  RETURN VALUE
//      The getuid() function shall always be successful and no return value is reserved to
//      indicate the error.
//
//  ERRORS
//      No errors are defined.
*/
int
w32_getuid (void)
{
    return 42;
}


/*
//  NAME
//      geteuid - get the effective user ID
//
//  SYNOPSIS
//      #include <unistd.h>
//
//      uid_t geteuid(void);
//
//  DESCRIPTION
//      The geteuid() function shall return the effective user ID of the calling process.
//
//  RETURN VALUE
//      The geteuid() function shall always be successful and no return value
//      is reserved to indicate an error.
//
//  ERRORS
//      No errors are defined.
*/
int
w32_geteuid (void)
{
    return 42;
}


/*
//  NAME
//      getgid - get the real group ID
//
//  SYNOPSIS
//      #include <unistd.h>
//
//      gid_t getgid(void);
//
//  DESCRIPTION
//      The getgid() function shall return the real group ID of the calling process.
//
//  RETURN VALUE
//      The getgid() function shall always be successful and no return value is reserved to
//      indicate an error.
//
//  ERRORS
//      No errors are defined.
*/
int
w32_getgid (void)
{
    return 42;
}



/*
//  NAME
//      getegid - get the effective group ID
//
//  SYNOPSIS
//      #include <unistd.h>
//
//      gid_t getegid(void);
//
//  DESCRIPTION
//      The getegid() function shall return the effective group ID of the calling process.
//
//  RETURN VALUE
//      The getegid() function shall always be successful and no return value is reserved
//      to indicate an error.
//
//  ERRORS
//      No errors are defined.
*/
int
w32_getegid (void)
{
    return 42;
}


/*
//  NAME
//      issetugid -  determine if current executable is running setuid or setgid
//
//  SYNOPSIS
//      #include <unistd.h>
//
//      int issetugid(void);
//
//  DESCRIPTION
//      The issetugid() function should be used to  determine if a path name returned
//      from a getenv(3C) call can be used safely to open the specified file. It is
//      often  not safe to open such a file because the status of the effective uid
//      is not known.
//
//  RETURN VALUE
//      The issetugid() function returns 1 if the process  was  made setuid or setgid
//      as  the result of the last or a previous call to execve(). Otherwise it returns 0.
//
//  ERRORS
//      No errors are defined.
*/
int
issetugid (void)
{
    return 0;
}



/*
//  NAME
//      getlogin, getlogin_r - get login name
//
//  SYNOPSIS
//
//      #include <unistd.h>
//
//      char *getlogin(void);
//      int getlogin_r(char *name, size_t namesize);
//
//  DESCRIPTION
//
//      The getlogin() function shall return a pointer to a string containing the user name
//      associated by the login activity with the controlling terminal of the current
//      process. If getlogin() returns a non-null pointer, then that pointer points to the
//      name that the user logged in under, even if there are several login names with the
//      same user ID.
//
//      The getlogin() function need not be reentrant. A function that is not required to
//      be reentrant is not required to be thread-safe.
//
//      The getlogin_r() function shall put the name associated by the login activity with
//      the controlling terminal of the current process in the character array pointed to
//      by name. The array is namesize characters long and should have space for the name
//      and the terminating null character. The maximum size of the login name is
//      {LOGIN_NAME_MAX}.
//
//      If getlogin_r() is successful, name points to the name the user used at login, even
//      if there are several login names with the same user ID. [Option End]
//
//  RETURN VALUE
//
//      Upon successful completion, getlogin() shall return a pointer to the login name or
//      a null pointer if the user's login name cannot be found. Otherwise, it shall return
//      a null pointer and set errno to indicate the error.
//
//      The return value from getlogin() may point to static data whose content is
//      overwritten by each call.
//
//      If successful, the getlogin_r() function shall return zero; otherwise, an error
//      number shall be returned to indicate the error. [Option End]
//
//  ERRORS
//
//      The getlogin() and getlogin_r() functions may fail if:
//
//      [EMFILE]
//          {OPEN_MAX} file descriptors are currently open in the calling process.
//
//      [ENFILE]
//          The maximum allowable number of files is currently open in the system.
//
//      [ENXIO]
//          The calling process has no controlling terminal.
//
//      The getlogin_r() function may fail if:
//
//      [ERANGE]
//          The value of namesize is smaller than the length of the string to be returned
//          including the terminating null character.
*/
const char *
getlogin (void)
{
    static char buffer[100];                    /* one-shot */
    DWORD size = sizeof(buffer);
    const char *p = buffer;

    if (!*p) {
        p = getenv("USER");

        if (p == NULL) p = getenv("USERNAME");  /* NT */

        if (GetUserName(buffer, &size))         /* requires: advapi32.lib */
            p = buffer;

        if (p == NULL)
            p = "dosuser";                      /* default */

        if (p != buffer) {
            strncpy(buffer, p, sizeof(buffer));
            buffer[ sizeof(buffer)-1 ] = '\0';
            p = buffer;
        }
    }
    return p;
}


int
getlogin_r (char *name, size_t namesize)
{
    const char *login = getlogin();
    size_t length = strlen(login);

    if (namesize >= length) {
        errno = ERANGE;
        return -1;
    }
    memcpy(name, login, length + 1);
    return (int)length;
}
/*end*/
