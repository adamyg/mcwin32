#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_dirent_c,"$Id: w32_dirent.c,v 1.29 2024/01/16 15:17:51 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 directory access services ...
 *
 *      opendir, closedir, readdir, seekdir, rewindir, telldir
 *
 * Copyright (c) 2007, 2012 - 2024 Adam Young.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT            0x0501          /* reparse */
#endif

#define _DIRENT_SOURCE
#include "win32_internal.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

#include "win32_io.h"
#include "win32_direct.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

#define SIZEOF_DIRENT           (sizeof(struct dirent) - sizeof(((struct dirent *)0)->d_name))

typedef BOOL (WINAPI *Wow64DisableWow64FsRedirection_t)(PVOID *OldValue);
typedef BOOL (WINAPI *Wow64RevertWow64FsRedirection_t)(PVOID OldValue);

static BOOL                     isshortcutA(const char *path);
static BOOL                     isshortcutW(const wchar_t *path);

static DIR *                    unc_populateA(const char *servername);
static DIR *                    unc_populateW(const wchar_t *servername);

static int                      dir_populateA(DIR *dp, const char *path);
static int                      dir_populateW(DIR *dp, const wchar_t *path);

static struct _dirlist *        dir_pushA(DIR *dp, const char *filename);
static struct _dirlist *        dir_pushW(DIR *dp, const wchar_t *filename);
static void                     dir_read(DIR *dp, struct dirent *ent);


static void                     dir_list_free(struct _dirlist *);

static int                      dir_ishpfA(const char *directory);
static int                      dir_ishpfW(const wchar_t *directory);

static int                      dir_errno(DWORD rc);

static BOOL                     d_Wow64DisableWow64FsRedirection(PVOID *OldValue);
static BOOL                     d_Wow64RevertWow64FsRedirection(PVOID OldValue);

static Wow64DisableWow64FsRedirection_t x_Wow64DisableWow64FsRedirection;
static Wow64RevertWow64FsRedirection_t x_Wow64RevertWow64FsRedirection;


/*
//  NAME
//      opendir - open a directory
//
//   SYNOPSIS
//      #include <sys/types.h>
//      #include <dirent.h>
//
//      DIR *opendir(const char *dirname);
//
//   DESCRIPTION
//      The opendir() function opens a directory stream corresponding to the directory
//      named by the dirname argument. The directory stream is positioned at the first
//      entry. If the type DIR, is implemented using a file descriptor, applications will
//      only be able to open up to a total of {OPEN_MAX} files and directories. A
//      successful call to any of the exec functions will close any directory streams that
//      are open in the calling process.
//
//   RETURN VALUE
//      Upon successful completion, opendir() returns a pointer to an object of type DIR.
//      Otherwise, a null pointer is returned and errno is set to indicate the error.
//
//   ERRORS
//      The opendir() function will fail if:
//
//      [EACCES]
//          Search permission is denied for the component of the path prefix of dirname or
//          read permission is denied for dirname.
//
//      [ELOOP]
//          Too many symbolic links were encountered in resolving path.
//
//      [ENAMETOOLONG]
//          The length of the dirname argument exceeds {PATH_MAX}, or a pathname component
//          is longer than {NAME_MAX}.
//
//      [ENOENT]
//          A component of dirname does not name an existing directory or dirname is an empty string.
//
//      [ENOTDIR]
//          A component of dirname is not a directory.
//
//      The opendir() function may fail if:
//
//      [EMFILE]
//          {OPEN_MAX} file descriptors are currently open in the calling process.
//
//      [ENAMETOOLONG]
//          Pathname resolution of a symbolic link produced an intermediate result whose
//          length exceeds {PATH_MAX}.
//
//      [ENFILE]
//          Too many files are currently open in the system.
*/
LIBW32_API DIR *
opendir(const char *dirname)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t wdirname[WIN32_PATH_MAX];

        if (NULL == dirname) {
            errno = EFAULT;
            return (DIR *)NULL;
        }

        if (w32_utf2wc(dirname, wdirname, _countof(wdirname)) > 0) {
            return opendirW(wdirname);
        }

        return NULL;
    }
#endif  //UTF8FILENAMES

    return opendirA(dirname);
}


