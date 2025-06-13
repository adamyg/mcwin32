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

#include <Shlwapi.h>
#include <Shellapi.h>
#include <ObjBase.h>

#include <stdio.h>
#include "getopt.h"
#include "..\buildinfo.h"

#ifndef _countof        /*MSVC _countof()*/
#define _countof(__type) (sizeof(__type)/sizeof(__type[0]))
#endif
#if defined(__GNUC__)   /*BOOST_GCC_VERSION equiv*/
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#define __L(__t)        L##__t
#define _L(__t)         __L(__t)
#define PROGNAME        _L("mcstart")

#define EV_SUCCESS      0       // Success.
#define EV_SYNTAX       1       // Error in command line syntax.
#define EV_NOT_FOUND    2       // One of the files passed on the command line did not exist.
#define EV_NOTOOL       3       // A required tool could not be found.
#define EV_FAILURE      4       // The action failed.

#pragma comment(lib, "Shlwapi.lib")

typedef BOOL (WINAPI *ShellExecuteExW_t)(SHELLEXECUTEINFOW *);

static void Usage();
static const wchar_t *ExitvalueString(int rc);

static int StartAssociation(const wchar_t *cmd);
static int ShellAssociation(const wchar_t *cmd);
static BOOL IsOpenWith(const wchar_t *cmd);
static void QueryAssociation(const wchar_t* cmd);

static BOOL overbose = 0;
static BOOL owait = 0;
static BOOL ogui = 0;

