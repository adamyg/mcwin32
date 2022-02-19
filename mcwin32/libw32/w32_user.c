#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_user_c,"$Id: w32_user.c,v 1.15 2022/02/17 16:05:00 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 user identification functionality
 *
 * Copyright (c) 2007, 2012 - 2022 Adam Young.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0501              /* enable xp+ features */
#endif

#include "win32_internal.h"
#include "win32_child.h"
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#include <sddl.h>                               /* ConvertSidToStringSid */
#include <Lm.h>

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Netapi32.lib")

static char x_passwd_name[WIN32_LOGIN_LEN];
static char x_passwd_passwd[32];
static char x_passwd_gecos[32];
static char x_passwd_dir[MAX_PATH];
static char x_passwd_shell[MAX_PATH];

static struct passwd x_passwd = {
    x_passwd_name,          /* pw_name */
    x_passwd_passwd,        /* pw_passwd */
    -1,                     /* pw_uid */
    -1,                     /* pw_gid */
    NULL,                   /* pw_age */
    NULL,                   /* pw_comment */
    x_passwd_gecos,         /* pw_gecos */
    x_passwd_dir,           /* pw_dir */
    x_passwd_shell,         /* pw_shell */
    0,                      /* pw_audid */
    0                       /* pw_audflg */
};

static char x_group_name[WIN32_LOGIN_LEN];
static char x_group_passwd[32];

static struct group x_group = {
    x_group_name,           /* gr_name */
    x_group_passwd,         /* gr_passwd */
    -1,                     /* gr_gid */
    NULL
};

