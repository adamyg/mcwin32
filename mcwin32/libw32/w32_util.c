#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_util_c,"$Id: w32_util.c,v 1.23 2025/04/01 16:15:15 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 util unix functionality.
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
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
 * license for more details.
 * ==end==
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#include "win32_internal.h"
#include "win32_child.h"
#include "win32_misc.h"

#include <unistd.h>
#include <wchar.h>
#include <assert.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shfolder.lib")
#include <shlobj.h>                             /* SHGetFolderPath */


/*
 *  w32_getshell ---
 *      Retrieve the default shell.
 */

LIBW32_API const char *
w32_getshell(void)
{
    return w32_getshellA();
}


LIBW32_API const char *
w32_getshellA(void)
{
    const char *shname;

    shname = getenv("SHELL");
    if (shname == NULL)
        shname = getenv("COMSPEC");
    if (shname == NULL)
        shname = getenv("ComSpec");
    if (shname == NULL) {
#if defined(_MSC_VER)
#pragma warning(disable:28159)                  // GetVersion() usage
#endif
        if (GetVersion() < 0x80000000) {
            shname = "CMD.EXE";                 // Windows NT/2000/XP
        }
        shname = "COMMAND.EXE";                 // ... others
    }
    return shname;
}


LIBW32_API const wchar_t *
w32_getshellW(void)
{
    const wchar_t *shname;

    shname = _wgetenv(L"SHELL");
    if (shname == NULL)
        shname = _wgetenv(L"COMSPEC");
    if (shname == NULL)
        shname = _wgetenv(L"ComSpec");
    if (shname == NULL) {
#if defined(_MSC_VER)
#pragma warning(disable:28159)                  // GetVersion() usage
#endif
        if (GetVersion() < 0x80000000) {
            shname = L"CMD.EXE";                // Windows NT/2000/XP
        }
        shname = L"COMMAND.EXE";                // ... others
    }
    return shname;
}


/*
 *  w32_gethome ---
 *      Retrieve the default home directory.
 */

LIBW32_API const char*
w32_gethome(int ignore_env)
{
#if defined(UTF8FILENAMES)
    static const char *x_home = NULL;

    if (w32_utf8filenames_state()) {
        const wchar_t *wpath;

        if (x_home) {                           // cached
            return x_home;
        }

        if (NULL != (wpath = w32_gethomeW(ignore_env))) {
            if (NULL != (x_home = w32_wc2utfa(wpath, NULL))) {
                return x_home;
            }
        }
     }
#endif  //UTF8FILENAMES

    return w32_gethomeA(ignore_env);
}


LIBW32_API const char *
w32_gethomeA(int ignore_env)
{
    static const char *x_home = NULL;

    if (NULL == x_home) {
        char t_path[MAX_PATH];
        const char *env;
        int len, done = FALSE;

        // <HOME>
        if (!ignore_env && (env = getenv("HOME")) != NULL && (len = (int)strlen(env)) > 0) {
            t_path[sizeof(t_path) - 1] = 0;
            if (0 == _access(t_path, 0)) {
                t_path[len+1] = 0;
                done = TRUE;
            }
        }

        // Personal settings
        //  o XP
        //      X:/Documents and Settings/<user>/home/
        //      X:/Documents and Settings/<user>/
        //
        //  o Windows7+
        //      X:/Users/<user>/home/
        //      X:/Users/<user>/
        //
        if (! done) {
            if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, t_path)) &&
                        (len = (int)strlen(t_path)) > 0) {
                t_path[sizeof(t_path) - 1] = 0;
                if (0 == _access(t_path, 0)) {
                    _snprintf(t_path + len, sizeof(t_path) - len, "/home/");
                    t_path[sizeof(t_path) - 1] = 0;
                    if (0 == _access(t_path, 0)) {
                        len += 6;
                    }
                    done = TRUE;
                }
            }
        }

        // <USERPROFILE>
        if (! done) {
            if ((env = getenv("USERPROFILE")) != NULL && (len = (int)strlen(env)) > 0) {
                t_path[sizeof(t_path) - 1] = 0;
                if (0 == _access(t_path, 0)) {
                    _snprintf(t_path + len, sizeof(t_path) - len, "/home/");
                    t_path[sizeof(t_path) - 1] = 0;
                    if (0 == _access(t_path, 0)) {
                        len += 6;
                    }
                    done = TRUE;
                }
            }
        }

        // completion
        if (done) {
            if (len <= (int)sizeof(t_path)) {
                if ('/' != t_path[len - 1] && '\\' != t_path[len - 1]) {
                    t_path[len++] = '/';
                    t_path[len] = 0;
                }
            }
            w32_dos2unixA(t_path);
        }

        x_home = WIN32_STRDUP(done ? t_path : "c:/");
    }
    return x_home;
}


