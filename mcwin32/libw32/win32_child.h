#ifndef LIBW32_WIN32_CHILD_H_INCLUDED
#define LIBW32_WIN32_CHILD_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_child_h,"$Id: win32_child.h,v 1.7 2018/10/12 00:52:05 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * child process support
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

#include <sys/cdefs.h>
#include <stdio.h>
#include <win32_include.h>

__BEGIN_DECLS

typedef struct win32_spawn {
    const char *        cmd;
    const char **       argv;
    const char **       envp;
    const char *        dir;
#define W32_SPAWNDETACHED               0x01
    int                 flags;
    unsigned long       _dwFlags;               /* reserved */
    unsigned long       _dwProcessId;           /* reserved */
} win32_spawn_t;

typedef struct {
    win32_spawn_t       spawn;
    HANDLE              hInput;
    HANDLE              hOutput;
    HANDLE              hError;
    HANDLE              hProc;
} win32_exec_t;

#if !defined(WNOHANG)
#define WNOHANG         1
#endif

LIBW32_API const char * w32_getshell (void);
LIBW32_API const char * w32_gethome (int ignore_env);

LIBW32_API int          w32_iscommand (const char *);
LIBW32_API int          w32_shell (const char *shell, const char *cmd,
                              const char *fstdin, const char *fstdout, const char *fstderr);

LIBW32_API int          w32_spawn (win32_spawn_t *args, int Stdout, int Stderr, int *Stdin);
LIBW32_API int          w32_spawn2 (win32_spawn_t *args, int *Stdin, int *Stdout, int *Stderr);

LIBW32_API int          w32_exec (win32_exec_t *args);

LIBW32_API HANDLE       w32_child_exec (struct win32_spawn *args, HANDLE hStdin, HANDLE hStdOut, HANDLE hStdErr);
LIBW32_API int          w32_child_wait (HANDLE hProc, int *status, int nowait);

/*stdio.h*/
LIBW32_API FILE *       w32_popen (const char *cmd, const char *mode);
LIBW32_API int          w32_pclose (FILE *file);
LIBW32_API int          w32_pread_err (FILE *file, char *buf, int length);

/*unistd,h*/
LIBW32_API ssize_t      pread (int fildes, void *buf, size_t nbyte, off_t offset);
LIBW32_API ssize_t      pwrite (int fildes, const void *buf, size_t nbyte, off_t offset);

__END_DECLS

#endif /*LIBW32_WIN32_CHILD_H_INCLUDED*/
