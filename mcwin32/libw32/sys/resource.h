#ifndef LIBW32_SYS_RESOURCE_H_INCLUDED
#define LIBW32_SYS_RESOURCE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_resource_h,"$Id: ")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 2020 - 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ==end==
 */

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */

#include <sys/time.h>                           /* timeval */

/*
 *  Priority
 */
#define PRIO_MIN            -20
#define PRIO_MAX            20

#define PRIO_PROCESS        1                   /* Identifies the who argument as a process ID. */
#define PRIO_PGRP           2                   /* Identifies the who argument as a process group ID. */
#define PRIO_USER           3                   /* Identifies the who argument as a user ID. */

/*
 *  Resource usage
 */
#define RUSAGE_SELF         0                   /* Returns information about the current process. */
#define RUSAGE_CHILDREN     1                   /* Returns information about children of the current process. */
#define RUSAGE_THREAD       RUSAGE_CHILDREN     /* mirror children */


/*
 *  Resource limits
 */
#define RLIMIT_CPU          0                   /* cpu time in milliseconds */
#define RLIMIT_FSIZE        1                   /* maximum file size */
#define RLIMIT_DATA         2                   /* data size */
#define RLIMIT_STACK        3                   /* stack size */
#define RLIMIT_CORE         4                   /* core file size */
#define RLIMIT_RSS          5                   /* resident set size */
#define RLIMIT_MEMLOCK      6                   /* locked-in-memory address space */
#define RLIMIT_NPROC        7                   /* number of processes */
#define RLIMIT_NOFILE       8                   /* number of open files */
#define RLIMIT_SBSIZE       9                   /* maximum size of all socket buffers */
#define RLIMIT_AS           10                  /* virtual process size (inclusive of mmap) */
#define RLIMIT_VMEM         RLIMIT_AS           /* common alias */
#define RLIMIT_NTHR         11                  /* number of threads */

__BEGIN_DECLS

struct rusage {
    struct timeval ru_utime;                    /* user CPU time used */
    struct timeval ru_stime;                    /* system CPU time used */
    long   ru_maxrss;                           /* maximum resident set size */
    long   ru_ixrss;                            /* integral shared memory size */
    long   ru_idrss;                            /* integral unshared data size */
    long   ru_isrss;                            /* integral unshared stack size */
    long   ru_minflt;                           /* page reclaims (soft page faults) */
    long   ru_majflt;                           /* page faults (hard page faults) */
    long   ru_nswap;                            /* swaps */
    long   ru_inblock;                          /* block input operations */
    long   ru_oublock;                          /* block output operations */
    long   ru_msgsnd;                           /* IPC messages sent */
    long   ru_msgrcv;                           /* IPC messages received */
    long   ru_nsignals;                         /* signals received */
    long   ru_nvcsw;                            /* voluntary context switches */
    long   ru_nivcsw;                           /* involuntary context switches */
};


#if !defined(HAVE_RLIM_T)
#if defined(_WIN64)
typedef unsigned long long rlim_t;
#define RLIM_INFINITY (rlim_t)-1LL              /* no limit */
#else
typedef unsigned long rlim_t;
#define RLIM_INFINITY (rlim_t)-1L               /* no limit */
#endif
#define RLIM_SAVED_MAX RLIM_INFINITY            /* unrepresentable hard limit */
#define RLIM_SAVED_CURRLIM_INFINITY             /* unrepresentable soft limit */
#define HAVE_RLIM_T
#endif

struct rlimit {
    rlim_t rlim_cur;                            /* current (soft) limit */
    rlim_t rlim_max;                            /* maximum value for rlim_cur */
};


/*  Not all fields are completed; unmaintained fields are set to zero.
    The unmaintained fields are provided for compatibility with other systems,
    and because they may one day be supported. */

//  LIBW32_API int          getpriority(int, id_t);
//  LIBW32_API int          setpriority(int, id_t, int);

LIBW32_API int              getrlimit(int, struct rlimit *);
LIBW32_API int              setrlimit(int, const struct rlimit *);

LIBW32_API int              getrusage(int, struct rusage *);

__END_DECLS

#endif /*LIBW32_SYS_RESOURCE_H_INCLUDED*/
