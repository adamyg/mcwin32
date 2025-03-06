/*
 * libshim -- test application.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

char **GetUTF8Arguments(int *pargc);

#include "../src/win32_argv.c"                  // GetUTF8Arguments

void
main(int argc, char **argv)
{
    char **uargv;
    int uargc, i;

    printf("cmdline:  >%s<\n\n", GetCommandLineA());

    printf("argc:     %d\n", argc);
    printf("argv:\n");
    for (i = 0; i < argc; ++i) {
        printf("%d]: %s\n", i, argv[i]);
    }

    if (NULL != (uargv = GetUTF8Arguments(&uargc))) {
        printf("utf8argc: %d\n", uargc);
        printf("utf8argv:\n");
        for (i = 0; i < uargc; ++i) {
            printf("%d]: %s\n", i, uargv[i]);
        }
        assert(NULL == uargv[i]);
    }
}

//end
