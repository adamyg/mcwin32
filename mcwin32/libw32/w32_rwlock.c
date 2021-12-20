#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_rwlock_c,"$Id: w32_rwlock.c,v 1.2 2021/11/30 13:06:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 rwlock functionality/emulation
 *
 * Copyright (c) 1998 - 2019, Adam Young.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==end==
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0601              /* enable vista features */
#endif

#include <sys/rwlock.h>

#define  WINDOWS_MEAN_AND_LEAN
#include <windows.h>
#include <assert.h>

typedef void (WINAPI *InitializeSRWLock_t)(PSRWLOCK);
typedef void (WINAPI *AcquireSRWLockShared_t)(PSRWLOCK);
typedef void (WINAPI *ReleaseSRWLockShared_t)(PSRWLOCK);
typedef void (WINAPI *AcquireSRWLockExclusive_t)(PSRWLOCK);
typedef void (WINAPI *ReleaseSRWLockExclusive_t)(PSRWLOCK);

typedef struct {
    CRITICAL_SECTION        reader_lock;
    CRITICAL_SECTION        write_lock;
    HANDLE                  noreader_cond;
    int                     readers;            /* >= 0 or -99 of writer */
} xpsrwlock_t;

typedef struct {
    unsigned                magic;
#define RW_MAGIC                0x57333272      /* W32r */
    union {
        SRWLOCK             srw;                /* Slim Reader/Writer (SRW) Locks (vista+) */
        xpsrwlock_t         rwlock;             /* legacy implementation */
    };
} rwlock_imp_t;

static void WINAPI          my_InitializeSRWLock(PSRWLOCK);
static void WINAPI          my_AcquireSRWLockShared(PSRWLOCK);
static void WINAPI          my_ReleaseSRWLockShared(PSRWLOCK);
static void WINAPI          my_AcquireSRWLockExclusive(PSRWLOCK);
static void WINAPI          my_ReleaseSRWLockExclusive(PSRWLOCK);

static InitializeSRWLock_t              initialize_srw_lock;
static AcquireSRWLockShared_t           acquire_srw_lock_shared;
static ReleaseSRWLockShared_t           release_srw_lock_shared;
static AcquireSRWLockExclusive_t        acquire_srw_lock_exclusive;
static ReleaseSRWLockExclusive_t        release_srw_lock_exclusive;
static HINSTANCE                        library;


static void
initialisation(void)
{
    if (NULL == initialize_srw_lock) {
        assert(sizeof(struct rwlock) >= sizeof(rwlock_imp_t));

        /*
         *  resolve
         */
        if (0 != (library = LoadLibrary("Kernel32"))) {
            initialize_srw_lock         = (InitializeSRWLock_t) GetProcAddress(library, "InitializeSRWLock");
            acquire_srw_lock_shared     = (AcquireSRWLockShared_t) GetProcAddress(library, "AcquireSRWLockShared");
            release_srw_lock_shared     = (ReleaseSRWLockShared_t) GetProcAddress(library, "ReleaseSRWLockShared");
            acquire_srw_lock_exclusive  = (AcquireSRWLockExclusive_t) GetProcAddress(library, "AcquireSRWLockExclusive");
            release_srw_lock_exclusive  = (ReleaseSRWLockExclusive_t) GetProcAddress(library, "ReleaseSRWLockExclusive");

            if (initialize_srw_lock &&
                    acquire_srw_lock_shared && release_srw_lock_shared &&
                    acquire_srw_lock_exclusive && release_srw_lock_exclusive) {
                return;                         // success
            }

            FreeLibrary(library);
        }

        /*
         *  local implemenation
         */
        initialize_srw_lock             = my_InitializeSRWLock;
        acquire_srw_lock_shared         = my_AcquireSRWLockShared;
        release_srw_lock_shared         = my_ReleaseSRWLockShared;
        acquire_srw_lock_exclusive      = my_AcquireSRWLockExclusive;
        release_srw_lock_exclusive      = my_ReleaseSRWLockExclusive;
    }
}


/////////////////////////////////////////////////////////////////////////////////
//  implementation
//

LIBW32_API void
rwlock_init(struct rwlock *rwlock)
{
    rwlock_imp_t *rw = (rwlock_imp_t *)rwlock;

    initialisation();
    memset(rwlock, 0, sizeof(struct rwlock));
    initialize_srw_lock(&rw->srw);
    rw->magic = RW_MAGIC;
}


LIBW32_API void
rwlock_rdlock(struct rwlock *rwlock)
{
    rwlock_imp_t *rw = (rwlock_imp_t *)rwlock;

    assert(RW_MAGIC == rw->magic);
    acquire_srw_lock_shared(&rw->srw);
}


LIBW32_API void
rwlock_wrlock(struct rwlock *rwlock)
{
    rwlock_imp_t *rw = (rwlock_imp_t *)rwlock;

    assert(RW_MAGIC == rw->magic);
    acquire_srw_lock_exclusive (&rw->srw);
}


LIBW32_API void
rwlock_rdunlock(struct rwlock *rwlock)
{
    rwlock_imp_t *rw = (rwlock_imp_t *)rwlock;

    assert(RW_MAGIC == rw->magic);
    release_srw_lock_shared(&rw->srw);
}


LIBW32_API void
rwlock_wrunlock(struct rwlock *rwlock)
{
    rwlock_imp_t *rw = (rwlock_imp_t *)rwlock;

    assert(RW_MAGIC == rw->magic);
    release_srw_lock_exclusive(&rw->srw);
}


/////////////////////////////////////////////////////////////////////////////////
//  SRW emulation
//

static void WINAPI
my_InitializeSRWLock(PSRWLOCK srw)
{
    xpsrwlock_t *rw = (xpsrwlock_t *)srw;

    InitializeCriticalSection(&rw->reader_lock);
    InitializeCriticalSection(&rw->write_lock);
    rw->noreader_cond = CreateEvent(NULL, TRUE, TRUE, NULL);
}


static void WINAPI
my_AcquireSRWLockShared(PSRWLOCK srw)
{
    xpsrwlock_t *rw = (xpsrwlock_t *)srw;

    EnterCriticalSection(&rw->write_lock);
        EnterCriticalSection(&rw->reader_lock);
            if (1 == ++rw->readers) {
                ResetEvent(rw->noreader_cond);
            }
        LeaveCriticalSection(&rw->reader_lock);
    LeaveCriticalSection(&rw->write_lock);
}


static void WINAPI
my_ReleaseSRWLockShared(PSRWLOCK srw)
{
    xpsrwlock_t *rw = (xpsrwlock_t *)srw;

    EnterCriticalSection(&rw->reader_lock);
    assert(rw->readers > 0);
    if (rw->readers > 0) {
        if (0 == --rw->readers) {
            SetEvent(rw->noreader_cond);
        }
    }
    LeaveCriticalSection(&rw->reader_lock);
}


static void WINAPI
my_AcquireSRWLockExclusive(PSRWLOCK srw)
{
    xpsrwlock_t *rw = (xpsrwlock_t *)srw;

    EnterCriticalSection(&rw->write_lock);
    if (rw->readers > 0) {
        WaitForSingleObject(rw->noreader_cond, INFINITE);
    }
    assert(0 == rw->readers);
    rw->readers = -99;
}


static void WINAPI
my_ReleaseSRWLockExclusive(PSRWLOCK srw)
{
    xpsrwlock_t *rw = (xpsrwlock_t *)srw;

    assert(-99 == rw->readers);
    if (-99 == rw->readers) {
        rw->readers = 0;
        LeaveCriticalSection(&rw->write_lock);
    }
}

/*end*/

