#ifndef LIBW32_WIN32IO_H_INCLUDED
#define LIBW32_WIN32IO_H_INCLUDED

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 io functionality.
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
 */

#undef DATADIR                                  /* namespace issue */

#if defined(_MSC_VER)
#if (_MSC_VER != 1200)                          /* MSVC 6 */
#if (_MSC_VER != 1400)                          /* MSVC 8/2005 */
#if (_MSC_VER != 1500)                          /* MSVC 9/2008 */
#if (_MSC_VER != 1600)                          /* MSVC 10/2010 */
#if (_MSC_VER != 1900)                          /* MSVC 19/2015 */
#if (_MSC_VER <  1910 || _MSC_VER > 1914)       /* MSVC 19.10 .. 14/2017 */
#error win32_io.h: untested MSVC Version (2005 -- 2017) only ...
	 //see: https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B
#endif //2017
#endif //2015
#endif //2010
#endif //2008
#endif //2005
#endif //_MSC_VER

#pragma  warning(disable:4115)

#elif defined(__WATCOMC__)
#if (__WATCOMC__ < 1200)
#error old WATCOM Version, upgrade to OpenWatcom ...
#endif

#elif defined(__MINGW32__)
#else
#error win32_io.h: unknown/unsupported compiler
#endif

#if !defined(_WIN32_WINCE)                      /* require winsock2.h */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x601               /* latest features */
#elif (_WIN32_WINNT) < 0x400
#pragma message("unistd: _WIN32_WINNT < 0400")
#endif
#endif  /*_WIN32_WINCE*/

#include <win32_include.h>

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */

__BEGIN_DECLS

/*fcntl.h*/
#if !defined(F_GETTL)
#define F_GETFL         1
#define F_SETFL         2
#endif

LIBW32_API int          fcntl (int fildes, int ctrl, int);
LIBW32_API int          w32_fsync (int fildes);

/*io.h*/
struct stat;

LIBW32_API int          w32_open (const char *name, int, ...);
LIBW32_API int          w32_stat (const char *name, struct stat *sb);
LIBW32_API int          w32_read (int fildes, void *buf, unsigned int nbyte);
LIBW32_API int          w32_write (int fildes, const void *buf, unsigned int nbyte);
LIBW32_API int          w32_close (int fildes);
LIBW32_API const char * w32_strerror (int errnum);
LIBW32_API int          w32_unlink (const char *fname);

LIBW32_API int          w32_mkstemp (char *path);
LIBW32_API int          w32_mkstempx (char *path);

LIBW32_API int          w32_link (const char *, const char *);
LIBW32_API int          w32_lstat (const char *, struct stat *);

LIBW32_API char *       w32_getcwd (char *buffer, int size);
LIBW32_API char *       w32_getcwdd (char drive, char *buffer, int size);

LIBW32_API int          w32_mkdir (const char *fname, int mode);
LIBW32_API int          w32_chdir (const char *fname);
LIBW32_API int          w32_rmdir (const char *fname);

/*support functions*/
LIBW32_API int          w32_root_unc (const char *path);

LIBW32_API const char * w32_strslash (const char *path);

LIBW32_API int          w32_errno_set (void);
LIBW32_API int          w32_errno_setas (unsigned nerrno);
LIBW32_API int          w32_errno_cnv (unsigned rc);
LIBW32_API int          w32_neterrno_set (void);

__END_DECLS

#endif /*LIBW32_WIN32IO_H_INCLUDED*/
