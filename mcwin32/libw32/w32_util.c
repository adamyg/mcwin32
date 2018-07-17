/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 util unix functionality.
 *
 * Copyright (c) 2007, 2012 - 2018 Adam Young.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * The Midnight Commander is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained 
 * online at http://www.opengroup.org/unix/online.html.
 * ==end==
 */

#include "win32_internal.h"
#include "win32_child.h"
#include "win32_misc.h"
#include <unistd.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shfolder.lib")
#include <shlobj.h>                             /* SHGetFolderPath */


/*
 *  w32_getshell ---
 *      Retrieve the default shell.
 */
const char *
w32_getshell(void)
{
    const char *shname;

    shname = getenv("SHELL");
    if (shname == NULL)
        shname = getenv("COMSPEC");
    if (shname == NULL)
        shname = getenv("ComSpec");
    if (shname == NULL) {
        if (GetVersion() < 0x80000000) {
            shname = "CMD.EXE";                 // Windows NT/2000/XP
        }
        shname = "COMMAND.EXE";                 // ... others
    }
    return shname;
}


/*
 *  w32_gethome ---
 *      Retrieve the default home directory.
 */
const char *
w32_gethome(int ignore_env)
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
            w32_dos2unix(t_path);
        }

        x_home = WIN32_STRDUP(done ? t_path : "c:/");
    }
    return x_home;
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


char *
w32_dos2unix(char *path)
{
    if (path) {
        char *p;
        for (p = path; *p; ++p) {
             if ('\\' == *p) *p = '/';               /* DOS<>Unix */
        }
    }
    return path;
}


char *
w32_unix2dos(char *path)
{
    if (path) {
	char *p;
        for (p = path; *p; ++p) {
            if ('/' == *p) *p = '\\';               /* Unix<>DOS */
        }
    }
    return path;
}


const char *
w32_strslash(const char *path)
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

enum w32ostype
w32_ostype(void)
{
    static int platform = 0;

    if (! platform) {
        OSVERSIONINFO ovi = {0};
        ovi.dwOSVersionInfoSize = sizeof(ovi);
        GetVersionEx(&ovi);
			// TODO: replace with RtlGetVersion() as GetVersionEx() is now defunct; 8.1+.
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
            //  Operating system    Version dw??Version	    Other
            //                              Major   Minor
            //
            //  Windows 10	        10.0*	    10	    0	    OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
            //  Windows Server 2016	10.0*	    10	    0	    OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
            //  Windows 8.1	        6.3*	    6	    3	    OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
            //  Windows Server 2012 R2	6.3*	    6	    3	    OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
            //  8	            6.2	    6	    2	    OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
            //  Server 2012         6.2	    6       2	    OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
            //  7	            6.1	    6	    1	    OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
            //  Server 2008 R2	    6.1	    6	    1	    OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
            //  Server 2008	    6.0	    6	    0	    OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
            //  Vista	            6.0     6       0	    OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
            //  Server 2003 R2	    5.2	    5	    2	    GetSystemMetrics(SM_SERVERR2) != 0
            //  Home Server	    5.2	    5	    2	    OSVERSIONINFOEX.wSuiteMask & VER_SUITE_WH_SERVER
            //  Server 2003	    5.2	    5	    2	    GetSystemMetrics(SM_SERVERR2) == 0
            //  XP Professional x64 5.2	    5	    2	    (OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION) && 
            //                                                      (SYSTEM_INFO.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
            //  XP	                5.1	    5	    1	    Not applicable
            //  2000	                5.0	    5	    0	    Not applicable
            //
            //      * For applications that have been manifested for Windows 8.1 or Windows 10.  Applications not manifested 
            //      for Windows 8.1 or Windows 10 will return the Windows 8 OS version value (6.2). To manifest your applications
            //      for Windows 8.1 or Windows 10, refer to Targeting your application for Windows.
            //
            platform = OSTYPE_WIN_NT;               // or 2000

            if (ovi.dwMajorVersion >= 10) {
                platform = OSTYPE_WIN_10;           // Windows 10+

            } else if (6 == ovi.dwMajorVersion) {
                platform = OSTYPE_WIN_VISTA;
                if (ovi.dwMajorVersion >= 2) {
                    platform = OSTYPE_WIN_8;        // or Server 2012

                } else if (1 == ovi.dwMajorVersion) {
                    platform = OSTYPE_WIN_7;        // or Server 2008 R2
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


int
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
