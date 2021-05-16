#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_pwd_c,"$Id: w32_pwd.c,v 1.8 2021/05/16 14:42:36 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 pwd(2) implementation
 *
 * Copyright (c) 2007, 2012 - 2021 Adam Young.
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

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0500
#endif

#include "win32_internal.h"
#include <win32_child.h>                        /* gethome */
#include <pwd.h>
#include <unistd.h>
#include <assert.h>

#include <sddl.h>                               /* ConvertSidToStringSid */
#include <Lm.h>

#pragma comment(lib, "Netapi32.lib")

static void                 fill_passwds(void);
static int                  fill_builtin(const struct WellKnownSID *wksid, struct passwd *pwd, char *name, size_t namlen);
static unsigned             RID(PSID sid);
static void                 fill_passwd(void);
static int                  copy_passwd(const struct passwd *passwd, struct passwd *dest, char *buffer, size_t bufsize);

static unsigned             x_passwds_count;
static int                  x_cursor;           /* getpwent cursor */
static struct passwd       *x_passwds;
static struct passwd        x_passwd;
static char                 x_buffer[MAX_PATH * 5];


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
//      void setpwent(void);
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
//      The passwd structure is defined in <pwd.h> as follows:
//
//          struct passwd {
//              char   *pw_name;        // username
//              char   *pw_passwd;      // user password
//              uid_t   pw_uid;         // user ID
//              gid_t   pw_gid;         // group ID
//              char   *pw_gecos;       // user information
//              char   *pw_dir;         // home directory
//              char   *pw_shell;       // shell program
//          };
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
LIBW32_API struct passwd *
getpwent(void)
{
    const unsigned cursor = x_cursor++;
    if (0 == cursor) {
        fill_passwds();
        return &x_passwd;
    } else if (cursor <= x_passwds_count) {
        return x_passwds + (cursor - 1);
    }
    return NULL;
}


LIBW32_API void
setpwent(void)
{
    x_cursor = 0;
}


