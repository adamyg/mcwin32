#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_errno_c,"$Id: w32_errno.c,v 1.18 2025/06/11 17:33:57 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 errno mapping support
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
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

#include "win32_internal.h"
#include <unistd.h>
#include <time.h>

/*
 *  Mapping for the first 499 system error messages (ie. base error codes), with all others are mapped to EIO.
 *
 *      https://docs.microsoft.com/en-us/windows/desktop/debug/system-error-codes--0-499-
 *
 *  Note, the follow represent a 'general' error code mapping, logic may require explicit mapping when error conditions are being replied upon.
 */
static const int    xlaterrno[] = {
 /* errno              Win32          Mnemonic                                 Description */
    0,              /* 0 (0x0)        ERROR_SUCCESS                            The operation completed successfully. */
    EINVAL,         /* 1 (0x1)        ERROR_INVALID_FUNCTION                   Incorrect function. */
    ENOENT,         /* 2 (0x2)        ERROR_FILE_NOT_FOUND                     The system cannot find the file specified. */
    ENOENT,         /* 3 (0x3)        ERROR_PATH_NOT_FOUND                     The system cannot find the path specified. */
    EMFILE,         /* 4 (0x4)        ERROR_TOO_MANY_OPEN_FILES                The system cannot open the file. */
    EACCES,         /* 5 (0x5)        ERROR_ACCESS_DENIED                      Access is denied. */
    EBADF,          /* 6 (0x6)        ERROR_INVALID_HANDLE                     The handle is invalid. */
    ENOMEM,         /* 7 (0x7)        ERROR_ARENA_TRASHED                      The storage control blocks were destroyed. */
    ENOMEM,         /* 8 (0x8)        ERROR_NOT_ENOUGH_MEMORY                  Not enough storage is available to process this command. */
    ENOMEM,         /* 9 (0x9)        ERROR_INVALID_BLOCK                      The storage control block address is invalid. */
    E2BIG,          /* 10 (0xA)       ERROR_BAD_ENVIRONMENT                    The environment is incorrect. */
    ENOEXEC,        /* 11 (0xB)       ERROR_BAD_FORMAT                         An attempt was made to load a program with an incorrect format. */
    EACCES,         /* 12 (0xC)       ERROR_INVALID_ACCESS                     The access code is invalid. */
    ERANGE,         /* 13 (0xD)       ERROR_INVALID_DATA                       The data is invalid. */
    ENOMEM,         /* 14 (0xE)       ERROR_OUTOFMEMORY                        Not enough storage is available to complete this operation. */
    ENODEV,         /* 15 (0xF)       ERROR_INVALID_DRIVE                      The system cannot find the drive specified. */
    EACCES,         /* 16 (0x10)      ERROR_CURRENT_DIRECTORY                  The directory cannot be removed. */
    EXDEV,          /* 17 (0x11)      ERROR_NOT_SAME_DEVICE                    The system cannot move the file to a different disk drive. */
    ENOENT,         /* 18 (0x12)      ERROR_NO_MORE_FILES                      There are no more files. */
    EROFS,          /* 19 (0x13)      ERROR_WRITE_PROTECT                      The media is write protected. */
    ENXIO,          /* 20 (0x14)      ERROR_BAD_UNIT                           The system cannot find the device specified. */
    EAGAIN,         /* 21 (0x15)      ERROR_NOT_READY                          The device is not ready. */
    -1,             /* 22 (0x16)      ERROR_BAD_COMMAND                        The device does not recognize the command. */
    -1,             /* 23 (0x17)      ERROR_CRC                                Data error (cyclic redundancy check). */
    E2BIG,          /* 24 (0x18)      ERROR_BAD_LENGTH                         The program issued a command but the command length is incorrect. */
    -1,             /* 25 (0x19)      ERROR_SEEK                               The drive cannot locate a specific area or track on the disk. */
    ENODEV,         /* 26 (0x1A)      ERROR_NOT_DOS_DISK                       The specified disk or diskette cannot be accessed. */
    -1,             /* 27 (0x1B)      ERROR_SECTOR_NOT_FOUND                   The drive cannot find the sector requested. */
    ENOSPC,         /* 28 (0x1C)      ERROR_OUT_OF_PAPER                       The printer is out of paper. */
    -1,             /* 29 (0x1D)      ERROR_WRITE_FAULT                        The system cannot write to the specified device. */
    -1,             /* 30 (0x1E)      ERROR_READ_FAULT                         The system cannot read from the specified device. */
    -1,             /* 31 (0x1F)      ERROR_GEN_FAILURE                        A device attached to the system is not functioning. */
    EACCES,         /* 32 (0x20)      ERROR_SHARING_VIOLATION                  The process cannot access the file because it is being used by another process. */
    EACCES,         /* 33 (0x21)      ERROR_LOCK_VIOLATION                     The process cannot access the file because another process has locked a portion of the file. */
    -1,             /* 34 (0x22)      ERROR_WRONG_DISK                         The wrong diskette is in the drive. Insert %2 (Volume Serial Number: %3) into drive %1. */
    -1,             /* 35             N/A */
    EACCES,         /* 36 (0x24)      ERROR_SHARING_BUFFER_EXCEEDED            Too many files opened for sharing. */
    -1,             /* 37             N/A */
    -1,             /* 38 (0x26)      ERROR_HANDLE_EOF                         Reached the end of the file. */
    ENOSPC,         /* 39 (0x27)      ERROR_HANDLE_DISK_FULL                   The disk is full. */
    -1,             /* 40             N/A */
    -1,             /* 41             N/A */
    -1,             /* 42             N/A */
    -1,             /* 43             N/A */
    -1,             /* 44             N/A */
    -1,             /* 45             N/A */
    -1,             /* 46             N/A */
    -1,             /* 47             N/A */
    -1,             /* 48             N/A */
    -1,             /* 49             N/A */
    ENOSYS,         /* 50 (0x32)      ERROR_NOT_SUPPORTED                      The request is not supported. */
    -1,             /* 51 (0x33)      ERROR_REM_NOT_LIST                       Windows cannot find the network path. Verify that the network path is correct and the destination computer is not busy or turned off. If Windows still cannot find the network path, contact your network administrator. */
    -1,             /* 52 (0x34)      ERROR_DUP_NAME                           You were not connected because a duplicate name exists on the network. If joining a domain, go to System in Control Panel to change the computer name and try again. If joining a workgroup, choose another workgroup name. */
    -1,             /* 53 (0x35)      ERROR_BAD_NETPATH                        The network path was not found. */
    -1,             /* 54 (0x36)      ERROR_NETWORK_BUSY                       The network is busy. */
    -1,             /* 55 (0x37)      ERROR_DEV_NOT_EXIST                      The specified network resource or device is no longer available. */
    -1,             /* 56 (0x38)      ERROR_TOO_MANY_CMDS                      The network BIOS command limit has been reached. */
    -1,             /* 57 (0x39)      ERROR_ADAP_HDW_ERR                       A network adapter hardware error occurred. */
    -1,             /* 58 (0x3A)      ERROR_BAD_NET_RESP                       The specified server cannot perform the requested operation. */
    -1,             /* 59 (0x3B)      ERROR_UNEXP_NET_ERR                      An unexpected network error occurred. */
    -1,             /* 60 (0x3C)      ERROR_BAD_REM_ADAP                       The remote adapter is not compatible. */
    -1,             /* 61 (0x3D)      ERROR_PRINTQ_FULL                        The printer queue is full. */
    ENOSPC,         /* 62 (0x3E)      ERROR_NO_SPOOL_SPACE                     Space to store the file waiting to be printed is not available on the server. */
    -1,             /* 63 (0x3F)      ERROR_PRINT_CANCELLED                    Your file waiting to be printed was deleted. */
    -1,             /* 64 (0x40)      ERROR_NETNAME_DELETED                    The specified network name is no longer available. */
    -1,             /* 65 (0x41)      ERROR_NETWORK_ACCESS_DENIED              Network access is denied. */
    -1,             /* 66 (0x42)      ERROR_BAD_DEV_TYPE                       The network resource type is not correct. */
    -1,             /* 67 (0x43)      ERROR_BAD_NET_NAME                       The network name cannot be found. */
    -1,             /* 68 (0x44)      ERROR_TOO_MANY_NAMES                     The name limit for the local computer network adapter card was exceeded. */
    -1,             /* 69 (0x45)      ERROR_TOO_MANY_SESS                      The network BIOS session limit was exceeded. */
    -1,             /* 70 (0x46)      ERROR_SHARING_PAUSED                     The remote server has been paused or is in the process of being started. */
    -1,             /* 71 (0x47)      ERROR_REQ_NOT_ACCEP                      No more connections can be made to this remote computer at this time because there are already as many connections as the computer can accept. */
    -1,             /* 72 (0x48)      ERROR_REDIR_PAUSED                       The specified printer or disk device has been paused. */
    -1,             /* 73             N/A */
    -1,             /* 74             N/A */
    -1,             /* 75             N/A */
    -1,             /* 76             N/A */
    -1,             /* 77             N/A */
    -1,             /* 78             N/A */
    -1,             /* 79             N/A */
    EEXIST,         /* 80 (0x50)      ERROR_FILE_EXISTS                        The file exists. */
    -1,             /* 81             N/A */
    ENOENT,         /* 82 (0x52)      ERROR_CANNOT_MAKE                        The directory or file cannot be created. */
    -1,             /* 83 (0x53)      ERROR_FAIL_I24                           Fail on INT 24. */
    ENOMEM,         /* 84 (0x54)      ERROR_OUT_OF_STRUCTURES                  Storage to process this request is not available. */
    EEXIST,         /* 85 (0x55)      ERROR_ALREADY_ASSIGNED                   The local device name is already in use. */
    EACCES,         /* 86 (0x56)      ERROR_INVALID_PASSWORD                   The specified network password is not correct. */
    EINVAL,         /* 87 (0x57)      ERROR_INVALID_PARAMETER                  The parameter is incorrect. */
    EFAULT,         /* 88 (0x58)      ERROR_NET_WRITE_FAULT                    A write fault occurred on the network. */
    EBUSY,          /* 89 (0x59)      ERROR_NO_PROC_SLOTS                      The system cannot start another process at this time. */
    -1,             /* 90             N/A */
    -1,             /* 91             N/A */
    -1,             /* 92             N/A */
    -1,             /* 93             N/A */
    -1,             /* 94             N/A */
    -1,             /* 95             N/A */
    -1,             /* 96             N/A */
    -1,             /* 97             N/A */
    -1,             /* 98             N/A */
    -1,             /* 99             N/A */
    -1,             /* 100 (0x64)     ERROR_TOO_MANY_SEMAPHORES                Cannot create another system semaphore. */
    EPERM,          /* 101 (0x65)     ERROR_EXCL_SEM_ALREADY_OWNED             The exclusive semaphore is owned by another process. */
    EACCES,         /* 102 (0x66)     ERROR_SEM_IS_SET                         The semaphore is set and cannot be closed. */
    EACCES,         /* 103 (0x67)     ERROR_TOO_MANY_SEM_REQUESTS              The semaphore cannot be set again. */
    -1,             /* 104 (0x68)     ERROR_INVALID_AT_INTERRUPT_TIME          Cannot request exclusive semaphores at interrupt time. */
    -1,             /* 105 (0x69)     ERROR_SEM_OWNER_DIED                     The previous ownership of this semaphore has ended. */
    -1,             /* 106 (0x6A)     ERROR_SEM_USER_LIMIT                     Insert the diskette for drive %1. */
    -1,             /* 107 (0x6B)     ERROR_DISK_CHANGE                        The program stopped because an alternate diskette was not inserted. */
    EACCES,         /* 108 (0x6C)     ERROR_DRIVE_LOCKED                       The disk is in use or locked by another process. */
    EPIPE,          /* 109 (0x6D)     ERROR_BROKEN_PIPE                        The pipe has been ended. */
    ENOENT,         /* 110 (0x6E)     ERROR_OPEN_FAILED                        The system cannot open the device or file specified. */
    ENAMETOOLONG,   /* 111 (0x6F)     ERROR_BUFFER_OVERFLOW                    The file name is too long. */
    ENOSPC,         /* 112 (0x70)     ERROR_DISK_FULL                          There is not enough space on the disk. */
    EMFILE,         /* 113 (0x71)     ERROR_NO_MORE_SEARCH_HANDLES             No more internal file identifiers available. */
    EBADF,          /* 114 (0x72)     ERROR_INVALID_TARGET_HANDLE              The target internal file identifier is incorrect. */
    -1,             /* 115            N/A */
    -1,             /* 116            N/A */
    -1,             /* 117 (0x75)     ERROR_INVALID_CATEGORY                   The IOCTL call made by the application program is not correct. */
    -1,             /* 118 (0x76)     ERROR_INVALID_VERIFY_SWITCH              The verify-on-write switch parameter value is not correct. */
    -1,             /* 119 (0x77)     ERROR_BAD_DRIVER_LEVEL                   The system does not support the command requested. */
    ENOSYS,         /* 120 (0x78)     ERROR_CALL_NOT_IMPLEMENTED               This function is not supported on this system. */
    -1,             /* 121 (0x79)     ERROR_SEM_TIMEOUT                        The semaphore timeout period has expired. */
    EINVAL,         /* 122 (0x7A)     ERROR_INSUFFICIENT_BUFFER                The data area passed to a system call is too small. */
    ENOENT,         /* 123 (0x7B)     ERROR_INVALID_NAME                       The filename, directory name, or volume label syntax is incorrect. */
    EINVAL,         /* 124 (0x7C)     ERROR_INVALID_LEVEL                      The system call level is not correct. */
    -1,             /* 125 (0x7D)     ERROR_NO_VOLUME_LABEL                    The disk has no volume label. */
    ENOSYS,         /* 126 (0x7E)     ERROR_MOD_NOT_FOUND                      The specified module could not be found. */
    ENOSYS,         /* 127 (0x7F)     ERROR_PROC_NOT_FOUND                     The specified procedure could not be found. */
    ECHILD,         /* 128 (0x80)     ERROR_WAIT_NO_CHILDREN                   There are no child processes to wait for. */
    ECHILD,         /* 129 (0x81)     ERROR_CHILD_NOT_COMPLETE                 The %1 application cannot be run in Win32 mode. */
    -1,             /* 130 (0x82)     ERROR_DIRECT_ACCESS_HANDLE               Attempt to use a file handle to an open disk partition for an operation other than raw disk I/O. */
    ESPIPE,         /* 131 (0x83)     ERROR_NEGATIVE_SEEK                      An attempt was made to move the file pointer before the beginning of the file. */
    ESPIPE,         /* 132 (0x84)     ERROR_SEEK_ON_DEVICE                     The file pointer cannot be set on the specified device or file. */
    ENOENT,         /* 133 (0x85)     ERROR_IS_JOIN_TARGET                     A JOIN or SUBST command cannot be used for a drive that contains previously joined drives. */
    ENOENT,         /* 134 (0x86)     ERROR_IS_JOINED                          An attempt was made to use a JOIN or SUBST command on a drive that has already been joined. */
    ENOENT,         /* 135 (0x87)     ERROR_IS_SUBSTED                         An attempt was made to use a JOIN or SUBST command on a drive that has already been substituted. */
    ENOENT,         /* 136 (0x88)     ERROR_NOT_JOINED                         The system tried to delete the JOIN of a drive that is not joined. */
    ENOENT,         /* 137 (0x89)     ERROR_NOT_SUBSTED                        The system tried to delete the substitution of a drive that is not substituted. */
    ENOENT,         /* 138 (0x8A)     ERROR_JOIN_TO_JOIN                       The system tried to join a drive to a directory on a joined drive. */
    ENOENT,         /* 139 (0x8B)     ERROR_SUBST_TO_SUBST                     The system tried to substitute a drive to a directory on a substituted drive. */
    ENOENT,         /* 140 (0x8C)     ERROR_JOIN_TO_SUBST                      The system tried to join a drive to a directory on a substituted drive. */
    ENOENT,         /* 141 (0x8D)     ERROR_SUBST_TO_JOIN                      The system tried to SUBST a drive to a directory on a joined drive. */
    EBUSY,          /* 142 (0x8E)     ERROR_BUSY_DRIVE                         The system cannot perform a JOIN or SUBST at this time. */
    ENOENT,         /* 143 (0x8F)     ERROR_SAME_DRIVE                         The system cannot join or substitute a drive to or for a directory on the same drive. */
    ENOENT,         /* 144 (0x90)     ERROR_DIR_NOT_ROOT                       The directory is not a subdirectory of the root directory. */
    EBUSY,          /* 145 (0x91)     ERROR_DIR_NOT_EMPTY                      The directory is not empty. */
    ENOENT,         /* 146 (0x92)     ERROR_IS_SUBST_PATH                      The path specified is being used in a substitute. */
    ENOENT,         /* 147 (0x93)     ERROR_IS_JOIN_PATH                       Not enough resources are available to process this command. */
    EBUSY,          /* 148 (0x94)     ERROR_PATH_BUSY                          The path specified cannot be used at this time. */
    ENOENT,         /* 149 (0x95)     ERROR_IS_SUBST_TARGET                    An attempt was made to join or substitute a drive for which a directory on the drive is the target of a previous substitute. */
    ENOEXEC,        /* 150 (0x96)     ERROR_SYSTEM_TRACE                       System trace information was not specified in your CONFIG.SYS file, or tracing is disallowed. */
    -1,             /* 151 (0x97)     ERROR_INVALID_EVENT_COUNT                The number of specified semaphore events for DosMuxSemWait is not correct. */
    -1,             /* 152 (0x98)     ERROR_TOO_MANY_MUXWAITERS                DosMuxSemWait did not execute; too many semaphores are already set. */
    -1,             /* 153 (0x99)     ERROR_INVALID_LIST_FORMAT                The DosMuxSemWait list is not correct. */
    -1,             /* 154 (0x9A)     ERROR_LABEL_TOO_LONG                     The volume label you entered exceeds the label character limit of the target file system. */
    ENOMEM,         /* 155 (0x9B)     ERROR_TOO_MANY_TCBS                      Cannot create another thread. */
    -1,             /* 156 (0x9C)     ERROR_SIGNAL_REFUSED                     The recipient process has refused the signal. */
    -1,             /* 157 (0x9D)     ERROR_DISCARDED                          The segment is already discarded and cannot be locked. */
    -1,             /* 158 (0x9E)     ERROR_NOT_LOCKED                         The segment is already unlocked. */
    EFAULT,         /* 159 (0x9F)     ERROR_BAD_THREADID_ADDR                  The address for the thread ID is not correct. */
    ENOEXEC,        /* 160 (0xA0)     ERROR_BAD_ARGUMENTS                      One or more arguments are not correct. */
    ENOENT,         /* 161 (0xA1)     ERROR_BAD_PATHNAME                       The specified path is invalid. */
    -1,             /* 162 (0xA2)     ERROR_SIGNAL_PENDING                     A signal is already pending. */
    -1,             /* 163            N/A */
    ENOMEM,         /* 164 (0xA4)     ERROR_MAX_THRDS_REACHED                  No more threads can be created in the system. */
    -1,             /* 165            N/A */
    -1,             /* 166            N/A */
    -1,             /* 167 (0xA7)     ERROR_LOCK_FAILED                        Unable to lock a region of a file. */
    -1,             /* 168            N/A */
    -1,             /* 169            N/A */
    EBUSY,          /* 170 (0xAA)     ERROR_BUSY                               The requested resource is in use. */
    -1,             /* 171 (0xAB)     ERROR_DEVICE_SUPPORT_IN_PROGRESS         Device's command support detection is in progress. */
    -1,             /* 172            N/A */
    EACCES,         /* 173 (0xAD)     ERROR_CANCEL_VIOLATION                   A lock request was not outstanding for the supplied cancel region. */
    ENOLCK,         /* 174 (0xAE)     ERROR_ATOMIC_LOCKS_NOT_SUPPORTED         The file system does not support atomic changes to the lock type. */
    -1,             /* 175            N/A */
    -1,             /* 176            N/A */
    -1,             /* 177            N/A */
    -1,             /* 178            N/A */
    -1,             /* 179            N/A */
    EFAULT,         /* 180 (0xB4)     ERROR_INVALID_SEGMENT_NUMBER             The system detected a segment number that was not correct. */
    -1,             /* 181            N/A */
    ENOEXEC,        /* 182 (0xB6)     ERROR_INVALID_ORDINAL                    The operating system cannot run %1. */
    EEXIST,         /* 183 (0xB7)     ERROR_ALREADY_EXISTS                     Cannot create a file when that file already exists. */
    -1,             /* 184            N/A */
    -1,             /* 185            N/A */
    EINVAL,         /* 186 (0xBA)     ERROR_INVALID_FLAG_NUMBER                The flag passed is not correct. */
    EINVAL,         /* 187 (0xBB)     ERROR_SEM_NOT_FOUND                      The specified system semaphore name was not found. */
    ENOEXEC,        /* 188 (0xBC)     ERROR_INVALID_STARTING_CODESEG           The operating system cannot run %1. */
    ENOEXEC,        /* 189 (0xBD)     ERROR_INVALID_STACKSEG                   The operating system cannot run %1. */
    ENOEXEC,        /* 190 (0xBE)     ERROR_INVALID_MODULETYPE                 The operating system cannot run %1. */
    ENOEXEC,        /* 191 (0xBF)     ERROR_INVALID_EXE_SIGNATURE              Cannot run %1 in Win32 mode. */
    ENOEXEC,        /* 192 (0xC0)     ERROR_EXE_MARKED_INVALID                 The operating system cannot run %1. */
    ENOEXEC,        /* 193 (0xC1)     ERROR_BAD_EXE_FORMAT                     %1 is not a valid Win32 application. */
    ENOEXEC,        /* 194 (0xC2)     ERROR_ITERATED_DATA_EXCEEDS_64k          The operating system cannot run %1. */
    ENOEXEC,        /* 195 (0xC3)     ERROR_INVALID_MINALLOCSIZE               The operating system cannot run %1. */
    ENOEXEC,        /* 196 (0xC4)     ERROR_DYNLINK_FROM_INVALID_RING          The operating system cannot run this application program. */
    ENOEXEC,        /* 197 (0xC5)     ERROR_IOPL_NOT_ENABLED                   The operating system is not presently configured to run this application. */
    ENOEXEC,        /* 198 (0xC6)     ERROR_INVALID_SEGDPL                     The operating system cannot run %1. */
    ENOEXEC,        /* 199 (0xC7)     ERROR_AUTODATASEG_EXCEEDS_64k            The operating system cannot run this application program. */
    ENOEXEC,        /* 200 (0xC8)     ERROR_RING2SEG_MUST_BE_MOVABLE           The code segment cannot be greater than or equal to 64K. */
    ENOEXEC,        /* 201 (0xC9)     ERROR_RELOC_CHAIN_XEEDS_SEGLIM           The operating system cannot run %1. */
    ENOEXEC,        /* 202 (0xCA)     ERROR_INFLOOP_IN_RELOC_CHAIN             The operating system cannot run %1. */
    ENOEXEC,        /* 203 (0xCB)     ERROR_ENVVAR_NOT_FOUND                   The system could not find the environment option that was entered. */
    -1,             /* 204            N/A */
    ENOEXEC,        /* 205 (0xCD)     ERROR_NO_SIGNAL_SENT                     No process in the command subtree has a signal handler. */
    EINVAL,         /* 206 (0xCE)     ERROR_FILENAME_EXCED_RANGE               The filename or extension is too long. */
    ENOEXEC,        /* 207 (0xCF)     ERROR_RING2_STACK_IN_USE                 The ring 2 stack is in use. */
    EINVAL,         /* 208 (0xD0)     ERROR_META_EXPANSION_TOO_LONG            The global filename characters, * or ?, are entered incorrectly or too many global filename characters are specified. */
    -1,             /* 209 (0xD1)     ERROR_INVALID_SIGNAL_NUMBER              The signal being posted is not correct. */
    -1,             /* 210 (0xD2)     ERROR_THREAD_1_INACTIVE                  The signal handler cannot be set. */
    -1,             /* 211            N/A */
    -1,             /* 212 (0xD4)     ERROR_LOCKED                             The segment is locked and cannot be reallocated. */
    -1,             /* 213            N/A */
    ENOEXEC,        /* 214 (0xD6)     ERROR_TOO_MANY_MODULES                   Too many dynamic-link modules are attached to this program or dynamic-link module. */
    ENOEXEC,        /* 215 (0xD7)     ERROR_NESTING_NOT_ALLOWED                Cannot nest calls to LoadModule. */
    ENOEXEC,        /* 216 (0xD8)     ERROR_EXE_MACHINE_TYPE_MISMATCH          This version of %1 is not compatible with the version of Windows you're running. Check your computer's system information and then contact the software publisher. */
    EACCES,         /* 217 (0xD9)     ERROR_EXE_CANNOT_MODIFY_SIGNED_BINARY    The image file %1 is signed, unable to modify. */
    EACCES,         /* 218 (0xDA)     ERROR_EXE_CANNOT_MODIFY_STRONG_SIGNED_BINARY The image file %1 is strong signed, unable to modify. */
    -1,             /* 219            N/A */
    -1,             /* 220 (0xDC)     ERROR_FILE_CHECKED_OUT                   This file is checked out or locked for editing by another user. */
    -1,             /* 221 (0xDD)     ERROR_CHECKOUT_REQUIRED                  The file must be checked out before saving changes. */
    -1,             /* 222 (0xDE)     ERROR_BAD_FILE_TYPE                      The file type being saved or retrieved has been blocked. */
    ENOSPC,         /* 223 (0xDF)     ERROR_FILE_TOO_LARGE                     The file size exceeds the limit allowed and cannot be saved. */
    -1,             /* 224 (0xE0)     ERROR_FORMS_AUTH_REQUIRED                Access Denied. Before opening files in this location, you must first add the web site to your trusted sites list, browse to the web site, and select the option to login automatically. */
    ENOEXEC,        /* 225 (0xE1)     ERROR_VIRUS_INFECTED                     Operation did not complete successfully because the file contains a virus or potentially unwanted software. */
    ENOEXEC,        /* 226 (0xE2)     ERROR_VIRUS_DELETED                      This file contains a virus or potentially unwanted software and cannot be opened. Due to the nature of this virus or potentially unwanted software, the file has been removed from this location. */
    -1,             /* 227            N/A */
    -1,             /* 228            N/A */
    EPIPE,          /* 229 (0xE5)     ERROR_PIPE_LOCAL                         The pipe is local. */
    EPIPE,          /* 230 (0xE6)     ERROR_BAD_PIPE                           The pipe state is invalid. */
    EBUSY,          /* 231 (0xE7)     ERROR_PIPE_BUSY                          All pipe instances are busy. */
    EPIPE,          /* 232 (0xE8)     ERROR_NO_DATA                            The pipe is being closed. */
    EPIPE,          /* 233 (0xE9)     ERROR_PIPE_NOT_CONNECTED                 No process is on the other end of the pipe. */
    EAGAIN,         /* 234 (0xEA)     ERROR_MORE_DATA                          More data is available. */
    -1,             /* 235            N/A */
    -1,             /* 236            N/A */
    -1,             /* 237            N/A */
    -1,             /* 238            N/A */
    -1,             /* 239            N/A */
    -1,             /* 240 (0xF0)     ERROR_VC_DISCONNECTED                    The session was canceled. */
    -1,             /* 241            N/A */
    -1,             /* 242            N/A */
    -1,             /* 243            N/A */
    -1,             /* 244            N/A */
    -1,             /* 245            N/A */
    -1,             /* 246            N/A */
    -1,             /* 247            N/A */
    -1,             /* 248            N/A */
    -1,             /* 249            N/A */
    -1,             /* 250            N/A */
    -1,             /* 251            N/A */
    -1,             /* 252            N/A */
    -1,             /* 253            N/A */
    EINVAL,         /* 254 (0xFE)     ERROR_INVALID_EA_NAME                    The specified extended attribute name was invalid. */
    -1,             /* 255 (0xFF)     ERROR_EA_LIST_INCONSISTENT               The extended attributes are inconsistent. */
    -1,             /* 256            N/A */
    -1,             /* 257            N/A */
    EAGAIN,         /* 258 (0x102)    WAIT_TIMEOUT                             The wait operation timed out. */
    -1,             /* 259 (0x103)    ERROR_NO_MORE_ITEMS                      No more data is available. */
    -1,             /* 260            N/A */
    -1,             /* 261            N/A */
    -1,             /* 262            N/A */
    -1,             /* 263            N/A */
    -1,             /* 264            N/A */
    -1,             /* 265            N/A */
    -1,             /* 266 (0x10A)    ERROR_CANNOT_COPY                        The copy functions cannot be used. */
    ENOTDIR,        /* 267 (0x10B)    ERROR_DIRECTORY                          The directory name is invalid. */
    -1,             /* 268            N/A */
    -1,             /* 269            N/A */
    -1,             /* 270            N/A */
    -1,             /* 271            N/A */
    -1,             /* 272            N/A */
    -1,             /* 273            N/A */
    -1,             /* 274            N/A */
    -1,             /* 275 (0x113)    ERROR_EAS_DIDNT_FIT                      The extended attributes did not fit in the buffer. */
    -1,             /* 276 (0x114)    ERROR_EA_FILE_CORRUPT                    The extended attribute file on the mounted file system is corrupt. */
    -1,             /* 277 (0x115)    ERROR_EA_TABLE_FULL                      The extended attribute table file is full. */
    EBADF,          /* 278 (0x116)    ERROR_INVALID_EA_HANDLE                  The specified extended attribute handle is invalid. */
    -1,             /* 279            N/A */
    -1,             /* 280            N/A */
    -1,             /* 281            N/A */
    ENOSYS,         /* 282 (0x11A)    ERROR_EAS_NOT_SUPPORTED                  The mounted file system does not support extended attributes. */
    -1,             /* 283            N/A */
    -1,             /* 284            N/A */
    -1,             /* 285            N/A */
    -1,             /* 286            N/A */
    -1,             /* 287            N/A */
    EPERM,          /* 288 (0x120)    ERROR_NOT_OWNER                          Attempt to release mutex not owned by caller. */
    -1,             /* 289            N/A */
    -1,             /* 290            N/A */
    -1,             /* 291            N/A */
    -1,             /* 292            N/A */
    -1,             /* 293            N/A */
    -1,             /* 294            N/A */
    -1,             /* 295            N/A */
    -1,             /* 296            N/A */
    -1,             /* 297            N/A */
    -1,             /* 298 (0x12A)    ERROR_TOO_MANY_POSTS                     Too many posts were made to a semaphore. */
    -1,             /* 299 (0x12B)    ERROR_PARTIAL_COPY                       Only part of a ReadProcessMemory or WriteProcessMemory request was completed. */
    EACCES,         /* 300 (0x12C)    ERROR_OPLOCK_NOT_GRANTED                 The oplock request is denied. */
    EINVAL,         /* 301 (0x12D)    ERROR_INVALID_OPLOCK_PROTOCOL            An invalid oplock acknowledgment was received by the system. */
    -1,             /* 302 (0x12E)    ERROR_DISK_TOO_FRAGMENTED                The volume is too fragmented to complete this operation. */
    EBUSY,          /* 303 (0x12F)    ERROR_DELETE_PENDING                     The file cannot be opened because it is in the process of being deleted. */
    -1,             /* 304 (0x130)    ERROR_INCOMPATIBLE_WITH_GLOBAL_SHORT_NAME_REGISTRY_SETTING - Short name settings may not be changed on this volume due to the global registry setting. */
    -1,             /* 305 (0x131)    ERROR_SHORT_NAMES_NOT_ENABLED_ON_VOLUME  Short names are not enabled on this volume. */
    -1,             /* 306 (0x132)    ERROR_SECURITY_STREAM_IS_INCONSISTENT    The security stream for the given volume is in an inconsistent state.Please run CHKDSK on the volume. */
    EINVAL,         /* 307 (0x133)    ERROR_INVALID_LOCK_RANGE                 A requested file lock operation cannot be processed due to an invalid byte range. */
    -1,             /* 308 (0x134)    ERROR_IMAGE_SUBSYSTEM_NOT_PRESENT        The subsystem needed to support the image type is not present. */
    -1,             /* 309 (0x135)    ERROR_NOTIFICATION_GUID_ALREADY_DEFINED  The specified file already has a notification GUID associated with it. */
    EINVAL,         /* 310 (0x136)    ERROR_INVALID_EXCEPTION_HANDLER          An invalid exception handler routine has been detected. */
    -1,             /* 311 (0x137)    ERROR_DUPLICATE_PRIVILEGES               Duplicate privileges were specified for the token. */
    ERANGE,         /* 312 (0x138)    ERROR_NO_RANGES_PROCESSED                No ranges for the specified operation were able to be processed. */
    EACCES,         /* 313 (0x139)    ERROR_NOT_ALLOWED_ON_SYSTEM_FILE         Operation is not allowed on a file system internal file. */
    ENOSPC,         /* 314 (0x13A)    ERROR_DISK_RESOURCES_EXHAUSTED           The physical resources of this disk have been exhausted. */
    EINVAL,         /* 315 (0x13B)    ERROR_INVALID_TOKEN                      The token representing the data is invalid. */
    ENOSYS,         /* 316 (0x13C)    ERROR_DEVICE_FEATURE_NOT_SUPPORTED       The device does not support the command feature. */
    -1,             /* 317 (0x13D)    ERROR_MR_MID_NOT_FOUND                   The system cannot find message text for message number 0x%1 in the message file for %2. */
    ENOENT,         /* 318 (0x13E)    ERROR_SCOPE_NOT_FOUND                    The scope specified was not found. */
    -1,             /* 319 (0x13F)    ERROR_UNDEFINED_SCOPE                    The Central Access Policy specified is not defined on the target machine. */
    EINVAL,         /* 320 (0x140)    ERROR_INVALID_CAP                        The Central Access Policy obtained from Active Directory is invalid. */
    ENXIO,          /* 321 (0x141)    ERROR_DEVICE_UNREACHABLE                 The device is unreachable. */
    -1,             /* 322 (0x142)    ERROR_DEVICE_NO_RESOURCES                The target device has insufficient resources to complete the operation. */
    -1,             /* 323 (0x143)    ERROR_DATA_CHECKSUM_ERROR                A data integrity checksum error occurred.Data in the file stream is corrupt. */
    -1,             /* 324 (0x144)    ERROR_INTERMIXED_KERNEL_EA_OPERATION     An attempt was made to modify both a KERNEL and normal Extended Attribute(EA) in the same operation. */
    ENOSYS,         /* 326 (0x146)    ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED      Device does not support file - level TRIM. */
    EFAULT,         /* 327 (0x147)    ERROR_OFFSET_ALIGNMENT_VIOLATION         The command specified a data offset that does not align to the device's granularity/alignment. */
    -1,             /* 328 (0x148)    ERROR_INVALID_FIELD_IN_PARAMETER_LIST    The command specified an invalid field in its parameter list. */
    EBUSY,          /* 329 (0x149)    ERROR_OPERATION_IN_PROGRESS              An operation is currently in progress with the device. */
    ENXIO,          /* 330 (0x14A)    ERROR_BAD_DEVICE_PATH                    An attempt was made to send down the command via an invalid path to the target device. */
    ENFILE,         /* 331 (0x14B)    ERROR_TOO_MANY_DESCRIPTORS               The command specified a number of descriptors that exceeded the maximum supported by the device. */
    -1,             /* 332 (0x14C)    ERROR_SCRUB_DATA_DISABLED                Scrub is disabled on the specified file. */
    ENOSYS,         /* 333 (0x14D)    ERROR_NOT_REDUNDANT_STORAGE              The storage device does not provide redundancy. */
    ENOSYS,         /* 334 (0x14E)    ERROR_RESIDENT_FILE_NOT_SUPPORTED        An operation is not supported on a resident file. */
    ENOSYS,         /* 335 (0x14F)    ERROR_COMPRESSED_FILE_NOT_SUPPORTED      An operation is not supported on a compressed file. */
    EISDIR,         /* 336 (0x150)    ERROR_DIRECTORY_NOT_SUPPORTED            An operation is not supported on a directory. */
    -1,             /* 337 (0x151)    ERROR_NOT_READ_FROM_COPY                 The specified copy of the requested data could not be read. */
#if (0)
    -1,             /* 350 (0x15E)    ERROR_FAIL_NOACTION_REBOOT               No action was taken as a system reboot is required. */
    -1,             /* 351 (0x15F)    ERROR_FAIL_SHUTDOWN                      The shutdown operation failed. */
    -1,             /* 352 (0x160)    ERROR_FAIL_RESTART                       The restart operation failed. */
    -1,             /* 353 (0x161)    ERROR_MAX_SESSIONS_REACHED               The maximum number of sessions has been reached. */
    -1,             /* 400 (0x190)    ERROR_THREAD_MODE_ALREADY_BACKGROUND     The thread is already in background processing mode. */
    -1,             /* 401 (0x191)    ERROR_THREAD_MODE_NOT_BACKGROUND         The thread is not in background processing mode. */
    -1,             /* 402 (0x192)    ERROR_PROCESS_MODE_ALREADY_BACKGROUND    The process is already in background processing mode. */
    -1,             /* 403 (0x193)    ERROR_PROCESS_MODE_NOT_BACKGROUND        The process is not in background processing mode. */
    -1,             /* 487 (0x1E7)    ERROR_INVALID_ADDRESS                    Attempt to access invalid address. */
#endif
    };


