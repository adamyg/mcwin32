#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_signal_c,"$Id: w32_signal.c,v 1.6 2021/11/30 13:06:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 signal support
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
#include <unistd.h>
#include <signal.h>

#if !defined(__MINGW32__)
/*
//  NAME
//      sigemptyset - initialize and empty a signal set
//
//  SYNOPSIS
//      #include <signal.h>
//
//      int sigemptyset(sigset_t *set); [Option End]
//
//  DESCRIPTION
//      The sigemptyset() function initializes the signal set pointed to by set, such that all signals defined in POSIX.1-2017 are excluded.
//
//  RETURN VALUE
//      Upon successful completion, sigemptyset() shall return 0; otherwise, it shall return -1 and set errno to indicate the error.
//
//  ERRORS
//      No errors are defined.
//
*/
LIBW32_API int
sigemptyset(sigset_t *ss)
{
//  if (ss) {
//      memset(ss, 0, sizeof(*ss));
//  }
    errno = EINVAL;
    return -1;
}


LIBW32_API int
sigaction(int sig, struct sigaction *sa, struct sigaction *osa)
{
//  if (sa) {
//      if (osa) {
//          osa->sa_handler = signal(sig, (void (__cdecl *)(int))sa->sa_handler);
//
//      } else {
//          signal(sig, (void (__cdecl *)(int))sa->sa_handler);
//      }
//  }

    errno = EINVAL;
    return -1;
}

#endif /*__MINGW32__*/

/*end*/

