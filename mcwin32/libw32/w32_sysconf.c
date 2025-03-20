#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_sysconf_c,"$Id: w32_sysconf.c,v 1.5 2025/03/20 17:23:09 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 interface support
 *
 * Copyright (c) 2021 - 2025 Adam Young.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x600)
#undef _WIN32_WINNT                             /* Vista+ features; FILE_INFO_BY_HANDLE_CLASS */
#define _WIN32_WINNT 0x0600
#endif

#include <sys/cdefs.h>

#include <sys/sysconf.h>
#include <sys/sysinfo.h>

#include <limits.h>

#include "win32_internal.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

struct ProcessorInfo {
    unsigned processorCoreCount;
    unsigned processorPackageCount;
    unsigned logicalProcessorCount;
    unsigned numaNodeCount;
    unsigned processorL1CacheCount;
    unsigned processorL2CacheCount;
    unsigned processorL3CacheCount;
};

static int  GetProcessorInfo(struct ProcessorInfo *pi);
static long NumberOfProcessors(void);
static long PageSize(void);
static long PhysicalPages(void);


/*
//  NAME
//      sysconf -- get configurable system variables
//
//  SYNOPSIS
//      #include <unistd.h>
//
//      long sysconf(int name);
//
//  DESCRIPTION
//      This interface is defined by IEEE Std 1003.1 - 1988 (``POSIX.1'').A far
//      more complete interface is available using sysctl(3).
//
//      The sysconf() function provides a method for applications to determine
//      the current value of a configurable system limit or option variable.The
//      name argument specifies the system variable to be queried.Symbolic constants
//      for each name value are found in the include file <unistd.h>.
//
*/

long
sysconf(int name)
{
    switch (name) {
    case _SC_NPROCESSORS_CONF:      
        return get_nprocs_conf();
    case _SC_NPROCESSORS_ONLN:
        return get_nprocs();
    case _SC_PAGESIZE:
        return PageSize();
    case _SC_PHYS_PAGES:
        return PhysicalPages();
    case _SC_HOST_NAME_MAX:
#if (MAXHOSTNAMELEN > MAX_COMPUTERNAME_LENGTH)
        return MAXHOSTNAMELEN;
#else
        return MAX_COMPUTERNAME_LENGTH;
#endif
#if defined(ATEXIT_MAX)
    case _SC_ATEXIT_MAX:
        return ATEXIT_MAX;
#endif
    default:
        break;
    }
    return -1;
}


/*
//  NAME
//      get_nprocs, get_nprocs_conf --- get number of processors
//
//  SYNOPSIS
//      #include <sys/sysinfo.h>
//      int get_nprocs(void);
//      int get_nprocs_conf(void);
//
//  DESCRIPTION
//      The function get_nprocs_conf() returns the number of processors configured 
//      by the operating system.
//
//      The function get_nprocs() returns the number of processors currently available
//      in the system.This may be less than the number returned by get_nprocs_conf() 
//      because processors may be off-line.
//
*/

int
get_nprocs()
{
    struct ProcessorInfo pi = { 0 };
    if (0 == GetProcessorInfo(&pi)) {
        return pi.logicalProcessorCount;
    }
    return -1;
}


int
get_nprocs_conf()
{
    return NumberOfProcessors();
}


static unsigned
CountSetBits(ULONG_PTR bitMask)
{
    DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
    unsigned i, bitSetCount = 0;

    for (i = 0; i <= LSHIFT; ++i) {
        bitSetCount += ((bitMask & bitTest) ? 1 : 0);
        bitTest /= 2;
    }
    return bitSetCount;
}