LIBW32_API const wchar_t *
w32_gethomeW(int ignore_env)
{
    static const wchar_t *x_home = NULL;

    if (NULL == x_home) {
        wchar_t t_path[MAX_PATH];
        const wchar_t *env;
        int len, done = FALSE;

        // <HOME>
        if (!ignore_env && (env = _wgetenv(L"HOME")) != NULL && (len = (int)wcslen(env)) > 0) {
            t_path[_countof(t_path) - 1] = 0;
            if (0 == _waccess(t_path, 0)) {
                t_path[len+1] = 0;
                done = TRUE;
            }
        }

        // Personal settings
        //  o XP
        //      X:/Documents and Settings/<user>/home/
        //      X:/Documents and Settings/<user>/
        //
        //  o Windows7+
        //      X:/Users/<user>/home/
        //      X:/Users/<user>/
        //
        if (! done) {
            if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, t_path)) &&
                        (len = (int)wcslen(t_path)) > 0) {
                t_path[_countof(t_path) - 1] = 0;
                if (0 == _waccess(t_path, 0)) {
                    _snwprintf(t_path + len, _countof(t_path) - len, L"/home/");
                    t_path[_countof(t_path) - 1] = 0;
                    if (0 == _waccess(t_path, 0)) {
                        len += 6;
                    }
                    done = TRUE;
                }
            }
        }

        // <USERPROFILE>
        if (! done) {
            if ((env = _wgetenv(L"USERPROFILE")) != NULL && (len = (int)wcslen(env)) > 0) {
                t_path[_countof(t_path) - 1] = 0;
                if (0 == _waccess(t_path, 0)) {
                    _snwprintf(t_path + len, _countof(t_path) - len, L"/home/");
                    t_path[_countof(t_path) - 1] = 0;
                    if (0 == _waccess(t_path, 0)) {
                        len += 6;
                    }
                    done = TRUE;
                }
            }
        }

        // completion
        if (done) {
            if (len <= (int)_countof(t_path)) {
                if ('/' != t_path[len - 1] && '\\' != t_path[len - 1]) {
                    t_path[len++] = '/';
                    t_path[len] = 0;
                }
            }
            w32_dos2unixW(t_path);
        }

        x_home = WIN32_STRDUPW(done ? t_path : L"c:/");
    }
    return x_home;
}


LIBW32_API int
w32_utf2wc(const char *src, wchar_t *dest, size_t maxlen)
{
    int ret;

    assert(src), assert(dest), assert(maxlen);
    if (NULL == src || NULL == dest) {
        errno = EFAULT;
        return -1;
    }

    dest[0] = 0;
    if ((ret = MultiByteToWideChar(CP_UTF8, 0, src, -1, dest, (int)maxlen)) > 0) {
        assert(ret <= (int)maxlen);
        if (ret == (int)maxlen)
            dest[maxlen - 1] = 0;

    } else {
        const DWORD rc = GetLastError();

        if (dest) dest[0] = 0;
        switch (rc) {
        case ERROR_INVALID_FLAGS:
        case ERROR_INVALID_PARAMETER:
            errno = EINVAL;
            break;
        case ERROR_INSUFFICIENT_BUFFER:
            errno = ENAMETOOLONG;
            break;
        case ERROR_NO_UNICODE_TRANSLATION:
        default:
            errno = ENOENT;
            break;
        }
        return -1;
    }
    return ret;
}


