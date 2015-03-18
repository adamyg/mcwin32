#ifndef LIBW32_W32CONFIG_H_INCLUDED
#define LIBW32_W32CONFIG_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <config.h> implementation
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained 
 * online at http://www.opengroup.org/unix/online.html.
 * ==end==
 */

#define __WIN32__
#ifndef WIN32
#define WIN32 0x400
#endif

#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE                /* disable deprecate warnings */
#endif

#if defined(_MSC_VER)
#pragma warning (disable : 4127)                // conditional expression is constant
#pragma warning (disable : 4201)                // nonstandard extension used : nameless struct/union
#pragma warning (disable : 4204)                // nonstandard extension used : non-constant aggregate initializer
#pragma warning (disable : 4702)                // unreachable code
#pragma warning (disable : 4706)                // assignment within conditional expression
#pragma warning (disable : 4996)                // 'xxx' was declared deprecated

#elif defined(__WATCOMC__)
#pragma disable_message(136)                    // Comparison equivalent to 'unsigned == 0'
#pragma disable_message(201)                    // Unreachable code
#pragma disable_message(202)                    // Unreferenced
#endif

#define HAVE_TRACE                              /* local configuration */

#define STDC_HEADERS 1
#define HAVE_FCNTL_H 1
#define HAVE_UNISTD_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_MALLOC_H 1
#define HAVE_DIRENT_H 1
#define HAVE_ALLOCA_H 1
#define HAVE_PWD_H 1
#define HAVE_GRP_H 1

#define HAVE_GETOPT_H 1

#define HAVE_TIME_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_MMAN_H 1
#define HAVE_SYS_MOUNT_H 1
#define HAVE_SYS_STATFS_H 1
#define HAVE_SYS_STATVFS_H 1    
#define HAVE_SYS_UTSNAME_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_INFOMOUNT_LIST 1
#define HAVE_F_FSTYPENAME 1                     /* statfs has f_fstypename */
#define MOUNTED_GETMNTINFO 1                    /* bsd mount list */
#define STAT_STATVFS 1                          /* SVR4 - style */

#define HAVE_GETUID 1
#define HAVE_GETGID 1
#define HAVE_GETEUID 1
#define HAVE_GETEGID 1

#define HAVE_ALLOCA 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMSET 1
#define HAVE_MEMCHR 1 
#define HAVE_MEMCPY 1
#define HAVE_MEMCMP 1
#define HAVE_MEMMOVE 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_TRUNCATE 1
#define HAVE_STRTOUL 1
#define HAVE_MKSTEMP 1

#define HAVE_SLANG 1
#define HAVE_SYSTEM_SLANG 1
#define IBMPC_SYSTEM 1

#endif /*LIBW32_W32CONFIG_H_INCLUDED*/