LIBW32_API void
endpwent(void)
{
    x_cursor = 0xffff;
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
//              size_t bufsize, struct passwd **result);
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
//      the requested entry is not found.
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
//      number shall be returned to indicate the error.
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
//          be referenced by the resulting passwd structure.
*/
LIBW32_API struct passwd *
getpwuid(int uid)
{
    if (uid == w32_getuid()) {
        fill_passwd();
        return &x_passwd;

    } else if (uid) {
        struct passwd *pwd, *end;
        fill_passwds();
        for (pwd = x_passwds, end = pwd + x_passwds_count; pwd != end; ++pwd) {

            if (uid == pwd->pw_uid) {
                return pwd;
            }
        }
    }
    return NULL;
}


LIBW32_API int
getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result)
{
    if (NULL == pwd || NULL == buffer || NULL == result) {
        if (result) *result = NULL;
        errno = EINVAL;
        return EINVAL; //invalid arguments
    }

    *result = NULL;
    if (uid == w32_getuid()) {
        const int rc = copy_passwd(w32_passwd_user(), pwd, buffer, bufsize);
        if (0 == rc) *result = pwd; //success
        return rc;

    } else if (uid) {
        struct passwd *pwd, *end;
        fill_passwds();
        for (pwd = x_passwds, end = pwd + x_passwds_count; pwd != end; ++pwd) {
            if (uid == pwd->pw_uid) {
                const int rc = copy_passwd(w32_passwd_user(), pwd, buffer, bufsize);
                if (0 == rc) *result = pwd; //success
                return rc;
            }
        }
    }
    return 0; //no-match
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
//              size_t bufsize, struct passwd **result);
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
//      the requested entry is not found.
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
//      number shall be returned to indicate the error.
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
//          be referenced by the resulting passwd structure.
*/
LIBW32_API struct passwd *
getpwnam(const char *name)
{
    if (name) {
        fill_passwds();
        if (0 == _stricmp(x_passwd.pw_name, name)) {
            return &x_passwd;
        }
    }
    return NULL;
}


LIBW32_API int
getpwnam_r(const char *name, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result)
{
    if (NULL == name || NULL == buffer || NULL == result) {
        if (result) *result = NULL;
        errno = EINVAL;
        return EINVAL; //invalid arguments
    }

    *result = NULL;
    if (0 == strcmp(name, getlogin())) {
        const int rc = copy_passwd(w32_passwd_user(), pwd, buffer, bufsize);
        if (0 == rc) *result = pwd; //success
        return rc;
    }
    return 0; //no-match
}


/////////////////////////////////////////////////////////////////////////////////////////
//  passwd's database implementation

static struct WellKnownSID {
    const char *name;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
    BYTE SubAuthCount;
    DWORD SubAuth[2];

} well_known_sids[] = {
    // See: "wmic sysaccount get name,sid"
    {"S-1-5-1", SECURITY_NT_AUTHORITY, 1, {SECURITY_DIALUP_RID}},
    {"S-1 5-2", SECURITY_NT_AUTHORITY, 1, {SECURITY_NETWORK_RID}},
    {"S-1-5-3", SECURITY_NT_AUTHORITY, 1, {SECURITY_BATCH_RID}},
    {"S-1-5-4", SECURITY_NT_AUTHORITY, 1, {SECURITY_INTERACTIVE_RID}},
    {"S-1-5-6", SECURITY_NT_AUTHORITY, 1, {SECURITY_SERVICE_RID}},
    {"S-1-5-11", SECURITY_NT_AUTHORITY, 1, {SECURITY_AUTHENTICATED_USER_RID}},
    {"S-1-5-15", SECURITY_NT_AUTHORITY, 1, {SECURITY_THIS_ORGANIZATION_RID}},
    {"S-1-5-18", SECURITY_NT_AUTHORITY, 1, {SECURITY_LOCAL_SYSTEM_RID}},
    {"S-1-5-19", SECURITY_NT_AUTHORITY, 1, {SECURITY_LOCAL_SERVICE_RID}},
    {"S-1-5-20", SECURITY_NT_AUTHORITY, 1, {SECURITY_NETWORK_SERVICE_RID}},
    {"S-1-5-32-544", SECURITY_NT_AUTHORITY, 2, {SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS}},
    {"S-1-5-32-545", SECURITY_NT_AUTHORITY, 2, {SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_USERS}},
    {"S-1-5-32-546", SECURITY_NT_AUTHORITY, 2, {SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_GUESTS}},
    {"S-1-5-32-547", SECURITY_NT_AUTHORITY, 2, {SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_POWER_USERS}},
    {"S-1-5-80-956008885-3418522649-1831038044-1853292631-2271478464"}
        // TrustedInstaller, RID:704 (short)
};


static void
fill_passwds(void)
{
    DWORD i, dwEntriesRead = 0, dwTotalEntries = 0;
    PUSER_INFO_20 users = NULL;
    NET_API_STATUS nStatus;

    fill_passwd();
    if (NULL != x_passwds)
        return;

    nStatus = NetUserEnum(NULL, 20 /*USER_INFO_20*/, 0, (LPBYTE *) &users,
                    MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, NULL);

    if (NERR_Success == nStatus) {
        unsigned bufsize = 0, total = 0;
        char name[MAX_PATH];
        int nlen;

        for (i = 0; i < dwEntriesRead; ++i) {
            const PUSER_INFO_20 user = users + i;
            if (user->usri20_user_id == x_passwd.pw_uid ||
                    (nlen = w32_wc2utf(user->usri20_name, name, sizeof(name))) <= 0) {
                continue;
            }
            bufsize += (nlen + 1);
            ++total;
        }

        for (i = 0; i < _countof(well_known_sids); ++i) {
            const int nlen = fill_builtin(well_known_sids + i, NULL, NULL, 0);
            if (nlen > 0) {
                bufsize += (nlen + 1);
                ++total;
            }
        }

        x_passwds = (struct passwd *)malloc((sizeof(struct passwd) * total) + bufsize + 1 /*non-zero*/);
        if (NULL != x_passwds) {
            struct passwd *pwd = x_passwds;
            char *cursor = (char *)(pwd + total);
//          wchar_t t_buffer[1024];

            for (i = 0; i < dwEntriesRead; ++i) {
                const PUSER_INFO_20 user = users + i;

//              swprintf_s(t_buffer, _countof(t_buffer), L"User:%s,FullName:%s,Comment:%s,RID:%u\n",
//                  user->usri20_name, user->usri20_full_name, user->usri20_comment, (unsigned)user->usri20_user_id);
//              OutputDebugStringW(t_buffer);

                if (user->usri20_user_id == x_passwd.pw_uid ||
                        (nlen = w32_wc2utf(user->usri20_name, cursor, bufsize)) <= 0) {
                    continue;
                }

                memset(pwd, 0, sizeof(*pwd));
                pwd->pw_name = cursor;
                _strlwr(cursor);
                pwd->pw_uid  = (int)user->usri20_user_id;
                pwd->pw_gid  = (int)user->usri20_user_id;
                cursor  += (nlen + 1);
                bufsize -= (nlen + 1);
                ++x_passwds_count;
                ++pwd;
            }

            for (i = 0; i < _countof(well_known_sids); ++i) {
                const int nlen = fill_builtin(well_known_sids + i, pwd, cursor, bufsize);
                if (nlen > 0) {
                    cursor  += (nlen + 1);
                    bufsize -= (nlen + 1);
                    ++x_passwds_count;
                    ++pwd;
                }
            }

            assert(total == x_passwds_count);
            assert(0 == bufsize);
        }
    }
    if (users) NetApiBufferFree(users);
}


static int
fill_builtin(const struct WellKnownSID *wksid,
        struct passwd *pwd, char *name, size_t namelen)
{
    PSID pSID = NULL;
    int ret = 0;

    if (wksid->SubAuthCount) {
        if (! AllocateAndInitializeSid((PSID_IDENTIFIER_AUTHORITY) &wksid->IdentifierAuthority,
                    wksid->SubAuthCount, wksid->SubAuth[0], wksid->SubAuth[1], 0, 0, 0, 0, 0, 0, &pSID)) {
            pSID = NULL;
        }
    } else {
        if (! ConvertStringSidToSidA(wksid->name, &pSID)) {
            pSID = NULL;
        }
    }

    if (pSID) {
        char t_name[WIN32_LOGIN_LEN], t_domain[1024];
        DWORD nlen = sizeof(t_name), dlen = sizeof(t_domain);
        SID_NAME_USE user_type = {0};

        if (LookupAccountSidA(NULL, pSID, t_name, &nlen, t_domain, &dlen, &user_type)) {
            if (name) strcpy_s(name, namelen, t_name);
            if (pwd) {
                memset(pwd, 0, sizeof(*pwd));
                pwd->pw_name = name;
                _strlwr(name);
                pwd->pw_uid = (short) RID(pSID);
                pwd->pw_gid = pwd->pw_uid;      // TODO
            }
            ret = nlen;
        } else {
            if (name) name[0] = 0;
        }

        assert(SidTypeWellKnownGroup == user_type || SidTypeAlias == user_type);
        FreeSid(pSID);
    }
    return ret;
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


static void
fill_passwd(void)
{
    const struct passwd *passwd = w32_passwd_user();
    copy_passwd(passwd, &x_passwd, x_buffer, sizeof(x_buffer));
}


static int
pw_strlen(const char *s, size_t *total)
{
    if (s && *s) {
        const slen = strlen(s);
        *total += (slen + 1);
        return slen;
    }
    *total += 1;
    return 0;
}


static char *
pw_strcpy(const char *s, size_t slen, char **cursor)
{
    char *dst = *cursor, *base = dst;
    if (slen) {
        memcpy(dst, s, slen), dst += slen;
    }
    *dst++  = 0;
    *cursor = dst;
    return base;
}


static int
copy_passwd(const struct passwd *pwd, struct passwd *dst, char *buffer, size_t bufsize)
{
    size_t total = 0;
    const size_t
        namelen     = pw_strlen(pwd->pw_name, &total),
        passwdlen   = pw_strlen(pwd->pw_passwd, &total),
        agelen      = pw_strlen(pwd->pw_age, &total),
        commentlen  = pw_strlen(pwd->pw_comment, &total),
        gecoslen    = pw_strlen(pwd->pw_gecos, &total),
        dirlen      = pw_strlen(pwd->pw_dir, &total),
        shelllen    = pw_strlen(pwd->pw_shell, &total);

    if (total > bufsize) {
        errno = ERANGE;
        return ERANGE;
    }

    dst->pw_name    = pw_strcpy(pwd->pw_name,   namelen,      &buffer);
    dst->pw_passwd  = pw_strcpy(pwd->pw_passwd, passwdlen,    &buffer);
    dst->pw_uid     = pwd->pw_uid;
    dst->pw_gid     = pwd->pw_gid;
    dst->pw_age     = pw_strcpy(pwd->pw_age,     agelen,      &buffer);
    dst->pw_comment = pw_strcpy(pwd->pw_comment, commentlen,  &buffer);
    dst->pw_gecos   = pw_strcpy(pwd->pw_gecos,   gecoslen,    &buffer);
    dst->pw_dir     = pw_strcpy(pwd->pw_dir,     dirlen,      &buffer);
    dst->pw_shell   = pw_strcpy(pwd->pw_shell,   shelllen,    &buffer);
    dst->pw_audid   = pwd->pw_audid;
    dst->pw_audflg  = pwd->pw_audflg;

    return 0; //success
}

/*end*/
