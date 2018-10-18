#ifndef LIBW32_WIN32_MISC_H_INCLUDED
#define LIBW32_WIN32_MISC_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_misc_h,"$Id: win32_misc.h,v 1.5 2018/10/12 00:52:05 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 public interface
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
 * ==end==
 */

#include <sys/cdefs.h>

__BEGIN_DECLS

enum w32ostype {            /* generalised machine types, ignoring server */
    OSTYPE_WIN_10,
    OSTYPE_WIN_8,
    OSTYPE_WIN_7,
    OSTYPE_WIN_VISTA,
    OSTYPE_WIN_NT,
    OSTYPE_WIN_CE,
    OSTYPE_WIN_95
};

#define SYSDIR_TEMP         0x000001

#define WIN32_PATH_MAX      1024                /* 255, unless UNC names are used */
#define WIN32_LINK_DEPTH    8

LIBW32_API enum w32ostype   w32_ostype (void);
LIBW32_API int              w32_getexedir (char *buf, int maxlen);
LIBW32_API int              w32_getsysdir (int id, char *buf, int maxlen);

LIBW32_API int              w32_regstrget(const char *subkey, const char *valuename, char *buf, int len);
LIBW32_API int              w32_regstrgetx(HKEY hkey, const char *subkey, const char *valuename, char *buf, int len);
LIBW32_API const char *     w32_getlanguage(char *buffer, int len);

LIBW32_API const char *     w32_selectfolder(const char *strMessage, char *szBuffer);

LIBW32_API int              w32_IsElevated(void);
LIBW32_API int              w32_IsAdministrator(void);

__END_DECLS

#endif /*LIBW32_WIN32_MISC_H_INCLUDED*/
