/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 errno mapping support
 *
 * Copyright (c) 2007, 2012 - 2015 Adam Young.
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

#include "win32_internal.h"
#include <unistd.h>
#include <time.h>

/*
 *  Mapping for the first 255 system error messages (ie. base error codes),
 *  with all others are mapped to EIO.
 *
 *  Note, the follow represent a 'general' error code mapping, logic may require
 *  explicit mapping when error conditions are being replied upon.
 */

static const int    xlat256[256] = {
 /* errno values    Win32 error values */
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
    -1,             /* 35 N/A */
    EACCES,         /* 36 (0x24)      ERROR_SHARING_BUFFER_EXCEEDED            Too many files opened for sharing. */
    -1,             /* 37 N/A */
    -1,             /* 38 (0x26)      ERROR_HANDLE_EOF                         Reached the end of the file. */
    ENOSPC,         /* 39 (0x27)      ERROR_HANDLE_DISK_FULL                   The disk is full. */
    -1,             /* 40 N/A */
    -1,             /* 41 N/A */
    -1,             /* 42 N/A */
    -1,             /* 43 N/A */
    -1,             /* 44 N/A */
    -1,             /* 45 N/A */
    -1,             /* 46 N/A */
    -1,             /* 47 N/A */
    -1,             /* 48 N/A */
    -1,             /* 49 N/A */
    -1,             /* 50 (0x32)      ERROR_NOT_SUPPORTED                      The request is not supported. */
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
    -1,             /* 73 N/A */
    -1,             /* 74 N/A */
    -1,             /* 75 N/A */
    -1,             /* 76 N/A */
    -1,             /* 77 N/A */
    -1,             /* 78 N/A */
    -1,             /* 79 N/A */
    EEXIST,         /* 80 (0x50)      ERROR_FILE_EXISTS                        The file exists. */
    -1,             /* 81 N/A */
    ENOENT,         /* 82 (0x52)      ERROR_CANNOT_MAKE                        The directory or file cannot be created. */
    -1,             /* 83 (0x53)      ERROR_FAIL_I24                           Fail on INT 24. */
    ENOMEM,         /* 84 (0x54)      ERROR_OUT_OF_STRUCTURES                  Storage to process this request is not available. */
    EEXIST,         /* 85 (0x55)      ERROR_ALREADY_ASSIGNED                   The local device name is already in use. */
    EACCES,         /* 86 (0x56)      ERROR_INVALID_PASSWORD                   The specified network password is not correct. */
    EINVAL,         /* 87 (0x57)      ERROR_INVALID_PARAMETER                  The parameter is incorrect. */
    EFAULT,         /* 88 (0x58)      ERROR_NET_WRITE_FAULT                    A write fault occurred on the network. */
    EBUSY,          /* 89 (0x59)      ERROR_NO_PROC_SLOTS                      The system cannot start another process at this time. */
    -1,             /* 90 N/A */
    -1,             /* 91 N/A */
    -1,             /* 92 N/A */
    -1,             /* 93 N/A */
    -1,             /* 94 N/A */
    -1,             /* 95 N/A */
    -1,             /* 96 N/A */
    -1,             /* 97 N/A */
    -1,             /* 98 N/A */
    -1,             /* 99 N/A */
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
    -1,             /* 115 N/A */
    -1,             /* 116 N/A */
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
    -1,             /* 223 (0xDF)     ERROR_FILE_TOO_LARGE                     The file size exceeds the limit allowed and cannot be saved. */
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
    -1,             /* 254 (0xFE)     ERROR_INVALID_EA_NAME                    The specified extended attribute name was invalid. */
    -1              /* 255 (0xFF)     ERROR_EA_LIST_INCONSISTENT               The extended attributes are inconsistent. */
    };


int
w32_errno_cnv(unsigned rc)
{
    int t_errno;

    if (rc > 255) rc = EIO;
    if (-1 == (t_errno = xlat256[rc])) {
        t_errno = EIO;
    }
    return t_errno;
}


int
w32_errno_set(void)
{
    errno = w32_errno_cnv(GetLastError());
    return -1;
}


