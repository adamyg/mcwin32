#ifndef LIBW32_SYS_SYSCONF_H_INCLUDED
#define LIBW32_SYS_SYSCONF_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_sysconf_h,"$Id: $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 sysconf() implementation
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

enum {
        _SC_ARG_MAX,
                // The maximum bytes of argument to execve(2).

        _SC_CHILD_MAX,
                // The maximum number of simultaneous processes per user id.

        _SC_CLK_TCK,
                // The frequency of the statistics clock in ticks per second.

        _SC_IOV_MAX,
                // The maximum number of elements in the I/O vector used by readv(2), writev(2),
                // recvmsg(2), and sendmsg(2).

        _SC_NGROUPS_MAX,
                // The maximum number of supplemental groups.

        _SC_NPROCESSORS_CONF,
                // The number of processors configured.

        _SC_NPROCESSORS_ONLN,
                // The number of processors currently on-line.

        _SC_OPEN_MAX,
                // One more than the maximum value the system may assign to a new file descriptor.

        _SC_PAGESIZE,
                // The size of a system page in bytes.

        _SC_PAGE_SIZE,
                // Equivalent to _SC_PAGESIZE.

        _SC_STREAM_MAX,
                // The minimum maximum number of streams that a process may have open at any one time.

        _SC_TZNAME_MAX,
                // The minimum maximum number of types supported for the name of a timezone.

        _SC_JOB_CONTROL,
                // Return 1 if job control is available on this system, otherwise -1.

        _SC_SAVED_IDS,
                // Returns 1 if saved set-group and saved set-user ID is available, otherwise -1.

        _SC_VERSION,
                // The version of IEEE Std 1003.1 ("POSIX.1") with which the system attempts to
                // comply.

        _SC_BC_BASE_MAX,
                // The maximum ibase/obase values in the bc(1) utility.

        _SC_BC_DIM_MAX,
                // The maximum array size in the bc(1) utility.

        _SC_BC_SCALE_MAX,
                // The maximum scale value in the bc(1) utility.

        _SC_BC_STRING_MAX,
                // The maximum string length in the bc(1) utility.

        _SC_COLL_WEIGHTS_MAX,
                // The maximum number of weights that can be assigned to any entry of the LC_COLLATE
                // order keyword in the locale definition file.

        _SC_EXPR_NEST_MAX,
                // The maximum number of expressions that can be nested within parenthesis by the
                // expr(1) utility.

        _SC_LINE_MAX,
                // The maximum length in bytes of a text-processing utility's input line.

        _SC_RE_DUP_MAX,
                // The maximum number of repeated occurrences of a regular expression permitted
                // when using interval notation.

        _SC_2_VERSION,
                // The version of IEEE Std 1003.2 ("POSIX.2") with which the system attempts to
                // comply.

        _SC_2_C_BIND,
                // Return 1 if the system's C-language development facilities support the
                // C-Language Bindings Option, otherwise -1.

        _SC_2_C_DEV,
                // Return 1 if the system supports the C-Language Development Utilities Option,
                // otherwise -1.

        _SC_2_CHAR_TERM,
                // Return 1 if the system supports at least one terminal type capable of all
                // operations described in IEEE Std 1003.2 ("POSIX.2"), otherwise -1.

        _SC_2_FORT_DEV,
                // Return  1 if the system supports the FORTRAN Development Utilities Option,
                // otherwise -1.

        _SC_2_FORT_RUN,
                // Return 1 if the system supports the FORTRAN Runtime Utilities Option, 
                // otherwise -1.

        _SC_2_LOCALEDEF,
                // Return 1 if the system supports the creation of locales, otherwise -1.

        _SC_2_SW_DEV,
                // Return 1 if the system supports the Software Development Utilities Option,
                // otherwise -1.

        _SC_2_UPE,
                // Return 1 if the system supports the User Portability Utilities Option,
                // otherwise -1.

        _SC_AIO_LISTIO_MAX,
                // Maximum number of I/O operations in a single list I/O call supported.

        _SC_AIO_MAX,
                // Maximum number of outstanding asynchronous I/O operations supported.

        _SC_AIO_PRIO_DELTA_MAX,
                // The maximum amount by which a process can decrease its asynchronous I/O
                // priority level from its own scheduling priority.

        _SC_DELAYTIMER_MAX,
                // Maximum number of timer expiration overruns.

        _SC_MQ_OPEN_MAX,
                // The maximum number of open message queue descriptors a process may hold.

        _SC_RTSIG_MAX,
                // Maximum number of real-time signals reserved for application use.

        _SC_SEM_NSEMS_MAX,
                // Maximum number of semaphores that a process may have.

        _SC_SEM_VALUE_MAX,
                // The maximum value a semaphore may have.

        _SC_SIGQUEUE_MAX,
                // Maximum number of queued signals that a process may send and have pending at the
                // receiver(s) at any time.

        _SC_TIMER_MAX,
                // Maximum number of timers per process supported.

        _SC_GETGR_R_SIZE_MAX,
                // Suggested initial value for the size of the group entry buffer.

        _SC_GETPW_R_SIZE_MAX,
                // Suggested initial value for the size of the password entry buffer.

        _SC_HOST_NAME_MAX,
                // Maximum length of a host name (not including the terminating null) as returned
                // from the gethostname() function.

        _SC_LOGIN_NAME_MAX,
                // Maximum length of a login name.

        _SC_THREAD_STACK_MIN,
                // Minimum size in bytes of thread stack storage.

        _SC_THREAD_THREADS_MAX,
                // Maximum number of threads that can be created per process.

        _SC_TTY_NAME_MAX,
                // Maximum length of terminal device name.

        _SC_SYMLOOP_MAX,
                // Maximum number of symbolic links that can be reliably traversed in the resolution
                // of a pathname in the absence of a loop.

        _SC_ATEXIT_MAX,
                // Maximum number of functions that may be registered with atexit().

        _SC_XOPEN_VERSION,
                // An integer value greater than or equal to 4, indicating the version of the X/Open
                // Portability Guide to which this system conforms.

        _SC_XOPEN_XCU_VERSION,
                // An integer value indicating the version of the XCU Specification to which this
                // system conforms.

        // These values also exist, but may not be standard :

        _SC_CPUSET_SIZE,
                // Size of the kernel cpuset.

        _SC_PHYS_PAGES,
                // The number of pages of physical memory. Note that it is possible that the product
                // of this value and the value of_SC_PAGESIZE will overflow a long in some configurations
                // on a 32bit machine.
};

LIBW32_API long sysconf(int name);

__END_DECLS

#endif /*LIBW32_SYS_SYSCONF_H_INCLUDED*/