LIBW32_API int
w32_errno_cnv(unsigned rc)
{
    const unsigned maprange = (sizeof(xlaterrno) / sizeof(xlaterrno[0]));
    int t_errno = -1;

    if (rc < maprange) {
        t_errno = xlaterrno[rc];                /* map */
    }

    if (-1 == t_errno) {
        t_errno = EIO;                          /* default */
        if (rc >= maprange) {
            switch (rc) {
            case ERROR_INVALID_ADDRESS:         /* 487          - Attempt to access invalid address. */
                t_errno = EFAULT; break;
            case ERROR_ARITHMETIC_OVERFLOW:     /* 534          - Arithmetic result exceeded 32 bits. */
                t_errno = ERANGE; break;
            case ERROR_PIPE_CONNECTED:          /* 535          - There is a process on other end of the pipe. */
                t_errno = EPIPE; break;
            case ERROR_PIPE_LISTENING:          /* 536          - Waiting for a process to open the other end of the pipe. */
                t_errno = EPIPE; break;
#if /*defined(__MINGW32__) ||*/ !defined(ERROR_CANT_WAIT)
#define ERROR_CANT_WAIT 554
#endif
            case ERROR_CANT_WAIT:               /* 554 (0x22A)  - Used to indicate that an operation cannot continue without blocking for I/O. */
                t_errno = EAGAIN; break;
            case ERROR_OPERATION_ABORTED:       /* 995          - The I/O operation has been aborted because of either a thread exit or an application request. */
                t_errno = EINTR; break;
            case ERROR_IO_INCOMPLETE:           /* 996          - Overlapped I/O event is not in a signaled state. */
                t_errno = EINTR; break;
            case ERROR_NOACCESS:                /* 998          - Invalid access to memory location. */
                t_errno = EFAULT; break;
            case ERROR_SWAPERROR:               /* 999          - Error performing inpage operation. */
                t_errno = ENOENT; break;
            case ERROR_STACK_OVERFLOW:          /* 1001         - Recursion too deep; the stack overflowed. */
                t_errno = ENOMEM; break;
            case ERROR_INVALID_FLAGS:           /* 1004         - Invalid flags. */
                t_errno = EINVAL; break;
            case ERROR_UNRECOGNIZED_VOLUME:     /* 1005         - The volume does not contain a recognized file system. */
                t_errno = ENODEV; break;
            case ERROR_FILE_INVALID:            /* 1006         - The volume for a file has been externally altered so that the opened file is no longer valid. */
                t_errno = ENODEV; break;
            case ERROR_IO_DEVICE:               /* 1117         - The request could not be performed because of an I/O device error. */
                t_errno = EIO; break;
            case ERROR_MAPPED_ALIGNMENT:        /* 1132         - The base address or the file offset specified does not have the proper alignment. */
                t_errno = EINVAL; break;
            case ERROR_PRIVILEGE_NOT_HELD:      /* 1314 (0X522) - A required privilege is not held by the client. */
                t_errno = EPERM;  break;
            case ERROR_ACCOUNT_DISABLED:        /* 1331         - This user can't sign in because this account is currently disabled. */
                t_errno = EACCES; break;
            case ERROR_LOGON_FAILURE:           /* 1326         - The user name or password is incorrect. */
                t_errno = EACCES; break;
            case ERROR_ACCOUNT_RESTRICTION:     /* 1327         - Account restrictions are preventing this user from signing in. */
                t_errno = EACCES; break;
            case ERROR_UNRECOGNIZED_MEDIA:      /* 1785         - The disk media is not recognized. It may not be formatted. */
                t_errno = ENXIO;  break;
            case ERROR_BAD_DEVICE:              /* 1200         - The specified device name is invalid. */
                t_errno = ENODEV; break;
            case ERROR_INVALID_OWNER:           /* 1307         - This security ID may not be assigned as the owner of this object. */
                t_errno = EINVAL; break;
            case ERROR_INVALID_PRIMARY_GROUP:   /* 1308         - This security ID may not be assigned as the primary group of an object. */
                t_errno = EINVAL; break;
            case ERROR_NO_SUCH_PRIVILEGE:       /* 1313         - A specified privilege does not exist. */
                t_errno = EACCES; break;
            case ERROR_INVALID_LOGON_HOURS:     /* 1328         - Your account has time restrictions that keep you from signing in right now. */
                t_errno = EACCES; break;
            case ERROR_INVALID_WORKSTATION:     /* 1329         - This user isn't allowed to sign in to this computer. */
                t_errno = EACCES; break;
            case ERROR_PASSWORD_EXPIRED:        /* 1330         - The password for this account has expired. */
                t_errno = EACCES; break;
            case ERROR_NONE_MAPPED:             /* 1332         - No mapping between account names and security IDs was done. */
                t_errno = EINVAL; break;
            case ERROR_OPEN_FILES:              /* 2401         - This network connection has files open or requests pending. */
                t_errno = EBUSY; break;
            case ERROR_BAD_USERNAME:            /* 2202L        - The specified username is invalid. */
                t_errno = EINVAL; break;
            case ERROR_DEVICE_IN_USE:           /* 2404L        - The device is in use by an active process and cannot be disconnected. */
                t_errno = EBUSY; break;
            }
        }
    }
    return t_errno;
}