const char *
w32_strerror(int errnum)
{
    char errbuffer[32];
    char *err = NULL;

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
    case ELOOP:             err = "Too many symbolic links"; break;
    case EOPNOTSUPP:        err = "Operation not supported on transport"; break;
    case EPFNOSUPPORT:      err = "Protocol family not supported"; break;
    case ECONNRESET:        err = "Connection reset by peer"; break;
    case ENOBUFS:           err = "No buffer space availabled"; break;
    case EAFNOSUPPORT:      err = "Family cannot be used with this socket"; break;
    case EPROTOTYPE:        err = "EPROTOTYPE"; break;
    case ENOTSOCK:          err = "Not a socket"; break;
    case ENOPROTOOPT:       err = "This option is unsupported"; break;
    case ESHUTDOWN:         err = "Connection shutdown"; break;
    case ECONNREFUSED:      err = "Connection refused"; break;
    case EADDRINUSE:        err = "Address already in use"; break;
    case ECONNABORTED:      err = "The connection was aborted"; break;
    case ENETUNREACH:       err = "The network can't be reached"; break;
    case ENETDOWN:          err = "Network down."; break;
    case ETIMEDOUT:         err = "Connection timed out"; break;
    case EHOSTDOWN:         err = "Host down"; break;
    case EHOSTUNREACH:      err = "Host unreachable"; break;
    case EINPROGRESS:       err = "In progress"; break;
    case EALREADY:          err = "Already"; break;
    case EDESTADDRREQ:      err = "EDESTADDRREQ"; break;
    case EMSGSIZE:          err = "Message size"; break;
    case EPROTONOSUPPORT:   err = "Protocol not supported"; break;
    case ESOCKTNOSUPPORT:   err = "ESOCKTNOSUPPORT"; break;
    case EADDRNOTAVAIL:     err = "EADDRNOTAVAIL"; break;
    case ENETRESET:         err = "Network reset"; break;
    case EISCONN:           err = "Socket is already connected"; break;
    case ENOTCONN:          err = "Socket is not connected"; break;
    case ETOOMANYREFS:      err = "ETOOMANYREFS"; break;
    case EPROCLIM:          err = "EPROCLIM"; break;
    case EUSERS:            err = "EUSERS"; break;
    case EDQUOT:            err = "EDQUOT"; break;
    case ESTALE:            err = "Stale"; break;
    case EREMOTE:           err = "Resource is remote"; break;
    case ENOTINITIALISED:   err = "Winsock not initialised"; break;
#if defined(EREFUSED)
    case EREFUSED:          err = "Refused"; break;
#endif

    /* BSD/SysV messages */
#if defined(ENOTBLK)
    case ENOTBLK:           err = "Block device required"; break;
#endif
#if defined(ETSTBSY)
    case ETXTBSY:           err = "Text file busy"; break;
#endif
#if defined(EUCLEAN)
    case EUCLEAN:           err = "UCLEAN"; break;
#endif
#if defined(EIDRM)
    case EIDRM:             err = "Identifier removed"; break;
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
#if defined(ENOSTR)
    case ENOSTR:            err = "Not a stream"; break;
#endif
#if defined(ENODATA)
    case ENODATA:           err = "No data (for no delay io)"; break;
#endif
#if defined(ETIME)
    case ETIME:             err = "Stream ioctl timeout"; break;
#endif
#if defined(ENOSR)
    case ENOSR:             err = "No stream resources"; break;
#endif
#if defined(ENONET)
    case ENONET:            err = "Machine is not on the network"; break;
#endif
#if defined(ENOPKG)
    case ENOPKG:            err = "No package"; break;
#endif
#if defined(ENOLINK)
    case ENOLINK:           err = "Virtual circuit is gone"; break;
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
#if defined(EPROTO)
    case EPROTO:            err = "Protocol error"; break;
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
#if defined(ENOMSG)
    case ENOMSG:            err = "No message"; break;
#endif
#if defined(EBADMSG)
    case EBADMSG:           err = "Bad message"; break;
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
#if defined(ENOTSUP)
    case ENOTSUP:           err = "Not supported"; break;
#endif
#if defined(ENOMEDIUM)
    case ENOMEDIUM:         err = "No medium"; break;
#endif
#if defined(ENOSHARE)
    case ENOSHARE:          err = "No such host or network path"; break;
#endif

    default:
        _snprintf(errbuffer, sizeof(errbuffer),
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
    return ((char *) err);
}
