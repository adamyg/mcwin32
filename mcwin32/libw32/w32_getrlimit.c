#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_getrlimit_c,"$Id: w32_getrlimit.c,v 1.4 2024/01/16 15:17:51 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 getrlimit() system calls
 *
 * Copyright (c) 2020 - 2024, Adam Young.
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

#include <sys/resource.h>
#include <stdio.h>
#include <unistd.h>


/*
NAME
    getrlimit, setrlimit - control maximum resource consumption

SYNOPSIS
    #include <sys/resource.h>

    int getrlimit(int resource, struct rlimit *rlp);
    int setrlimit(int resource, const struct rlimit *rlp); [Option End]

DESCRIPTION
    The getrlimit() function shall get, and the setrlimit() function shall set, limits on the consumption of a variety of resources.

    Each call to either getrlimit() or setrlimit() identifies a specific resource to be operated upon as well as a resource limit.
    A resource limit is represented by an rlimit structure. The rlim_cur member specifies the current or soft limit and the rlim_max 
    member specifies the maximum or hard limit. Soft limits may be changed by a process to any value that is less than or equal to
    the hard limit. A process may (irreversibly) lower its hard limit to any value that is greater than or equal to the soft limit.
    Only a process with appropriate privileges can raise a hard limit. Both hard and soft limits can be changed in a single
    call to setrlimit() subject to the constraints described above.

    The value RLIM_INFINITY, defined in <sys/resource.h>, shall be considered to be larger than any other limit value.
    If a call to getrlimit() returns RLIM_INFINITY for a resource, it means the implementation shall not enforce limits on that resource.
    Specifying RLIM_INFINITY as any resource limit value on a successful call to setrlimit() shall inhibit enforcement of that resource limit.

    The following resources are defined:

    RLIMIT_CORE
        This is the maximum size of a core file, in bytes, that may be created by a process.
        A limit of 0 shall prevent the creation of a core file. If this limit is exceeded, the writing of a core file shall terminate at this size.

    RLIMIT_CPU
        This is the maximum amount of CPU time, in seconds, used by a process.
        If this limit is exceeded, SIGXCPU shall be generated for the process.
        If the process is catching or ignoring SIGXCPU, or all threads belonging to that process are blocking SIGXCPU, the behavior is unspecified.

    RLIMIT_DATA
        This is the maximum size of a data segment of the process, in bytes.
        If this limit is exceeded, the malloc() function shall fail with errno set to [ENOMEM].

    RLIMIT_FSIZE
        This is the maximum size of a file, in bytes, that may be created by a process.
        If a write or truncate operation would cause this limit to be exceeded, SIGXFSZ shall be generated for the thread.
        If the thread is blocking, or the process is catching or ignoring SIGXFSZ, continued attempts to increase the size
        of a file from end-of-file to beyond the limit shall fail with errno set to [EFBIG].

    RLIMIT_NOFILE
        This is a number one greater than the maximum value that the system may assign to a newly-created descriptor.
        If this limit is exceeded, functions that allocate a file descriptor shall fail with errno set to [EMFILE].
        This limit constrains the number of file descriptors that a process may allocate.

    RLIMIT_STACK
        This is the maximum size of the initial thread's stack, in bytes.
        The implementation does not automatically grow the stack beyond this limit.
        If this limit is exceeded, SIGSEGV shall be generated for the thread.
        If the thread is blocking SIGSEGV, or the process is ignoring or catching SIGSEGV and has not made arrangements
        to use an alternate stack, the disposition of SIGSEGV shall be set to SIG_DFL before it is generated.

    RLIMIT_AS
        This is the maximum size of total available memory of the process, in bytes.
        If this limit is exceeded, the malloc() and mmap() functions shall fail with errno set to [ENOMEM].
        In addition, the automatic stack growth fails with the effects outlined above.

    When using the getrlimit() function, if a resource limit can be represented correctly in an object of type rlim_t,
    then its representation is returned; otherwise, if the value of the resource limit is equal to that of the corresponding 
    saved hard limit, the value returned shall be RLIM_SAVED_MAX; otherwise, the value returned shall be RLIM_SAVED_CUR.

    When using the setrlimit() function, if the requested new limit is RLIM_INFINITY, the new limit shall be "no limit''; otherwise, 
    if the requested new limit is RLIM_SAVED_MAX, the new limit shall be the corresponding saved hard limit; otherwise, if the 
    requested new limit is RLIM_SAVED_CUR, the new limit shall be the corresponding saved soft limit; otherwise, the new limit 
    shall be the requested value. In addition, if the corresponding saved limit can be represented correctly in an object
    of type rlim_t then it shall be overwritten with the new limit.

    The result of setting a limit to RLIM_SAVED_MAX or RLIM_SAVED_CUR is unspecified unless a previous call to getrlimit()
    returned that value as the soft or hard limit for the corresponding resource limit.

    The determination of whether a limit can be correctly represented in an object of type rlim_t is implementation-defined.
    For example, some implementations permit a limit whose value is greater than RLIM_INFINITY and others do not.

    The exec family of functions shall cause resource limits to be saved.

RETURN VALUE
    Upon successful completion, getrlimit() and setrlimit() shall return 0. Otherwise, these functions shall return -1 and set errno to indicate the error.

ERRORS
    The getrlimit() and setrlimit() functions shall fail if:

    [EINVAL]
        An invalid resource was specified; or in a setrlimit() call, the new rlim_cur exceeds the new rlim_max.
    [EPERM]
        The limit specified to setrlimit() would have raised the maximum limit value, and the calling process does not have appropriate privileges.

    The setrlimit() function may fail if:

    [EINVAL]
        The limit specified cannot be lowered because current usage is already higher than the limit.
*/

int
getrlimit(int resource, struct rlimit *rlp)
{
    if (NULL == rlp) {
        errno = EINVAL;
    } else {
        switch (resource) {
        case RLIMIT_NOFILE:
#if defined(__WATCOMC__)
            rlp->rlim_cur = _grow_handles(_NFILES);
            rlp->rlim_max = 2048; /*assumed limit*/
#else
            rlp->rlim_cur = _getmaxstdio();
#if defined(_MSC_VER) && (_MSC_VER >= 1920)
            rlp->rlim_max = 8192; /*2019, documented limit*/
#else
            rlp->rlim_max = 2048; /*documented limit*/
#endif
#endif
            if (rlp->rlim_cur > rlp->rlim_max) {
                rlp->rlim_max = rlp->rlim_cur;
            }
            return 0;
        }
        errno = ENOSYS;
    }
    return -1;
}

/*end*/
