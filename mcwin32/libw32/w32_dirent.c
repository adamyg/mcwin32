/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 directory access services ...
 *
 *      opendir, closedir, readdir, seekdir, rewindir, telldir
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
 *
 * Sourced originally from a public domain implementation and highly.
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT            0x0501
#endif

#define _DIRENT_SOURCE
#include "win32_internal.h"
#include <unistd.h>

#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Advapi32.lib")
#include <lm.h>                                 /* NetEnum... */
#include <winioctl.h>                           /* DeviceIoControls */
#if defined(HAVE_NTIFS_H)
#include <ntifs.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

#define DIR_FISHPF              0x0001
#define DIR_MAGIC               0x57333264      /* W32d */

typedef BOOL (WINAPI *Wow64DisableWow64FsRedirection_t)(PVOID *OldValue);
typedef BOOL (WINAPI *Wow64RevertWow64FsRedirection_t)(PVOID OldValue);

#define DISABLE_HARD_ERRORS     SetErrorMode (0)
#define ENABLE_HARD_ERRORS      SetErrorMode (SEM_FAILCRITICALERRORS | \
                                        SEM_NOOPENFILEERRORBOX)

static int                      ReadReparse(const char *name, char *buf, int maxlen);

static DIR *                    unc_populate(const char *path);

static int                      dir_populate(DIR *dp, const char *path);
static struct _dirlist *        dir_list_push(DIR *dp, const char *filename);
static void                     dir_list_free(struct _dirlist *);
static int                      dir_ishpf(const char *directory);

static BOOL                     d_Wow64DisableWow64FsRedirection(PVOID *OldValue);
static BOOL                     d_Wow64RevertWow64FsRedirection(PVOID OldValue);

static Wow64DisableWow64FsRedirection_t x_Wow64DisableWow64FsRedirection;
static Wow64RevertWow64FsRedirection_t x_Wow64RevertWow64FsRedirection;

static int              x_dirid = 1;            /* singleton */


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
DIR *
opendir(const char *name)
{
    char fullpath[ MAX_PATH ], reparse[ MAX_PATH ],
            *path = fullpath;
    LPVOID OldValue = NULL;
    DIR *dp;
    int i, len;
    int rc;

    /* Copy to working buffer */
    if (0 == (len = strlen(name))) {
        errno = ENOTDIR;
        return (DIR *)NULL;
    }

    /* Convert path (note, UNC safe) */
    if (NULL == _fullpath(fullpath, name, sizeof(fullpath))) {
        char *last;                             /* unknown, assume DOS */

        strncpy(fullpath, name, sizeof(fullpath));
        fullpath[sizeof(fullpath)-1] = 0;
        for (i = 0; fullpath[i]; ++i) {
            if (fullpath[i] == '/') {
                fullpath[i] = '\\';             /* convert */
            }
        }
        last = &fullpath[len - 1];

        /*
         *  DOS is very picky about its directory names.
         *  The following are valid.
         *      c:/
         *      c:.
         *      c:name/name1
         *
         *  the following are not valid
         *      c:name/
         */
        if ((*last == '\\') && (len > 1) && (!((len == 3) &&
                    (fullpath[1] == ':')))) {
            *(last--) = 0;
        }
    }

    /* Is a directory ? */
    if (0 != strcmp(path, ".")) {
        DWORD attr;

        rc = 0;
        DISABLE_HARD_ERRORS;
        if ((attr = GetFileAttributes(path)) == INVALID_FILE_ATTRIBUTES ||
                0 == (attr & FILE_ATTRIBUTE_DIRECTORY)) {
            rc = ENOTDIR;
        }
        ENABLE_HARD_ERRORS;
        if (rc) {
            if (w32_root_unc(path)) {
                return unc_populate(path);      /* //servername[/] */
            }
            errno = ENOTDIR;
            return (DIR *)NULL;
        }
        if (FILE_ATTRIBUTE_REPARSE_POINT & attr) {
            if (-1 == ReadReparse(path, reparse, sizeof(reparse))) {
                errno = EACCES;
                return (DIR *)NULL;
            }
            path = reparse;
        }
    }

    /* Strip trailing slashes, so we can append "\*.*" */
    len = strlen(path);
    while (len > 0) {
        len--;
        if (path[len] == '\\') {
            path[len] = '\0';                   /* remove slash */
        } else {
            len++;                              /* end of path */
            break;
        }
    }

    path[len++] = '\\';                         /* insert pattern */
    path[len++] = '*';
    path[len++] = '.';
    path[len++] = '*';
    path[len++] = 0;

    /* Create DIR structure */
    if (NULL == (dp = (DIR *)calloc(sizeof(DIR), 1)) ||
            NULL == (dp->dd_buf = (char *)calloc(sizeof(struct dirent), 1))) {
        free(dp);
        return (DIR *)NULL;
    }

    dp->dd_magic = DIR_MAGIC;                   /* structure magic */
    dp->dd_id = ++x_dirid;                      /* generate unique directory identifier */
    dp->dd_fd = -1;                             /* file descriptor */

    /* Open directory
     *
     *    If you are writing a 32-bit application to list all the files in a
     *    directory and the application may be run on a 64-bit computer, you should
     *    call Wow64DisableWow64FsRedirection before calling FindFirstFileEx and call
     *    Wow64RevertWow64FsRedirection after the last call to FindNextFile.
     *
     *    For more information, see File System Redirector.
     */
    if (d_Wow64DisableWow64FsRedirection(&OldValue)) {
        const int ret = dir_populate(dp, path);

        if (! d_Wow64RevertWow64FsRedirection(OldValue) || ret) {
            closedir(dp);
            errno = (ret ? ret : EIO);
            dp = NULL;
        }
    }
    return dp;
}