LIBW32_API DIR *
opendirA(const char *dirname)
{
    char fullpath[ MAX_PATH ], symlink[ MAX_PATH ], reparse[ MAX_PATH ],
        *path = fullpath;
    LPVOID OldValue = NULL;
    DIR *dp;
    size_t len;
    int i;

    /* Copy to working buffer */
    if (NULL == dirname) {
        errno = EFAULT;
        return (DIR *)NULL;
    } else if (0 == (len = strlen(dirname))) {
        errno = ENOTDIR;
        return (DIR *)NULL;
    }

    memset(symlink, 0, sizeof(symlink));
    memset(reparse, 0, sizeof(reparse));

    /* Convert path (note, UNC safe) */
    if (NULL == w32_realpathA(dirname, fullpath, _countof(fullpath))) {
        char *last;                             /* unknown, assume DOS */

        strncpy(fullpath, dirname, _countof(fullpath));
        fullpath[_countof(fullpath)-1] = 0;
        for (i = 0; fullpath[i]; ++i) {
            if (fullpath[i] == '/') {
                fullpath[i] = '\\';             /* convert */
            }
        }
        last = &fullpath[len - 1];

        /*
         *  o/s can be very picky about its directory names; the following are valid.
         *      c:/   c:.   c:name/name1
         *
         *  whereas the following are not valid
         *      c:name/
         */
        if ((*last == '\\') && (len > 1) && (!((len == 3) &&
                    (fullpath[1] == ':')))) {
            *(last--) = 0;
        }
    }

    /* Is a directory ? */
    if (0 != strcmp(path, ".")) {
        UINT  errormode;
        DWORD attr;
        int rc = 0;

        errormode = SetErrorMode(0);            // disable hard errors
        if (INVALID_FILE_ATTRIBUTES == (attr = GetFileAttributesA(path))) {
            switch (GetLastError()) {
            case ERROR_ACCESS_DENIED:
            case ERROR_SHARING_VIOLATION:
                rc = EACCES;  break;
            case ERROR_FILE_NOT_FOUND:
                rc = ENOENT;  break;
            case ERROR_PATH_NOT_FOUND:
            case ERROR_INVALID_DRIVE:
                rc = ENOTDIR; break;
            default:
                rc = EIO;
            }

        } else if (0 == (FILE_ATTRIBUTE_DIRECTORY & attr)) {
            rc = ENOTDIR;
            if (isshortcutA(path)) {            // possible shortcut
                if (w32_readlinkA(path, symlink, _countof(symlink)) > 0) {
                    if ((attr = GetFileAttributesA(symlink)) != INVALID_FILE_ATTRIBUTES &&
                            (FILE_ATTRIBUTE_DIRECTORY & attr)) {
                        path = symlink;         // redirect
                        rc = 0;
                    }
                }
            }
        }
        (void) SetErrorMode(errormode);         // restore errors

        if (rc) {
            if (w32_unc_rootA(path, NULL)) {    // "//servername[/]"
                return unc_populateA(path);
            }
            errno = rc;
            return (DIR *)NULL;
        }

        if (attr & FILE_ATTRIBUTE_REPARSE_POINT) {
            if (-1 == w32_reparse_readA(path, reparse, _countof(reparse))) {
                errno = EACCES;
                return (DIR *)NULL;
            }
            path = reparse;
        }
    }

    /* Strip trailing slashes, so we can append "\*.*" */
    len = strlen(path);
    while (len > 0) {
        --len;
        if (path[len] == '\\') {
            path[len] = '\0';                   // remove slash
        } else {
            ++len;                              // end of path
            break;
        }
    }

    path[len++] = '\\';                         // insert pattern
    path[len++] = '*';
    path[len++] = '.';
    path[len++] = '*';
    path[len++] = 0;

    /* Open directory
     *
     *    If you are writing a 32-bit application to list all the files in a directory and the
     *    application may be running on a 64-bit computer, you should call Wow64DisableWow64FsRedirection
     *    before calling FindFirstFileEx and call Wow64RevertWow64FsRedirection after the last call to FindNextFile.
     *
     *    For more information, see File System Redirector.
     */
    if (NULL == (dp = w32_dir_alloc())) {
        return (DIR *)NULL;
    }

    if (d_Wow64DisableWow64FsRedirection(&OldValue)) {
        const int ret = dir_populateA(dp, path);

        if (! d_Wow64RevertWow64FsRedirection(OldValue) || ret) {
            closedir(dp);
            errno = (ret ? ret : EIO);
            dp = NULL;
        }
    }
    return dp;
}


