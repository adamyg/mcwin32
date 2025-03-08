#ifndef LIBW32_SYS_SYSINFO_H_INCLUDED
#define LIBW32_SYS_SYSINFO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_sysinit_h,"$Id: $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 sysinfo() implementation
 *
 * Copyright (c) 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
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

#include <sys/cdefs.h>

__BEGIN_DECLS

struct sysinfo {
    long uptime;                    /* Seconds since boot */
    unsigned long   loads[3];       /* 1, 5, and 15 minute load averages */
    unsigned long   totalram;       /* Total usable main memory size */
    unsigned long   freeram;        /* Available memory size */
    unsigned long   sharedram;      /* Amount of shared memory */
    unsigned long   bufferram;      /* Memory used by buffers */
    unsigned long   totalswap;      /* Total swap space size */
    unsigned long   freeswap;       /* swap space still available */
    unsigned short  procs;          /* Number of current processes */
    unsigned short  pad;            /* leaving this for linux compatibility */
    unsigned long   totalhigh;      /* Total high memory size */
    unsigned long   freehigh;       /* Available high memory size */
    unsigned int    mem_unit;       /* Memory unit size in bytes */
    char _f[20 - 2 * sizeof(long) - sizeof(int)];
                                    /* Padding to 64 bytes */
};

LIBW32_API int sysinfo(struct sysinfo *info);

LIBW32_API int get_nprocs(void);
LIBW32_API int get_nprocs_conf(void);

__END_DECLS

#endif /*LIBW32_SYS_SYSINFO_H_INCLUDED*/
