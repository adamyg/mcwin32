#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_iconv_c,"$Id: w32_iconv.c,v 1.10 2023/09/17 13:04:58 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 iconv dynamic loader.
 *
 * Copyright (c) 1998 - 2023, Adam Young.
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
 */

#if defined(_WIN32) || defined(WIN32)
#include <w32config.h>

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include "win32_include.h"
#include "win32_iconv.h"

#define ICONV_NULL      ((void *)-1)

#if defined(LIBW32_STATIC)
//#define DO_TRACE_LOG                          // trace_log()
#endif

#if defined(HAVE_LIBICONV_DLL)
#define DLLLINKAGE      __cdecl
#else
#define DLLLINKAGE      /**/
#endif
typedef void *          (DLLLINKAGE * iconvopenfn_t)(const char *to, const char *from);
typedef void            (DLLLINKAGE * iconvclosefn_t)(void *fd);
typedef int             (DLLLINKAGE * iconvfn_t)(void *fd, const char **from, size_t *fromlen, char **to, size_t *tolen);
typedef int             (DLLLINKAGE * iconverrnofn_t)(void);

#if defined(HAVE_LIBCITRUS)
typedef const char *    (DLLLINKAGE * iconv_PATH_ESDB_t)(void);
typedef const char *    (DLLLINKAGE * iconv_PATH_CSMAPPER_t)(void);
typedef const char *    (DLLLINKAGE * iconv_PATH_ICONV_t)(void);
#endif

static HINSTANCE        x_iconvdll;
static HINSTANCE        x_msvcrtdll;

static iconvopenfn_t    x_iconv_open;
static iconvclosefn_t   x_iconv_close;
static iconvfn_t        x_iconv;
//  static void *       x_iconvctl;
//  static void *       x_iconv_errno;

static const char *     x_iconvnames[] = {
        NULL,                                   // place-holder
#if defined(ICONVDLL_NAME)
        ICONVDLL_NAME,                          // compile-time configuration
#else
#define ICONVDLL_NAME   "[lib]iconv[2].dll"     // dynamic
        "libiconv2.dll",
        "libiconv.dll",
        "iconv2.dll",
        "iconv.dll",
#endif
       };


LIBW32_API void
w32_iconv_dllname(const char *dllname)
{
    if (x_iconvnames[0]) free((void *)x_iconvnames[0]);
    x_iconvnames[0] = (dllname && *dllname ? strdup(dllname) : NULL);
}


LIBW32_API int
w32_iconv_connect(int verbose)
{
#ifndef MSVCRTDLL_NAME
#define MSVCRTDLL_NAME  "msvcrt.dll"
#endif
    const char *name;
    char fullname[1024], *end;
    unsigned i;

    if (x_iconvdll && x_msvcrtdll) {
        return TRUE;
    }

    fullname[0] = 0;
    GetModuleFileNameA(GetModuleHandle(NULL), fullname, sizeof(fullname));
    if (NULL != (end = (char *)strrchr(fullname, '\\'))) {
        *++end = 0;
    }

    for (i = 0; i < sizeof(x_iconvnames)/sizeof(x_iconvnames[0]); ++i) {

        if (NULL == (name = x_iconvnames[i])) {
            continue;                           // empty slot
        }

#if defined(DO_TRACE_LOG)
        trace_log("iconv_dll(%s)\n", name);
#endif

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
#if defined(DO_TRACE_LOG)
            trace_log("iconv_dll(%s)\n", env);
#endif
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
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    GetModuleFileNameA(x_iconvdll, fullname, sizeof(fullname));
    if (NULL != (x_iconv = (iconvfn_t)GetProcAddress(x_iconvdll, "libiconv"))) {
        x_iconv_open = (iconvopenfn_t)GetProcAddress(x_iconvdll, "libiconv_open");
        x_iconv_close = (iconvclosefn_t)GetProcAddress(x_iconvdll, "libiconv_close");

    } else if (NULL != (x_iconv = (iconvfn_t)GetProcAddress(x_iconvdll, "iconv"))) {
        x_iconv_open = (iconvopenfn_t)GetProcAddress(x_iconvdll, "iconv_open");
        x_iconv_close = (iconvclosefn_t)GetProcAddress(x_iconvdll, "iconv_close");
    }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif

//  x_iconvctl = (void *)GetProcAddress(x_iconvdll, "libiconvctl");
//  x_iconv_errno = (void *)GetProcAddress(x_iconvdll, "libiconv_errno");
//  if (NULL == x_iconv_errno) {
//      x_iconv_errno = (void *)GetProcAddress(x_msvcrtdll, "_errno");
//  }

#if defined(DO_TRACE_LOG)
    trace_log("iconv Functions (%s)\n", fullname);
    trace_log("\ticonv:       %p\n", x_iconv);
    trace_log("\ticonv_open:  %p\n", x_iconv_open);
    trace_log("\ticonv_close: %p\n", x_iconv_close);
//  trace_log("\ticonv_errno: %p\n", x_iconv_errno);
//  trace_log("\ticonvctl:    %p\n", x_iconvctl);
#endif

    if (NULL == x_iconv || NULL== x_iconv_open || NULL == x_iconv_close /*|| NULL == x_iconv_errno*/) {
        if (verbose) {
            char buffer[128];

            (void) _snprintf(buffer, sizeof(buffer), "Unable to resolve symbols from %s", ICONVDLL_NAME);
            buffer[sizeof(buffer) - 1] = 0;
            MessageBoxA(0, buffer, "Error", MB_OK);
        }
        w32_iconv_shutdown();
        return FALSE;
    }
    return TRUE;
}


LIBW32_API void
w32_iconv_shutdown(void)
{
    x_iconv = NULL;
    x_iconv_open = NULL;
    x_iconv_close = NULL;
//  x_iconvctl = NULL;
//  x_iconv_errno = NULL;

    if (x_iconvdll) {
       FreeLibrary(x_iconvdll);
       if (x_msvcrtdll) {
           FreeLibrary(x_msvcrtdll);
       }
       x_iconvdll = x_msvcrtdll = 0;
   }
}


LIBW32_API void *
w32_iconv_open(const char *to, const char *from)
{
#if defined(DEBUG)
    if (w32_iconv_connect(TRUE) && x_iconv_open) {
#else
    if (w32_iconv_connect(FALSE) && x_iconv_open) {
#endif
        return (x_iconv_open)(to, from);
    }
    return ICONV_NULL;
}


LIBW32_API void
w32_iconv_close(void *fd)
{
    if (x_iconv_close) {
        (x_iconv_close)(fd);
    }
}


LIBW32_API int
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
