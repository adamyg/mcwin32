#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_wdirent_c,"$Id: w32_wdirent.c,v 1.10 2025/02/16 12:04:05 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 directory access services ...
 *
 *      _wopendir, _wclosedir, _wreaddir, _wseekdir, _wrewindir, _wtelldir
 *
 * Copyright (c) 2021 - 2025 Adam Young.
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

#define SIZEOF_WDIRENT          (sizeof(struct _wdirent) - sizeof(((struct _wdirent *)0)->d_name))

typedef BOOL (WINAPI *Wow64DisableWow64FsRedirection_t)(PVOID *OldValue);
typedef BOOL (WINAPI *Wow64RevertWow64FsRedirection_t)(PVOID OldValue);

static BOOL                     isshortcut(const wchar_t *path);

static _WDIR *                  unc_populate(const wchar_t *servername);

static int                      dir_populate(_WDIR *dp, const wchar_t *path);
static _WDIR *                  dir_alloc(void);
static void                     dir_free(_WDIR *dp);
static struct _wdirlist *       dir_push(_WDIR *dp, const wchar_t *filename);
static void                     dir_read(_WDIR *dp, struct _wdirent *entry);
static void                     dir_list_free(struct _wdirlist *);
static int                      dir_ishpf(const wchar_t *directory);
static int                      dir_errno(DWORD rc);

static BOOL                     d_Wow64DisableWow64FsRedirection(PVOID *OldValue);
static BOOL                     d_Wow64RevertWow64FsRedirection(PVOID OldValue);

static Wow64DisableWow64FsRedirection_t x_Wow64DisableWow64FsRedirection;
static Wow64RevertWow64FsRedirection_t x_Wow64RevertWow64FsRedirection;


/*
//  NAME
//      opendir - open a directory
//
//  SYNOPSIS
//      #include <sys/types.h>
//      #include <dirent.h>
//
//      _WDIR *_wopendir(const char *dirname);
*/

