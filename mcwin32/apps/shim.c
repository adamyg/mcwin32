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

#ifndef _countof
#define _countof(__type) (sizeof(__type)/sizeof(__type[0]))
#endif
#if defined(__WATCOMC__)
#define wcsdup(__x) _wcsdup(__x)
#endif

BOOL WINAPI CtrlHandler(DWORD ctrlType);

BOOL WINAPI
CtrlHandler(DWORD ctrlType)
{
    switch (ctrlType) {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        // forward to child
        return TRUE;
    default:
        return FALSE;
    }
}


static BOOL
Diagnostics(void)
{
    const char *mcshim_diagositics = getenv("MCSHIM_DIAGNOSTICS"); // optional runtime diagnostics
    return (mcshim_diagositics && mcshim_diagositics[0] && mcshim_diagositics[0] != '0'); // non-zero
}


static wchar_t *
Basename(wchar_t *path)
{
    wchar_t *base = path;

    for (;*path; ++path) {
        if (*path == '\\' || *path == '/') { // path delimiter
            base = path + 1; // next element
        }
    }
    return base;
}


static BOOL
CreateChild(PROCESS_INFORMATION *ppi, const wchar_t *name, const wchar_t *path, wchar_t *cmdline)
{
    STARTUPINFOW si = {0};

    GetStartupInfoW(&si); // process information

    if (! CreateProcessW(path, cmdline, NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &si, ppi)) {
        const DWORD error = GetLastError();
        wchar_t *message = NULL;

        if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &message, 0, NULL) && message) {

            wchar_t *nl, *rt; // trailing newline/return

            nl = wcsrchr(message, '\n'); // trim
            rt = wcsrchr(message, '\r'); // trim

            if (nl) *nl = 0;
            if (rt) *rt = 0;

            wprintf(L"%s: unable to execute child : %s (0x%08lx)\n", name, message, error);

        } else {
            wprintf(L"%s: unable to execute child : 0x%08lx.\n", name, error);
        }

        LocalFree(message);
        return FALSE;
    }
    return TRUE;
}


static wchar_t orgpath[1024] = {0};
static wchar_t newpath[1024] = {0};

/*
 *  ApplicationShim - application execution redirect shim.
 *
 *  Parameters:
 *      name - Application name.
 *      alias - Replacement application name, same path assumed.
 *
 *  Returns:
 *      On success no-return, otherwise returns on error.
 */
void
ApplicationShim(const wchar_t *name, const wchar_t *alias)
{
    const BOOL diagositics = Diagnostics(); // optional runtime diagnostics
    const unsigned aliassz = wcslen(alias), // alias length, in characters.
        pathsz = GetModuleFileNameW(NULL, orgpath, _countof(orgpath)); // fully qualified path.
    wchar_t *cmdline = wcsdup(GetCommandLineW()); // original command line, cloned.
    wchar_t *base;

    HANDLE job = INVALID_HANDLE_VALUE;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION joli = {0};
    PROCESS_INFORMATION pi = {0};
    DWORD excode = 0;

    setvbuf(stdout, NULL, _IONBF, 0); // disable buffering

    // build path
    if (pathsz >= _countof(orgpath)) {
        wprintf(L"%s: command line too long.\n", name);
        return;
    }

    if (diagositics) {
        wprintf(L"ORG: %s\n", orgpath);
        wprintf(L"CMD: %s\n", cmdline);
    }

    wmemcpy(newpath, orgpath, pathsz);
    if (NULL == (base = Basename(newpath)) || !*base) {
        wprintf(L"%s: command line invalid.\n", name);
        return;

    } else if ((base + aliassz) >= (newpath + _countof(newpath))) {
        wprintf(L"%s: command line too long.\n", name);
        return;
    }
    wmemcpy(base, alias, aliassz + 1 /*NUL*/);

        //wmemcpy(newpath, L".\\testapp.exe", 13 + 1);
    if (diagositics) {
        wprintf(L"NEW: %s\n", newpath);
    }

    // execute child
    job = CreateJobObject(NULL, NULL);
    joli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;
    SetInformationJobObject(job, JobObjectExtendedLimitInformation, &joli, sizeof(joli));

    if (! CreateChild(&pi, name, newpath, cmdline)) {
        return;
    }

    // redirect signals and monitor termination
    SetConsoleCtrlHandler(CtrlHandler, TRUE);
    AssignProcessToJobObject(job, pi.hProcess);
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &excode);
    CloseHandle(pi.hProcess);
    CloseHandle(job);

    ExitProcess(excode);
    //no-return
}

//end