LIBW32_API DIR *
opendirW(const wchar_t *dirname)
{
    wchar_t fullpath[ MAX_PATH ], symlink[ MAX_PATH ], reparse[ MAX_PATH ],
        *path = fullpath;
    LPVOID OldValue = NULL;
    DIR *dp;
    size_t len;
    int i;

    /* Copy to working buffer */
    if (NULL == dirname) {
        errno = EFAULT;
        return (DIR *)NULL;
    } else if (0 == (len = wcslen(dirname))) {
        errno = ENOTDIR;
        return (DIR *)NULL;
    }

    /* Convert path (note, UNC safe) */
    if (NULL == w32_realpathW(dirname, fullpath, _countof(fullpath))) {
        wchar_t *last;                          /* unknown, assume DOS */

        wcsncpy(fullpath, dirname, _countof(fullpath));
        fullpath[_countof(fullpath)-1] = 0;
        for (i = 0; fullpath[i]; ++i) {
            if (fullpath[i] == '/') {
                fullpath[i] = '\\';             /* convert */
            }
        }
        last = &fullpath[len - 1];

        /*
         *  o/s can be very picky about its directory names; the following are valid.
         *      c:/  c:.  c:name/name1
         *
         *  whereas the following are not valid
         *      c:name/
         */
        if ((*last == '\\') && (len > 1) && (!((len == 3) &&
                    (fullpath[1] == ':')))) {
            *(last--) = 0;
        }
    }

    /* Is a directory ? */
    if (0 != wcscmp(path, L".")) {
        UINT  errormode;
        DWORD attr;
        int rc = 0;

        errormode = SetErrorMode(0);            // disable hard errors
        if (INVALID_FILE_ATTRIBUTES == (attr = GetFileAttributesW(path))) {
            switch(GetLastError()) {
            case ERROR_ACCESS_DENIED:
            case ERROR_SHARING_VIOLATION:
                rc = EACCES;  break;
            case ERROR_FILE_NOT_FOUND:
                rc = ENOENT;  break;
            case ERROR_PATH_NOT_FOUND:
            case ERROR_INVALID_DRIVE:
                rc = ENOTDIR; break;
            default:
                rc = EIO;
            }

        } else if (0 == (FILE_ATTRIBUTE_DIRECTORY & attr)) {
            rc = ENOTDIR;
            if (isshortcutW(path)) {            // possible shortcut
                if (w32_readlinkW(path, symlink, _countof(symlink)) > 0) {
                    if ((attr = GetFileAttributesW(symlink)) != INVALID_FILE_ATTRIBUTES &&
                            (FILE_ATTRIBUTE_DIRECTORY & attr)) {
                        path = symlink;         // redirect
                        rc = 0;
                    }
                }
            }
        }
        (void) SetErrorMode(errormode);         // restore errors

        if (rc) {
            if (w32_unc_rootW(path, NULL)) {    // "//servername[/]"
                return unc_populateW(path);
            }
            errno = rc;
            return (DIR *)NULL;
        }

        if (attr & FILE_ATTRIBUTE_REPARSE_POINT) {
            if (-1 == w32_reparse_readW(path, reparse, _countof(reparse))) {
                errno = EACCES;
                return (DIR *)NULL;
            }
            path = reparse;
        }
    }

    /* Strip trailing slashes, so we can append "\*.*" */
    len = wcslen(path);
    while (len > 0) {
        --len;
        if (path[len] == '\\') {
            path[len] = '\0';                   // remove slash
        } else {
            ++len;                              // end of path
            break;
        }
    }

    path[len++] = '\\';                         // insert pattern
    path[len++] = '*';
    path[len++] = '.';
    path[len++] = '*';
    path[len++] = 0;

    /* Open directory
     *
     *    If you are writing a 32-bit application to list all the files in a directory and the
     *    application may be running on a 64-bit computer, you should call Wow64DisableWow64FsRedirection
     *    before calling FindFirstFileEx and call Wow64RevertWow64FsRedirection after the last call to FindNextFile.
     *
     *    For more information, see File System Redirector.
     */
    if (NULL == (dp = w32_dir_alloc())) {
        return (DIR *)NULL;
    }
    dp->dd_flags |= DIR_FISUTF8;

    if (d_Wow64DisableWow64FsRedirection(&OldValue)) {
        const int ret = dir_populateW(dp, path);

        if (! d_Wow64RevertWow64FsRedirection(OldValue) || ret) {
            closedir(dp);
            errno = (ret ? ret : EIO);
            dp = NULL;
        }
    }
    return dp;
}


static BOOL
isshortcutA(const char *name)
{
    const size_t len = strlen(name);
    const char *cursor;

    for (cursor = name + len; --cursor >= name;) {
        if (*cursor == '.') {                   // extension
            return (*++cursor && 0 == w32_io_stricmp(cursor, "lnk"));
        }
        if (*cursor == '/' || *cursor == '\\') {
            break;                              // delimiter
        }
    }
    return FALSE;
}


static BOOL
isshortcutW(const wchar_t *name)
{
    const size_t len = wcslen(name);
    const wchar_t *cursor;

    for (cursor = name + len; --cursor >= name;) {
        if (*cursor == '.') {                   // extension
            return (*++cursor && 0 == w32_io_wstricmp(cursor, "lnk"));
        }
        if (*cursor == '/' || *cursor == '\\') {
            break;                              // delimiter
        }
    }
    return FALSE;
}


static int
unc_push(void *data, const wchar_t *filename)
{
    if (dir_pushW((DIR *)data, filename)) {
        return 0; //success
    }
    return -1; //error
}


static DIR *
unc_populateA(const char *servername)
{
    DIR *dp;

    if (NULL == (dp = w32_dir_alloc()) ||       // alloc and populate
            -1 == w32_unc_iterateA(servername, unc_push, dp)) {
        w32_dir_free(dp);
        return (DIR *)NULL;
    }

    dp->dd_current = dp->dd_contents;           // seed cursor
    return dp;
}


static DIR *
unc_populateW(const wchar_t *servername)
{
    DIR *dp;

    if (NULL == (dp = w32_dir_alloc()) ||       // alloc and populate
            -1 == w32_unc_iterateW(servername, unc_push, dp)) {
        w32_dir_free(dp);
        return (DIR *)NULL;
    }

    dp->dd_current = dp->dd_contents;           // seed cursor
    return dp;
}