static void         initialise_user(void);
static unsigned     RID(PSID sid);


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
LIBW32_API int
w32_getuid (void)
{
    initialise_user();
    return x_passwd.pw_uid;
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
    initialise_user();
    return x_passwd.pw_uid;
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
LIBW32_API int
w32_getgid (void)
{
    initialise_user();
    return x_passwd.pw_gid;
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
    initialise_user();
    return x_passwd.pw_gid;
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
//      The issetugid() function should be used to determine if a path name returned
//      from a getenv(3C) call can be used safely to open the specified file. It is
//      often not safe to open such a file because the status of the effective uid
//      is not known.
//
//  RETURN VALUE
//      The issetugid() function returns 1 if the process was made setuid or setgid
//      as  the result of the last or a previous call to execve(). Otherwise it returns 0.
//
//  ERRORS
//      No errors are defined.
*/
LIBW32_API int
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
LIBW32_API const char *
getlogin (void)
{
    static char buffer[WIN32_LOGIN_LEN];
    if (getlogin_r(buffer, sizeof(buffer)) > 0) {
        return buffer;
    }
    return NULL;
}


LIBW32_API int
getlogin_r (char *name, size_t namesize)
{
    int length;

    if (name == NULL || namesize == 0) {
        errno = EINVAL;
        return -1;
    }

    initialise_user();
    length = strlen(x_passwd_name);
    if (namesize <= (size_t)length) {
        errno = ERANGE;
        return -1;
    }
    memcpy(name, x_passwd_name, length + 1);
    return length;
}


LIBW32_API const struct passwd *
w32_passwd_user (void)
{
    initialise_user();
    return &x_passwd;
}


LIBW32_API const struct group *
w32_group_user (void)
{
    initialise_user();
    return &x_group;
}


static void
initialise_user()
{
    char login[WIN32_LOGIN_LEN], group[WIN32_GROUP_LEN];
    char domain[1024 + 1];

    TOKEN_USER *tu = NULL;
    TOKEN_PRIMARY_GROUP *pg = NULL;
    HANDLE hToken = NULL;

    login[0] = 0, group[0] = 0, domain[0] = 0;

    if (x_passwd.pw_uid >= 0) {
        return;
    }

    // defaults
    x_passwd.pw_uid = 42;
    x_passwd.pw_gid = 42;

    strncpy(x_passwd_name, "user", sizeof(x_passwd_name) - 1);
    strncpy(x_passwd_passwd, "*", sizeof(x_passwd_passwd) - 1);
    strncpy(x_passwd_gecos, "pcuser", sizeof(x_passwd_gecos) - 1);
    strncpy(x_group_name, "user", sizeof(x_group_name) - 1);

    // process identify
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        SID_NAME_USE user_type = {0};
        DWORD cbSize, cbSize2;

        cbSize = 0;
        if (! GetTokenInformation(hToken, TokenUser, NULL, 0, &cbSize) &&
                GetLastError () == ERROR_INSUFFICIENT_BUFFER && cbSize &&
                    NULL != (tu = alloca(sizeof(char) + (cbSize + 1)))) {

            if (GetTokenInformation(hToken, TokenUser, tu, cbSize, &cbSize2)) {
                DWORD ulen = sizeof(login), dlen = sizeof(domain);

                if (! LookupAccountSidA(NULL, tu->User.Sid, login, &ulen, domain, &dlen, &user_type)) {
                    login[0] = 0;
                }
            }
        }

        cbSize = 0;
        if (! GetTokenInformation(hToken, TokenPrimaryGroup, NULL, 0, &cbSize) &&
                GetLastError () == ERROR_INSUFFICIENT_BUFFER && cbSize &&
                    NULL != (pg = alloca(sizeof(char) + (cbSize + 1)))) {

            if (GetTokenInformation(hToken, TokenPrimaryGroup, pg, cbSize, &cbSize2)) {
                DWORD glen = sizeof(group), dlen = sizeof(domain);

                if (! LookupAccountSidA(NULL, pg->PrimaryGroup, group, &glen, NULL, &dlen, &user_type)) {
                    group[0] = 0;
                }

            } else {
                pg = NULL;
            }
        }

        CloseHandle(hToken);
    }

    // apply
    if (login[0]) {
        char *sid = NULL;

        strncpy(x_passwd_name, login, sizeof(x_passwd_name)-1);

        if (0 == _stricmp("Administrator", login)) {
            x_passwd.pw_uid = 500;              // Built-in admin account.
            x_passwd.pw_gid = 513;              // PrimaryGroupID.
                //
                // By default, all Active Directory users have a PrimaryGroupID of 513,
                // which is associated with the Domain Users group.
                // However, if the user needed to be seen as a Domain Admin for POSIX,
                // the PrimaryGroupID needed to be 512, the RID for that group.
                // The Enterprise Admins group, 519, is also used to grant this level in POSIX.
                //

        } else {
            x_passwd.pw_uid = (short) RID(tu->User.Sid);
                // Note: Unfortunately st_uid/st_gid are short's resulting in RID truncation.
            x_passwd.pw_gid = x_passwd.pw_uid;
            if (pg) {
                x_passwd.pw_gid = (short) RID(pg->PrimaryGroup);
            }
        }

        if (ConvertSidToStringSidA(tu->User.Sid, &sid)) {
            strncpy(x_passwd_gecos, sid, sizeof(x_passwd_gecos) - 1);
            LocalFree(sid);
        }

    // old-school
    } else {
        DWORD cbBuffer = sizeof(x_passwd_name) - 1;

        if (! GetUserNameA(x_passwd_name, &cbBuffer)) {
            const char *name = NULL;

            if (NULL == name) name = getenv("USER");
            if (NULL == name) name = getenv("USERNAME");
            if (NULL == name) name = "dosuser";
            strncpy(x_passwd_name, name, sizeof(x_passwd_name) - 1);
        }

        if (0 == _stricmp("Administrator", x_passwd_name)) {
            x_passwd.pw_uid = 0;
        }

        x_passwd.pw_gid = x_passwd.pw_uid;
    }

    // additional account attributes
    _strlwr(x_passwd_name);
    strncpy(x_passwd_dir, w32_gethome(FALSE), sizeof(x_passwd_dir) - 1);
    strncpy(x_passwd_shell, w32_getshell(), sizeof(x_passwd_shell) - 1);

    // group
    if (group[0]) {
        strncpy(x_group_name, group, sizeof(x_group_name) - 1);
        _strlwr(x_group_name);
    }
    x_group.gr_gid = x_passwd.pw_gid;
    x_group.gr_mem = NULL;
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


#if defined(_MSC_VER) && (_MSC_VER < 1500)
#define TokenElevation  20
typedef struct _TOKEN_ELEVATION {
    DWORD TokenIsElevated;
} TOKEN_ELEVATION;
typedef TOKEN_ELEVATION *PTOKEN_ELEVATION;
#endif

LIBW32_API int
w32_IsElevated(void)
{
    BOOL fRet = FALSE;
    HANDLE hToken = NULL;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);

        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
            fRet = Elevation.TokenIsElevated;
        }
        CloseHandle(hToken);
    }
    return fRet;
}


/*  Function:           w32_IsAdministrator
 *      This routine returns TRUE if the caller's process is a member of the
 *      Administrators local group.
 *
 *      Caller is NOT expected to be impersonating anyone and is expected
 *      to be able to open its own process and process token.
 *
 *  Arguments:
 *      None
 *
 *  Return Value:
 *      TRUE    - Caller has Administrators local group.
 *      FALSE   - Caller does not have Administrators local group.
 */
LIBW32_API int
w32_IsAdministrator(void)
{
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    BOOL b;

    b = AllocateAndInitializeSid(
            &NtAuthority, 2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &AdministratorsGroup);

    if (b) {
        if (! CheckTokenMembership(NULL, AdministratorsGroup, &b)) {
            b = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }
    return b;
}

/*end*/
