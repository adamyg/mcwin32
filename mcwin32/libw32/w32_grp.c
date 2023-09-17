#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_grp_c, "$Id: w32_grp.c,v 1.16 2023/09/17 13:04:58 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 pwd() implementation
 *
 * Copyright (c) 2007, 2012 - 2023 Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 * or (at your option) any later version.
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

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0500
#endif

#include "win32_internal.h"
#include <stdlib.h>
#include <unistd.h>
#include <grp.h>
#include <assert.h>

#include <sddl.h>                               /* ConvertSidToStringSid */
#include <Lm.h>

#pragma comment(lib, "Netapi32.lib")

static void                 fill_groups(void);
static unsigned             RID(PSID sid);
static void                 fill_group(void);
static int                  copy_group(const struct group *grp, struct group *dest, char *buffer, size_t bufsize);

static unsigned             x_groups_count;
static unsigned             x_cursor;           /* getgrent cursor */
static struct group         x_group;
static struct group        *x_groups;
static char                 x_buffer[MAX_PATH * 2];


/*
//  NAME
//      endgrent, getgrent, setgrent - group database entry functions
//
//  SYNOPSIS
//      #include <grp.h>
//
//      void endgrent(void);
//      struct group *getgrent(void);
//      void setgrent(void);
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
    x_cursor = 0;
}


LIBW32_API struct group *
getgrent(void)
{
    const unsigned cursor = x_cursor++;

    if (0 == cursor) {
        fill_groups();
        return &x_group;

    } else if (cursor <= x_groups_count) {
        return x_groups + (cursor - 1);
    }
    return NULL;
}


LIBW32_API int
getgrent_r(struct group *grp, char *buffer, size_t bufsize, struct group **result)
{
    struct group *it = NULL;
    unsigned cursor;

    if (NULL == grp || NULL == buffer || NULL == result) {
        if (result) *result = NULL;
        errno = EINVAL;
        return EINVAL;                          // invalid arguments
    }

    cursor = x_cursor++;
    if (0 == cursor) {
        fill_groups();
        it = &x_group;
    } else if (cursor <= x_groups_count) {
        it = x_groups + (cursor - 1);
    }

    *result = NULL;
    if (it) {
        const int rc = copy_group(it, grp, buffer, bufsize);
        if (0 == rc) *result = grp;             // success
        return rc;
    }

    errno = EINVAL;
    return ENOENT;                              // no-match
}


LIBW32_API void
endgrent(void)
{
    x_cursor = 1;
}


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
    const struct group *current = w32_group_user();

    if (gid == current->gr_gid) {
        fill_group();
        return &x_group;

    } else {
        struct group *it, *end;
        fill_groups();
        for (it = x_groups, end = it + x_groups_count; it != end; ++it) {
            if (gid == it->gr_gid) {
                return it;
            }
        }
    }
    return NULL;
}


LIBW32_API int
getgrgid_r(gid_t gid, struct group *grp, char *buffer, size_t bufsize, struct group **result)
{
    const struct group *current = w32_group_user();

    if (NULL == grp || NULL == buffer || NULL == result) {
        if (result) *result = NULL;
        errno = EINVAL;
        return EINVAL;                          // invalid arguments
    }

    *result = NULL;
    if (gid == current->gr_gid) {
        const int rc = copy_group(current, grp, buffer, bufsize);
        if (0 == rc) *result = grp;             // success
        return rc;

    } else {
        const struct group *it, *end;
        fill_groups();
        for (it = x_groups, end = it + x_groups_count; it != end; ++it) {
            if (gid == it->gr_gid) {
                const int rc = copy_group(it, grp, buffer, bufsize);
                if (0 == rc) *result = grp;     // success
                return rc;
            }
        }
    }
    return 0;                                   // no-match
}


/*
//  NAME
//      getgrnam - search group database for a name
//
//  SYNOPSIS
//      #include <grp.h>
//      struct group *getgrnam(const char *name);
//      int getgrnam_r(const char *name, struct group *grp, char *buffer,
//              size_t bufsize, struct group **result);
//
//  DESCRIPTION
//      The getgrnam() function shall search the group database for an entry with a
//      matching name.
//
//      The getgrnam() function need not be reentrant. A function that is not required to
//      be reentrant is not required to be thread-safe.
//
//      The getgrnam_r() function shall update the group structure pointed to by grp and store
//      a pointer to that structure at the location pointed to by result. The structure shall
//      contain an entry from the group database with a matching gid or name. Storage referenced
//      by the group structure is allocated from the memory provided with the buffer parameter,
//      which is bufsize bytes in size. The maximum size needed for this buffer can be determined
//      with the {_SC_GETGR_R_SIZE_MAX} sysconf() parameter. A NULL pointer is returned at the
//      location pointed to by result on error or if the requested entry is not found.
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
//
//      The getgrnam_r() function may fail if:
//
//      [ERANGE]
//          Insufficient storage was supplied via buffer and bufsize to contain the data to
//          be referenced by the resulting group structure.
*/
LIBW32_API struct group *
getgrnam(const char *name)
{
    if (name) {
        const struct group *current = w32_group_user();

        if (0 == _stricmp(name, current->gr_name)) {
            fill_group();
            return &x_group;

        } else {
            struct group *it, *end;
            fill_groups();
            for (it = x_groups, end = it + x_groups_count; it != end; ++it) {
                if (0 == _stricmp(name, it->gr_name)) {
                    return it;
                }
            }
        }
    }
    return NULL;
}