static int
dir_populateA(DIR *dp, const char *path)
{
    WIN32_FIND_DATAA fd = {0};
    HANDLE hSearch = INVALID_HANDLE_VALUE;
    struct _dirlist *dplist;
    UINT errormode;
    BOOL isHPFS = FALSE;
    int rc, ret = 0;

    errormode = SetErrorMode(0);                // disable hard errors
    hSearch = FindFirstFileA(path, &fd);
    (void) SetErrorMode(errormode);             // restore errors

    if (INVALID_HANDLE_VALUE == hSearch) {
        return dir_errno(GetLastError());
    }

    isHPFS = dir_ishpfA(path);                  // extended file system
    if (isHPFS) dp->dd_flags = DIR_FISHPF;

    do {

#if defined(FILE_ATTRIBUTE_VOLUME)              // skip volume labels
        // Not listed by Microsoft but it's there.
        //  Indicates a directory entry without corresponding file, used only to denote the name of a hard drive volume.
        //  Was used to 'hack' the long file name system of Windows 95.
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_VOLUME) {
            continue;
        }
#endif
                                                // skip '.'
        if ('.' == fd.cFileName[0] && 0 == fd.cFileName[1]) {
            continue;
        }

        if (NULL == (dplist = dir_pushA(dp, fd.cFileName))) {
            FindClose(hSearch);
            return ENOMEM;
        }

        dplist->dl_size2 = fd.nFileSizeHigh;
        dplist->dl_size = fd.nFileSizeLow;
        dplist->dl_attr = fd.dwFileAttributes;

        dplist->dl_type = DT_UNKNOWN;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            dplist->dl_type = DT_DIR;
        } else if ((fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && (fd.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT)) {
            dplist->dl_type = DT_LNK;
        } else {
            dplist->dl_type = DT_REG;
        }

    } while (FindNextFileA(hSearch, &fd));

    if ((rc = GetLastError()) == ERROR_NO_MORE_FILES) {
        dp->dd_current = dp->dd_contents;       // seed cursor
        dp->dd_flags |= DIR_FHAVESTATS;
    } else {
        ret = dir_errno(rc);
    }

    FindClose(hSearch);
    return ret;
}


static int
dir_populateW(DIR *dp, const wchar_t *path)
{
    WIN32_FIND_DATAW fd = {0};
    struct _dirlist *dplist = NULL;
    HANDLE hSearch = INVALID_HANDLE_VALUE;
    UINT errormode;
    BOOL isHPFS = FALSE;
    int rc, ret = 0;

    errormode = SetErrorMode(0);                // disable hard errors
    hSearch = FindFirstFileW(path, &fd);
    (void) SetErrorMode(errormode);             // restore errors

    if (INVALID_HANDLE_VALUE == hSearch) {
        return dir_errno(GetLastError());
    }

    isHPFS = dir_ishpfW(path);                  // extended file system
    if (isHPFS) dp->dd_flags |= DIR_FISHPF;
    dp->dd_flags |= DIR_FISUTF8;

    do {

#if defined(FILE_ATTRIBUTE_VOLUME)              // skip volume labels
        // Not listed by Microsoft but it's there.
        //  Indicates a directory entry without corresponding file, used only to denote the name of a hard drive volume.
        //  Was used to 'hack' the long file name system of Windows 95.
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_VOLUME) {
            continue;
        }
#endif
                                                // skip '.'
        if ('.' == fd.cFileName[0] && 0 == fd.cFileName[1]) {
            continue;
        }

        if (NULL == (dplist = dir_pushW(dp, fd.cFileName))) {
            FindClose(hSearch);
            return ENOMEM;
        }

        dplist->dl_size2 = fd.nFileSizeHigh;
        dplist->dl_size = fd.nFileSizeLow;
        dplist->dl_attr = fd.dwFileAttributes;

        dplist->dl_type = DT_UNKNOWN;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            dplist->dl_type = DT_DIR;
        } else if ((fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && (fd.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT)) {
            dplist->dl_type = DT_LNK;
        } else {
            dplist->dl_type = DT_REG;
        }

    } while (FindNextFileW(hSearch, &fd));

    if ((rc = GetLastError()) == ERROR_NO_MORE_FILES) {
        dp->dd_current = dp->dd_contents;       // seed cursor
        dp->dd_flags |= DIR_FHAVESTATS;
    } else {
        ret = dir_errno(rc);
    }

    FindClose(hSearch);
    return ret;
}


DIR *
w32_dir_alloc(void)
{
    const int dd_len = SIZEOF_DIRENT + (sizeof(char) * DIRBLKSIZ); /* working dirent storage */
    DIR *dp;

#if defined(NAME_MAX)
    assert(MAXNAMLEN >= NAME_MAX);              /* POSIX requirement, verify */
#endif
    assert(DIRBLKSIZ > MAXNAMLEN);

    if (NULL == (dp = (DIR *)calloc(sizeof(DIR), 1)) ||
            NULL == (dp->dd_buf = (void *)calloc(dd_len, 1))) {
        free(dp);
        return (DIR *)NULL;
    }

    dp->dd_magic = DIR_MAGIC;                   /* structure magic */
    dp->dd_len = dd_len;                        /* underlying dd_buf length, in bytes */
    dp->dd_id = w32_dir_identifier();           /* generate unique directory identifier */
    dp->dd_fd = -1;                             /* file descriptor */

    return dp;
}


int
w32_dir_identifier(void)
{
    static int dir_identifer;
    return ++dir_identifer;
}


void
w32_dir_free(DIR *dp)
{
    if (dp) {
        assert(DIR_MAGIC == dp->dd_magic);
        dir_list_free(dp->dd_contents);
        free((void *)dp->dd_buf);               /* working dirent storage */
        free((void *)dp);
    }
}