LIBW32_API _WDIR *
_wopendir(const wchar_t *dirname)
{
    wchar_t fullpath[ MAX_PATH ], symlink[ MAX_PATH ], reparse[ MAX_PATH ],
        *path = fullpath;
    LPVOID OldValue = NULL;
    _WDIR *dp;
    size_t len;
    int i;

    /* Copy to working buffer */
    if (NULL == dirname) {
        errno = EFAULT;
        return (_WDIR *)NULL;
    } else if (0 == (len = wcslen(dirname))) {
        errno = ENOTDIR;
        return (_WDIR *)NULL;
    }

    memset(symlink, 0, sizeof(symlink));
    memset(reparse, 0, sizeof(reparse));

    /* Convert path (note, UNC safe) */
    if (NULL == w32_realpathW(dirname, fullpath, _countof(fullpath))) {
        wchar_t *last;                          // unknown, assume DOS

        wcsncpy(fullpath, dirname, _countof(fullpath));
        fullpath[_countof(fullpath)-1] = 0;
        for (i = 0; fullpath[i]; ++i) {
            if (fullpath[i] == '/') {
                fullpath[i] = '\\';             // convert
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
            if (isshortcut(path)) {             // possible shortcut
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
                return unc_populate(path);
            }
            errno = rc;
            return (_WDIR *)NULL;
        }

        if (attr & FILE_ATTRIBUTE_REPARSE_POINT) {
            if (-1 == w32_reparse_readW(path, reparse, _countof(reparse))) {
                errno = EACCES;
                return (_WDIR *)NULL;
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
    if (NULL == (dp = dir_alloc())) {
        return (_WDIR *)NULL;
    }
    dp->dd_flags |= DIR_FISUTF8;

    if (d_Wow64DisableWow64FsRedirection(&OldValue)) {
        const int ret = dir_populate(dp, path);

        if (! d_Wow64RevertWow64FsRedirection(OldValue) || ret) {
            _wclosedir(dp);
            errno = (ret ? ret : EIO);
            dp = NULL;
        }
    }
    return dp;
}


static BOOL
isshortcut(const wchar_t *name)
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


/*
 *  populate a unc root directory.
 */
static int
unc_push(void *data, const wchar_t *filename)
{
    if (dir_push((_WDIR *)data, filename)) {
        return 0; //success
    }
    return -1; //error
}


static _WDIR *
unc_populate(const wchar_t *servername)
{
    _WDIR *dp;

    if (NULL == (dp = dir_alloc()) ||           // alloc and populate
            -1 == w32_unc_iterateW(servername, unc_push, dp)) {
        dir_free(dp);
        return (_WDIR *)NULL;
    }

    dp->dd_current = dp->dd_contents;           // seed cursor
    return dp;
}


/*
 *  populate a directory.
 */
static int
dir_populate(_WDIR *dp, const wchar_t *path)
{
    WIN32_FIND_DATAW fd = {0};
    struct _wdirlist *dplist = NULL;
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

    isHPFS = dir_ishpf(path);                   // extended file system
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

        if (NULL == (dplist = dir_push(dp, fd.cFileName))) {
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


/*
 *  allocate a directory structure.
 */
static _WDIR *
dir_alloc(void)
{
    const int dd_len = SIZEOF_WDIRENT + (sizeof(wchar_t) * DIRBLKSIZ); /* working dirent storage */
    _WDIR *dp;

#if defined(NAME_MAX)
    assert(MAXNAMLEN >= NAME_MAX);              /* POSIX requirement, verify */
#endif
    assert(DIRBLKSIZ > MAXNAMLEN);

    if (NULL == (dp = (_WDIR *)calloc(1, sizeof(_WDIR))) ||
            NULL == (dp->dd_buf = (void *)calloc(dd_len, sizeof(char)))) {
        free(dp);
        return (_WDIR *)NULL;
    }

    dp->dd_magic = DIR_WMAGIC;                  /* structure magic */
    dp->dd_len = dd_len;                        /* underlying dd_buf length, in bytes */
    dp->dd_id = w32_dir_identifier();           /* generate unique directory identifier */
    dp->dd_fd = -1;                             /* file descriptor */

    return dp;
}


/*
 *  release a directory structure.
 */
static void
dir_free(_WDIR *dp)
{
    if (dp) {
        assert(DIR_WMAGIC == dp->dd_magic);
        dir_list_free(dp->dd_contents);
        free((void *)dp->dd_buf);               /* working dirent storage */
        free((void *)dp);
    }
}


/*
 *  errno mapping.
 */
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
 *  Create a directory list element.
 */
static struct _wdirlist *
dir_push(_WDIR *dp, const wchar_t *filename)
{
    size_t nambytes, namlen = wcslen(filename);
    struct _wdirlist *dplist;

    assert(namlen < DIRBLKSIZ);                 // d_name limit
    if (namlen >= DIRBLKSIZ) namlen = DIRBLKSIZ-1;

    nambytes = sizeof(wchar_t) * (namlen + 1 /*nul*/);
    if (NULL == (dplist =
            (struct _wdirlist *)malloc(sizeof(struct _wdirlist) + nambytes))) {
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
 *  dir_read ---
 *      Read the next directory element.
 */
static void
dir_read(_WDIR *dp, struct _wdirent *ent)
{
    struct _wdirlist *dplist = dp->dd_current;
    size_t nambytes;

    assert(dplist);

    /* Standard fields */
    assert((dplist->dl_namlen + 1 /*nul*/) <= DIRBLKSIZ);
    nambytes = sizeof(wchar_t) * (dplist->dl_namlen + 1 /*nul*/);
    memcpy(ent->d_name, dplist->dl_name, nambytes);
    ent->d_namlen = dplist->dl_namlen;
    ent->d_reclen = (unsigned short)(SIZEOF_WDIRENT + nambytes);

    if (0 == (dp->dd_flags & DIR_FISHPF)) {     // not HPFS, convert case.
#if defined(_WIN32) && (defined(_MSC_VER) || defined(__WATCOMC__))
        _wcslwr(ent->d_name);
#else
        wcslwr(ent->d_name);
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
dir_list_free(struct _wdirlist *dplist)
{
    struct _wdirlist *odplist;

    while (dplist) {
        dplist = (odplist = dplist)->dl_next;
        free(odplist);
    }
}


/*
//  NAME
//      _wclosedir - close a directory stream
//
//  SYNOPSIS
//      #include <dirent.h>
//
//      int _wclosedir(_WDIR *dirp);
//
*/
LIBW32_API int
_wclosedir(_WDIR *dp)
{
    if (NULL == dp) {
        errno = EBADF;
        return -1;
    }
    dir_free(dp);
    return 0;
}


/*
//  NAME
//      _wreaddir, readdir_r - read a directory
//
//  SYNOPSIS
//      #include <dirent.h>
//
//      struct dirent *_wreaddir(_WDIR *dirp);
//      int _wreaddir_r(_WDIR *restrict dirp, struct _wdirent *restrict entry, struct _wdirent **restrict result);
//
*/
LIBW32_API struct _wdirent *
_wreaddir(_WDIR *dp)
{
    struct _wdirent *entry = (struct _wdirent *)dp->dd_buf; // working buffer.

    if (NULL == dp) {
        errno = EBADF;
        return (struct _wdirent *)NULL;
    }
    assert(DIR_WMAGIC == dp->dd_magic);
    if (DIR_WMAGIC != dp->dd_magic) {
        errno = EBADF;
        return (struct _wdirent *)NULL;
    }

    if ((struct _wdirlist *)NULL == dp->dd_current) {
        errno = ENOENT;
        return (struct _wdirent *)NULL;
    }

    dir_read(dp, entry);                        // retrieve next entry.

    return entry;
}


LIBW32_API int
_wreaddir_r(_WDIR *dp, struct _wdirent *entry, struct _wdirent **result)
{
    if (NULL == dp) {
        return EBADF;
    } else if (NULL == entry || NULL == result) {
        return EINVAL;
    }
    assert(DIR_WMAGIC == dp->dd_magic);
    if (DIR_WMAGIC != dp->dd_magic) {
        *result = NULL;
        return EBADF;
    }

    if ((struct _wdirlist *)NULL == dp->dd_current) {
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
//      _wseekdir - set the position of a directory stream
//
//  SYNOPSIS
//      #include <dirent.h>
//
//      void _wseekdir(_WDIR *dirp, long loc); [Option End]
//
*/
LIBW32_API void
_wseekdir(_WDIR *dp, long off)
{
    struct _wdirlist *dplist;
    long i = off;

    if (NULL == dp) {
        errno = EBADF;
        return;
    }
    assert(DIR_WMAGIC == dp->dd_magic);

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
//      _wrewinddir - reset position of directory stream to the beginning of a directory
//
//  SYNOPSIS
//      #include <sys/types.h>
//      #include <dirent.h>
//
//      void _wrewinddir(_WDIR *dirp);
//
*/
LIBW32_API void
_wrewinddir(_WDIR *dp)
{
    if (NULL == dp) {
        errno = EBADF;
        return;
    }
    assert(DIR_WMAGIC == dp->dd_magic);
    _wseekdir(dp, 0);
}


/*
//  NAME
//      _wtelldir - current location of a named directory stream
//
//  SYNOPSIS
//      #include <dirent.h>
//
//      long _wtelldir(_WDIR *dirp);
//
*/
LIBW32_API long
_wtelldir(_WDIR *dp)
{
    if (dp == (_WDIR *)NULL) {
        errno = EBADF;
        return -1;
    }
    assert(DIR_WMAGIC == dp->dd_magic);
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
            if (hinst) FreeLibrary(hinst);
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
dir_ishpf(const wchar_t *directory)
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