LIBW32_API int
getgrnam_r(const char *name, struct group *grp, char *buffer, size_t bufsize, struct group **result)
{
    const struct group *current = w32_group_user();

    if (NULL == name || NULL == grp || NULL == buffer || NULL == result) {
        if (result) *result = NULL;
        errno = EINVAL;
        return EINVAL;                          // invalid arguments
    }

    *result = NULL;
    if (0 == _stricmp(name, current->gr_name)) {
        const int rc = copy_group(current, grp, buffer, bufsize);
        if (0 == rc) *result = grp;             // success
        return rc;

    } else {
        const struct group *it, *end;
        fill_groups();
        for (it = x_groups, end = it + x_groups_count; it != end; ++it) {
            if (0 == _stricmp(name, it->gr_name)) {
                const int rc = copy_group(it, grp, buffer, bufsize);
                if (0 == rc) *result = grp;     // success
                return rc;
            }
        }
    }
    return 0;                                   // no-match
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
            grouplist[0] = w32_getgid();
            return 1;
        }
    }
    errno = EINVAL;
    return -1;
}


/*
//  NAME
//      setgroups -- set group access list
//
//  SYNOPSIS
//       #include <sys/param.h>
//       #include <unistd.h>
//
//      int setgroups(int ngroups, const gid_t *gidset);
//
//  DESCRIPTION
//       The setgroups() system call sets the group access list of the current user process according
//       to the array gidset.  The ngroups argument indicates the number of entries in the array and
//       must be no more than {NGROUPS_MAX}+1.
//
//  RETURN VALUES
//      The setgroups() function returns the value 0 if successful; otherwise the value -1 is returned
//      and the global variable errno is set to indicate the error.
//
//  ERRORS
//      The setgroups() system call will fail if:
//
//      [EPERM]
//          The caller is not the super-user.
//
//      [EINVAL]
//          The number specified in the ngroups argument is larger than the {NGROUPS_MAX}+1 limit.
//
//      [EFAULT]
//          The address specified for gidset is outside the process address space.
*/
LIBW32_API int
setgroups(size_t size, const gid_t *gidset)
{
    (void) size;
    (void) gidset;
    errno = EINVAL;
    return -1;
}


/////////////////////////////////////////////////////////////////////////////////////////
//  group's database implementation