static int
dir_errno(DWORD rc)
{
    switch (rc) {
#if defined(ERROR_EMPTY_DIR)
    case ERROR_EMPTY_DIR:
        return 0;
#endif
    case ERROR_NO_MORE_FILES:
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
        return ENOENT;
    case ERROR_NOT_ENOUGH_MEMORY:
        return ENOMEM;
    }
    return EINVAL;
}


/*
 *  dir_pushA ---
 *      Create a directory list element.
 */
static struct _dirlist *
dir_pushA(DIR *dp, const char *filename)
{
    size_t nambytes, namlen = strlen(filename);
    struct _dirlist *dplist;

    assert(namlen < DIRBLKSIZ);                 // d_name limit
    if (namlen >= DIRBLKSIZ) namlen = DIRBLKSIZ-1;

    nambytes = sizeof(wchar_t) * (namlen + 1 /*nul*/);
    if (NULL == (dplist =
            (struct _dirlist *)malloc(sizeof(struct _dirlist) + nambytes))) {
        return NULL;
    }

    memset(dplist, 0, sizeof(*dplist));
    if (dp->dd_contents) {
        dp->dd_current =
            dp->dd_current->dl_next = dplist;
    } else {
        dp->dd_contents = dp->dd_current = dplist;
    }

    dplist->dl_namlen = (unsigned short)namlen;
    memcpy(dplist->dl_name, filename, nambytes);
    dplist->dl_next = NULL;
    return dplist;
}


/*
 *  dir_pushW ---
 *      Create a directory list element.
 */
struct _dirlist *
dir_pushW(DIR *dp, const wchar_t *filename)
{
    char t_filename[DIRBLKSIZ];                 // d_name limit

    w32_wc2utf(filename, t_filename, _countof(t_filename));
    return dir_pushA(dp, t_filename);
}



/*
 *  dir_read ---
 *      Read the next directory element.
 */
static void
dir_read(DIR *dp, struct dirent *ent)
{
    struct _dirlist *dplist = dp->dd_current;
    size_t nambytes;

    assert(dplist);

    /* Standard fields */
    assert((dplist->dl_namlen + 1 /*nul*/) <= DIRBLKSIZ);
    nambytes = sizeof(char) * (dplist->dl_namlen + 1 /*nul*/);
    memcpy(ent->d_name, dplist->dl_name, nambytes);
    ent->d_namlen = dplist->dl_namlen;
    ent->d_reclen = (unsigned short)(SIZEOF_DIRENT + nambytes);

    if (0 == (dp->dd_flags & DIR_FISHPF)) {     // not HPFS, convert case.
#if defined(_WIN32) && defined(_MSC_VER)
        _strlwr(ent->d_name);
#else
        strlwr(ent->d_name);
#endif
    }
    ent->d_fileno = 0;

    /* Extension fields */
    ent->d_ctime = dplist->dl_ctime;
    ent->d_mtime = dplist->dl_mtime;
    ent->d_size = dplist->dl_size;
    ent->d_attr = dplist->dl_attr;
    ent->d_type = dplist->dl_type;

    /* Update current location */
    dp->dd_current = dplist->dl_next;
    ++dp->dd_loc;
}


/*
 *  dir_list_free ---
 *      Dispose of the directory list.
 */
static void
dir_list_free(struct _dirlist *dplist)
{
    struct _dirlist *odplist;

    while (dplist) {
        dplist = (odplist = dplist)->dl_next;
        free(odplist);
    }
}


/*
//  NAME
//      closedir - close a directory stream
//
//  SYNOPSIS
//      #include <dirent.h>
//      int closedir(DIR *dirp);
//
//  DESCRIPTION
//      The closedir() function shall close the directory stream referred to by the
//      argument dirp. Upon return, the value of dirp may no longer point to an accessible
//      object of the type DIR. If a file descriptor is used to implement type DIR, that
//      file descriptor shall be closed.
//
//  RETURN VALUE
//      Upon successful completion, closedir() shall return 0; otherwise, -1 shall be
//      returned and errno set to indicate the error.
//
//   ERRORS
//      The closedir() function may fail if:
//
//      [EBADF]
//          The dirp argument does not refer to an open directory stream.
//
//      [EINTR]
//          The closedir() function was interrupted by a signal.
*/
LIBW32_API int
closedir(DIR *dp)
{
    if (NULL == dp) {
        errno = EBADF;
        return -1;
    }
    w32_dir_free(dp);
    return 0;
}