#if !defined(HAVE_NTIFS_H) && !defined(__MINGW32__)
typedef struct _REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG  Flags;
            WCHAR  PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR  PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR DataBuffer[1];
        } GenericReparseBuffer;
    };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#ifndef IO_REPARSE_TAG_MOUNT_POINT
#define IO_REPARSE_TAG_MOUNT_POINT  0xA0000003L
#endif
#ifndef IO_REPARSE_TAG_SYMLINK
#define IO_REPARSE_TAG_SYMLINK      0xA000000CL
#endif

#endif	//HAVE_NTIFS_H

static int
ReadReparse(const char *name, char *buf, int maxlen)
{
#define MAX_REPARSE_SIZE        (512+(16*1024)) /* Header + 16k */
    HANDLE fileHandle;
    BYTE reparseBuffer[ MAX_REPARSE_SIZE ];
    PREPARSE_GUID_DATA_BUFFER reparseInfo = (PREPARSE_GUID_DATA_BUFFER) reparseBuffer;
    PREPARSE_DATA_BUFFER rdb = (PREPARSE_DATA_BUFFER) reparseBuffer;
    DWORD returnedLength;
                                                /* open the file image */
    if ((fileHandle = CreateFile(name, 0,
                FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS, NULL)) == INVALID_HANDLE_VALUE) {
        return -1;
    }

                                                /* retrieve reparse details */
    if (DeviceIoControl(fileHandle, FSCTL_GET_REPARSE_POINT,
                NULL, 0, reparseInfo, sizeof(reparseBuffer), &returnedLength, NULL)) {
//      if (IsReparseTagMicrosoft(reparseInfo->ReparseTag)) {
            int length;

            switch (reparseInfo->ReparseTag) {
            case IO_REPARSE_TAG_SYMLINK:
                //
                //  Symbolic links
                //
                if ((length = rdb->SymbolicLinkReparseBuffer.SubstituteNameLength) >= 4) {
                    const size_t offset = rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
                    const wchar_t* symlink = rdb->SymbolicLinkReparseBuffer.PathBuffer + offset;

                    wcstombs(buf, symlink, maxlen);
                    return 0;
                }
                break;

            case IO_REPARSE_TAG_MOUNT_POINT:
                //
                //  Mount points and junctions
                //
                if ((length = rdb->MountPointReparseBuffer.SubstituteNameLength) > 0) {
                    const size_t offset = rdb->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
                    const wchar_t* mount = rdb->MountPointReparseBuffer.PathBuffer + offset;

                    wcstombs(buf, mount, maxlen);
                    return 0;
                }
                break;
            }
//      }
    }
    CloseHandle(fileHandle);
    return -1;
}