static void
fill_groups(void)
{
    DWORD_PTR resume_handle = 0;
    NET_API_STATUS nStatus;
    unsigned cbufsz = 0;
    char name[MAX_PATH];
    int nlen;

    fill_group();
    if (NULL != x_groups)
        return;

    assert(0 == x_groups_count);
    do {
        DWORD i, dwEntriesRead = 0, dwTotalEntries = 0;
        const unsigned ototal = x_groups_count;
        unsigned bufsz = 0, count = 0;
        PGROUP_INFO_2 groups = NULL;

        nStatus = NetGroupEnum(NULL, 2 /*GROUP_INFO_2*/, (LPBYTE*) &groups,
                    MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, &resume_handle);

        switch (nStatus) {
        case NERR_Success:
        case ERROR_MORE_DATA:
            break;
        default:
            return;
        }

        // size storage
        for (i = 0; i < dwEntriesRead; ++i) {
            const PGROUP_INFO_2 group = groups + i;
            if ((int)group->grpi2_group_id == x_group.gr_gid ||
                    (nlen = w32_wc2utf(group->grpi2_name, name, sizeof(name))) <= 0) {
                continue;
            }
            bufsz += (nlen + 1);
            ++count;
        }

#if (TODO)
        if (NERR_Success == nStatus)            // last iteration.
            for (i = 0; i < _countof(well_known_sids); ++i) {
                const int nlen = fill_builtin(well_known_sids + i, NULL, NULL, 0);
                if (nlen > 0) {
                    bufsz += (nlen + 1);
                    ++count;
                }
            }
#endif

        // new elements
        if (count && bufsz) {
            const unsigned ntotal =
                    ototal + count;             // resulting total pwd's

            // allocate/expand
            if (x_groups) {
                struct group *t_groups = (struct group *)realloc(x_groups,
                                            (sizeof(struct group) * ntotal) + cbufsz + bufsz);
                const int addrdiff = ((char *)t_groups - (char *)x_groups) +
                                        (sizeof(struct group) * count);

                if (NULL == t_groups) {         // realloc failure
                    NetApiBufferFree(groups);
                    break;
                }

                // reorg storage, insert 'count' pwd elements and adjust buffer addr's.
                memmove(t_groups + ntotal, (const void *)(t_groups + ototal), cbufsz);
                for (i = 0; i < ototal; ++i) {
                    t_groups[i].gr_name += addrdiff;
                }
                x_groups = t_groups;

            } else {
                x_groups = (struct group *)malloc((sizeof(struct group) * count) + bufsz /*non-zero*/);
            }

            // publish
            if (NULL != x_groups) {
                struct group *grp = x_groups + ototal;
                char *cursor = ((char *)(x_groups + ntotal)) + cbufsz;
#if defined(_DEBUG)
                wchar_t t_buffer[1024];
#endif

                cbufsz += bufsz;                // resulting name storage (inc nul)

                for (i = 0; i < dwEntriesRead; ++i) {
                    const PGROUP_INFO_2 group = groups + i;

                    if ((int)group->grpi2_group_id == x_group.gr_gid ||
                            (nlen = w32_wc2utf(group->grpi2_name, name, sizeof(name))) <= 0) {
                        continue;
                    }

#if defined(_DEBUG)
                    swprintf_s(t_buffer, _countof(t_buffer), L"Group:%s,FullName:%s,RID:%u\n",
                          group->grpi2_name, group->grpi2_comment, (unsigned)group->grpi2_group_id);
                    OutputDebugStringW(t_buffer);
#endif

                    memset(grp, 0, sizeof(*grp));
                    grp->gr_name = cursor;
                    _strlwr(cursor);
                    grp->gr_gid = (short) group->grpi2_group_id;
                    cursor += (nlen + 1);
                    bufsz -= (nlen + 1);
                    ++x_groups_count;
                    --count;
                    ++grp;
                }

#if (TODO)
                if (NERR_Success == nStatus)    // last iteration.
                    for (i = 0; i < _countof(well_known_sids); ++i) {
                        const int nlen = fill_builtin(well_known_sids + i, pwd, cursor, bufsz);
                        if (nlen > 0) {
                            cursor += (nlen + 1);
                            bufsz -= (nlen + 1);
                            ++x_passwds_count;
                            --count;
                            ++pwd;
                        }
                    }
#endif

                assert(0 == count);
                assert(0 == bufsz);

            } else {
                nStatus = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        NetApiBufferFree(groups);

    } while (ERROR_MORE_DATA == nStatus);
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
fill_group(void)
{
    const struct group *grp = w32_group_user();
    copy_group(grp, &x_group, x_buffer, sizeof(x_buffer));
}


static int
gr_strlen(const char *s, size_t *total)
{
    if (s && *s) {
        const int slen = strlen(s);
        *total += (slen + 1);
        return slen;
    }
    *total += 1;
    return 0;
}


static char *
gr_strcpy(const char *s, size_t slen, char **cursor)
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
copy_group(const struct group *grp, struct group *dst, char *buffer, size_t bufsize)
{
    size_t total = 0;
    const size_t
        namelen     = gr_strlen(grp->gr_name, &total),
        passwdlen   = gr_strlen(grp->gr_passwd, &total);

    if (total > bufsize) {
        return (errno = ERANGE);
    } else if (NULL == dst) {
        return (errno = EINVAL);
    }

    dst->gr_name    = gr_strcpy(grp->gr_name,    namelen,     &buffer);
    dst->gr_passwd  = gr_strcpy(grp->gr_passwd,  passwdlen,   &buffer);
    dst->gr_gid     = grp->gr_gid;
    dst->gr_mem     = NULL;

    return 0; //success
}

/*end*/
