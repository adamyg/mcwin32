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

#include <stdio.h>
#include <stdlib.h>

#ifndef _countof
#define _countof(__type) (sizeof(__type)/sizeof(__type[0]))
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
    const char *mcshim_diagositics = getenv("LIBSHIM_DIAGNOSTICS"); // optional runtime diagnostics
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


int
ShimCreateChild(PROCESS_INFORMATION *ppi, const wchar_t *name, const wchar_t *path, const wchar_t *cmdline)
{
    wchar_t *t_cmdline = _wcsdup(cmdline); // command line, cloned.
    STARTUPINFOW si = {0};

    GetStartupInfoW(&si); // process information

    if (NULL == t_cmdline ||
            ! CreateProcessW(path, t_cmdline, NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &si, ppi)) {

        ShimErrorMessageEx(name, path, GetLastError());
        return FALSE;
    }
    return TRUE;
}


void
ShimErrorMessage(const wchar_t *name, DWORD wrc)
{
    ShimErrorMessageEx(name, NULL, wrc);
}


void
ShimErrorMessageEx(const wchar_t *name, const wchar_t *path, DWORD wrc)
{
    wchar_t *message = NULL;

    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, wrc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &message, 0, NULL) && message) {

        wchar_t *nl, *rt; // trailing newline/return

        nl = wcsrchr(message, '\n'); // trim
        rt = wcsrchr(message, '\r'); // trim

        if (nl) *nl = 0;
        if (rt) *rt = 0;
    }

    if (path) {
        wprintf(L"%ls: unable to execute child <%ls>: %ls (0x%08lx)\n",
            name, path, (message ? message : L""), wrc);
    } else {
        wprintf(L"%ls: unable to execute child: %ls (0x%08lx)\n",
            name, (message ? message : L""), wrc);
    }

    LocalFree(message);
}

static wchar_t orgpath[1024] = {0};
static wchar_t newpath[1024] = {0};

/*
 *  ApplicationShim -
 *      application execution redirect shim.
 *
 *  ApplicationShim0 -
 *      application shim replacing the first command-line argument with 'name'.
 *
 *  ApplicationShimCmd -
 *      shim with an explicit command-line.
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
    ApplicationShimCmd(name, alias, GetCommandLineW());
}


void
ApplicationShim0(const wchar_t *name, const wchar_t *alias)
{
    const BOOL diagositics = Diagnostics(); // optional runtime diagnostics
    const wchar_t *cmdline = GetCommandLineW();
    wchar_t *ncmdline = NULL, *arg1 = NULL;
    size_t ocmdlen = 0, ncmdlen = 0;

    if (diagositics) {
        wprintf(L"ARG:  %ls\n", cmdline);
    }

    if (cmdline[0] == '"' || cmdline[0] == '\'') { // quoted arg0
        arg1 = wcschr(cmdline + 1, cmdline[0]);
        if (NULL == arg1) {
            wprintf(L"%ls: application name invalid <%ls>.\n", name, cmdline);
            return;
        }
        arg1++; // consume closing quote

    } else {
        arg1 = wcspbrk(cmdline, L" \t"); // first whitespace
    }

    if (arg1) {
        while (*arg1 == ' ' || *arg1 == '\t') {
            ++arg1; // consume whitespace
        }
    }

    ocmdlen = (arg1 ? wcslen(arg1) + 1 : 0);
    ncmdlen = wcslen(name) + ocmdlen + 1;

    if (NULL == (ncmdline = (wchar_t *) calloc(ncmdlen, sizeof(wchar_t)))) {
        wprintf(L"%ls: memory allocation error.\n", name);
        return;
    }

    wcscpy_s(ncmdline, ncmdlen, name);
    if (arg1) {
        wcscat_s(ncmdline, ncmdlen, L" ");
        wcscat_s(ncmdline, ncmdlen, arg1);
    }

    if (diagositics) {
        wprintf(L"NARG: %ls\n", ncmdline);
    }

    ApplicationShimCmd(name, alias, ncmdline);
}


void
ApplicationShimCmd(const wchar_t *name, const wchar_t *alias, const wchar_t *cmdline)
{
    const BOOL diagositics = Diagnostics(); // optional runtime diagnostics
    const DWORD aliassz = (DWORD) wcslen(alias), // alias length, in characters.
        pathsz = GetModuleFileNameW(NULL, orgpath, _countof(orgpath)); // fully qualified path.
    wchar_t *base;

    HANDLE job = INVALID_HANDLE_VALUE;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION joli = {0};
    PROCESS_INFORMATION pi = {0};
    DWORD excode = 0;

    setvbuf(stdout, NULL, _IONBF, 0); // disable buffering

    // build path
    if (pathsz >= _countof(orgpath)) {
        wprintf(L"%ls: command line too long.\n", name);
        return;
    }

    if (diagositics) {
        wprintf(L"ORG:  %ls\n", orgpath);
        wprintf(L"CMD:  %ls\n", cmdline);
    }

    wmemcpy(newpath, orgpath, pathsz);
    if (NULL == (base = Basename(newpath)) || !*base) {
        wprintf(L"%ls: command line invalid.\n", name);
        return;

    } else if ((base + aliassz) >= (newpath + _countof(newpath))) {
        wprintf(L"%ls: command line too long.\n", name);
        return;
    }
    wmemcpy(base, alias, (size_t)aliassz + 1 /*NUL*/);

    if (diagositics) {
        wprintf(L"NEW:  %ls\n", newpath);
    }

    // execute child
#if defined(_MSC_VER)
#pragma warning(disable:6387) // handle maybe 0
#endif

    job = CreateJobObject(NULL, NULL);
    joli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;
    SetInformationJobObject(job, JobObjectExtendedLimitInformation, &joli, sizeof(joli));

    if (! ShimCreateChild(&pi, name, newpath, cmdline)) {
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