// If successful, returns the number of bytes including NULL required.
LIBW32_API int
w32_utf2wcl(const char *src)
{
    int ret;

    assert(src);
    if (NULL == src) {
        errno = EFAULT;
        return -1;
    }

    if ((ret = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0)) > 0) {
        return ret;
    } else {
        const DWORD rc = GetLastError();
        switch (rc) {
        case ERROR_INVALID_FLAGS:
        case ERROR_INVALID_PARAMETER:
            errno = EINVAL;
            break;
        case ERROR_INSUFFICIENT_BUFFER:
            errno = ENAMETOOLONG;
            break;
        case ERROR_NO_UNICODE_TRANSLATION:
        default:
            errno = ENOENT;
            break;
        }
    }
    return -1;
}


LIBW32_API wchar_t *
w32_utf2wca(const char *src, size_t *len)
{
    int ret;

    assert(src);
    if ((ret = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0)) > 0) {
        wchar_t *dest = malloc(sizeof(wchar_t) * ret);
        if (dest) {
            (void) MultiByteToWideChar(CP_UTF8, 0, src, -1, dest, ret);
            if (len) *len = ret;
            return dest;
        }

    } else {
        const DWORD rc = GetLastError();
        switch (rc) {
        case ERROR_INVALID_FLAGS:
        case ERROR_INVALID_PARAMETER:
            errno = EINVAL;
            break;
        case ERROR_INSUFFICIENT_BUFFER:
            errno = ENAMETOOLONG;
            break;
        case ERROR_NO_UNICODE_TRANSLATION:
        default:
            errno = ENOENT;
            break;
        }
    }
    return NULL;
}


LIBW32_API int
w32_wc2utf(const wchar_t *src, char *dest, size_t maxlen)
{
    int ret;

    assert(src), assert(dest), assert(maxlen);
    if (NULL == src || NULL == dest) {
        errno = EFAULT;
        return -1;
    }

    dest[0] = 0;
    if ((ret = WideCharToMultiByte(CP_UTF8, 0, src, -1, dest, (int)maxlen, NULL, NULL)) > 0) {
        assert(ret <= (int)maxlen);
        if (ret == (int)maxlen) {
            dest[maxlen - 1] = 0;
            --ret;
        }

    } else {
        const DWORD rc = GetLastError();

        if (dest) dest[0] = 0;
        switch (rc) {
        case ERROR_INVALID_FLAGS:
        case ERROR_INVALID_PARAMETER:
            errno = EINVAL;
            break;
        case ERROR_INSUFFICIENT_BUFFER:
            errno = ENAMETOOLONG;
            break;
        case ERROR_NO_UNICODE_TRANSLATION:
        default:
            errno = ENOENT;
            break;
        }
        return -1;
    }
    return ret;
}


LIBW32_API char *
w32_wc2utfa(const wchar_t *src, size_t *len)
{
    int ret;

    assert(src);
    if (NULL == src) {
        errno = EFAULT;
        return NULL;
    }

    if ((ret = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL)) > 0) {
        char *dest = malloc(sizeof(char) * ret);
        if (dest) {
            WideCharToMultiByte(CP_UTF8, 0, src, -1, dest, ret, NULL, NULL);
            if (len) *len = ret;
            return dest;
        }

    } else {
        const DWORD rc = GetLastError();

        switch (rc) {
        case ERROR_INVALID_FLAGS:
        case ERROR_INVALID_PARAMETER:
            errno = EINVAL;
            break;
        case ERROR_INSUFFICIENT_BUFFER:
            errno = ENAMETOOLONG;
            break;
        case ERROR_NO_UNICODE_TRANSLATION:
        default:
            errno = ENOENT;
            break;
        }
    }
    return NULL;
}


