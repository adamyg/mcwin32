#ifndef KBCONSOLE_H_INCLUDED
#define KBCONSOLE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(kbconsole_h,"$Id: kbconsole.h,v 1.5 2025/01/29 13:33:04 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * console keyboard test application
 *
 * Copyright (c) 2024 - 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of the WinXSH project.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

void ConsoleClear(HANDLE console);
void ConsoleCEOS(HANDLE console);
void ConsoleCEOL(HANDLE console);

void ConsoleHome(HANDLE console);
int  ConsoleSizeSet(HANDLE console, int width, int height);

void cprinta(const char *fmt, ...);
void cprintw(const wchar_t *fmt, ...);

#endif //KBCONSOLE_H_INCLUDED

//end
