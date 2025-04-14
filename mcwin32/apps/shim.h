/* -*- mode: c; indent-width: 4; -*- */
/*
 * libshim -- application shim
 *
 * Copyright (c) 2024 - 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of WIN32 Midnight Commander.
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
 * License for more details.
 * ==end==
 */

#if defined(_WIN32_WINNT) && (_WIN32_WINNT < 0x0601)
#undef  _WIN32_WINNT                            // ASSOCSTR's
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#if defined(__WATCOMC__)
#define _WIN32_IE 0x0800                        // ASSOCSTR's
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <stdlib.h>                             // EXIT_FAILURE
#include <wchar.h>                              // wchar_t

#ifdef __cplusplus
extern "C" {
#endif

void ApplicationShim(const wchar_t *name, const wchar_t *alias);
void ApplicationShimCmd(const wchar_t *name, const wchar_t *alias, const wchar_t *cmdline);

int  ShimCreateChild(PROCESS_INFORMATION *ppi, const wchar_t *name, const wchar_t *path, const wchar_t *cmdline);
void ShimErrorMessage(const wchar_t *name, DWORD wrc);

#ifdef __cplusplus
}
#endif

//end

