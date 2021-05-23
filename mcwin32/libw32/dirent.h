#ifndef LIBW32_DIRENT_H_INCLUDED
#define LIBW32_DIRENT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_dirent_h,"$Id: dirent.h,v 1.9 2021/05/23 10:19:20 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <dirent.h> implementation
 *
 * Copyright (c) 2007, 2012 - 2021 Adam Young.
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
 */

#include <limits.h>                             /* PATH_MAX, NAME_MAX */
#include <time.h>

#define DIRBLKSIZ       1024
#if !defined(MAXPATHLEN)
#define MAXPATHLEN      1024                    /* PATH_MAX */
#endif

struct dirent {
#define d_ino           d_fileno                /* Backward compatibility */
    unsigned long       d_fileno;               /* File number directory */
    unsigned short      d_reclen;               /* Length of this record */
    unsigned short      d_namlen;               /* Length of string in d_name, not including terminating null. */
#ifndef _DIRENT_HAVE_D_NAMLEN
#define _DIRENT_HAVE_D_NAMLEN                   /* BSD extension */
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
#endif  /*_POSIX_SOURCE*/
#if (0)
    unsigned char       d_type;                 /* Type of the file, possibly unknown */
#ifndef _DIRENT_HAVE_D_TYPE
#define _DIRENT_HAVE_D_TYPE                     /* BSD extension */
#endif
#define DT_UNKNOWN      0   // The type is unknown. Only some filesystems have full support to return the type of the file, others might always return this value.
#define DT_FIFO         1   // A named pipe, or FIFO. See FIFO Special Files.
#define DT_CHR          2   // A character device.
#define DT_DIR          4   // A directory.
#define DT_REG          8   // A regular file.
#define DT_LNK          10  // A symbolic link.
#define DT_SOCK         12  // A local-domain socket.
#define DT_BLK          14  // A block device.

#define	IFTODT(mode)	(((mode) & 0170000) >> 12)
#define	DTTOIF(dirtype)	((dirtype) << 12)
#endif
#define	MAXNAMLEN       255
     char               d_name[MAXNAMLEN + 1];  /* File name */
};


#if defined(_POSIX_SOURCE) && !defined(_DIRENT_SOURCE)
typedef void *DIR;
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
        unsigned short  dl_namlen;              /* length of buffer d_name */
        char            dl_name[1];
            // trailing  name storage
    };
#endif /*_DIRENT_SOURCE*/

typedef struct _dirdesc {
    int                 dd_fd;                  /* File descriptor associated with directory */
    int                 dd_id;                  /* Uniquely identify open dir */
    long                dd_loc;                 /* Offset in current buffer */
    long                dd_size;                /* Amount of data returned by getdirentries */
    char *              dd_buf;                 /* Data buffer */
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
#endif  /*_POSIX_SOURCE*/

#include <sys/cdefs.h>

__BEGIN_DECLS

LIBW32_API DIR *        opendir __P((const char *));
LIBW32_API DIR *        opendirA __P((const char *));
LIBW32_API DIR *        opendirW __P((const wchar_t *));
LIBW32_API int          closedir __P((DIR *));
LIBW32_API struct dirent * readdir __P((DIR *));
LIBW32_API void         rewinddir __P((DIR *));
#ifndef _POSIX_SOURCE
LIBW32_API void         seekdir __P((DIR *, long));
LIBW32_API long         telldir __P((DIR *));
LIBW32_API int          alphasort __P((const void *, const void *));
LIBW32_API int          scandir __P((void));
LIBW32_API int          getdirentries __P((int, char *, int, long *));
#endif  /*_POSIX_SOURCE*/

__END_DECLS

#endif /*LIBW32_DIRENT_H_INCLUDED*/