/*
//  NAME
//      readdir, readdir_r - read a directory
//
//  SYNOPSIS
//      #include <dirent.h>
//
//      struct dirent *readdir(DIR *dirp);
//      int readdir_r(DIR *restrict dirp, struct dirent *restrict entry,
//              struct dirent **restrict result);
//
//  DESCRIPTION
//      The type DIR, which is defined in the <dirent.h> header, represents a directory
//      stream, which is an ordered sequence of all the directory entries in a particular
//      directory. Directory entries represent files; files may be removed from a directory
//      or added to a directory asynchronously to the operation of readdir().
//
//      The readdir() function shall return a pointer to a structure representing the
//      directory entry at the current position in the directory stream specified by the
//      argument dirp, and position the directory stream at the next entry. It shall return
//      a null pointer upon reaching the end of the directory stream. The structure dirent
//      defined in the <dirent.h> header describes a directory entry.
//
//      The readdir() function shall not return directory entries containing empty names.
//      If entries for dot or dot-dot exist, one entry shall be returned for dot and one
//      entry shall be returned for dot-dot; otherwise, they shall not be returned.
//
//      The pointer returned by readdir() points to data which may be overwritten by
//      another call to readdir() on the same directory stream. This data is not
//      overwritten by another call to readdir() on a different directory stream.
//
//      If a file is removed from or added to the directory after the most recent call to
//      opendir() or rewinddir(), whether a subsequent call to readdir() returns an entry
//      for that file is unspecified.
//
//      The readdir() function may buffer several directory entries per actual read
//      operation; readdir() shall mark for update the st_atime field of the directory each
//      time the directory is actually read.
//
//      After a call to fork(), either the parent or child (but not both) may continue
//      processing the directory stream using readdir(), rewinddir(), [XSI] [Option Start]
//      or seekdir(). [Option End] If both the parent and child processes use these
//      functions, the result is undefined.
//
//      If the entry names a symbolic link, the value of the d_ino member is unspecified.
//
//      The readdir() function need not be reentrant. A function that is not required to be
//      reentrant is not required to be thread-safe.
//
//      [TSF] [Option Start] The readdir_r() function shall initialize the dirent structure
//      referenced by entry to represent the directory entry at the current position in the
//      directory stream referred to by dirp, store a pointer to this structure at the
//      location referenced by result, and position the directory stream at the next entry.
//
//      The storage pointed to by entry shall be large enough for a dirent with an array of
//      char d_name members containing at least {NAME_MAX}+1 elements.
//
//      Upon successful return, the pointer returned at *result shall have the same value
//      as the argument entry. Upon reaching the end of the directory stream, this pointer
//      shall have the value NULL.
//
//      The readdir_r() function shall not return directory entries containing empty names.
//
//      If a file is removed from or added to the directory after the most recent call to
//      opendir() or rewinddir(), whether a subsequent call to readdir_r() returns an entry
//      for that file is unspecified.
//
//      The readdir_r() function may buffer several directory entries per actual read
//      operation; the readdir_r() function shall mark for update the st_atime field of the
//      directory each time the directory is actually read. [Option End]
//
//      Applications wishing to check for error situations should set errno to 0 before
//      calling readdir(). If errno is set to non-zero on return, an error occurred.
//
//  RETURN VALUE
//      Upon successful completion, readdir() shall return a pointer to an object of type
//      struct dirent. When an error is encountered, a null pointer shall be returned and
//      errno shall be set to indicate the error. When the end of the directory is
//      encountered, a null pointer shall be returned and errno is not changed.
//
//      If successful, the readdir_r() function shall return zero; otherwise, an error
//      number shall be returned to indicate the error. [Option End]
//
//  ERRORS
//      The readdir() function shall fail if:
//
//      [EOVERFLOW]
//          One of the values in the structure to be returned cannot be represented correctly.
//
//      The readdir() function may fail if:
//
//      [EBADF]
//          The dirp argument does not refer to an open directory stream.
//
//      [ENOENT]
//          The current position of the directory stream is invalid.
//
//      The readdir_r() function may fail if:
//
//      [EBADF]
//          The dirp argument does not refer to an open directory stream.
//
//      [ENAMETOOLONG]
//          A directory entry whose name was too long to be read was encountered.
//
*/
LIBW32_API struct dirent *
readdir(DIR *dp)
{
    struct dirent *entry = (struct dirent *)dp->dd_buf; // working buffer.

    if (NULL == dp) {
        errno = EBADF;
        return (struct dirent *)NULL;
    }
    assert(DIR_MAGIC == dp->dd_magic);
    if (DIR_MAGIC != dp->dd_magic) {
        errno = EBADF;
        return (struct dirent *)NULL;
    }

    if ((struct _dirlist *)NULL == dp->dd_current) {
        errno = ENOENT;
        return (struct dirent *)NULL;
    }

    dir_read(dp, entry);                        // retrieve next entry.

    return entry;
}


LIBW32_API int
readdir_r(DIR *dp, struct dirent *entry, struct dirent **result)
{
    if (NULL == dp) {
        return EBADF;
    } else if (NULL == entry || NULL == result) {
        return EINVAL;
    }
    assert(DIR_MAGIC == dp->dd_magic);
    if (DIR_MAGIC != dp->dd_magic) {
        *result = NULL;
        return EBADF;
    }

    if ((struct _dirlist *)NULL == dp->dd_current) {
        *result = NULL;
        return ENOENT;
    }

    if (dp->dd_current->dl_namlen > MAXNAMLEN) {
        *result = NULL;
        return ENAMETOOLONG;
    }

    dir_read(dp, entry);                        // retrieve next entry.
    *result = entry;

    return 0;
}


