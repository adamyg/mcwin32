#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_signal_c,"$Id: w32_signal.c,v 1.16 2025/04/30 19:29:57 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 signal support
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
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
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 */

#include "win32_internal.h"
#include <unistd.h>
#include <signal.h>

#if !defined(__MINGW32__) || defined(__MINGW64_VERSION_MAJOR)
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
    if (ss) {
        memset(ss, 0, sizeof(*ss));
        return 0;
    }
    errno = EINVAL;
    return -1;
}


LIBW32_API int
sigaction(int sig, struct sigaction *sa, struct sigaction *osa)
{
    switch (sig) {
    case SIGPIPE:
    case SIGCHLD:
        return 0;
    }

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