LIBW32_API char *
w32_extendedpathA(const char *path)
{
    if (NULL == path || !*path)
        return NULL;

    if (path[1] == ':' && isalpha((unsigned char)path[0]) &&
                ISSLASH(path[2])) {             // abs path, "X:/"
        //
        //  Utilise extended format; long-path(259+) and/or trailing dot/space
        //
        const size_t len = strlen(path);
        const char eol = path[len - 1];

        if (len >= MAX_PATH || eol == '.' || eol == ' ') {
            char *expath;

            if (NULL != (expath = malloc((len + 5) * sizeof(char)))) {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:6386) // buffer overflow
#endif
                memcpy(expath, "\\\\?\\", 4);
                memcpy(expath + 4, path, len + 1 /*NUL*/);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
                w32_unix2dosA(expath + 4);
                return expath;
            }
        }

    } else if (ISSLASH(path[0]) && ISSLASH(path[1]) && path[2] == '?') {
        //
        //  explicit prefix, unix2dos required
        //
        const size_t tlen = strlen(path) + 1;
        char *expath;

        if (NULL != (expath = malloc(tlen * sizeof(char)))) {
            memcpy(expath, path, tlen);

            w32_unix2dosA(expath + 4);
            return expath;
        }
    }
    return NULL;
}


LIBW32_API wchar_t *
w32_extendedpathW(const wchar_t *path)
{
    if (NULL == path || !*path)
        return NULL;

    if (path[1] == ':' && iswalpha(path[0]) &&
                ISSLASH(path[2])) {             // abs path, "X:/"
        //
        //  Utilise extended format; long-path(259+) and/or trailing dot/space
        //
        const size_t len = wcslen(path);
        const wchar_t eol = path[len - 1];

        if (len >= MAX_PATH || eol == '.' || eol == ' ') {
            wchar_t *expath;

            if (NULL != (expath = malloc((len + 5) * sizeof(wchar_t)))) {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:6386) // buffer overflow
#endif
                wmemcpy(expath, L"\\\\?\\", 4);
                wmemcpy(expath + 4, path, len + 1 /*NUL*/);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
                w32_unix2dosW(expath + 4);
                return expath;
            }
        }

    } else if (ISSLASH(path[0]) && ISSLASH(path[1]) && path[2] == '?') {
        //
        //  explicit prefix, unix2dos required
        //
        const size_t tlen = wcslen(path) + 1;
        wchar_t *expath;

        if (NULL != (expath = malloc(tlen * sizeof(wchar_t)))) {
            wmemcpy(expath, path, tlen);
            w32_unix2dosW(expath);

            return expath;
        }            
    }
    return NULL;
}


//  int
//  w32_is64bit(void)
//  {
//      int arch = 32;
//
//      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
//              _T("SYSTEM\CurrentControlSet\\Control\\Session Manager\\Environment"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
//          LPSTR szArch[100] = {0}:
//
//          if (RegQueryValueEx(hKey, _T("PROCESSOR_ARCHITECTURE"), NULL, NULL, (LPBYTE)szArch, &dwSize) == ERROR_SUCCESS) {
//              if (0 == strcmp(szArch, "AMD64"))
//                  arch = 64;
//              else
//                  arch = 32;
//          } else {
//              arch = (sizeof(PVOID) == 4 ? 32 : 64);
//          }
//          RegCloseKey(hKey);
//      }
//      return arch;
//  }


/*
 *  w32_filenamecmpA --- 
 *      Lexically compares two filenames to determine whether one is less than, equal to, or greater than the other.   
 * 
 *      Comparison assumes case-insensitivity and forward/backwards slashes are equal. 
 */