/*
//  NAME
//      seekdir - set the position of a directory stream
//
//  SYNOPSIS
//      #include <dirent.h>
//
//      void seekdir(DIR *dirp, long loc);
//
//  DESCRIPTION
//      The seekdir() function shall set the position of the next readdir() operation on
//      the directory stream specified by dirp to the position specified by loc. The value
//      of loc should have been returned from an earlier call to telldir(). The new
//      position reverts to the one associated with the directory stream when telldir() was
//      performed.
//
//      If the value of loc was not obtained from an earlier call to telldir(), or if a
//      call to rewinddir() occurred between the call to telldir() and the call to
//      seekdir(), the results of subsequent calls to readdir() are unspecified.
//
//  RETURN VALUE
//      The seekdir() function shall not return a value.
//
//  ERRORS
//      No errors are defined.
*/
LIBW32_API void
seekdir(DIR *dp, long off)
{
    struct _dirlist *dplist;
    long i = off;

    if (NULL == dp) {
        errno = EBADF;
        return;
    }
    assert(DIR_MAGIC == dp->dd_magic);

    if (off >= 0) {
        for (dplist = dp->dd_contents; --i >= 0 && dplist; dplist = dplist->dl_next) {
            /*continue*/;
        }
        dp->dd_loc = off - (i + 1);
        dp->dd_current = dplist;
    }
}


/*
//  NAME
//      rewinddir - reset position of directory stream to the beginning of a directory
//
//  SYNOPSIS
//      #include <sys/types.h>
//      #include <dirent.h>
//
//      void rewinddir(DIR *dirp);
//
//  DESCRIPTION
//      The rewinddir() function resets the position of the directory stream to which dirp
//      refers to the beginning of the directory. It also causes the directory stream to
//      refer to the current state of the corresponding directory, as a call to opendir()
//      would have done. If dirp does not refer to a directory stream, the effect is
//      undefined.
//
//      After a call to the fork() function, either the parent or child (but not both) may
//      continue processing the directory stream using readdir(), rewinddir() or seekdir().
//      If both the parent and child processes use these functions, the result is undefined.
//
//  RETURN VALUE
//      The rewinddir() function does not return a value.
//
//  ERRORS
//      No errors are defined.
*/
LIBW32_API void
rewinddir(DIR *dp)
{
    if (NULL == dp) {
        errno = EBADF;
        return;
    }
    assert(DIR_MAGIC == dp->dd_magic);
    seekdir(dp, 0);
}


/*
//  NAME
//      telldir - current location of a named directory stream
//
//  SYNOPSIS
//      #include <dirent.h>
//      long telldir(DIR *dirp);
//
//  DESCRIPTION
//      The telldir() function shall obtain the current location associated with the
//      directory stream specified by dirp.
//
//      If the most recent operation on the directory stream was a seekdir(), the directory
//      position returned from the telldir() shall be the same as that supplied as a loc
//      argument for seekdir().
//
//  RETURN VALUE
//      Upon successful completion, telldir() shall return the current location of the
//      specified directory stream.
//
//  ERRORS
//      No errors are defined.
*/
LIBW32_API long
telldir(DIR *dp)
{
    if (dp == (DIR *)NULL) {
        errno = EBADF;
        return -1;
    }
    assert(DIR_MAGIC == dp->dd_magic);
    return (dp->dd_loc);
}


/*
 *  Wow64DisableWow64FsRedirection/Wow64RevertWow64FsRedirection --
 *      Disables file system redirection for the calling thread. File system
 *      redirection is enabled by default.
 *
 *      This function is useful for 32-bit applications that want to gain access to the
 *      native system32 directory. By default, WOW64 file system redirection is enabled.
 *
 *      The Wow64DisableWow64FsRedirection/Wow64RevertWow64FsRedirection function
 *      pairing is a replacement for the functionality of the
 *      Wow64EnableWow64FsRedirection function.
 *
 *      To restore file system redirection, call the Wow64RevertWow64FsRedirection
 *      function. Every successful call to the Wow64DisableWow64FsRedirection function
 *      must have a matching call to the Wow64RevertWow64FsRedirection function. This
 *      will ensure redirection is re-enabled and frees associated system resources.
 *
 *      Note The Wow64DisableWow64FsRedirection function affects all file operations
 *      performed by the current thread, which can have unintended consequences if file
 *      system redirection is disabled for any length of time. For example, DLL loading
 *      depends on file system redirection, so disabling file system redirection will
 *      cause DLL loading to fail. Also, many feature implementations use delayed
 *      loading and will fail while redirection is disabled. The failure state of the
 *      initial delay-load operation is persisted, so any subsequent use of the
 *      delay-load function will fail even after file system redirection is re-enabled.
 *      To avoid these problems, disable file system redirection immediately before
 *      calls to specific file I/O functions (such as CreateFile) that must not be
 *      redirected, and re-enable file system redirection immediately afterward using
 *      Wow64RevertWow64FsRedirection.
 *
 *      Disabling file system redirection affects only operations made by the current
 *      thread. Some functions, such as CreateProcessAsUser, do their work on another
 *      thread, which is not affected by the state of file system redirection in the
 *      calling thread.
 */

static BOOL WINAPI
my_Wow64DisableWow64FsRedirection(PVOID *OldValue)
{
    (void)OldValue;
    return TRUE;
}


