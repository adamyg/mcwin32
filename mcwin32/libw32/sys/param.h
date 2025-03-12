#ifndef LIBW32_SYS_PARAM_H_INCLUDED
#define LIBW32_SYS_PARAM_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_param_h,"$Id: param.h,v 1.9 2025/03/08 16:40:00 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <sys/param.h>
 *
 * Copyright (c) 2012 - 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ==end==
 */

#include <limits.h>				/*PATH_MAX*/
#include <stdio.h>				/*PATH_MAX*/
#include <stdlib.h>				/*_MAX_PATH*/

#define ALIGNBYTES		3		/*FIXME , 32/64 bit machines*/
#define ALIGN(p)		(((unsigned int)(p) + ALIGNBYTES) &~ ALIGNBYTES)

#if !defined(PATH_MAX)
#define PATH_MAX		_MAX_PATH
#define NAME_MAX		_MAX_PATH
#endif

/*limits.h*/
#define _POSIX2_LINE_MAX	2048
#define LINE_MAX		2048

#endif /*LIBW32_SYS_PARAM_H_INCLUDED*/
