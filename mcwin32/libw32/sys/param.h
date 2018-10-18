#ifndef LIBW32_SYS_PARAM_H_INCLUDED
#define LIBW32_SYS_PARAM_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_param_h,"$Id: param.h,v 1.4 2018/09/29 02:22:55 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <sys/param.h>
 *
 * Copyright (c) 2012 - 2018, Adam Young.
 * All rights reserved.
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
