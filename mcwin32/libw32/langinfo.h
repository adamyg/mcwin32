#ifndef LIBW32_LANGINFO_H_INCLUDED
#define LIBW32_LANGINFO_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <langinfo.h> implementation
 *
 * Copyright (c) 2007, 2012 - 2015 Adam Young.
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

typedef enum {
    CODESET         = 0

//  To be defined/implemented as required
//
//  D_T_FMT         = 1,        /* string for formatting date and time */
//  D_FMT           = 2,        /* date format string */
//  T_FMT           = 3,        /* time format string */
//  T_FMT_AMPM      = 4,        /* a.m. or p.m. time formatting string */
//  AM_STR          = 5,        /* Ante Meridian affix */
//  PM_STR          = 6,        /* Post Meridian affix */
//
//  DAY_1           = 7,        /* week day names */
//  DAY_2           = 8,
//  DAY_3           = 9,
//  DAY_4           = 10,
//  DAY_5           = 11,
//  DAY_6           = 12,
//  DAY_7           = 13,
//
//  ABDAY_1         = 14,       /* abbreviated week day names */
//  ABDAY_2         = 15,
//  ABDAY_3         = 16,
//  ABDAY_4         = 17,
//  ABDAY_5         = 18,
//  ABDAY_6         = 19,
//  ABDAY_7         = 20,
//                      
//  MON_1           = 21,       /* month names */
//  MON_2           = 22,
//  MON_3           = 23,
//  MON_4           = 24,
//  MON_5           = 25,
//  MON_6           = 26,
//  MON_7           = 27,
//  MON_8           = 28,
//  MON_9           = 29,
//  MON_10          = 30,
//  MON_11          = 31,
//  MON_12          = 32,
//
//  ABMON_1         = 33,       /* abbreviated month names */
//  ABMON_2         = 34,
//  ABMON_3         = 35,
//  ABMON_4         = 36,
//  ABMON_5         = 37,
//  ABMON_6         = 38,
//  ABMON_7         = 39,
//  ABMON_8         = 40,
//  ABMON_9         = 41,
//  ABMON_10        = 42,
//  ABMON_11        = 43,
//  ABMON_12        = 44,
//
//  ERA             = 45,       /* era description segments */
//  ERA_D_FMT       = 46,       /* era date format string */
//  ERA_D_T_FMT     = 47,       /* era date and time format string */
//  ERA_T_FMT       = 48,       /* era time format string */
//  ALT_DIGITS      = 49,       /* alternative symbols for digits */
//
//  RADIXCHAR       = 50,       /* radix char */
//  THOUSEP         = 51,       /* separator for thousands */
//
//  YESEXPR         = 52,       /* affirmative response expression */
//  NOEXPR          = 53,       /* negative response expression */
//
//  #if !defined(_ANSI_SOURCE)
//  YESSTR          = 54,       /* affirmative response for yes/no queries */
//  NOSTR           = 55,       /* negative response for yes/no queries */
//  #endif
//
//  CRNCYSTR        = 56,       /* currency symbol */
//
//  #if !defined(_ANSI_SOURCE)
//  D_MD_ORDER      = 57        /* month/day order (local extension) */
//  #endif
} nl_item;


const char *        nl_langinfo(nl_item);

__END_DECLS

#endif /*LIBW32_LANGINFO_H_INCLUDED*/
