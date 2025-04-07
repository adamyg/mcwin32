/*
 * mchelp application.
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

#include "shim.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "getopt.h"

#ifndef _countof
#define _countof(__type) (sizeof(__type)/sizeof(__type[0]))
#endif

static wchar_t *Cmdline(const char *topic, unsigned section);
static void Usage();

static const char PROGNAME[] = {"mchelp"};
static const wchar_t WMCVIEW[] = {L"mcview"};

static const char MANPATH[] = {"share\\man\\"};
static const wchar_t WMANPATH[] = {L"share\\man\\"};


int
main(int argc, char **argv)
{
    unsigned list = 0, section = 1;
    int ch;

    while (-1 != (ch = Updater::Getopt(argc, argv, "s:h"))) {
        switch (ch) {
        case 's':   /* section */
            section = static_cast<unsigned>(strtoul(Updater::optarg, NULL, 10));
            break;
        case 'h':
        default:
            Usage();
            break;
        }
    }

    argv += Updater::optind;
    if ((argc -= Updater::optind) < 1) {
        std::cerr << "\n" <<
            PROGNAME << ": expected a topic" << std::endl;
        Usage();

    } else if (argc > 1) {
        std::cerr << "\n" <<
            PROGNAME << ": unexpected arguments '" << argv[1] << "' ..." << std::endl;
        Usage();

    } else {
        ApplicationShimCmd(WMCVIEW, L"mc.exe", Cmdline(argv[0], section));
    }

    return EXIT_FAILURE;
}


static wchar_t *
Cmdline(const char *topic, unsigned section)
{
    char suffix[32];
    _snprintf(suffix, _countof(suffix), ".%u.man", section);

    const size_t total_len = (_countof(WMCVIEW) + MAX_PATH) + 32 + strlen(topic) + strlen(suffix);
    wchar_t *buf = static_cast<wchar_t *>(calloc(total_len, sizeof(wchar_t)));
    size_t aptlen, len;

    aptlen = swprintf(buf, total_len, L"%s \"", WMCVIEW);

    if ((len = GetModuleFileNameW(NULL, buf + aptlen, MAX_PATH)) > 0) {
        wchar_t *cp;

        len += aptlen;

        for (cp = buf + len; (cp > (buf + aptlen)) && (*cp != '\\'); cp--)
            --len;                              // remove application-name

        if ('\\' == *cp) ++cp;

        cp += swprintf(cp, total_len - len, WMANPATH);
        for (const char *cursor = topic; *cursor; ++cursor) {
            *cp++ = *cursor;
        }
        for (const char *cursor = suffix; *cursor; ++cursor) {
            *cp++ = *cursor;
        }

        *cp++ = '"';
        *cp = '\0';

        for (; (cp > (buf + aptlen)); cp--)
            if (*cp == '\\') *cp = '/';         // path conversion
    }
    return buf;
}


static void
Usage()
{
    char path[MAX_PATH];
    size_t len;

    std::cerr <<
        "\nusage: " << PROGNAME << " -s [section] topic\n";

    if ((len = GetModuleFileNameA(NULL, path, MAX_PATH)) > 0) {
        char *cp;

        for (cp = path + len; (cp > path) && (*cp != '\\'); cp--)
            --len;                              // remove application-name

        if ('\\' == *cp) {                      // expand
            WIN32_FIND_DATAA ffd = {0};
            HANDLE h = INVALID_HANDLE_VALUE;

            _snprintf(cp, _countof(path) - len, "\\%s\\*.man", MANPATH);

            if ((h = FindFirstFileA(path, &ffd)) != INVALID_HANDLE_VALUE) {
                std::cerr <<
                    "\ntopics:\n   ";

                do {
                    if (0 == (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        ffd.cFileName[strlen(ffd.cFileName) - 4] = 0;
                        std::cerr << ' ' << ffd.cFileName;
                    }
                } while (FindNextFileA(h, &ffd));

                FindClose(h);

                std::cerr << "\n";
            }
        }
    }

    std::cerr << std::endl;
    std::exit(EXIT_FAILURE);
}

//end
