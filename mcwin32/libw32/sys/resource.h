#ifndef LIBW32_SYS_RESOURCE_H_INCLUDED
#define LIBW32_SYS_RESOURCE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_resource_h,"$Id: ")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 2020, Adam Young.
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

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */

#include <sys/time.h>                           /* timeval */

/* getpriority() and setpriority() */
#define PRIO_PROCESS        1   /* Identifies the who argument as a process ID. */
#define PRIO_PGRP           2   /* Identifies the who argument as a process group ID. */
#define PRIO_USER           3   /* Identifies the who argument as a user ID. */

/* getrusage() */
#define RUSAGE_SELF         0   /* Returns information about the current process. */
#define RUSAGE_CHILDREN     1   /* Returns information about children of the current process. */
#define RUSAGE_THREAD       RUSAGE_CHILDREN     /* mirror children */

struct rusage {
    struct timeval ru_utime;    /* user CPU time used */
    struct timeval ru_stime;    /* system CPU time used */
    long   ru_maxrss;           /* maximum resident set size */
    long   ru_ixrss;            /* integral shared memory size */
    long   ru_idrss;            /* integral unshared data size */
    long   ru_isrss;            /* integral unshared stack size */
    long   ru_minflt;           /* page reclaims (soft page faults) */
    long   ru_majflt;           /* page faults (hard page faults) */
    long   ru_nswap;            /* swaps */
    long   ru_inblock;          /* block input operations */
    long   ru_oublock;          /* block output operations */
    long   ru_msgsnd;           /* IPC messages sent */
    long   ru_msgrcv;           /* IPC messages received */
    long   ru_nsignals;         /* signals received */
    long   ru_nvcsw;            /* voluntary context switches */
    long   ru_nivcsw;           /* involuntary context switches */
};

/*  Not all fields are completed; unmaintained fields are set to zero.
    The unmaintained fields are provided for compatibility with other systems,
    and because they may one day be supported. */

__BEGIN_DECLS

//  LIBW32_API int          getpriority(int, id_t);
//  LIBW32_API int          getrlimit(int, struct rlimit *);
LIBW32_API int          getrusage(int, struct rusage *);
//  LIBW32_API int          setpriority(int, id_t, int);
//  LIBW32_API int          setrlimit(int, const struct rlimit *);

__END_DECLS

#endif  /*LIBW32_SYS_RESOURCE_H_INCLUDED*/
