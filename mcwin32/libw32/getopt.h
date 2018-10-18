#ifndef LIBW32_GETOPT_H_INCLUDED
#define LIBW32_GETOPT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_getopt_h,"$Id: getopt.h,v 1.5 2018/10/15 08:46:48 cvsuser Exp $")
__CPRAGMA_ONCE
/* -*- mode: c; indent-width: 4; -*- */
/*
 * win <getopt.h>
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

LIBW32_API extern int   opterr;                 /* if error message should be printed */
LIBW32_API extern int   optind;                 /* index into parent argv vector */
LIBW32_API extern int   optopt;                 /* character checked for validity */
LIBW32_API extern int   optreset;               /* reset getopt */
LIBW32_API extern char *optarg;

LIBW32_API int          getopt(int nargc, char * const *nargv, const char *options);

__END_DECLS

#endif /*LIBW32_GETOPT_H_INCLUDED*/