LIBW32_API int
w32_filenamecmpA(const char *f1, const char *f2, unsigned flags)
{
    if (f1 != f2) {
        size_t idx = 0;

        for (;;++idx) {
            unsigned char c1 = *((const unsigned char *)f1),
                c2 = *((const unsigned char *)f2);

            if (ISSLASH(c1) && ISSLASH(f1[1])) {
                if (idx) ++f1;                  // compress slashes; except leading, UNC
            }

            if (ISSLASH(c2) && ISSLASH(f2[1])) {
                if (idx) ++f2;                  // compress slashes; except leading, UNC
            }

            if (c1 != c2) {                     // difference

                if (0 == (flags & FNCMP_CASE_SENSITIVE)) {
                    c1 = tolower(c1);
                    c2 = tolower(c2);
                }

                if (c1 != c2) {
                    if (!(ISSLASH(c1) && ISSLASH(c2))) {

                        if ((flags & FNCMP_FILENAME) == 0) {
                                                // trailing slash optional
                            if (0 == c1 && ISSLASH(c2) && 0 == f2[1]) {
                                return 0;
                            }
                            if (0 == c2 && ISSLASH(c1) && 0 == f1[1]) {
                                return 0;
                            }
                        }
                        return (c1 - c2);
                    }
                }
            }

            if (c1 == '\0') {
                break;                          // EOS
            }

            ++f1, ++f2;
        }
    }
    return 0;                                   // match
}


/*
 *  w32_filenamecmpW --- 
 *      Lexically compares two filenames to determine whether one is less than, equal to, or greater than the other.
 *
 *      Comparison assumes case-insensitivity and forward/backwards slashes are equal.
 */

LIBW32_API int
w32_filenamecmpW(const wchar_t *f1, const wchar_t *f2, unsigned flags)
{
    if (f1 != f2) {
        size_t idx = 0;

        for (;;++idx) {
            wchar_t c1 = *f1, c2 = *f2;

            if (ISSLASH(c1) && ISSLASH(f1[1])) {
                if (idx) ++f1;                  // compress slashes; except leading, UNC
            }

            if (ISSLASH(c2) && ISSLASH(f2[1])) {
                if (idx) ++f2;                  // compress slashes; except leading, UNC
            }

            if (c1 != c2) {                     // difference

                if (0 == (flags & FNCMP_CASE_SENSITIVE)) {
                    c1 = towlower(c1);
                    c2 = towlower(c2);
                }

                if (c1 != c2) {
                    if (!(ISSLASH(c1) && ISSLASH(c2))) {

                        if ((flags & FNCMP_FILENAME) == 0) {
                                                // trailing slash optional
                            if (0 == c1 && ISSLASH(c2) && 0 == f2[1]) {
                                return 0;
                            }

                            if (0 == c2 && ISSLASH(c1) && 0 == f1[1]) {
                                return 0;
                            }
                        }
                        return (c1 - c2);
                    }
                }
            }

            if (c1 == '\0') {
                break;                          // EOS
            }

            ++f1, ++f2;
        }
    }
    return 0;                                   // match
}


LIBW32_API char *
w32_dos2unixA(char *path)
{
    if (path) {
        char *p;
        for (p = path; *p; ++p) {
             if ('\\' == *p) *p = '/';          /* DOS<>Unix */
        }
    }
    return path;
}


LIBW32_API wchar_t *
w32_dos2unixW(wchar_t *path)
{
    if (path) {
        wchar_t *p;
        for (p = path; *p; ++p) {
             if ('\\' == *p) *p = '/';          /* DOS<>Unix */
        }
    }
    return path;
}


LIBW32_API char *
w32_unix2dosA(char *path)
{
    if (path) {
        char *p;
        for (p = path; *p; ++p) {
            if ('/' == *p) *p = '\\';           /* Unix<>DOS */
        }
    }
    return path;
}


LIBW32_API wchar_t *
w32_unix2dosW(wchar_t *path)
{
    if (path) {
        wchar_t *p;
        for (p = path; *p; ++p) {
            if ('/' == *p) *p = '\\';           /* Unix<>DOS */
        }
    }
    return path;
}


LIBW32_API const char *
w32_strslashA(const char *path)
{
    if (path) {
        for (;*path; ++path) {
            if (ISSLASH(*path)) {
                return path;
            }
        }
    }
    return NULL;
}


