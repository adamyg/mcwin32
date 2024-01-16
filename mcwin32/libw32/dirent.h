#ifndef LIBW32_DIRENT_H_INCLUDED
#define LIBW32_DIRENT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_dirent_h,"$Id: dirent.h,v 1.16 2023/09/17 13:04:55 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <dirent.h> implementation
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
 */

#include <limits.h>                             /* PATH_MAX, NAME_MAX */
#include <time.h>

#define DIRBLKSIZ       1024
#if !defined(MAXPATHLEN)
#define MAXPATHLEN      1024                    /* PATH_MAX */
#endif

/*
 *  Notes: d_name field
 *
 *      The dirent structure definition shown above is taken from the Unix headers,
 *      and shows the d_name field with a fixed size.
 *
 *      Warning: applications should avoid any dependence on the size of the d_name field.
 *      POSIX defines it as char d_name[], a character array of unspecified size,
 *      with at most NAME_MAX characters preceding the terminating null byte ('\0').
 *
 *      POSIX.1 explicitly notes that this field should not be used as an lvalue.
 *      The standard also notes that the use of sizeof(d_name) is incorrect;
 *      use strlen(d_name) instead.
 *
 *      On some systems, this field is defined as char d_name[1]. By implication,
 *      the use sizeof(struct dirent) to capture the size of the record including
 *      the size of d_name is also incorrect.
 */

struct dirent {
#define d_ino           d_fileno                /* Backward compatibility */
    unsigned long       d_fileno;               /* File number directory */
    unsigned short      d_reclen;               /* Length of this record, in bytes */
    unsigned short      d_namlen;               /* Length of string in d_name, excluding terminating null; in characters. */

#ifndef _DIRENT_HAVE_D_RECLEN
#define _DIRENT_HAVE_D_RECLEN                   /* BSD extension */
#endif
#ifndef _DIRENT_HAVE_D_NAMLEN
#define _DIRENT_HAVE_D_NAMLEN                   /* BSD extension */
#define _D_EXACT_NAMLEN(d) ((d)->d_namlen)
#define _D_ALLOC_NAMLEN(d) (_D_EXACT_NAMLEN (d) + 1)
#endif
#ifndef _DIRENT_HAVE_D_TYPE
#define _DIRENT_HAVE_D_TYPE                     /* BSD extension */
#endif

#if defined(_POSIX_SOURCE) && !defined(_DIRENT_SOURCE)
    time_t              d_reserved1;
    time_t              d_reserved2;
    unsigned long       d_reserved3;
    unsigned long       d_reserved4;
#else
    time_t              d_ctime;                /* Creation time */
    time_t              d_mtime;                /* Modification time */
    unsigned long       d_size;                 /* File size */
    unsigned long       d_attr;                 /* File attributes */
#endif /*_POSIX_SOURCE*/

    unsigned char       d_type;                 /* Type of the file, possibly unknown */

#define DT_UNKNOWN      0   // The type is unknown. Only some filesystems have full support to return the type of the file, others might always return this value.
#define DT_FIFO         1   // A named pipe, or FIFO. See FIFO Special Files.
#define DT_CHR          2   // A character device.
#define DT_DIR          4   // A directory.
#define DT_REG          8   // A regular file.
#define DT_LNK          10  // A symbolic link.
#define DT_SOCK         12  // A local-domain socket.
#define DT_BLK          14  // A block device.

#define	MAXNAMLEN       255
    char                d_name[MAXNAMLEN + 1];  /* File name, NUL terminated */
};


struct _wdirent {
    unsigned long       d_fileno;               /* File number directory */
    unsigned short      d_reclen;               /* Length of this record, in bytes */
    unsigned short      d_namlen;               /* Length of string in d_name, excluding terminating null; in characters. */
#if defined(_POSIX_SOURCE) && !defined(_DIRENT_SOURCE)
    time_t              d_reserved1;
    time_t              d_reserved2;
    unsigned long       d_reserved3;
    unsigned long       d_reserved4;
#else
    time_t              d_ctime;                /* Creation time */
    time_t              d_mtime;                /* Modification time */
    unsigned long       d_size;                 /* File size */
    unsigned long       d_attr;                 /* File attributes */
#endif /*_POSIX_SOURCE*/
    unsigned char       d_type;
    wchar_t             d_name[MAXNAMLEN + 1];  /* File name, NUL terminated */
};


#if defined(_POSIX_SOURCE) && !defined(_DIRENT_SOURCE)
typedef void *DIR;
typedef void *_WDIR;

