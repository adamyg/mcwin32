#ifndef LIBW32_WIN32_INCLUDE_H_INCLUDED
#define LIBW32_WIN32_INCLUDE_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/*
 * winsock2.h and windows.h include guard
 *
 * Copyright (c) 2007, 2012 - 2017 Adam Young.
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

#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE                /* disable deprecate warnings */
#endif

#if !defined(_CRT_NO_POSIX_ERROR_CODES)
#define _CRT_NO_POSIX_ERROR_CODES               /* disable POSIX error number, see <errno.h> */
#endif

#if !defined(HAVE_WINSOCK2_H_INCLUDED)
#define HAVE_WINSOCK2_H_INCLUDED
#undef gethostname                              /* unistd.h name mangling */
#if defined(u_char)
#undef u_char                                   /* namespace issues (_BSD_SOURCE) */
#endif
#include <winsock2.h>
#include <ws2tcpip.h>                           /* getaddrinfo() */
#endif /*HAVE_WINSOCK2_H_INCLUDED*/

#if !defined(HAVE_WINDOWS_H_INCLUDED)
#define HAVE_WINDOWS_H_INCLUDED
#ifndef WINDOWS_NOT_MEAN_AND_LEAN
#define WINDOWS_MEAN_AND_LEAN
#endif
#include <windows.h>
#endif

#endif /*LIBW32_WIN32_INCLUDE_H_INCLUDED*/
