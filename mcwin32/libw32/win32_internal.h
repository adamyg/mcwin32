#ifndef LIBW32_WIN32_INTERNAL_H_INCLUDED
#define LIBW32_WIN32_INTERNAL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_internal_h,"$Id: win32_internal.h,v 1.15 2021/05/09 11:33:43 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * internal definitions
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

#include "w32config.h"

#if defined(__WATCOMC__) && !defined(__STDC_WANT_LIB_EXT1__)
#define __STDC_WANT_LIB_EXT1__ 1
#endif

#include <win32_include.h>
#include <unistd.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1400) || \
    defined(__WATCOMC__)
#define WIN32_OPEN      _open
#define WIN32_WOPEN     _wopen
#define WIN32_CLOSE     _close
#define WIN32_READ      _read
#define WIN32_WRITE     _write
#define WIN32_CHMOD     _chmod
#define WIN32_WCHMOD    _wchmod
#define WIN32_LSEEK     _lseek
#define WIN32_STRICMP   _stricmp
#define WIN32_STRNICMP  _strnicmp
#define WIN32_STRDUP    _strdup
#define WIN32_TZSET     _tzset
#else
#define WIN32_OPEN      open
#define WIN32_WOPEN     wopen
#define WIN32_CLOSE     close
#define WIN32_READ      read
#define WIN32_WRITE     write
#define WIN32_CHMOD     chmod
#define WIN32_WCHMOD    wchmod
#define WIN32_LSEEK     lseek
#define WIN32_STRICMP   stricmp
#define WIN32_STRNICMP  strnicmp
#define WIN32_STRDUP    strdup
#define WIN32_TZSET     tzset
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define WIN32_GETPID    _getpid
#else
#define WIN32_GETPID    getpid
#endif

#define SLASHCHAR       '\\'
#define XSLASHCHAR      '/'
#define SLASH           "\\"
#define DELIMITER       ";"
#define ISSLASH(c)      (((c) == SLASHCHAR) || ((c) == XSLASHCHAR))

#include <sys/cdefs.h>

__BEGIN_DECLS

#define WIN32_FILDES_DEF    (512)
#define WIN32_FILDES_MAX    (8*1024)            /* was 2048, now 8192/2019 */

extern int              x_w32_cwdn;             /* current/last working drive number, A=1 etc */
extern const char *     x_w32_cwdd[26];         /* last directory, prr drive */
extern const char *     x_w32_vfscwd;           /* current UNC path, if any */

int                     IO_STRICMP (const char *s1, const char *s2);
int                     IO_STRNICMP (const char *s1, const char *s2, int slen);
int                     IO_WSTRICMP (const wchar_t *s1, const char *s2);
int                     IO_WSTRNICMP (const wchar_t *s1, const char *s2, int slen);

LIBW32_API ino_t        w32_ino_hash (const char *name);
LIBW32_API ino_t        w32_ino_whash (const wchar_t *name);
LIBW32_API ino_t        w32_ino_gen (const DWORD fileIndexLow, const DWORD fileIndexHigh);
LIBW32_API ino_t        w32_ino_handle (HANDLE handle);
LIBW32_API ino_t        w32_ino_fildes (int fildes);
LIBW32_API ino_t        w32_ino_file (const char *name);
LIBW32_API ino_t        w32_ino_wfile(const wchar_t *name);

LIBW32_API int          w32_utf2wc(const char *src, wchar_t *dest, size_t max);
LIBW32_API int          w32_wc2utf(const wchar_t *src, char *dest, size_t max);

LIBW32_API char *       w32_dos2unix (char *path);
LIBW32_API wchar_t *    w32_wdos2unix (wchar_t *path);
LIBW32_API char *       w32_unix2dos (char *path);
LIBW32_API wchar_t *    w32_wunix2dos (wchar_t *path);

LIBW32_API const char * w32_strslash (const char *path);
LIBW32_API const wchar_t * w32_wcsslash (const wchar_t *path);

LIBW32_API int          w32_neterrno_map (int nerrno);
LIBW32_API int          w32_neterrno_set (void);
LIBW32_API int          w32_errno_set (void);
LIBW32_API int          w32_errno_setas (unsigned nerrno);
LIBW32_API int          w32_errno_cnv (unsigned rc);

LIBW32_API SOCKET       w32_sockhandle (int fd);

LIBW32_API void         w32_sockfd_init (void);
LIBW32_API int          w32_sockfd_limit (int limit);
LIBW32_API void         w32_sockfd_open (int fd, SOCKET s);
LIBW32_API SOCKET       w32_sockfd_get (int fd);
LIBW32_API void         w32_sockfd_close (int fd, SOCKET s);
LIBW32_API int          w32_issockfd (int fd, SOCKET *s);

LIBW32_API int          w32_reparse_readA (const char *name, char *buf, int maxlen);
LIBW32_API int          w32_reparse_readW (const wchar_t *name, wchar_t *buf, int maxlen);

__END_DECLS

#endif /*LIBW32_WIN32_INTERNAL_H_INCLUDED*/