#else
#if defined(_DIRENT_SOURCE) || defined(DEBUG)
    /*
     *  _dirlist -- linked list of directory entries only required within 'dirent.c'.
     */
    struct _dirlist {
        time_t          dl_ctime;
        time_t          dl_mtime;
        struct _dirlist *dl_next;
        unsigned long   dl_size;
        unsigned long   dl_size2;
        unsigned long   dl_attr;
        unsigned short  dl_namlen;              /* length of buffer d_name, excluding nul; in characters */
        unsigned char   dl_type;
        char            dl_name[1];
            // plus trailing  name storage.
    };

    /*
     *  _wdirlist -- linked list of directory entries only required within 'wdirent.c'.
     */
    struct _wdirlist {
        time_t          dl_ctime;
        time_t          dl_mtime;
        struct _wdirlist *dl_next;
        unsigned long   dl_size;
        unsigned long   dl_size2;
        unsigned long   dl_attr;
        unsigned short  dl_namlen;              /* length of buffer d_name, excluding nul; in characters */
        unsigned char   dl_type;
        wchar_t         dl_name[1];
            // plus trailing  name storage.
    };
#endif /*_DIRENT_SOURCE*/

typedef struct _dirdesc {
    int                 dd_fd;                  /* File descriptor associated with directory */
    int                 dd_id;                  /* Uniquely identify open dir */
    long                dd_loc;                 /* Offset in current buffer */
    long                dd_size;                /* Amount of data returned by getdirentries */
    void *              dd_buf;                 /* Data buffer */
    int                 dd_len;                 /* Size of data buffer */
    long                dd_seek;                /* Magic cookie returned by getdirentries */
    void  *             dd_ddloc;               /* Linked list of ddloc structs for telldir/seekdir */

    /*extensions/internal*/
    unsigned long       dd_magic;               /* Structure magic */
    unsigned long       dd_flags;
    struct _dirlist *   dd_contents;
    struct _dirlist *   dd_current;
    void *              dd_handle;
/* End of extensions */
} DIR;

typedef struct _wdirdesc {
    int                 dd_fd;                  /* File descriptor associated with directory */
    int                 dd_id;                  /* Uniquely identify open dir */
    long                dd_loc;                 /* Offset in current buffer */
    long                dd_size;                /* Amount of data returned by getdirentries */
    void *              dd_buf;                 /* Data buffer */
    int                 dd_len;                 /* Size of data buffer */
    long                dd_seek;                /* Magic cookie returned by getdirentries */
    void  *             dd_ddloc;               /* Linked list of ddloc structs for telldir/seekdir */

    /*extensions/internal*/
    unsigned long       dd_magic;               /* Structure magic */
    unsigned long       dd_flags;
    struct _wdirlist *  dd_contents;
    struct _wdirlist *  dd_current;
    void *              dd_handle;
/* End of extensions */
} _WDIR;

#endif  /*_POSIX_SOURCE*/

#include <sys/cdefs.h>

__BEGIN_DECLS

LIBW32_API DIR *        opendir __P((const char *));
LIBW32_API DIR *        opendirA __P((const char *));
LIBW32_API DIR *        opendirW __P((const wchar_t *));
LIBW32_API int          closedir __P((DIR *));
LIBW32_API struct dirent * readdir __P((DIR *));
LIBW32_API void         rewinddir __P((DIR *));

LIBW32_API _WDIR *      _wopendir __P((const wchar_t *));
LIBW32_API int          _wclosedir __P((_WDIR *));
LIBW32_API struct _wdirent * _wreaddir __P((_WDIR *));
LIBW32_API void         _wrewinddir __P((_WDIR *));

#ifndef _POSIX_SOURCE
LIBW32_API void         seekdir __P((DIR *, long));
LIBW32_API long         telldir __P((DIR *));
LIBW32_API int          readdir_r __P((DIR *, struct dirent *, struct dirent **)); /*deprecated*/

LIBW32_API void         _wseekdir __P((_WDIR *, long));
LIBW32_API long         _wtelldir __P((_WDIR *));
LIBW32_API int          _wreaddir_r __P((_WDIR *, struct _wdirent *, struct _wdirent **)); /*deprecated*/

LIBW32_API int          alphasort __P((const void *, const void *));
LIBW32_API int          scandir __P((void));
LIBW32_API int          getdirentries __P((int, char *, int, long *));
#endif  /*_POSIX_SOURCE*/

__END_DECLS

#endif /*LIBW32_DIRENT_H_INCLUDED*/

