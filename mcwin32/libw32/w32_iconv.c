#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_iconv_c,"$Id: w32_iconv.c,v 1.2 2017/03/07 15:38:31 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 iconv dynamic loader.
 *
 * Copyright (c) 1998 - 2018, Adam Young.
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

#if defined(WIN32)
#include <w32config.h>

#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include "win32_include.h"
#include "win32_iconv.h"

#define ICONV_NULL      ((void *)-1)

#if defined(HAVE_LIBICONV_DLL)
#define DLLLINKAGE      __cdecl
#else
#define DLLLINKAGE      /**/
#endif
typedef void *          (DLLLINKAGE * iconvopenfn_t)(const char *to, const char *from);
typedef void            (DLLLINKAGE * iconvclosefn_t)(void *fd);
typedef int             (DLLLINKAGE * iconvfn_t)(void *fd, const char **from, size_t *fromlen, char **to, size_t *tolen);

static HINSTANCE        x_iconvdll;
static HINSTANCE        x_msvcrtdll;

static iconvopenfn_t    x_iconv_open;
static iconvclosefn_t   x_iconv_close;
static iconvfn_t        x_iconv;
static void *           x_iconvctl;
static void *           x_iconv_errno;

int
w32_iconv_connect(int verbose)
{
#ifndef MSVCRTDLL_NAME
#define MSVCRTDLL_NAME  "msvcrt.dll"
#endif
   static const char *iconvnames[] = {
#if defined(ICONVDLL_NAME)
        ICONVDLL_NAME,                          // configuration
#else
#define ICONVDLL_NAME   "[lib]iconv[2].dll"
        "libiconv2.dll",
        "libiconv.dll",
        "iconv2.dll",
        "iconv.dll",
#endif
       NULL
       };

    const char *name;
    char fullname[1024], *end;
    unsigned i;

    if (x_iconvdll && x_msvcrtdll) {
        return TRUE;
    }

    fullname[0] = 0;
    GetModuleFileName(GetModuleHandle(NULL), fullname, sizeof(fullname));
    if (NULL != (end = (char *)strrchr(fullname, '\\'))) {
        *++end = 0;
    }

    for (i = 0; NULL != (name = iconvnames[i]); ++i) {
//      trace_log("iconv_dll(%s)\n", name);
        if (end) {
            strncpy(end, name, sizeof(fullname) - (end - fullname));
            fullname[sizeof(fullname)-1] = 0;
            if (NULL != (x_iconvdll = LoadLibraryA(fullname))) {
                break;                          // relative to task
            }
        }

        if (NULL != (x_iconvdll = LoadLibraryA(name))) {
            break;                              // general, within search path
        }
    }

    if (NULL == x_iconvdll) {                   // environment override
        const char *env;

        if (NULL != (env = getenv("ICONVDLL"))) {
//          trace_log("iconv_dll(%s)\n", env);
            x_iconvdll = LoadLibraryA(env);
        }
    }

    if (x_iconvdll) {                           // required when using mixed library configurations.
        x_msvcrtdll = LoadLibraryA(MSVCRTDLL_NAME);
    }

    if (NULL == x_iconvdll || NULL == x_msvcrtdll) {
        if (verbose) {
            const DWORD err = GetLastError();
            char buffer[128];
            _snprintf(buffer, sizeof(buffer), "Unable to load %s, rc: %ld",
                        (0 == x_iconvdll ? ICONVDLL_NAME : MSVCRTDLL_NAME), (long)err);
            MessageBoxA(0, buffer, "Error", MB_OK);
        }
        w32_iconv_shutdown();
        return FALSE;
    }

    fullname[0] = 0;                            // resolve symbols, iconvctl() is optional
    GetModuleFileNameA(x_iconvdll, fullname, sizeof(fullname));
    x_iconv       = (iconvfn_t)GetProcAddress(x_iconvdll, "libiconv");
    x_iconv_open  = (iconvopenfn_t)GetProcAddress(x_iconvdll, "libiconv_open");
    x_iconv_close = (iconvclosefn_t)GetProcAddress(x_iconvdll, "libiconv_close");
    x_iconvctl    = (void *)GetProcAddress(x_iconvdll, "libiconvctl");
    x_iconv_errno = (void *)GetProcAddress(x_iconvdll, "libiconv_errno");
    if (NULL == x_iconv_errno) {
        x_iconv_errno = (void *)GetProcAddress(x_msvcrtdll, "_errno");
    }

//  trace_log("iconv Functions (%s)\n", fullname);
//  trace_log("\ticonv:       %p\n", x_iconv);
//  trace_log("\ticonv_open:  %p\n", x_iconv_open);
//  trace_log("\ticonv_close: %p\n", x_iconv_close);
//  trace_log("\ticonv_errno: %p\n", x_iconv_errno);
//  trace_log("\ticonvctl:    %p\n", x_iconvctl);

    if (NULL == x_iconv || NULL== x_iconv_open || NULL == x_iconv_close || NULL == x_iconv_errno) {
        if (verbose) {
            char buffer[128];

            _snprintf(buffer, sizeof(buffer), "Unable to resolve symbols from %s", ICONVDLL_NAME);
            MessageBoxA(0, buffer, "Error", MB_OK);
        }
        w32_iconv_shutdown();
        return FALSE;
    }
    return TRUE;
}


void
w32_iconv_shutdown(void)
{
    x_iconv = NULL;
    x_iconv_open = NULL;
    x_iconv_close = NULL;
    x_iconvctl = NULL;
    x_iconv_errno = NULL;

    if (x_iconvdll) {
       FreeLibrary(x_iconvdll);
       if (x_msvcrtdll) {
           FreeLibrary(x_msvcrtdll);
       }
       x_iconvdll = x_msvcrtdll = 0;
   }
}


void *
w32_iconv_open(const char *to, const char *from)
{
    if (w32_iconv_connect(TRUE) && x_iconv_open) {
        return (x_iconv_open)(to, from);
    }
    return ICONV_NULL;
}


void
w32_iconv_close(void *fd)
{
    if (x_iconv_close) {
        (x_iconv_close)(fd);
    }
}


int
w32_iconv(void * fd, const char **from, size_t *fromlen, char **to, size_t *tolen)
{
    if (x_iconv) {
        return (x_iconv)(fd, from, fromlen, to, tolen);
    }
    errno = EIO;
    return -1;
}

#endif  /*WIN32*/
/*end*/