#if defined(__MINGW32__)
extern "C"
#endif
int wmain(int argc, wchar_t *argv[])
{
    BOOL ocmd = 0;
    int ch;

    while (-1 != (ch = Updater::Getopt(argc, argv, "CGWvVh"))) {
        switch (ch) {
        case 'C':   // Command
            ocmd = 1;
            break;
        case 'G':   // GUI
            ogui = 1;
            break;
        case 'W':   // Wait
            owait = 1;
            break;
        case 'v':   // verbose
            overbose = 1;
            break;
        case 'V':   // Version
            fputws(PROGNAME _L(" ") _L(VERSION) _L(".") _L(BUILD_NUMBER) _L(" (") _L(BUILD_DATE) _L(")\n"), stderr);
            return EV_FAILURE;
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
        const wchar_t *argv0 = argv[0];

        if ((argv0[0] == '-' || argv0[0] == '/') &&
                (0 == wcscmp(argv0+1, L"?") || 0 == wcscmp(argv0+1, L"help"))) {
            Usage();

        } else {
            int rc;

            if (overbose) QueryAssociation(argv0);
            if (ogui || ! IsOpenWith(argv0)) {
                if (ocmd) {
                    rc = StartAssociation(argv0);
                } else {
                    rc = ShellAssociation(argv0);
                }
            } else {
                rc = EV_NOTOOL;
            }    
            if (overbose) fwprintf(stdout, L"result=%d (%ls)\n", rc, ExitvalueString(rc));
            return rc;
        }
    }

    //NOTREACHED
    return EV_FAILURE;
}


static void
Usage()
{
    const static wchar_t *usage[] = {
        L"",
        L"Usage: " PROGNAME L" [options] file|url",
        L"",
        L"Options:",
        L"    -C    Execute using cmd start, otherwise shell execute (default).",
        L"    -W    Wait for the child to exit.",
        L"    -G    Enable GUI prompts, including OpenWith.",
        L"    -V    Version/build information.",
        L"    -v    Verbose output.",
        L"    -h    Command line usage.",
        L"",
        L"Description:",
        L"    mcstart opens a file or URL in the user's preferred application. If a URL is",
        L"    provided the URL will be opened in the user's preferred web browser. If a file",
        L"    is provided the file will be opened in the preferred application for files of",
        L"    that type. mcstart supports any file with an assigned file-association.",
        L"    See ASSOC command for details.",
        L"",
        L"Exit Codes:",
        L"    An exit code of 0 indicates success while a non-zero exit code indicates",
        L"    failure. The following failure codes can be returned:",
        L"",
        L"    1 - Error in command line syntax, or help.",
        L"    2 - One of the files passed on the command line did not exist.",
        L"    3 - A required tool could not be found.",
        L"    4 - The action failed. ",
        L""
        };
    for (unsigned i = 0; i != _countof(usage); ++i) {
        fwprintf(stderr, L"  %ls\n", usage[i]);
    }
    exit(EV_SYNTAX);
}


static const wchar_t *
ExitvalueString(int rc)
{
    switch (rc)
    {
    case EV_SUCCESS:    return L"success";
    case EV_SYNTAX:     return L"command line syntax";
    case EV_NOT_FOUND:  return L"file not found";
    case EV_NOTOOL:     return L"no tool available";
    case EV_FAILURE:    return L"failure";
    }
    return L"undefined";
}


/*
 *  StartAssociation ---  
 *      Open the file/url using the command shell start verb.
 */
static int
StartAssociation(const wchar_t *argv0)
{
    //  START ["title"]
    //      [/D path] [/I] [/MIN] [/MAX] [/SEPARATE | /SHARED]
    //          [/LOW | /NORMAL | /HIGH | /REALTIME | /ABOVENORMAL | /BELOWNORMAL]
    //      /NODE <NUMA node>] [/AFFINITY <hex affinity mask>] [/WAIT] [/B]
    //          [command/program] [parameters]
    //
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION joli = {0};
    PROCESS_INFORMATION pi = {0};

    const wchar_t *comspec = _wgetenv(L"ComSpec");
    if (NULL == comspec || !*comspec) comspec = L"cmd";
    wchar_t cmd[1024];

    swprintf(cmd, _countof(cmd), L"%ls /C start %ls", comspec, argv0);
    if (overbose) fwprintf(stdout, L"CMD: %ls, %ls\n", comspec, cmd);

    if (! ShimCreateChild(&pi, L"start", comspec, cmd)) {
        const DWORD attrs = GetFileAttributesW(argv0);
        int rc = EV_FAILURE;

        if (attrs == INVALID_FILE_ATTRIBUTES ||
                (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            if (overbose) fwprintf(stderr, L"\n%ls: <%ls> not found\n", PROGNAME, argv0);
            rc = EV_NOT_FOUND;
        }
        return rc;
    }

    if (pi.hThread) {
        ResumeThread(pi.hThread);
        CloseHandle(pi.hThread);
    }

    if (pi.hProcess) {
        if (owait) {
            DWORD dwExitCode = 0;
            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &dwExitCode);
            if (overbose) fwprintf(stderr, L"\n%ls: rc=%u\n", PROGNAME, (unsigned)dwExitCode);
        }
        CloseHandle(pi.hProcess);
    }

    return EV_SUCCESS;
}


/*
 *  ShellAssociation ---
 *      Open the file/url using the ShellExecute interface.
 */
static int
ShellAssociation(const wchar_t *argv0)
{
    SHELLEXECUTEINFOW sei;
    HMODULE hShell32;
    ShellExecuteExW_t shellExecuteEx;
    int rc = EV_FAILURE;

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    hShell32 = LoadLibraryA("shell32.dll");
    if (hShell32 == NULL) {
        fwprintf(stderr, L"\n%ls: couldn't load shell32.dll\n", PROGNAME);
        return EV_FAILURE;
    }
    shellExecuteEx = (ShellExecuteExW_t) GetProcAddress(hShell32, "ShellExecuteExW");
    if (shellExecuteEx == NULL) {
        fwprintf(stderr, L"\n%ls: couldn't resolve ShellExecuteExW in shell32.dll\n", PROGNAME);
        FreeLibrary(hShell32);
        return EV_FAILURE;
    }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif

    if (overbose) fwprintf(stdout, L"SHELL: %ls\n", argv0);

    (void) CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    memset(&sei, 0, sizeof(sei));
    sei.cbSize = sizeof(sei);

    sei.fMask = SEE_MASK_NOCLOSEPROCESS|SEE_MASK_NO_CONSOLE;
    if (! ogui) sei.fMask |= SEE_MASK_FLAG_NO_UI;
        // SEE_MASK_NOCLOSEPROCESS
        //      Use to indicate that the hProcess member receives the process handle.
        // SEE_MASK_NO_CONSOLE
        //      inherit the parent's console for the new process instead of having it create a new console.
        // SEE_MASK_FLAG_NO_UI
        //      Do not display user interface (UI) error dialogs that would normally be presented.

    sei.lpVerb = L"open";
        // edit - Launches an editor and opens the document for editing.
        // open - Launches an application. If this file is not an executable file, its associated application is launched.

    sei.lpFile = argv0;
    sei.lpParameters = NULL;
    sei.lpDirectory = NULL;
    sei.nShow = SW_SHOWNORMAL;

    if (! shellExecuteEx(&sei)) {
        const DWORD dwExitCode = GetLastError();

        switch(dwExitCode) {
        case ERROR_FILE_NOT_FOUND:  // The specified file was not found.
        case ERROR_PATH_NOT_FOUND:  // The specified path was not found.
            rc = EV_NOT_FOUND;
            break;
        case ERROR_CANCELLED: {     // The function prompted the user for additional information, but the user canceled the request.
                const DWORD attrs = GetFileAttributesW(argv0);
                if (attrs == INVALID_FILE_ATTRIBUTES ||
                        (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                    rc = EV_NOT_FOUND;
                }
            }
            break;
        case ERROR_NO_ASSOCIATION:  // There is no application associated with the specified file name extension.
        case ERROR_DLL_NOT_FOUND:   // One of the library files necessary to run the application can't be found.
            rc = EV_NOTOOL;
            break;
        }
        ShimErrorMessage(PROGNAME, dwExitCode);

    } else {
        rc = EV_SUCCESS;
        if (sei.hProcess) {
            if (owait) {
                DWORD dwExitCode = 0;
                WaitForSingleObject(sei.hProcess, INFINITE);
                GetExitCodeProcess(sei.hProcess, &dwExitCode);
                if (overbose) fwprintf(stderr, L"\n%ls: rc=%u\n", PROGNAME, (unsigned)dwExitCode);
            }
            CloseHandle(sei.hProcess);
        }
    }

    CoUninitialize();
    FreeLibrary(hShell32);

    return rc;
}


/*
 *  IsOpenWith --- 
 *      Determine whether the association is the default 'OpenWith' command.
 */
static BOOL
IsOpenWith(const wchar_t* argv0)
{
    const wchar_t *ext = wcsrchr(argv0, '.');
    BOOL ret = FALSE;

    if (ext) {
        // Examples:
        //  COMMAND = C:\WINDOWS\system32\OpenWith.exe "%1"
        //  EXECUTABLE = C:\WINDOWS\system32\OpenWith.exe
        //
        wchar_t buffer[256];
        DWORD buflen = _countof(buffer);

        buffer[0] = 0;
        if (S_OK != AssocQueryStringW(ASSOCF_REMAPRUNDLL, ASSOCSTR_EXECUTABLE, ext, L"open", buffer, &buflen) || 0 == buflen) {
            buflen = _countof(buffer);
            AssocQueryStringW(ASSOCF_REMAPRUNDLL, ASSOCSTR_COMMAND, ext, L"open", buffer, &buflen);
        }
        ret = (wcsstr(buffer, L"OpenWith.exe") != NULL);
    }
    return ret;
}


/*
 *  QueryAssociation ---
 *      Export the association definition for the specified application.
 */
 static void
QueryAssociation(const wchar_t* cmd)
{
    const static struct assocstr {
        int id;
        const wchar_t* desc;
    } assocstr[] = {
#define __ASSOCSTR(__x)     { __x, L ## #__x },
#define ASSOCSTR(__x)       __ASSOCSTR(__x)

        ASSOCSTR(ASSOCSTR_COMMAND)
        ASSOCSTR(ASSOCSTR_EXECUTABLE)
        ASSOCSTR(ASSOCSTR_FRIENDLYDOCNAME)
        ASSOCSTR(ASSOCSTR_FRIENDLYAPPNAME)
        ASSOCSTR(ASSOCSTR_NOOPEN)
        ASSOCSTR(ASSOCSTR_SHELLNEWVALUE)
        ASSOCSTR(ASSOCSTR_DDECOMMAND)
        ASSOCSTR(ASSOCSTR_DDEIFEXEC)
        ASSOCSTR(ASSOCSTR_DDEAPPLICATION)
        ASSOCSTR(ASSOCSTR_DDETOPIC)
        ASSOCSTR(ASSOCSTR_INFOTIP)
        ASSOCSTR(ASSOCSTR_QUICKTIP)
#if defined(__WATCOMC__)
        ASSOCSTR(ASSOCSTR_TITLEINFO) // typo
#else
        ASSOCSTR(ASSOCSTR_TILEINFO)
#endif
        ASSOCSTR(ASSOCSTR_CONTENTTYPE)
        ASSOCSTR(ASSOCSTR_DEFAULTICON)
        ASSOCSTR(ASSOCSTR_SHELLEXTENSION)
        ASSOCSTR(ASSOCSTR_DROPTARGET)
        ASSOCSTR(ASSOCSTR_DELEGATEEXECUTE)
    };

    wchar_t buffer[256];
    DWORD buflen = _countof(buffer);
    const wchar_t *ext = wcsrchr(cmd, '.');
    HRESULT hres;

    if (ext) {
        for (unsigned k = 0; k < _countof(assocstr); ++k) {
            buffer[0] = 0;
            hres = AssocQueryStringW(ASSOCF_REMAPRUNDLL, (ASSOCSTR)assocstr[k].id, ext, L"open", buffer, &buflen);
            if (S_OK == hres) {
                wprintf(L"AS_%ls=%ls\n", assocstr[k].desc + 9, buffer);
            }
        }
    }
}

//end
