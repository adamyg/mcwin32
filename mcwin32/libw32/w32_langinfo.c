#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_langinfo_c,"$Id: w32_langinfo.c,v 1.5 2018/10/12 00:52:04 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 langinfo() implementation
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

#include "win32_internal.h"
#include <langinfo.h>

/*
//  NAME
//      nl_langinfo - language information
//
//  SYNOPSIS
//      #include <langinfo.h>
//
//      char *nl_langinfo(nl_item item);
//
//  DESCRIPTION
//      The nl_langinfo() function shall return a pointer to a string containing
//      information relevant to the particular language or cultural area defined in the
//      program's locale (see <langinfo.h>). The manifest constant names and values of item
//      are defined in <langinfo.h>. For example:
//
//      nl_langinfo(ABDAY_1)
//
//      would return a pointer to the string "Dom" if the identified language was
//      Portuguese, and "Sun" if the identified language was English.
//
//      Calls to setlocale() with a category corresponding to the category of item (see
//      <langinfo.h>), or to the category LC_ALL , may overwrite the array pointed to by
//      the return value.
//
//      The nl_langinfo() function need not be reentrant. A function that is not required
//      to be reentrant is not required to be thread-safe.
//
//  RETURN VALUE
//      In a locale where langinfo data is not defined, nl_langinfo() shall return a
//      pointer to the corresponding string in the POSIX locale. In all locales,
//      nl_langinfo() shall return a pointer to an empty string if item contains an invalid
//      setting.
//
//      This pointer may point to static data that may be overwritten on the next call.
//
//  ERRORS
//      No errors are defined.
*/
LIBW32_API const char *
nl_langinfo(nl_item item)
{
    switch (item) {
    case CODESET:               /* assume terminal is UTF-8 */
        return "UTF-8";
    }
    return "n/a";
}

/*end*/