static DIR *
unc_populate(const char *path)
{
    SHARE_INFO_502 *buffer = NULL;
    NET_API_STATUS res = 0;
    DIR *dp;

    (void)path;

    if (NULL == (dp = (DIR *)calloc(sizeof(DIR), 1)) ||
            NULL == (dp->dd_buf = (char *)calloc(sizeof(struct dirent), 1))) {
        free(dp);
        return (DIR *)NULL;
    }

    do {
        DWORD entries = -1, tr = 0, resume = 0;

        res = NetShareEnum(NULL, 502, (LPBYTE *)&buffer, MAX_PREFERRED_LENGTH, &entries, &tr, &resume);

        if (ERROR_SUCCESS == res || ERROR_MORE_DATA == res) {
            const SHARE_INFO_502 *ent;
            DWORD e;

            for (e = 0, ent = buffer; e < entries; ++e, ++ent) {
                if (STYPE_DISKTREE == ent->shi502_type) {
                    //
                    //  build directory ..
                    //
                    char filename[MAX_PATH+1];

                    WideCharToMultiByte(CP_ACP, 0,
                        (void *)ent->shi502_netname, -1, filename, sizeof(filename)-1, NULL, NULL);
                    filename[sizeof(filename) - 1] = 0;

                    if (0 == strcmp(filename, "prnproc$") ||
                            0 == strcmp(filename, "print$")) {
                        continue;
                    }

                    if (NULL == dir_list_push(dp, filename)) {
                        break;
                    }
                }
            }
            NetApiBufferFree(buffer);
        }
    } while (ERROR_MORE_DATA == res);

    dp->dd_current = dp->dd_contents;           /* seed cursor */
    dp->dd_magic = DIR_MAGIC;                   /* structure magic */
    dp->dd_id = ++x_dirid;                      /* generate unique directory identifier */
    dp->dd_fd = -1;                             /* file descriptor */

    return dp;
}


static int
dir_populate(DIR *dp, const char *path)
{
    struct _dirlist *dplist;
    WIN32_FIND_DATA finddata;
    HANDLE hSearch;
    BOOL isHPFS = FALSE;
    int rc, ret = 0;

    DISABLE_HARD_ERRORS;
    hSearch = FindFirstFile(path, &finddata);
    ENABLE_HARD_ERRORS;

    if (INVALID_HANDLE_VALUE == hSearch) {
        rc = GetLastError();
#if defined(ERROR_EMPTY_DIR)
        if (rc == ERROR_EMPTY_DIR)  {
            return 0;                           /* entry list */
        }
#endif
        switch (rc) {
        case ERROR_NO_MORE_FILES:
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return ENOENT;
            break;
        case ERROR_NOT_ENOUGH_MEMORY:
            return ENOMEM;
        default:
            break;
        }
        return EINVAL;
    }

    isHPFS = dir_ishpf(path);                   /* extended file system */
    if (isHPFS) {
        dp->dd_flags = DIR_FISHPF;
    }

    do {
#if defined(FILE_ATTRIBUTE_VOLUME)              /* skip volume labels */
        if (finddata.ff_attrib & FILE_ATTRIBUTE_VOLUME) {
            continue;
        }
#endif
                                                /* skip '.' */
        if (0 == strcmp(finddata.cFileName, ".")) {
            continue;
        }

                                                /* create new entry */
        if (NULL == (dplist = dir_list_push(dp, finddata.cFileName))) {
            FindClose(hSearch);
            return ENOMEM;
        }

        if (! isHPFS) {                         /* not HPFS, convert case */
#if defined(WIN32) && defined(_MSC_VER)
            _strlwr(dplist->dl_entry);
#else
            strlwr(dplist->dl_entry);
#endif
        }
        dplist->dl_size2 = finddata.nFileSizeHigh;
        dplist->dl_size = finddata.nFileSizeLow;
        dplist->dl_attr = finddata.dwFileAttributes;

    } while (FindNextFile(hSearch, &finddata));

    if ((rc = GetLastError()) == ERROR_NO_MORE_FILES) {
        dp->dd_current = dp->dd_contents;       /* seed cursor */
    } else {
        switch (rc) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            ret = ENOENT;
            break;
        case ERROR_NOT_ENOUGH_MEMORY:
            ret = ENOMEM;
            break;
        default:
            break;
        }
        ret = EINVAL;
    }
    FindClose(hSearch);
    return ret;
}