static int
GetProcessorInfo(struct ProcessorInfo *pi)
{
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION *slpi = NULL;
    DWORD len = 0;

    while (! GetLogicalProcessorInformation(slpi, &len)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            if (NULL == (slpi = malloc(len))) {
                return -1;
            }
        } else {
            errno = EINVAL;
            return -1;
        }
    }

    if (slpi && len) {
        const SYSTEM_LOGICAL_PROCESSOR_INFORMATION *cursor = slpi;
        DWORD offset = 0;

        while (offset < len) {
            switch (cursor->Relationship) {
            case RelationNumaNode:
                pi->numaNodeCount++;
                break;
            case RelationProcessorCore:
                pi->logicalProcessorCount += CountSetBits(cursor->ProcessorMask);
                pi->processorCoreCount++;
                break;
            case RelationCache: {
                    // CACHE_DESCRIPTOR structure for each cache. 
                    const CACHE_DESCRIPTOR *Cache = &cursor->Cache;
                    if (Cache->Level == 1) {
                        pi->processorL1CacheCount++;
                    } else if (Cache->Level == 2) {
                        pi->processorL2CacheCount++;
                    } else if (Cache->Level == 3) {
                        pi->processorL3CacheCount++;
                    }
                }
                break;
            case RelationProcessorPackage:
                pi->processorPackageCount++;
                break;
            default:
                break;
            }
            offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
            ++cursor;
        }
    }

    free(slpi);
    return 0;
}


static long
NumberOfProcesses(void)
{
    //TODO
    return 0;
}


static long
NumberOfProcessors(void)
{
    SYSTEM_INFO si = {0};
    GetNativeSystemInfo(&si);
    return si.dwNumberOfProcessors;
}


static long
PageSize(void)
{
    SYSTEM_INFO si = {0};
    GetNativeSystemInfo(&si);
    return si.dwPageSize;
}


static long
PhysicalPages()
{
    MEMORYSTATUSEX ms = {0};
    ms.dwLength = sizeof(ms);
    if (GlobalMemoryStatusEx(&ms)) {
        return (long)(ms.ullTotalPhys / PageSize());
    }
    return -1;
}


////////////////////////////////////////////////////////////////////////////////
//  https://learn.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntquerysysteminformation?redirectedfrom=MSDN
//

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemTimeOfDayInformation = 3
        // NOTE: numerous other values
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_TIMEOFDAY_INFORMATION {
    LARGE_INTEGER BootTime;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER TimeZoneBias;
    ULONG TimeZoneId;
    ULONG Reserved;
    ULONGLONG BootTimeBias;
    ULONGLONG SleepTimeBias;
} SYSTEM_TIMEOFDAY_INFORMATION;

typedef DWORD (WINAPI * NtQuerySystemInformation_t)(
                SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);

/*
//  NAME
//      sysinfo -- return system information.
//
//  SYNOPSIS
//      #include <sys/sysinfo.h>
//
//      int sysinfo(struct sysinfo* info);
//
//  DESCRIPTION
//      sysinfo() returns certain statistics on memoryand swap usage, as well as the load average.
//
*/

int
sysinfo(struct sysinfo *info) 
{
    const unsigned page_size = (unsigned)PageSize();
    MEMORYSTATUSEX ms = {0};

    if (NULL == info) {
        errno = EFAULT;
        return -1;
    }

    memset(info, 0, sizeof(*info));             // zero unsupported fields

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    {   SYSTEM_TIMEOFDAY_INFORMATION sti = {0};
        HMODULE ntdll = GetModuleHandleA("ntdll");
        NtQuerySystemInformation_t fNtQuerySystemInformation =
                (NtQuerySystemInformation_t) GetProcAddress(ntdll, "NtQuerySystemInformation");
        long uptime = 0;

        if (fNtQuerySystemInformation &&        // uptime
                NO_ERROR == fNtQuerySystemInformation(SystemTimeOfDayInformation, (PVOID) &sti, sizeof(sti), NULL)) {
            uptime = (long)((sti.CurrentTime.QuadPart - sti.BootTime.QuadPart) / 10000000ULL);
        } else {                                // ticks / 1000
            uptime = (long)(GetTickCount64() / 1000ULL);
        }
        info->uptime = uptime;
    }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif

    ms.dwLength = sizeof(ms);
    if (! GlobalMemoryStatusEx(&ms)) {
        return -1;
    }

    info->totalram  = (unsigned long)(ms.ullTotalPhys / page_size);
    info->freeram   = (unsigned long)(ms.ullAvailPhys / page_size);
    info->totalswap = (unsigned long)((ms.ullTotalPageFile - ms.ullTotalPhys) / page_size);
    info->freeswap  = (unsigned long)((ms.ullAvailPageFile - ms.ullTotalPhys) / page_size);
    info->procs     = (unsigned short)NumberOfProcesses();
    info->mem_unit  = page_size;

    return 0;
}

//end
