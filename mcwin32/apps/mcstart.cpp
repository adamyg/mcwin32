/*
 * mcstart application.
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

#include <Shellapi.h>
#include <ObjBase.h>

#include <stdio.h>
#include "getopt.h"

#ifndef _countof        /*MSVC _countof()*/
#define _countof(__type) (sizeof(__type)/sizeof(__type[0]))
#endif
#if defined(__GNUC__)   /*BOOST_GCC_VERSION equiv*/
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#define PROGNAME L"mcstart"

typedef BOOL (WINAPI *ShellExecuteExW_t)(SHELLEXECUTEINFOW *);

static void Usage();

#if defined(__MINGW32__)
extern "C"
#endif
int wmain(int argc, wchar_t *argv[])
{
    BOOL owait = 0, ocmd = 0;
    int ch;

    while (-1 != (ch = Updater::Getopt(argc, argv, "CWh"))) {
        switch (ch) {
        case 'C':   // command
            ocmd = 1;
            break;
        case 'W':   // wait
            owait = 1;
            break;
        case 'h':
        default:
            Usage();
            break;
        }
    }

    argv += Updater::optind;
    if ((argc -= Updater::optind) < 1) {
        fwprintf(stderr, L"\n%ls: expected a target\n", PROGNAME);
        Usage();

    } else if (argc > 1) {
        fwprintf(stderr, L"\n%ls: unexpected arguments '%ls' ...", PROGNAME, argv[1]);
        Usage();

    } else {
        if (ocmd) {
            //  START ["title"]
            //      [/D path] [/I] [/MIN] [/MAX] [/SEPARATE | /SHARED]
            //          [/LOW | /NORMAL | /HIGH | /REALTIME | /ABOVENORMAL | /BELOWNORMAL]
            //      /NODE <NUMA node>] [/AFFINITY <hex affinity mask>] [/WAIT] [/B]
            //          [command/program] [parameters]
            //
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION joli = {0};
            PROCESS_INFORMATION pi = {0};
            DWORD dwExitCode = 0;

            const wchar_t *comspec = _wgetenv(L"ComSpec");
            if (NULL == comspec || !*comspec) comspec = L"cmd.exe";
            wchar_t cmd[1024];

            swprintf(cmd, _countof(cmd), L"%ls /C start %ls", comspec, argv[0]);
            fwprintf(stdout, L"CMD: %ls, %ls\n", comspec, cmd);

            if (! ShimCreateChild(&pi, L"start", comspec, cmd)) {
                return EXIT_FAILURE;
            }

            ResumeThread(pi.hThread);
            CloseHandle(pi.hThread);

            if (owait && pi.hProcess) {
                WaitForSingleObject(pi.hProcess, INFINITE);
                GetExitCodeProcess(pi.hProcess, &dwExitCode);
                CloseHandle(pi.hProcess);
                ExitProcess(dwExitCode);
            }

            CloseHandle(pi.hProcess);
            return EXIT_SUCCESS;

        } else {
            //
            //  ShellExecute
            //
            SHELLEXECUTEINFOW sei;
            HMODULE hShell32;
            ShellExecuteExW_t shellExecuteEx;
            DWORD dwExitCode = 0;
            BOOL ret;

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
            hShell32 = LoadLibraryA("shell32.dll");
            if (hShell32 == NULL) {
                fwprintf(stderr, L"\n%ls: couldn't load shell32.dll\n", PROGNAME);
                return EXIT_FAILURE;
            }
            shellExecuteEx = (ShellExecuteExW_t) GetProcAddress(hShell32, "ShellExecuteExW");
            if (shellExecuteEx == NULL) {
                fwprintf(stderr, L"\n%ls: couldn't resolve ShellExecuteExW in shell32.dll\n", PROGNAME);
                FreeLibrary(hShell32);
                return EXIT_FAILURE;
            }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif

            CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

            memset(&sei, 0, sizeof(sei));
            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_NOCLOSEPROCESS|SEE_MASK_NO_CONSOLE;
            sei.lpVerb = L"open";
            sei.lpFile = argv[0];
            sei.lpParameters = NULL;
            sei.lpDirectory = NULL;
            sei.nShow = SW_SHOWNORMAL;
            ret = shellExecuteEx(&sei);
            if (! ret) {
                dwExitCode = GetLastError();
                ShimErrorMessage(PROGNAME, dwExitCode);

            } else if (owait && sei.hProcess) {
                WaitForSingleObject(sei.hProcess, INFINITE);
                GetExitCodeProcess(sei.hProcess, &dwExitCode);
            }

            CoUninitialize();
            FreeLibrary(hShell32);

            if (owait) {
                ExitProcess(dwExitCode);
            }
            return ret ? EXIT_SUCCESS : EXIT_FAILURE;
        }
    }

    return EXIT_FAILURE;
}


static void
Usage()
{
    fputws(L"\nusage: " PROGNAME L" target\n", stderr);
    exit(EXIT_FAILURE);
}

//end