LIBW32_API const wchar_t *
w32_strslashW(const wchar_t *path)
{
    if (path) {
        for (;*path; ++path) {
            if (ISSLASH(*path)) {
                return path;
            }
        }
    }
    return NULL;
}


LIBW32_API enum w32ostype
w32_ostype(void)
{
    static int platform = 0;

    if (! platform) {
        OSVERSIONINFO ovi = {0};
        ovi.dwOSVersionInfoSize = sizeof(ovi);

        GetVersionEx(&ovi);             // dependent on app-manifest
        switch (ovi.dwPlatformId) {
        case VER_PLATFORM_WIN32s:
        case VER_PLATFORM_WIN32_WINDOWS:
            platform = OSTYPE_WIN_95;
            break;
#if defined(VER_PLATFORM_WIN32_CE)
        case VER_PLATFORM_WIN32_CE:
            platform = OSTYPE_WIN_CE;
            break;
#endif
        case VER_PLATFORM_WIN32_NT:
            //  Operating system        Version                     Other
            //                          Number      Major   Minor
            //
            //  Windows 11              10.0*       10      0       OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
            //  Windows 10              10.0*       10      0       OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
            //  Windows Server 2016     10.0*       10      0       OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
            //  Windows 8.1             6.3*        6       3       OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
            //  Windows Server 2012 R2  6.3*        6       3       OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
            //  8                       6.2         6       2       OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
            //  Server 2012             6.2         6       2       OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
            //  7                       6.1         6       1       OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
            //  Server 2008 R2          6.1         6       1       OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
            //  Server 2008             6.0         6       0       OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
            //  Vista                   6.0         6       0       OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
            //  Server 2003 R2          5.2         5       2       GetSystemMetrics(SM_SERVERR2) != 0
            //  Home Server             5.2         5       2       OSVERSIONINFOEX.wSuiteMask & VER_SUITE_WH_SERVER
            //  Server 2003             5.2         5       2       GetSystemMetrics(SM_SERVERR2) == 0
            //  XP Professional x64     5.2         5       2       (OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION) &&
            //                                                      (SYSTEM_INFO.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
            //  XP                      5.1         5       1       Not applicable
            //  2000                    5.0         5       0       Not applicable
            //
            //      * For applications that have been manifested for Windows 8.1 or Windows 10.  Applications not manifested
            //        for Windows 8.1 or Windows 10 will return the Windows 8 OS version value (6.2). To manifest your applications
            //        for Windows 8.1 or Windows 10, refer to Targeting your application for Windows.
            //
            platform = OSTYPE_WIN_NT;           // or 2000

            if (ovi.dwMajorVersion >= 10) {
                if (ovi.dwBuildNumber >= 22000) {
                    platform = OSTYPE_WIN_11;   // Windows 11
                } else {
                    platform = OSTYPE_WIN_10;   // Windows 10
                }

            } else if (6 == ovi.dwMajorVersion) {
                platform = OSTYPE_WIN_VISTA;
                if (ovi.dwMajorVersion >= 2) {
                    platform = OSTYPE_WIN_8;    // or Server 2012

                } else if (1 == ovi.dwMajorVersion) {
                    platform = OSTYPE_WIN_7;    // or Server 2008 R2
                }
            }
            break;

        default:
            platform = OSTYPE_WIN_NT;
            break;
        }
    }
    return platform;
}


LIBW32_API int
w32_getexedir(char *buf, int maxlen)
{
    if (GetModuleFileNameA(NULL, buf, maxlen)) {
        const int len = (int)strlen(buf);
        char *cp;

        for (cp = buf + len; (cp > buf) && (*cp != '\\'); cp--)
            /*cont*/;
        if ('\\' == *cp) {
            cp[1] = '\0';                       // remove program
            return (int)((cp - buf) + 1);
        }
        return len;
    }
    return -1;
}


