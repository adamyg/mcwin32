#ifndef LIBW32_GETOPT_H_INCLUDED
#define LIBW32_GETOPT_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <getopt.h> implementation
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

extern  int             opterr,                 /* if error message should be printed */
                        optind,                 /* index into parent argv vector */
                        optopt,                 /* character checked for validity */
                        optreset;               /* reset getopt */
extern char             *optarg;                /* argument associated with option */

LIBW32_API int          getopt __P((int nargc, char * const *nargv, const char *ostr));

__END_DECLS

#endif /*LIBW32_GETOPT_H_INCLUDED*/

