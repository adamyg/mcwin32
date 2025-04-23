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

#include <stdio.h>
#include <iostream>

#include "getopt.h"
#include "..\buildinfo.h"

#ifndef _countof
#define _countof(__type) (sizeof(__type)/sizeof(__type[0]))
#endif
#define __L(__t)    L##__t
#define _L(__t)     __L(__t)

static wchar_t *CommandLine(const char *topic, const char *locale, unsigned section);
static const char *GetLocale(void);
static void Usage();

static const wchar_t PROGNAME[] = {L"mchelp"};
static const wchar_t MANPATH[] = {L"share\\man\\"};
static const wchar_t MCVIEW[] = { L"mcview" };

int
main(int argc, char *argv[])
{
    const char *locale = GetLocale();
    unsigned section = 1;
    int ch;

    while (-1 != (ch = Updater::Getopt(argc, argv, "l:s:Vh"))) {
        switch (ch) {
        case 'l':   // locale
            locale = Updater::optarg;
            break;
        case 's':   // section
            section = static_cast<unsigned>(strtoul(Updater::optarg, NULL, 10));
            break;
        case 'V':   // version
            fwprintf(stderr, L"%ls: " _L(VERSION) _L(".") _L(BUILD_NUMBER) _L(" (") _L(BUILD_DATE) _L(")\n"), PROGNAME);
            return EXIT_FAILURE;
        case 'h':
        default:
            Usage();
            break;
        }
    }

    argv += Updater::optind;
    if ((argc -= Updater::optind) < 1) {
        fwprintf(stderr, L"\n%ls: expected a topic\n", PROGNAME);
        Usage();

    } else if (argc > 1) {
        fwprintf(stderr, L"\n%ls: unexpected argument(s) after topic", PROGNAME);
        Usage();

    } else {
        ApplicationShimCmd(MCVIEW, L"mc.exe", CommandLine(argv[0], locale, section));
    }

    return EXIT_FAILURE;
}


/**
 *  CommandLine ---
 *      Build the mcview command line, resolving help source.
 */
static wchar_t *
CommandLine(const char *topic, const char *locale, unsigned section)
{
    char suffix[32];
    _snprintf(suffix, _countof(suffix), ".%u.man", section);

    const size_t total_len = (_countof(MCVIEW) + MAX_PATH + 8) +
                    strlen(locale) + strlen(topic) + strlen(suffix);
    wchar_t *buf = static_cast<wchar_t *>(calloc(total_len, sizeof(wchar_t)));
    size_t aptlen, len;

    aptlen = swprintf(buf, total_len, L"%ls \"", MCVIEW);

    if ((len = GetModuleFileNameW(NULL, buf + aptlen, MAX_PATH)) > 0) {
        wchar_t *path = buf + aptlen, *cp;

        len += aptlen;

        // remove application-name
        for (cp = buf + len; (cp > (buf + aptlen)) && (*cp != '\\'); cp--) {
            --len;
        }

        if ('\\' == *cp) ++cp;
        cp += swprintf(cp, total_len - len, MANPATH);

        // search locale
        do {
            wchar_t *end = cp;

            if (locale) {                       // ../<locale>/<topic>
                if (!*locale) {
                    locale = NULL; // empty
                } else {
                    for (const char *cursor = locale; *cursor; ++cursor) {
                        *end++ = *cursor;
                    }
                    *end++ = '\\';
                }
            }

            for (const char *cursor = topic; *cursor; ++cursor) {
                *end++ = *cursor;
            }

            for (const char *cursor = suffix; *cursor; ++cursor) {
                *end++ = *cursor;
            }
            *end = '\0';

            if (locale) {
                const DWORD attrs = GetFileAttributesW(path);

                if (attrs == INVALID_FILE_ATTRIBUTES ||
                            (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                    if ((locale = strchr(locale, '_')) != NULL) {
                        ++locale; // next component.
                    }
                    continue; // next component, otherwise without. 
                }
            }
            cp = end;
            break;

        } while (1);

        *cp++ = '"';
        *cp = '\0';

        // path conversion
        for (; (cp > (buf + aptlen)); cp--) {
            if (*cp == '\\') *cp = '/';
        }

    } else {
        swprintf(buf, total_len, L"%ls \"mc\"", MCVIEW);
    }
    return buf;
}


/**
 *  GetLocale ---
 *      Retrieve current locale.
 */
static const char *
GetLocale(void)
{
    static char x_lang[64] = {0};

    if (0 == x_lang[0]) {
        char iso639[16] = {0}, iso3166[16] = {0};
        LCID lcid = GetThreadLocale();

        if (GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME, iso639, sizeof(iso639)) &&
                GetLocaleInfoA(lcid, LOCALE_SISO3166CTRYNAME, iso3166, sizeof(iso3166))) {
            snprintf(x_lang, sizeof(x_lang), "%s_%s", iso639, iso3166); // "9_9"
            x_lang[sizeof(x_lang) - 1] = '\0';
        }
    }
    return x_lang[0] ? x_lang : NULL;
}


/**
 *  Usage ---
 *      Command line usage, plus list of available topics.
 */
static void
Usage()
{
    wchar_t path[MAX_PATH];
    size_t len;

    fwprintf(stderr, L"\nusage: %ls -l [locale] -s [section] topic\n", PROGNAME);

    if ((len = GetModuleFileNameW(NULL, path, _countof(path))) > 0) {
        wchar_t *cp;

        for (cp = path + len; (cp > path) && (*cp != '\\'); cp--)
            --len;                              // remove application-name

        if ('\\' == *cp) {                      // expand
            WIN32_FIND_DATAW ffd = {0};
            HANDLE h = INVALID_HANDLE_VALUE;

            _snwprintf(cp, _countof(path) - len, L"\\%ls\\*.man", MANPATH);

            if ((h = FindFirstFileW(path, &ffd)) != INVALID_HANDLE_VALUE) {
                fwprintf(stderr, L"\ntopics:\n");

                do {
                    if (0 == (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        ffd.cFileName[wcslen(ffd.cFileName) - 4] = 0;
                        fwprintf(stderr, L" %ls", ffd.cFileName);
                    }
                } while (FindNextFileW(h, &ffd));

                FindClose(h);

                fwprintf(stderr, L"\n");
            }
        }
    }

    fwprintf(stderr, L"\n");
    std::exit(EXIT_FAILURE);
}

//end