LIBW32_API const char *
w32_syserrorA(DWORD dwError, char *buf, size_t buflen)
{
    return w32_vsyserrorA(dwError, buf, buflen, NULL);
}


LIBW32_API const char *
w32_vsyserrorA(DWORD dwError, char *buf, size_t buflen, ...)
{
    if (buf && buflen > 0) {
        const DWORD ret = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, dwError,
                                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, (DWORD)(--buflen) /*nul*/, NULL);
        if (0 == ret) {                         // error, overflow etc
            size_t len;

            if ((len = buflen) > sizeof("unknown error")) {
                len = sizeof("unknown error");
            }
            memcpy(buf, "unknown error", len * sizeof(char));

        } else {
            size_t len = (size_t)ret; 
            const char *arg;
            va_list ap;
            int idx = 1; 

            // remove trailing whitespace
            while (--len) {
                const char ch = buf[len];
                if (ch == ' ' || ch == '.' || ch == '\n' || ch == '\r') {
                    continue; // consume
                }
                break; // done
            }
            buf[++len] = 0;

            // replace %1...x
            va_start(ap, buflen);
            while (NULL != (arg = va_arg(ap, const char *)) && idx <= 9 && len < buflen) {
                char *cursor = buf;

                while (NULL != (cursor = strchr(cursor, '%'))) {
                    if (cursor[1] == ('0' + idx)) {
                        size_t arglen = strlen(arg);

                        len -= 2; // %x being replace
                        if ((len + arglen) >= buflen) {
                            arglen = buflen - len; // overflow, truncate
                        }
                        memmove(cursor + arglen, cursor + 2, strlen(cursor + 2) + 1 /*nul*/);
                        memcpy(cursor, arg, arglen);
                        len += arglen;
                        break;
                    }

                    if ('%' == *++cursor) {
                        ++cursor; // %%
                    }
                }
                ++idx;
            }
            va_end(ap);
        }
        return buf;
    }
    return NULL;
}


LIBW32_API const wchar_t *
w32_syserrorW(DWORD dwError, wchar_t *buf, size_t buflen)
{
    return w32_vsyserrorW(dwError, buf, buflen, NULL);
}


LIBW32_API const wchar_t *
w32_vsyserrorW(DWORD dwError, wchar_t *buf, size_t buflen, ...)
{
    if (buf && buflen > 0) {
        const DWORD ret = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, dwError,
                                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, (DWORD)(--buflen) /*nul*/,  NULL);
        if (0 == ret) {                         // error, overflow etc
            size_t len;

            if ((len = buflen) > sizeof("unknown error")) {
                len = sizeof("unknown error");
            }
            memcpy(buf, L"unknown error", len * sizeof(wchar_t));

        } else {
            size_t idx = 1, len = (size_t)ret;
            const wchar_t *arg;
            va_list ap;

            // remove trailing whitespace
            while (--len) {
                const wchar_t ch = buf[len];
                if (ch == ' ' || ch == '.' || ch == '\n' || ch == '\r') {
                    continue; // consume
                }
                break; // done
            }
            buf[++len] = 0; // terminate, len inc nul

            // replace %1...x
            va_start(ap, buflen);
            while (NULL != (arg = va_arg(ap, const wchar_t *)) && idx <= 9 && len < buflen) {
                wchar_t *cursor = buf;

                while (NULL != (cursor = wcschr(cursor, '%'))) {
                    if (cursor[1] == ('0' + idx)) {
                        size_t arglen = wcslen(arg);

                        len -= 2; // %x being replace
                        if ((len + arglen) >= buflen) {
                            arglen = buflen - len; // overflow, truncate
                        }
                        wmemmove(cursor + arglen, cursor + 2, wcslen(cursor + 2) + 1 /*nul*/);
                        wmemcpy(cursor, arg, arglen);
                        len += arglen;
                        break;
                    }

                    if ('%' == *++cursor) {
                        ++cursor; // %%
                    }
                }
                ++idx;
            }
            va_end(ap);
        }
        return buf;
    }
    return NULL;
}

/*end*/
