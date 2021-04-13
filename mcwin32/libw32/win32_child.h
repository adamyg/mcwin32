#ifndef LIBW32_WIN32_CHILD_H_INCLUDED
#define LIBW32_WIN32_CHILD_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_child_h,"$Id: win32_child.h,v 1.8 2021/04/13 15:49:35 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * child process support
 *
 * Copyright (c) 2007, 2012 - 2018 Adam Young.
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
