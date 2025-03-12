#include <edidentifier.h>
__CIDENT_RCSID(btest_c,"$Id: kbconsole.c,v 1.7 2025/03/12 17:39:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * console keyboard test application
 *
 * Copyright (c) 2015 - 2025, Adam Young.
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

#include "kbconsole.h"

#include <stdio.h>
#include <stdlib.h>

#include "kbutil.h"

/*
 * ConsoleClear ---
 *	Clear the console.
 **/
void
ConsoleClear(HANDLE console)
{
	COORD coord = {0,0};
	CONSOLE_SCREEN_BUFFER_INFO sbi = {0,0};
	DWORD dwConSize, cCharsWritten;

	GetConsoleScreenBufferInfo(console, &sbi);
	dwConSize = sbi.dwSize.X * sbi.dwSize.Y;
	FillConsoleOutputCharacterA(console, ' ', dwConSize, coord, &cCharsWritten);
	FillConsoleOutputAttribute(console, sbi.wAttributes, dwConSize, coord, &cCharsWritten);
	SetConsoleCursorPosition(console, coord);
}


/*
 *  ConsoleClearEOS ---
 *	Clear to End-Of-Screen.
 **/
void
ConsoleCEOS(HANDLE console)
{
	CONSOLE_SCREEN_BUFFER_INFO sbi = {0,0};
	DWORD dwConSize, cCharsWritten;

	GetConsoleScreenBufferInfo(console, &sbi);
	dwConSize = sbi.dwSize.X - sbi.dwCursorPosition.X; // EOL
	if (--sbi.dwCursorPosition.Y < sbi.dwSize.Y) dwConSize += (sbi.dwSize.Y - sbi.dwCursorPosition.Y) * sbi.dwSize.Y; // EOS
	FillConsoleOutputCharacterA(console, ' ', dwConSize, sbi.dwCursorPosition, &cCharsWritten);
	FillConsoleOutputAttribute(console, sbi.wAttributes, dwConSize, sbi.dwCursorPosition, &cCharsWritten);
}


/*
 * ConsoleClearEOL ---
 *	Clear to End-Of-Line.
 **/
void
ConsoleCEOL(HANDLE console)
{
	CONSOLE_SCREEN_BUFFER_INFO sbi = {0,0};
	DWORD dwConSize, cCharsWritten;

	GetConsoleScreenBufferInfo(console, &sbi);
	dwConSize = sbi.dwSize.X - sbi.dwCursorPosition.X;
	FillConsoleOutputCharacterA(console, ' ', dwConSize, sbi.dwCursorPosition, &cCharsWritten);
	FillConsoleOutputAttribute(console, sbi.wAttributes, dwConSize, sbi.dwCursorPosition, &cCharsWritten);
}


/*
 *  ConsoleSizeSet ---
 *	Set the console size.
 **/
int 
ConsoleSizeSet(HANDLE console, int width, int height)
{
	if (INVALID_HANDLE_VALUE == console)
		return -1; // invalid handle

	if (width < 0 && height < 0)
		return TRUE; // done

	COORD coord = {0};
	coord.X = (SHORT)(width);
	coord.Y = (SHORT)(height);
	if (SetConsoleScreenBufferSize(console, coord) == FALSE)
		return 0; // error

	SMALL_RECT rect = {0};
	rect.Bottom = coord.X - 1;
	rect.Right = coord.Y - 1;
	return (SetConsoleWindowInfo(console, TRUE, &rect) != FALSE);
}


/*
 *  ConsoleHome ---
 *	Home cursor.
 **/
void
ConsoleHome(HANDLE console)
{
	COORD iHome = {0,0};
	SetConsoleCursorPosition(console, iHome);
}


/*
 *  Console formatted output
 **/

void 
cprinta(const char *fmt, ...)
{
	HANDLE cout = GetStdHandle(STD_OUTPUT_HANDLE);
	char *cursor, *nl;
	char buffer[1024];
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = vsnprintf(buffer, _countof(buffer), fmt, ap);
	SetConsoleTextAttribute(cout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	cursor = buffer;
	while ((nl = strchr(cursor, '\n')) != NULL) {
		const int part = (nl - cursor) + 1;
		WriteConsoleA(cout, cursor, part - 1, NULL, NULL);
		ConsoleCEOL(cout);
		WriteConsoleA(cout, "\n\r", 2, NULL, NULL);
		cursor += part;
		len -= part;
	}
	WriteConsoleA(cout, cursor, len, NULL, NULL);
	va_end(ap);
}


void 
cprintw(const wchar_t *fmt, ...)
{
	HANDLE cout = GetStdHandle(STD_OUTPUT_HANDLE);
	wchar_t *cursor, *nl;
	wchar_t buffer[1024];
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = _vsnwprintf(buffer, _countof(buffer), fmt, ap);
	SetConsoleTextAttribute(cout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	cursor = buffer;
	while ((nl = wcschr(cursor, '\n')) != NULL) {
		const int part = (nl - cursor) + 1;
		WriteConsoleW(cout, cursor, part - 1, NULL, NULL);
		ConsoleCEOL(cout);
		WriteConsoleA(cout, "\n\r", 2, NULL, NULL);
		cursor += part;
		len -= part;
	}
	WriteConsoleW(cout, cursor, len, NULL, NULL);
	va_end(ap);
}

/*end*/
