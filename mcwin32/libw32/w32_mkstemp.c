#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_mkstemp_c,"$Id: w32_mkstemp.c,v 1.5 2018/10/12 00:52:04 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 mkstemp() implementation
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#include "win32_internal.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#define DISABLE_HARD_ERRORS     (void)SetErrorMode (0);
#define ENABLE_HARD_ERRORS      (void)SetErrorMode (SEM_FAILCRITICALERRORS | \
                                        SEM_NOOPENFILEERRORBOX);


/*
//  NAME
//      mkstemp - make a unique filename
//
//  SYNOPSIS
//      #include <stdlib.h>
//
//      int mkstemp(char *template);
//
//  DESCRIPTION
//
//      The mkstemp() function shall replace the contents of the string pointed to by
//      template by a unique filename, and return a file descriptor for the file open for
//      reading and writing. The function thus prevents any possible race condition between
//      testing whether the file exists and opening it for use.
//
//      The string in template should look like a filename with six trailing 'X' s;
//      mkstemp() replaces each 'X' with a character from the portable filename character
//      set. The characters are chosen such that the resulting name does not duplicate the
//      name of an existing file at the time of a call to mkstemp().
//
//      Each successful call to mkstemp modifies template. In each subsequent call from the same
//      process or thread with the same template argument, mkstemp checks for filenames that
//      match names returned by mkstemp in previous calls. If no file exists for a given name,
//      mkstemp returns that name. If files exist for all previously returned names, mkstemp
//      creates a new name by replacing the alphabetic character it used in the previously
//      returned name with the next available lowercase letter, in order, from 'a' through 'z'.
//
//  RETURN VALUE
//
//      Upon successful completion, mkstemp() shall return an open file descriptor.
//      Otherwise, -1 shall be returned if no suitable file could be created.
//
//  ERRORS
//      No errors are defined.
*/

#define GETTEMP_SUCCESS     1
#define GETTEMP_ERROR       0

static int                  gettemp(char *path, register int *fd, int temporary);


LIBW32_API int
w32_mkstemp(char *path)
{
    int fildes = -1;
    return (GETTEMP_SUCCESS == gettemp(path, &fildes, FALSE) ? fildes : -1);
}


LIBW32_API int
w32_mkstempx(char *path)
{
    int fildes = -1;
    return (GETTEMP_SUCCESS == gettemp(path, &fildes, TRUE) ? fildes : -1);
}


static int
gettemp(char *path, register int *fildes, int temporary)
{
    register char *start, *trv;
    struct stat sbuf;
    unsigned pid;
    int rc;
    char c;

    pid = (unsigned) WIN32_GETPID();
    for (trv = path; *trv; ++trv)           /* extra X's get set to 0's */
        /*continue*/;
    while (*--trv == 'X' && trv >= path) {
        *trv = (char)((pid % 10) + '0');
        pid /= 10;
    }

    /*
     *  check the target directory; if you have six X's and it
     *  doesn't exist this runs for a *very* long time.
     */
    for (start = trv + 1;; --trv) {
        if (trv <= path) {
            break;
        }

        if ((c = *trv) == '/' || c == '\\') {
            *trv = '\0';
            if (trv[-1] == ':') {
                *trv = c;
                break;
            }
            DISABLE_HARD_ERRORS
            rc = stat(path, &sbuf);
            ENABLE_HARD_ERRORS
            if (rc) {
                return GETTEMP_ERROR;
            }
            if (!(sbuf.st_mode & S_IFDIR)) {
                errno = ENOTDIR;
                return GETTEMP_ERROR;
            }
            *trv = c;
            break;
        }
    }


    /*
     *  Create file as temporary; file is deleted when last file descriptor is closed.
     */
#if defined(_O_TEMPORARY)
#define O_MODEX     (O_CREAT|O_EXCL|O_RDWR|O_BINARY|_O_TEMPORARY)
#elif defined(O_TEMPORARY)
#define O_MODEX	    (O_CREAT|O_EXCL|O_RDWR|O_BINARY|O_TEMPORARY)
#else
#define O_MODEX	    (O_CREAT|O_EXCL|O_RDWR|O_BINARY)
#endif

#define O_MODE	    (O_CREAT|O_EXCL|O_RDWR|O_BINARY)

    for (;;) {
        errno = 0;
        if (fildes) {
                if ((*fildes = WIN32_OPEN(path, (temporary ? O_MODEX : O_MODE), 0600)) >= 0) {
                    return GETTEMP_SUCCESS;
                }
                if (EEXIST != errno) {
                    return GETTEMP_ERROR;
                }
        } else {
                DISABLE_HARD_ERRORS
                rc = stat(path, &sbuf);
                ENABLE_HARD_ERRORS
                if (rc) {
#ifndef ENMFILE
                    return (((ENOENT == errno)) ? GETTEMP_SUCCESS : GETTEMP_ERROR);
#else
                    return (((ENOENT == errno) || (ENMFILE == errno)) ? GETTEMP_ERROR);
#endif
                }
        }

        /* next is sequence */
        for (trv = start;;) {
            if (*trv == '\0') {                 /* EOS */
                return GETTEMP_ERROR;
            }

            if ('z' == *trv) {
                *trv++ = 'a';
            } else {
                if (isdigit(*trv)) {
                    *trv = 'a';
                } else {
                    ++*trv;
                }
                break;
            }
        }
    }
    /*NOTREACHED*/
}

/*end*/