static BOOL
d_Wow64DisableWow64FsRedirection(PVOID *OldValue)
{
    if (NULL == x_Wow64DisableWow64FsRedirection) {
#if defined(ENABLE_WOW64)   /*11/01/14*/
        HINSTANCE hinst;                        // Vista+

        if (0 == (hinst = LoadLibrary("Kernel32")) ||
                0 == (x_Wow64DisableWow64FsRedirection =
                    (Wow64DisableWow64FsRedirection_t)
                            GetProcAddress(hinst, "Wow64DisableWow64FsRedirection")) ||
                0 == (x_Wow64RevertWow64FsRedirection =
                    (Wow64RevertWow64FsRedirection_t)
                            GetProcAddress(hinst, "Wow64RevertWow64FsRedirection"))) {
                                                // XP+
            x_Wow64DisableWow64FsRedirection = my_Wow64DisableWow64FsRedirection;
            x_Wow64RevertWow64FsRedirection = NULL;
            if (hInst) FreeLibrary(hinst);
        }
#else
        x_Wow64DisableWow64FsRedirection = my_Wow64DisableWow64FsRedirection;
        x_Wow64RevertWow64FsRedirection = NULL;
#endif
    }
    return x_Wow64DisableWow64FsRedirection(OldValue);
}


static BOOL
d_Wow64RevertWow64FsRedirection(PVOID OldValue)
{
    if (x_Wow64RevertWow64FsRedirection) {
        return x_Wow64RevertWow64FsRedirection(OldValue);
    }
    return TRUE;
}


/*
 *  IsHPFS ---
 *      Is High Performance File System.
 */
static int
dir_ishpfA(const char *directory)
{
    int namelen;
    UINT errormode;
    DWORD flags = 0, maxname;
    BOOL rc = 0;

    if ((namelen = w32_unc_validA(directory)) > 0) {
        char rootdir[MAXHOSTNAMELEN + MAX_PATH],
           *cursor = rootdir, *end = cursor + (_countof(rootdir) - 4);
        int i;

        directory += 2;                         // "//" or "\\"
        *cursor++ = '\\'; *cursor++ = '\\';
        for (i = namelen; i > 0; --i) {
            *cursor++ = *directory++;
        }
        *cursor++ = '\\';
        if (*directory++) {                     // component
            char ch;
            while (cursor < end && (ch = *directory++) != 0) {
                if (IS_PATH_SEP(ch)) break;
                *cursor++ = ch;
            }
            *cursor++ = '\\';
        }
        *cursor = 0;

        errormode = SetErrorMode(0);            // disable hard errors
        rc = GetVolumeInformationA(rootdir, (LPSTR)NULL, 0,
                    (LPDWORD)NULL, &maxname, &flags, (LPSTR)NULL, 0);
        (void) SetErrorMode(errormode);         // restore errors

    } else {
        char rootdir[4] = "x:\\";
        int driveno;

        if (directory &&
                isalpha((unsigned char)directory[0]) && directory[1] == ':') {
            driveno = toupper(directory[0]) - 'A';
        } else {
            if (0 == (driveno = w32_getdrive())) {
                return 0;
            }
            --driveno;
        }

        rootdir[0] = (char)(driveno + 'A');
        errormode = SetErrorMode(0);            // disable hard errors
        rc = GetVolumeInformationA(rootdir, (LPSTR)NULL, 0,
                    (LPDWORD)NULL, &maxname, &flags, (LPSTR)NULL, 0);
        (void) SetErrorMode(errormode);         // restore errors
    }

    return ((rc) &&
        (flags & (FS_CASE_SENSITIVE | FS_CASE_IS_PRESERVED))) ? TRUE : FALSE;
}


static int
dir_ishpfW(const wchar_t *directory)
{
    int namelen;
    UINT errormode;
    DWORD flags = 0, maxname;
    BOOL rc = 0;

    if ((namelen = w32_unc_validW(directory)) > 0) {
        wchar_t rootdir[MAXHOSTNAMELEN + MAX_PATH],
           *cursor = rootdir, *end = cursor + (_countof(rootdir) - 4);
        int i;

        directory += 2;                         // "//" or "\\"
        *cursor++ = '\\'; *cursor++ = '\\';
        for (i = namelen; i > 0; --i) {
            *cursor++ = *directory++;
        }
        *cursor++ = '\\';
        if (*directory++) {                     // component
            wchar_t ch;
            while (cursor < end && (ch = *directory++) != 0) {
                if (IS_PATH_SEP(ch)) break;
                *cursor++ = ch;
            }
            *cursor++ = '\\';
        }
        *cursor = 0;

        errormode = SetErrorMode(0);            // disable hard errors
        rc = GetVolumeInformationW(rootdir, (LPWSTR)NULL, 0,
                    (LPDWORD)NULL, &maxname, &flags, (LPWSTR)NULL, 0);
        (void) SetErrorMode(errormode);         // restore errors

    } else {
        wchar_t rootdir[4] = L"x:\\";
        int driveno;

        if (directory &&
                isalpha((unsigned char)directory[0]) && directory[1] == ':') {
            driveno = toupper(directory[0]) - 'A';
        } else {
            if (0 == (driveno = w32_getdrive())) {
                return 0;
            }
            --driveno;
        }

        rootdir[0] = (char)(driveno + 'A');
        errormode = SetErrorMode(0);            // disable hard errors
        rc = GetVolumeInformationW(rootdir, (LPWSTR)NULL, 0,
                    (LPDWORD)NULL, &maxname, &flags, (LPWSTR)NULL, 0);
        (void) SetErrorMode(errormode);         // restore errors
    }

    return ((rc) &&
        (flags & (FS_CASE_SENSITIVE | FS_CASE_IS_PRESERVED))) ? TRUE : FALSE;
}

/*end*/