LIBW32_API int
w32_errno_setas(unsigned rc)
{
    errno = w32_errno_cnv(rc);
    return -1; //REVIEW/FIXME
}


LIBW32_API int
w32_errno_set(void)
{
    return w32_errno_setas(GetLastError());
}


LIBW32_API const char *
w32_strerror(int errnum)
{
    static char errbuffer[32];                  // TODO/TLS
    const char *err = NULL;

#undef strerror
    if (errnum >= 0 && errnum < _sys_nerr) {
        return strerror(errnum);
    }

    switch (errnum) {
    /* Standard library */
    case EPERM:             err = "Operation not permitted"; break;
    case ENOENT:            err = "No such file or directory"; break;
    case ESRCH:             err = "No such process"; break;
    case EINTR:             err = "Interrupted system call"; break;
    case EIO:               err = "Input/output error"; break;
    case ENXIO:             err = "No such device or address"; break;
    case E2BIG:             err = "Arg list too long"; break;
    case ENOEXEC:           err = "Exec format error"; break;
    case EBADF:             err = "Bad file descriptor"; break;
    case ECHILD:            err = "No children"; break;
    case EAGAIN:            err = "Resource temporarily unavailable"; break;
    case ENOMEM:            err = "Not enough memory"; break;
    case EACCES:            err = "Permission denied"; break;
    case EFAULT:            err = "Bad address"; break;
    case EBUSY:             err = "Device or resource busy"; break;
    case EEXIST:            err = "File exists"; break;
    case EXDEV:             err = "Cross-device link"; break;
    case ENODEV:            err = "No such device"; break;
    case ENOTDIR:           err = "Not a directory"; break;
    case EISDIR:            err = "Is a directory"; break;
    case EINVAL:            err = "Invalid argument"; break;
    case ENFILE:            err = "Too many open files in system"; break;
    case EMFILE:            err = "Too many open files"; break;
    case ENOTTY:            err = "Not a character device"; break;
    case EFBIG:             err = "File too large"; break;
    case ENOSPC:            err = "No space left on device"; break;
    case ESPIPE:            err = "Illegal seek"; break;
    case EROFS:             err = "Read-only file system"; break;
    case EMLINK:            err = "Too many links"; break;
    case EPIPE:             err = "Broken pipe"; break;
    case EDOM:              err = "Math arg out of domain of func"; break;
    case ERANGE:            err = "Result too large"; break;
    case EDEADLK:           err = "Deadlock condition"; break;
    case ENAMETOOLONG:      err = "File or path name too long"; break;
    case ENOSYS:            err = "Function not implemented"; break;
    case ENOLCK:            err = "No lock"; break;
    case ENOTEMPTY:         err = "Directory not empty"; break;
    case EILSEQ:            err = "Illegal byte sequence"; break;
#if defined(EDEADLOCK) && (EDEADLOCK != EDEADLK)
    case EDEADLOCK:         err = "Deadlock condition"; break;
#endif

    /* Socket */
    case WSAEINTR:          /*10004*/ err = "Interrupted system call"; break;
    case WSAEBADF:          /*10009*/ err = "Bad file number"; break;
    case WSAEACCES:         /*10013*/ err = "Permission denied"; break;
    case WSAEFAULT:         /*10014*/ err = "Bad address"; break;
    case WSAEINVAL:         /*10022*/ err = "Invalid argument"; break;
    case WSAEMFILE:         /*10024*/ err = "Too many open files"; break;
    case WSAEWOULDBLOCK:    /*10035*/ err = "Operation would block"; break;
    case WSAEINPROGRESS:    /*10036*/ err = "Operation now in progress"; break;
    case WSAEALREADY:       /*10037*/ err = "Operation already in progress"; break;
    case WSAENOTSOCK:       /*10038*/ err = "Socket operation on non-socket"; break;
    case WSAEDESTADDRREQ:   /*10039*/ err = "Destination address required"; break;
    case WSAEMSGSIZE:       /*10040*/ err = "Message too long"; break;
    case WSAEPROTOTYPE:     /*10041*/ err = "Protocol wrong type for socket"; break;
    case WSAENOPROTOOPT:    /*10042*/ err = "Bad protocol option"; break;
    case WSAEPROTONOSUPPORT:/*10043*/ err = "Protocol not supported"; break;
    case WSAESOCKTNOSUPPORT:/*10044*/ err = "Socket type not supported"; break;
    case WSAEOPNOTSUPP:     /*10045*/ err = "Operation not supported on socket"; break;
    case WSAEPFNOSUPPORT:   /*10046*/ err = "Protocol family not supported"; break;
    case WSAEAFNOSUPPORT:   /*10047*/ err = "Address family not supported by protocol family"; break;
    case WSAEADDRINUSE:     /*10048*/ err = "Address already in use"; break;
    case WSAEADDRNOTAVAIL:  /*10049*/ err = "Can't assign requested address"; break;
    case WSAENETDOWN:       /*10050*/ err = "Network is down"; break;
    case WSAENETUNREACH:    /*10051*/ err = "Network is unreachable"; break;
    case WSAENETRESET:      /*10052*/ err = "Net dropped connection or reset"; break;
    case WSAECONNABORTED:   /*10053*/ err = "Software caused connection abort"; break;
    case WSAECONNRESET:     /*10054*/ err = "Connection reset by peer"; break;
    case WSAENOBUFS:        /*10055*/ err = "No buffer space available"; break;
    case WSAEISCONN:        /*10056*/ err = "Socket is already connected"; break;
    case WSAENOTCONN:       /*10057*/ err = "Socket is not connected"; break;
    case WSAESHUTDOWN:      /*10058*/ err = "Can't send after socket shutdown"; break;
    case WSAETOOMANYREFS:   /*10059*/ err = "Too many references, can't splice"; break;
    case WSAETIMEDOUT:      /*10060*/ err = "Connection timed out"; break;
    case WSAECONNREFUSED:   /*10061*/ err = "Connection refused"; break;
    case WSAELOOP:          /*10062*/ err = "Too many levels of symbolic links"; break;
#if (ENAMETOOLONG != WSAENAMETOOLON)
    case WSAENAMETOOLONG:   /*10063*/ err = "File name too long"; break;
#endif
    case WSAEHOSTDOWN:      /*10064*/ err = "Host is down"; break;
    case WSAEHOSTUNREACH:   /*10065*/ err = "No Route to Host"; break;
#if (ENOTEMPTY != WSAENOTEMPTY)
    case WSAENOTEMPTY:      /*10066*/ err = "Directory not empty"; break;
#endif
    case WSAEPROCLIM:       /*10067*/ err = "Too many processes"; break;
    case WSAEUSERS:         /*10068*/ err = "Too many users"; break;
    case WSAEDQUOT:         /*10069*/ err = "Disc Quota Exceeded"; break;
    case WSAESTALE:         /*10070*/ err = "Stale NFS file handle"; break;
    case WSASYSNOTREADY:    /*10091*/ err = "Network SubSystem is unavailable"; break;
    case WSAVERNOTSUPPORTED:/*10092*/ err = "WINSOCK DLL Version out of range"; break;
    case WSANOTINITIALISED: /*10093*/ err = "Successful WSASTARTUP not yet performed"; break;
    case WSAEREMOTE:        /*10071*/ err = "Too many levels of remote in path"; break;
    case WSAHOST_NOT_FOUND: /*11001*/ err = "Host not found"; break;
    case WSATRY_AGAIN:      /*11002*/ err = "Non-Authoritative Host not found"; break;
    case WSANO_RECOVERY:    /*11003*/ err = "Non-Recoverable errors"; break;
    case WSANO_DATA:        /*11004*/ err = "Valid name, no data record of requested type"; break;
#if (WSANO_DATA != WSANO_DATA)
    case WSANO_ADDRESS:     /*11004*/ err = "No address, look for MX record "; break;
#endif

    /* BSD/POSIX Socket Errors */
//  case EINTR:             err = "Interrupted system call"; break;
//  case EBADF:             err = "Bad file number"; break;
//  case EACCES:            err = "Permission denied"; break;
//  case EFAULT:            err = "Bad address"; break;
//  case EINVAL:            err = "Invalid argument"; break;
//  case EMFILE:            err = "Too many open files"; break;
#if (EWOULDBLOCK != WSAEWOULDBLOCK)
    case EWOULDBLOCK:       err = "Operation would block"; break;
#endif
#if (EINPROGRESS != WSAEINPROGRESS)
    case EINPROGRESS:       err = "Operation now in progress"; break;
#endif
#if (EALREADY != WSAEALREADY)
    case EALREADY:          err = "Operation already in progress"; break;
#endif
#if (ENOTSOCK != WSAENOTSOCK)
    case ENOTSOCK:          err = "Socket operation on non-socket"; break;
#endif
#if (EDESTADDRREQ != WSAEDESTADDRREQ)
    case EDESTADDRREQ:      err = "Destination address required"; break;
#endif
#if (EMSGSIZE != WSAEMSGSIZE)
    case EMSGSIZE:          err = "Message too long"; break;
#endif
#if (EPROTOTYPE != WSAEPROTOTYPE)
    case EPROTOTYPE:        err = "Protocol wrong type for socket"; break;
#endif
#if (ENOPROTOOPT != WSAENOPROTOOPT)
    case ENOPROTOOPT:       err = "Bad protocol option"; break;
#endif
#if (EPROTONOSUPPORT != WSAEPROTONOSUPPORT)
    case EPROTONOSUPPORT:   err = "Protocol not supported"; break;
#endif
#if (ESOCKTNOSUPPORT != WSAESOCKTNOSUPPORT)
    case ESOCKTNOSUPPORT:   err = "Socket type not supported"; break;
#endif
#if (EOPNOTSUPP != WSAEOPNOTSUPP)
    case EOPNOTSUPP:        err = "Operation not supported on socket"; break;
#endif
#if (EPFNOSUPPORT != WSAEPFNOSUPPORT)
    case EPFNOSUPPORT:      err = "Protocol family not supported"; break;
#endif
#if (EAFNOSUPPORT != WSAEAFNOSUPPORT)
    case EAFNOSUPPORT:      err = "Address family not supported by protocol family"; break;
#endif
#if (EADDRINUSE != WSAEADDRINUSE)
    case EADDRINUSE:        err = "Address already in use"; break;
#endif
#if (EADDRNOTAVAIL != WSAEADDRNOTAVAIL)
    case EADDRNOTAVAIL:     err = "Can't assign requested address"; break;
#endif
#if (ENETDOWN != WSAENETDOWN)
    case ENETDOWN:          err = "Network is down"; break;
#endif
#if (ENETUNREACH != WSAENETUNREACH)
    case ENETUNREACH:       err = "Network is unreachable"; break;
#endif
#if (ENETRESET != WSAENETRESET)
    case ENETRESET:         err = "Net dropped connection or reset"; break;
#endif
#if (ECONNABORTED != WSAECONNABORTED)
    case ECONNABORTED:      err = "Software caused connection abort"; break;
#endif
#if (ECONNRESET != WSAECONNRESET)
    case ECONNRESET:        err = "Connection reset by peer"; break;
#endif
#if (ENOBUFS != WSAENOBUFS)
    case ENOBUFS:           err = "No buffer space available"; break;
#endif
#if (EISCONN != WSAEISCONN)
    case EISCONN:           err = "Socket is already connected"; break;
#endif
#if (ENOTCONN != WSAENOTCONN)
    case ENOTCONN:          err = "Socket is not connected"; break;
#endif
#if (ESHUTDOWN != WSAESHUTDOWN)
    case ESHUTDOWN:         err = "Can't send after socket shutdown"; break;
#endif
#if (ETOOMANYREFS != WSAETOOMANYREFS)
    case ETOOMANYREFS:      err = "Too many references, can't splice"; break;
#endif
#if (ETIMEDOUT != WSAETIMEDOUT)
    case ETIMEDOUT:         err = "Connection timed out"; break;
#endif
#if (ECONNREFUSED != WSAECONNREFUSED)
    case ECONNREFUSED:      err = "Connection refused"; break;
#endif
#if (ELOOP != WSAELOOP)
    case ELOOP:             err = "Too many levels of symbolic links"; break;
#endif
//#if (ENAMETOOLONG != WSAENAMETOOLONG)
//  case ENAMETOOLONG:      err = "File name too long"; break;
//#endif
#if (EHOSTDOWN != WSAEHOSTDOWN)
    case EHOSTDOWN:         err = "Host is down"; break;
#endif
#if (EHOSTUNREACH != WSAEHOSTUNREACH)
    case EHOSTUNREACH:      err = "No Route to Host"; break;
#endif
//#if (ENOTEMPTY != WSAENOTEMPTY)
//  case ENOTEMPTY:         err = "Directory not empty"; break;
//#endif
#if (EPROCLIM != WSAEPROCLIM)
    case EPROCLIM:          err = "Too many processes"; break;
#endif
#if (EUSERS != WSAEUSERS)
    case EUSERS:            err = "Too many users"; break;
#endif
#if (EDQUOT != WSAEDQUOT)
    case EDQUOT:            err = "Disc Quota Exceeded"; break;
#endif
#if (ESTALE != WSAESTALE)
    case ESTALE:            err = "Stale NFS file handle"; break;
#endif
#if (ENOTINITIALISED != WSANOTINITIALISED)
    case ENOTINITIALISED:   err = "Successful WSASTARTUP not yet performed"; break;
#endif

    /* POSIX */
//  case EADDRINUSE:        /*100*/ err = "Address already in use"; break;
//  case EADDRNOTAVAIL:     /*101*/ err = "EADDRNOTAVAIL"; break;
//  case EAFNOSUPPORT:      /*102*/ err = "Family cannot be used with this socket"; break;
//  case EALREADY:          /*103*/ err = "Already"; break;
    case EBADMSG:           /*104*/ err = "Bad message"; break;
    case ECANCELED:         /*105*/ err = "Operation cancelled"; break;
//  case ECONNABORTED:      /*106*/ err = "The connection was aborted"; break;
//  case ECONNREFUSED:      /*107*/ err = "Connection refused"; break;
//  case ECONNRESET:        /*108*/ err = "Connection reset by peer"; break;
//  case EDESTADDRREQ:      /*109*/ err = "EDESTADDRREQ"; break;
//  case EHOSTUNREACH:      /*110*/ err = "Host unreachable"; break;
    case EIDRM:             /*111*/ err = "Identifier removed"; break;
//  case EINPROGRESS:       /*112*/ err = "In progress"; break;
//  case EISCONN:           /*113*/ err = "Socket is already connected"; break;
//  case ELOOP:             /*114*/ err = "Too many symbolic links"; break;
//  case EMSGSIZE:          /*115*/ err = "Message size"; break;
//  case ENETDOWN:          /*116*/ err = "Network down"; break;
//  case ENETRESET:         /*117*/ err = "Network reset"; break;
//  case ENETUNREACH:       /*118*/ err = "The network can't be reached"; break;
//  case ENOBUFS:           /*119*/ err = "No buffer space availabled"; break;
    case ENODATA:           /*120*/ err = "No data (for no delay io)"; break;
    case ENOLINK:           /*121*/ err = "Virtual circuit is gone"; break;
    case ENOMSG:            /*122*/ err = "No message"; break;
//  case ENOPROTOOPT:       /*123*/ err = "This option is unsupported"; break;
    case ENOSR:             /*124*/ err = "No stream resources"; break;
    case ENOSTR:            /*125*/ err = "Not a stream"; break;
//  case ENOTCONN:          /*126*/ err = "Socket is not connected"; break;
//  case ENOTRECOVERABLE:   /*127*/ err = "Not recoverable"; break;
//  case ENOTSOCK:          /*128*/ err = "Not a socket"; break;
    case ENOTSUP:           /*129*/ err = "Not supported"; break;
//  case EOPNOTSUPP:        /*130*/ err = "Operation not supported on transport"; break;
    case EOTHER:            /*131*/ err = "Other error"; break;
    case EOVERFLOW:         /*132*/ err = "Overflow"; break;
    case EOWNERDEAD:        /*133*/ err = "Owner dead"; break;
    case EPROTO:            /*134*/ err = "Protocol error"; break;
//  case EPROTONOSUPPORT:   /*135*/ err = "Protocol not supported"; break;
//  case EPROTOTYPE:        /*136*/ err = "Protype error"; break;
    case ETIME:             /*137*/ err = "Stream ioctl timeout"; break;
//  case ETIMEDOUT:         /*138*/ err = "Connection timed out"; break;
#if defined(ETXTBSY)
    case ETXTBSY:           /*139*/ err = "Text file busy"; break;
#endif
//  case EWOULDBLOCK:       /*140*/ err = "Operation would block"; break;

    /* BSD/SysV messages */
#if defined(ENOTBLK)
    case ENOTBLK:           err = "Block device required"; break;
#endif
#if defined(EUCLEAN)
    case EUCLEAN:           err = "UCLEAN"; break;
#endif
#if defined(ECHRNG)
    case ECHRNG:            err = "Channel number out of range"; break;
#endif
#if defined(EL2NSYNC)
    case EL2NSYNC:          err = "Level 2 not synchronized"; break;
#endif
#if defined(EL3HLT)
    case EL3HLT:            err = "Level 3 halted"; break;
#endif
#if defined(EL3RST)
    case EL3RST:            err = "Level 3 reset"; break;
#endif
#if defined(ELNRNG)
    case ELNRNG:            err = "Link number out of range"; break;
#endif
#if defined(EUNATCH)
    case EUNATCH:           err = "Protocol driver not attached"; break;
#endif
#if defined(ENOCSI)
    case ENOCSI:            err = "No CSI structure available"; break;
#endif
#if defined(EL2HLT)
    case EL2HLT:            err = "Level 2 halted"; break;
#endif
#if defined(EBADE)
    case EBADE:             err = "Invalid exchange"; break;
#endif
#if defined(EBADR)
    case EBADR:             err = "Invalid request descriptor"; break;
#endif
#if defined(EXFULL)
    case EXFULL:            err = "Exchange full"; break;
#endif
#if defined(ENOANO)
    case ENOANO:            err = "No anode"; break;
#endif
#if defined(EBADRQC)
    case EBADRQC:           err = "Invalid request code"; break;
#endif
#if defined(EBADSLT)
    case EBADSLT:           err = "Invalid slot"; break;
#endif
#if defined(EBFONT)
    case EBFONT:            err = "Bad font file fmt"; break;
#endif
#if defined(ENOPKG)
    case ENOPKG:            err = "No package"; break;
#endif
#if defined(EADV)
    case EADV:              err = "Advertise error"; break;
#endif
#if defined(ESRMNT)
    case ESRMNT:            err = "Srmount error"; break;
#endif
#if defined(ECOMM)
    case ECOMM:             err = "Communication error"; break;
#endif
#if defined(EMULTIHOP)
    case EMULTIHOP:         err = "Multihop attempted"; break;
#endif
#if defined(ELBIN)
    case ELBIN:             err = "Inode is remote"; break;
#endif
#if defined(EDOTDOT)
    case EDOTDOT:           err = "Cross mount point"; break;
#endif
#if defined(ENOTUNIQ)
    case ENOTUNIQ:          err = "Name not unique"; break;
#endif
#if defined(EBADFD)
    case EBADFD:            err = "Invalid file descriptor"; break;
#endif
#if defined(EREMCHG)
    case EREMCHG:           err = "Remote address changed"; break;
#endif
#if defined(ENMFILE)
    case ENMFILE:           err = "No more files"; break;
#endif
#if defined(ENOMEDIUM)
    case ENOMEDIUM:         err = "No medium"; break;
#endif
#if defined(ENOSHARE)
    case ENOSHARE:          err = "No such host or network path"; break;
#endif

    default:
        _snprintf(errbuffer, sizeof(errbuffer), "%s [%d]",
             (errnum >= WSABASEERR ? "unknown winsock error" : "unknown error"), errnum);
        err = errbuffer;
        break;
    }

#if defined(_MSC_VER)
    {
        /*
         *  Dont look, but overwrite the internal strerr() buffer which, note
         *  the max length of message = user_string(94)+system_string+2
         */
        char *err2 = strerror(EINVAL);
        (void)strcpy(err2, (const char *)err);
        err = err2;
    }
#endif
    return (err);
}

/*end*/