static struct _dirlist *
dir_list_push(DIR *dp, const char *filename)
{
    const size_t namelen = strlen(filename) + 1;
    struct _dirlist *dplist;

    if (NULL == (dplist = (struct _dirlist *)calloc(1, sizeof(struct _dirlist))) ||
            NULL == (dplist->dl_entry = (char *)malloc(namelen))) {
        free(dplist);
        return NULL;
    }
    if (dp->dd_contents) {
        dp->dd_current  =
            dp->dd_current->dl_next = dplist;
    } else {
        dp->dd_contents = dp->dd_current = dplist;
    }
    memcpy(dplist->dl_entry, filename, namelen);
    dplist->dl_next = NULL;
    return dplist;
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
int
closedir(DIR *dp)
{
    if (NULL == dp) {
        errno = EBADF;
        return -1;
    }
    assert(DIR_MAGIC == dp->dd_magic);
    dir_list_free(dp->dd_contents);
    free((char *)dp->dd_buf);
    free((char *)dp);
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
//              struct dirent **restrict result); [Option End]
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
//          One of the values in the structure to be returned cannot be represented
//          correctly.
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
*/
struct dirent *
readdir(DIR *dp)
{
    struct _dirlist *pEntry;
    struct dirent *dpent;

    if (NULL == dp) {
        errno = EBADF;
        return NULL;
    }
    assert(DIR_MAGIC == dp->dd_magic);

    /* Retrieve associated fields */
    if (dp->dd_current == (struct _dirlist *)NULL) {
        return (struct dirent *)NULL;
    }
    pEntry = dp->dd_current;
    dpent = (struct dirent *)dp->dd_buf;

    /* Standard fields */
    strcpy(dpent->d_name, pEntry->dl_entry);
    dpent->d_namlen = (u_short) strlen(dpent->d_name);
    dpent->d_reclen = sizeof(struct dirent);
    if (0 == (dp->dd_flags & DIR_FISHPF)) {     /* not HPFS, convert case */
#if defined(WIN32) && defined(_MSC_VER)
        _strlwr(dpent->d_name);
#else
        strlwr(dpent->d_name);
#endif
    }
    dpent->d_fileno = 0;

    /* The following field are extensions */
    dpent->d_ctime = pEntry->dl_ctime;
    dpent->d_mtime = pEntry->dl_mtime;
    dpent->d_size  = pEntry->dl_size;
    dpent->d_attr  = pEntry->dl_attr;

    /* Update current location */
    dp->dd_current = pEntry->dl_next;
    dp->dd_loc++;

    return dpent;
}


/*
//  NAME
//      seekdir - set the position of a directory stream
//
//  SYNOPSIS
//      #include <dirent.h>
//
//      void seekdir(DIR *dirp, long loc); [Option End]
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
void
seekdir(DIR *dp, long off)
{
    struct _dirlist *dplist;
    long i = off;

    if (NULL == dp) {
        errno = EBADF;
        return;
    }
    assert(DIR_MAGIC == dp->dd_magic);

    if (off > 0) {
        for (dplist = dp->dd_contents; --i >= 0 && dplist; dplist = dplist->dl_next) {
            /*cont*/;
        }
        dp->dd_loc = off - (i+1);
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
void
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
long
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
 *  dir_list_free ---
 *      Dispose of the directory list.
 */
static void
dir_list_free(struct _dirlist *dplist)
{
    struct _dirlist *odplist;

    while (dplist) {
        free(dplist->dl_entry);
        dplist = (odplist = dplist)->dl_next;
        free(odplist);
    }
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
            FreeLibrary(hinst);
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
dir_ishpf(const char *directory)
{
    unsigned nDrive;
    char szCurDir[MAX_PATH + 1];
    char bName[4] = "x:\\";
    DWORD flags, maxname;
    BOOL rc;

    if (directory &&
            isalpha((unsigned char)directory[0]) && directory[1] == ':') {
        nDrive = toupper(directory[0]) - 'A';
    } else {
        GetCurrentDirectory (MAX_PATH, szCurDir);
        nDrive = toupper(szCurDir[0]) - 'A';
    }
    bName[0] = (char)(nDrive + 'A');

    DISABLE_HARD_ERRORS;
    rc = GetVolumeInformation(bName, (LPTSTR)NULL, 0,
                (LPDWORD)NULL, &maxname, &flags, (LPTSTR)NULL, 0);
    ENABLE_HARD_ERRORS;

    return ((rc) &&
        (flags & (FS_CASE_SENSITIVE | FS_CASE_IS_PRESERVED))) ? TRUE : FALSE;
}
/*end*/
