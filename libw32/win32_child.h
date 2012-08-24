#ifndef LIBW32_WIN32_CHILD_H_INCLUDED
#define LIBW32_WIN32_CHILD_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/*
 * child process support
 *
 * Copyright (c) 2007, 2012, Adam Young.
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

#if !defined(WNOHANG)
#define WNOHANG         1
#endif

const char *            w32_getshell (void);
const char *            w32_gethome (void);

int                     w32_iscommand (const char *);
int                     w32_shell (const char *shell, const char *cmd,
                                        const char *fstdin, const char *fstdout, const char *fstderr);

int                     w32_spawn (win32_spawn_t *args, int Stdout, int Stderr, int *Stdin);
int                     w32_spawn2 (win32_spawn_t *args, int *Stdin, int *Stdout, int *Stderr);

HANDLE                  w32_child_exec (struct win32_spawn *args, HANDLE hStdin, HANDLE hStdOut, HANDLE hStdErr);
int                     w32_child_wait (HANDLE hProc, int *status, int nowait);

FILE *                  w32_popen (const char *cmd, const char *mode);
int                     w32_pclose (FILE *file);
int                     w32_pread_err (FILE *file, char *buf, int length);

__END_DECLS

#endif /*LIBW32_WIN32_CHILD_H_INCLUDED*/

