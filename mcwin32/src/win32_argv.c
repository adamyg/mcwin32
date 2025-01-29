/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 argument support
 *
 * Copyright (c) 2024 - 2025 Adam Young.
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

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x501
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <stdlib.h>

static int
ArgumentSplit(char *cmd, char **argv, int cnt)
{
    char *start, *end;
    int argc;

    if (cmd == NULL) {
        return -1;
    }

    argc = 0;
    for (;;) {
        // Skip white-space
        while (*cmd == ' '|| *cmd == '\t' || *cmd == '\n') {
            ++cmd;
        }

        if (! *cmd)
            break;

        // element start
        if ('\"' == *cmd || '\'' == *cmd) { // quoted argument
            char quote = *cmd++;

            start = end = cmd;
            for (;;) {
                const char ch = *cmd;

                if (0 == ch)
                    break; // eos

                if ('\n' == ch || ch == quote) {
                    ++cmd; // delimiter
                    break;
                }

                if ('\\' == ch) { // quote
                    if (cmd[1] == '\"' || cmd[1] == '\'' || cmd[1] == '\\') {
                        ++cmd;
                    }
                }

                if (argv) *end++ = *cmd;
                ++cmd;
            }

        } else {
            start = end = cmd;
            for (;;) {
                const char ch = *cmd;

                if (0 == ch)
                    break; // eos

                if (ch == '\n' || ' ' == ch || '\t' == ch) {
                    ++cmd; // delimiter
                    break;
                }

                if ('\\' == ch) { // quote
                    if (cmd[1] == '\"' || cmd[1] == '\'' || cmd[1] == '\\') {
                        ++cmd;
                    }
                }

                if (argv) *end++ = *cmd;
                ++cmd;
            }
        }

        // element completion
        if (NULL == argv || cnt > 0) {
            if (argv) {
                argv[ argc ] = start;
                *end = '\0';
            }
            ++argc;
            --cnt;
        }
    }

    if (argv && cnt) {
        argv[argc] = NULL;
    }
    return argc;
}


/**
 *  UTF8Arguments ---
 *      Generate a UTF8-8 encoding argument vector from the wide-char command-line.
 *
 *  Parameters:
 *      pargv = Buffer populated with the argument count.
 *
 *  Returns:
 *      Argument vector, otherwise NULL.
 */

char **
GetUTF8Arguments(int *pargc)
{
    const wchar_t *wcmdline;

    if (NULL == (wcmdline = GetCommandLineW()) || wcmdline[0] == 0) {
        // import application name; fully qualified path; on empty command-line
        unsigned pathsz = GetModuleFileNameW(NULL, NULL, 0); // length, excluding terminating null character.

        if (pathsz) {
            if (NULL != (wcmdline = calloc(pathsz + 1 /*NUL*/, sizeof(wchar_t)))) {
                GetModuleFileNameW(NULL, (wchar_t *)wcmdline, pathsz + 1 /*NUL*/);
            }
        }
    }

    if (NULL != wcmdline) {
        // split into arguments
        const unsigned wcmdsz = wcslen(wcmdline),
            cmdsz = WideCharToMultiByte(CP_UTF8, 0, wcmdline, (int)wcmdsz, NULL, 0, NULL, NULL);
        char *cmd;

        if (NULL != (cmd = calloc(cmdsz + 1 /*NUL*/, sizeof(char)))) {
            char **argv = NULL;
            int argc;

            WideCharToMultiByte(CP_UTF8, 0, wcmdline, (int)wcmdsz, cmd, cmdsz + 1, NULL, NULL);
            if ((argc = ArgumentSplit(cmd, NULL, -1)) > 0) { // argument count
                if (NULL != (argv = calloc(argc + 1, sizeof(char *)))) {
                    ArgumentSplit(cmd, argv, argc + 1); // populate arguments
                    if (pargc) *pargc = argc;
                    return argv;
                }
            }
        }
        free(cmd);
    }

    if (pargc) *pargc = 0;
    return NULL;
}


char *
GetUTF8Argument0(void)
{
    const wchar_t *wcmd, *wmodule = NULL;
    char *arg0 = NULL;

    if (NULL == (wcmd = GetCommandLineW()) || wcmd[0] == 0) {
        // import application name; fully qualified path; on empty command-line
        unsigned pathsz = GetModuleFileNameW(NULL, NULL, 0); // length, excluding terminating null character.

        if (pathsz) {
            if (NULL != (wmodule = calloc(pathsz + 1 /*NUL*/, sizeof(wchar_t)))) {
                GetModuleFileNameW(NULL, (wchar_t *)wmodule, pathsz + 1 /*NUL*/);
                wcmd = wmodule;
            }
        }
    }

    if (NULL != wcmd) {
        // derive argv[0]
        const unsigned wcmdsz = wcslen(wcmd),
            cmdsz = WideCharToMultiByte(CP_UTF8, 0, wcmd, (int)wcmdsz, NULL, 0, NULL, NULL);
        char *cursor;

        if (NULL != (cursor = malloc(cmdsz))) {

            arg0 = cursor;
            WideCharToMultiByte(CP_UTF8, 0, wcmd, (int)wcmdsz, arg0, cmdsz, NULL, NULL);

            if (arg0[0] == '"') {
                if (NULL != (cursor = strchr(arg0 + 1, '"'))) { // quoted
                    *cursor = 0;
                    ++arg0;
                }
            } else {
                if (NULL != (cursor = strchr(arg0, ' '))) { // unquoted
                    *cursor = 0;
                }
            }
        }
    }

    free((void *)wmodule); // working storage
    return arg0;
}

//end


