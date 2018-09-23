/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 uname() system calls.
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

#include <sys/utsname.h>
#include <unistd.h>


/*
//  NAME
//      uname - get the name of the current system
//
//  SYNOPSIS
//
//      #include <sys/utsname.h>
//
//      int uname(struct utsname *name);
//
//  DESCRIPTION
//
//      The uname() function shall store information identifying the current system in the
//      structure pointed to by name.
//
//      The uname() function uses the utsname structure defined in <sys/utsname.h>.
//
//      The uname() function shall return a string naming the current system in the
//      character array sysname. Similarly, nodename shall contain the name of this node
//      within an implementation-defined communications network. The arrays release and
//      version shall further identify the operating system. The array machine shall
//      contain a name that identifies the hardware that the system is running on.
//
//      The format of each member is implementation-defined.
//
//  RETURN VALUE
//
//      Upon successful completion, a non-negative value shall be returned. Otherwise, -1
//      shall be returned and errno set to indicate the error.
//
//  ERRORS
//
//      No errors are defined.
*/
int
uname(struct utsname *u)
{
    static char u_sysname[32], u_version[16], u_release[32], u_machine[32];

    if (u_sysname[0] == '\0') {
        unsigned        osmajor, osminor, osbuild;
        char            osname_unknown[ 32 ];
        const char *    osname = "unknown", *cpu = "unknown";
        DWORD           dwVersion;
        OSVERSIONINFO   ovi;
        OSVERSIONINFOEX oviex;
        SYSTEM_INFO     si;

        ovi.dwOSVersionInfoSize = sizeof(ovi);
        oviex.dwOSVersionInfoSize = sizeof (oviex);

        /* osmajor, osminor, osbuild */
        if (FALSE == GetVersionEx(&ovi)) {
                // TODO: replace with RtlGetVersion() as GetVersionEx() is now defunct; 8.1+
            /*
             *  Error ... try the old way
             */
            dwVersion = GetVersion();
            osmajor = (unsigned)LOBYTE(LOWORD(dwVersion));
            osminor = (unsigned)HIBYTE(LOWORD(dwVersion));

            if (dwVersion < 0x80000000) {       // Windows NT/2000
                osbuild = (unsigned)(HIWORD(dwVersion));
            } else if (osmajor < 4) {           // Win32s
                osbuild = (unsigned)(HIWORD(dwVersion) & ~0x8000);
            } else {
                osbuild = 0;                    // Windows 95/98 -- No build number
            }
        } else {
            /*
             *  Extended info available ...
             */
            osmajor = (unsigned)ovi.dwMajorVersion;
            osminor = (unsigned)ovi.dwMinorVersion;
            osbuild = (unsigned)ovi.dwBuildNumber;

            switch (ovi.dwPlatformId){
            case VER_PLATFORM_WIN32s:           // 3.1 (running Win32s)
                osname = "32s";
                break;

            case VER_PLATFORM_WIN32_WINDOWS:    // 95, 98, or Me.
                if (ovi.dwMinorVersion == 0) {  // Windows 95
                    if (strchr(ovi.szCSDVersion, 'C')) {
                        osname = "95OSR2";
                    } else {
                        osname = "95";
                    }
                } else if (ovi.dwMinorVersion == 10) {
                    if (strchr(ovi.szCSDVersion, 'A')) {
                        osname = "98SE";        // .. second edition
                    } else {
                        osname = "98";          // Windows 98
                    }
                } else if (ovi.dwMinorVersion == 90) {
                    osname = "ME";              // Windows ME
                } else {
                    osname = "9x";              // Unknown
                }
                break;

            case VER_PLATFORM_WIN32_NT:         // NT, 2000, XP, or 2003 family.
                /*
                 *  4   Windows NT 4.0.
                 *
                 *  5   Windows Server 2003 R2, Windows Server 2003, Windows XP, or Windows 2000.
                 *
                 *  6   Windows Vista, Windows Server "Longhorn". Vista or Windows 7.
                 *
                 *  The following table summarizes the most recent operating system version numbers.
                 *   
                 *      Operating system            Version number
                 *      Windows 8                   6.2
                 *      Windows Server 2012         6.2
                 *      Windows 7                   6.1
                 *      Windows Server 2008 R2      6.1
                 *      Windows Server 2008         6.0
                 *      Windows Vista               6.0
                 *      Windows Server 2003 R2      5.2
                 *      Windows Server 2003         5.2
                 *      Windows XP 64-Bit Edition   5.2
                 *      Windows XP                  5.1
                 *      Windows 2000                5.0
                 */
                if (ovi.dwMajorVersion < 5) {
                    osname = "NT";              // NT 4 or 3.51
                } else {
                    if (FALSE == GetVersionEx ((OSVERSIONINFO *) &oviex)) {
                        oviex.dwMajorVersion = 0;
                    }

                    if (ovi.dwMajorVersion >= 10) {
                        osname = "Windows 10";  // Windows 10

                        if (oviex.dwMinorVersion > 0) {
                            osname = "Windows 10+";
                        }

                    } else if (ovi.dwMajorVersion >= 6) {
                        osname = "Vista";       // vista or greater

                        if (0 == oviex.dwMinorVersion) {
                            if (VER_NT_WORKSTATION == oviex.wProductType) {
                                osname = "Windows Vista";
#if defined(VER_SUITE_ENTERPRISE)
                                if (oviex.dwMajorVersion) {
                                    if (oviex.wSuiteMask & VER_SUITE_PERSONAL) {
                                        osname = "XP Home-Basic";
                                    } else if (oviex.wSuiteMask & (VER_SUITE_ENTERPRISE|VER_SUITE_DATACENTER)) {
                                        osname = "Longhorn";
                                    }
                                }
#endif
                            } else {
                                osname = "Windows Server 2008";
                            }
                        } else if (1 == oviex.dwMinorVersion) {
                            if (VER_NT_WORKSTATION == oviex.wProductType) {
                                osname = "Windows 7";
                            } else {
                                osname = "Windows Server 2008 R2";
                            }
                        } else if (2 == oviex.dwMinorVersion) {
                            if (VER_NT_WORKSTATION == oviex.wProductType) {
                                osname = "Windows 8";
                            } else {
                                osname = "Windows Server 2012";
                            }
                        } else {
                            if (VER_NT_WORKSTATION == oviex.wProductType) {
                                osname = "Windows 8.1";
                            } else {
                                osname = "Windows Server 2012+";
                            }
                        }
#if defined(TODO)
                        pGPI = (PGPI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
                        pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);
                        switch( dwType ) {
                        case PRODUCT_ULTIMATE:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Ultimate Edition"));
                           break;
                        case PRODUCT_PROFESSIONAL:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Professional"));
                           break;
                        case PRODUCT_HOME_PREMIUM:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Home Premium Edition"));
                           break;
                        case PRODUCT_HOME_BASIC:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Home Basic Edition"));
                           break;
                        case PRODUCT_ENTERPRISE:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition"));
                           break;
                        case PRODUCT_BUSINESS:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Business Edition"));
                           break;
                        case PRODUCT_STARTER:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Starter Edition"));
                           break;
                        case PRODUCT_CLUSTER_SERVER:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Cluster Server Edition"));
                           break;
                        case PRODUCT_DATACENTER_SERVER:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter Edition"));
                           break;
                        case PRODUCT_DATACENTER_SERVER_CORE:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter Edition (core installation)"));
                           break;
                        case PRODUCT_ENTERPRISE_SERVER:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition"));
                           break;
                        case PRODUCT_ENTERPRISE_SERVER_CORE:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition (core installation)"));
                           break;
                        case PRODUCT_ENTERPRISE_SERVER_IA64:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition for Itanium-based Systems"));
                           break;
                        case PRODUCT_SMALLBUSINESS_SERVER:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Small Business Server"));
                           break;
                        case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Small Business Server Premium Edition"));
                           break;
                        case PRODUCT_STANDARD_SERVER:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Standard Edition"));
                           break;
                        case PRODUCT_STANDARD_SERVER_CORE:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Standard Edition (core installation)"));
                           break;
                        case PRODUCT_WEB_SERVER:
                           StringCchCat(pszOS, BUFSIZE, TEXT("Web Server Edition"));
                           break;
                        }
#endif
                    } else if (ovi.dwMinorVersion == 0) {
                        osname = "2000";
#if defined(VER_SUITE_ENTERPRISE)
                        if (oviex.dwMajorVersion) {
                            if (oviex.wProductType == VER_NT_SERVER ||
                                        oviex.wProductType == VER_NT_DOMAIN_CONTROLLER) {
                                if (oviex.wSuiteMask & VER_SUITE_DATACENTER) {
                                    osname = "2000 Dataenter-Server";
                                } else if (oviex.wSuiteMask & VER_SUITE_ENTERPRISE) {
                                    osname = "2000 Advanced-Server";
                                } else {
                                    osname = "2000 Server";
                                }
                            } else {
                                osname = "2000 Professional";
                            }
                        }
#endif
                    } else if (ovi.dwMinorVersion == 1) {
                        osname = "XP";
#if defined(VER_SUITE_ENTERPRISE)
                        if (oviex.dwMajorVersion) {
                            if (oviex.wSuiteMask & VER_SUITE_PERSONAL) {
                                osname = "XP Home-Edition";
#if defined(VER_EMBEDDEDNT)
                            } else if (oviex.wSuiteMask & VER_EMBEDDEDNT) {
                                osname = "XP Embedded";
#endif
                            } else {
                                osname = "XP Professional";
                            }
                        }
#endif
                    } else if (ovi.dwMinorVersion == 2) {
                        osname = "2003";        // 2003
#if defined(VER_SUITE_ENTERPRISE)
                        if (oviex.dwMajorVersion) {
                            if (oviex.wSuiteMask & (VER_SUITE_ENTERPRISE|VER_SUITE_DATACENTER)) {
                                osname = "2003 Server";
                            }
                        }
#endif
                    } else {                    // unknown -- guess
                        sprintf(osname_unknown, "2%03d", (int)oviex.dwMajorVersion + 2);
                        osname = osname_unknown;
                    }
                }
                break;
            default:
                osname = "Unknown";
                break;
            }
        }

        /* machine */
        GetSystemInfo(&si);
        switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_ALPHA:
            switch (si.wProcessorLevel) {
            case 21064: cpu = "alpha 21064"; break;
            case 21066: cpu = "alpha 21066"; break;
            case 21164: cpu = "Alpha 21164"; break;
            default:
                cpu = "alpha";
            }
            break;

        case PROCESSOR_ARCHITECTURE_INTEL:
            switch (si.wProcessorLevel) {
            case 3: cpu = "i386";   break;
            case 4: cpu = "i486";   break;
            case 5: cpu = "i586";   break;
            case 6: cpu = "i686";   break;
            case 7:
            case 15: cpu = "i786";  break;
            default:
                cpu = "ix86";
            }
            break;

        case PROCESSOR_ARCHITECTURE_PPC:
            switch (si.wProcessorLevel) {
            case 1: cpu = "PPC 601";  break;
            case 3: cpu = "PPC 603";  break;
            case 4: cpu = "PPC 604";  break;
            case 6: cpu = "PPC 603+"; break;
            case 9: cpu = "PPC 604+"; break;
            case 20: cpu = "PPC 620"; break;
            default:
                cpu = "ppc";
            }
            break;

        case PROCESSOR_ARCHITECTURE_ARM:
            cpu = "arm";
            break;

        case PROCESSOR_ARCHITECTURE_IA64:
            cpu = "IA64";
            break;

        case PROCESSOR_ARCHITECTURE_MIPS:
            cpu = "mips";
            break;

        case PROCESSOR_ARCHITECTURE_SHX:
            cpu = "sh";
            break;

#ifdef PROCESSOR_ARCHITECTURE_MSIL
        case PROCESSOR_ARCHITECTURE_MSIL:
            cpu = "msil";
            break;
#endif

#ifdef PROCESSOR_ARCHITECTURE_IA32_ON_WIN64
        case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
            cpu = "IA32 on Win64";
            break;
#endif

#ifdef PROCESSOR_ARCHITECTURE_AMD64
        case PROCESSOR_ARCHITECTURE_AMD64:
            cpu = "amd64";
            break;
#endif

        case PROCESSOR_ARCHITECTURE_ALPHA64:
            cpu = "alpha64";
            break;

        case PROCESSOR_ARCHITECTURE_UNKNOWN:
        default:
            cpu = "unknown";
        }

        /* publish */
        sprintf(u_sysname, "Win%s", osname);
        if (osbuild) {
            sprintf(u_release, "%u.%u", osmajor, osminor);
            sprintf(u_version, "Build_%u", osbuild);

        } else {
            sprintf(u_release, "%u", osmajor);
            sprintf(u_version, "%u", osminor);
        }
        strcpy(u_machine, cpu);
    }

    /* populate */
    if (u) {
        memset(u, 0, sizeof(*u));

        strncpy(u->sysname, u_sysname, sizeof(u->sysname) - 1);
        u->nodename[0] = '\0';                  /* not available */
        strncpy(u->release, u_release, sizeof(u->release) - 1);
        strncpy(u->version, u_version, sizeof(u->version) - 1);
        strncpy(u->machine, u_machine, sizeof(u->machine) - 1);
    }
    return 0;
}

/*end*/

