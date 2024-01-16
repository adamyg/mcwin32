#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_getrusage_c,"$Id: w32_getrusage.c,v 1.5 2023/11/06 15:07:42 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 getrusage() system calls
 *
 * Copyright (c) 2020 - 2023, Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 * or (at your option) any later version.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#include "win32_internal.h"

#include <sys/resource.h>
#include <unistd.h>

#include <psapi.h>


/*
//  NAME
//      getrusage - get information about resource utilization
//
//  SYNOPSIS
//      [XSI] [Option Start] #include <sys/resource.h>
//
//      int getrusage(int who, struct rusage *r_usage); [Option End]
//
//  DESCRIPTION
//
//      The getrusage() function shall provide measures of the resources used by the current process or its terminated
//      and waited-for child processes. If the value of the who argument is RUSAGE_SELF, information shall be returned
//      about resources used by the current process. If the value of the who argument is RUSAGE_CHILDREN, information
//      shall be returned about resources used by the terminated and waited-for children of the current process.
//
//      If the child is never waited for (for example, if the parent has SA_NOCLDWAIT set or sets SIGCHLD to SIG_IGN),
//      the resource information for the child process is discarded and not included in the resource information
//      provided by getrusage().
//
//      The r_usage argument is a pointer to an object of type struct rusage in which the returned information is stored.
//
//  RETURN VALUE
//
//      Upon successful completion, getrusage() shall return 0; otherwise, -1 shall be returned
//      and errno set to indicate the error.
//
//  ERRORS
//
//      The getrusage() function shall fail if:
//
//      [EINVAL]
//          The value of the who argument is not valid.
*/

static void
totimeval(const FILETIME *ft, struct timeval *tv)
{
    ULARGE_INTEGER time;

    time.LowPart = ft->dwLowDateTime;
    time.HighPart = ft->dwHighDateTime;

    tv->tv_sec  = (long) ( time.QuadPart / 10000000);
    tv->tv_usec = (long) ((time.QuadPart % 10000000) / 10);
}


LIBW32_API int
getrusage(int who, struct rusage *usage)
{
    int ret = -1;

    if (NULL == usage ||
            (who < RUSAGE_SELF || who > RUSAGE_CHILDREN)) {
        errno = EINVAL;

    } else {
        FILETIME creationTime, exittime, kerneltime, usertime;
        PROCESS_MEMORY_COUNTERS pmc = {0};

        (void) memset(usage, 0, sizeof(struct rusage));

        if (RUSAGE_SELF == who) {
            const HANDLE hProcess = GetCurrentProcess();

            if (! GetProcessTimes(hProcess, &creationTime, &exittime, &kerneltime, &usertime) ||
                ! GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                w32_errno_set();

            } else {
                totimeval(&kerneltime, &usage->ru_stime);   // system CPU time used
                totimeval(&usertime, &usage->ru_utime);     // user CPU time used
                usage->ru_majflt = pmc.PageFaultCount;      // page faults (hard page faults)
                usage->ru_maxrss = (long)(pmc.PeakWorkingSetSize / 1024); // maximum resident set size
                ret = 0;
            }

        } else if (RUSAGE_THREAD == who) {
            const HANDLE hThread = GetCurrentThread();

            if (! GetProcessTimes(hThread, &creationTime, &exittime, &kerneltime, &usertime)) {
                w32_errno_set();
            } else {
                totimeval(&kerneltime, &usage->ru_stime);   // system CPU time used
                totimeval(&usertime, &usage->ru_utime);     // user CPU time used
                ret = 0;
            }
        }
    }
    return ret;
}

/*end*/
