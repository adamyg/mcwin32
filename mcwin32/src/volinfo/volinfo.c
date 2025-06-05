/*
 * Volume information.
 *
 * Build using: "cl volinfo.c"
 *
 * Copyright (c) 2025, Adam Young.
 * All rights reserved.
 */

#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x601)
#undef  _WIN32_WINNT
#undef  _WIN32_VER
#define _WIN32_WINNT 0x601
#define _WIN32_VER 0x601
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <wchar.h>

#define WINDOWS_MEAN_AND_LEAN
#include <Windows.h>

#if !defined(__GNUC__)
#pragma comment(lib, "mpr.lib")
#endif

#ifndef _countof
#define _countof(__type)    (sizeof(__type)/sizeof(__type[0]))
#endif

static void EnumDriveTypes(void);
static void EnumVolumes(void);
static void EnumNetwork(void);
static void DisplayVolume(const wchar_t *name);
static void StatDrive(const wchar_t *drive);
static void HumanReadable(const char *label, uint64_t size);

static void Tally(ULONGLONG start, const char *label);
static void Profile(void);

static const char *IsOption(const char *argv, const char *option);
static void Usage(void);

static void OutputA(const char *, ...);
static void OutputW(const wchar_t *, ...);

static int otimestamp       = 0;
static int overbose         = 0;
static int onetconnected    = 1;

static int ovolumeinfo      = 0;
static int odrivetype       = 0;
static int oattributes      = 0;
static int ofreespace       = 0;
static int ostatfs          = 0;


int
main(int argc, char *argv[])
{
        int arg;

        if (argc <= 1) {
                Usage();
                return EXIT_FAILURE;
        }

        for (arg = 1; arg < argc; ++arg) {
                const char *option = argv[arg], *val;

                if (option[0] != '-') {
                        break;
                }

                if ((val = IsOption(option, "--time")) != NULL) {
                        otimestamp = 1;
                } else if ((val = IsOption(option, "--verbose")) != NULL) {
                        overbose = 1;

                } else if ((val = IsOption(option, "--netconnected")) != NULL) {
                        onetconnected = 1;
                } else if ((val = IsOption(option, "--netremembered")) != NULL) {
                        onetconnected = 0;

                } else if ((val = IsOption(option, "--volumeinfo")) != NULL) {
                        ovolumeinfo = 1;
                } else if ((val = IsOption(option, "--drivetype")) != NULL) {
                        odrivetype  = 1;
                } else if ((val = IsOption(option, "--freespace")) != NULL) {
                        ofreespace  = 1;
                } else if ((val = IsOption(option, "--attributes")) != NULL) {
                        oattributes = 1;
                } else if ((val = IsOption(option, "--statfs")) != NULL) {
                        ostatfs = 1;

                } else {
                        if ((val = IsOption(option, "--help")) == NULL) {
                                fprintf(stderr, "volinfo: invalid option <%s>\n\n", option);
                        }
                        Usage();
                        return EXIT_FAILURE;
                }
        }

        argv += arg;
        argc -= arg;
        if (argc == 0) {
                fprintf(stderr, "volinfo: expected an operation\n");
                return EXIT_FAILURE;

        } else if (argc != 1) {
                if (argc > 2) {
                        fprintf(stderr, "volinfo: unexpected options <%s ...>\n", argv[1]);
                } else {
                        fprintf(stderr, "volinfo: unexpected option <%s>\n", argv[1]);
                }
                return EXIT_FAILURE;

        } else {
                const char *op = argv[0];
                unsigned operations = 0;

                if (0 == strcmp(op, "volumes")) {
                        operations = 0x04;
                } else if (0 == strcmp(op, "network")) {
                        operations = 0x02;
                } else if (0 == strcmp(op, "drives")) {
                        operations = 0x01;
                } else if (0 == strcmp(op, "all")) {
                        operations = 0xff;
                } else {
                        fprintf(stderr, "volinfo: invalid operation <%s>\n", op);
                        return EXIT_FAILURE;
                }

                if (operations & 0x04) {
                        EnumVolumes();
                }

                if (operations & 0x02) {
                        EnumNetwork();
                }

                if (operations & 0x01) {
                        EnumDriveTypes();
                }
        }
        return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Enumerate all drives in the system.
//

static void
EnumDriveTypes(void)
{
        wchar_t buffer[1024];

        OutputA("\nEnumDriveTypes:\n\n");

        buffer[0] = 0;
        if (GetLogicalDriveStringsW(_countof(buffer), buffer)) {
                const wchar_t *drive = NULL;

                for (drive = buffer; drive[0] != L'\0'; drive += wcslen(drive) + 1) {
                        StatDrive(drive);
                }
                OutputA("\n");

        } else {
                OutputA("GetLogicalDriveStrings(): failure %u\n", (unsigned)GetLastError());
        }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Enumerate all volumes in the system.
//

static void
EnumVolumes(void)
{
        HANDLE findHandle = INVALID_HANDLE_VALUE;
        WCHAR  deviceName[MAX_PATH] = L"";
        WCHAR  volumeName[MAX_PATH] = L"";
        DWORD  charCount = 0;

        OutputA("\nEnumVolumes:\n\n");

        findHandle = FindFirstVolumeW(volumeName, _countof(volumeName));
        if (findHandle == INVALID_HANDLE_VALUE) {
                const DWORD rc = GetLastError();
                OutputA("FindFirstVolume(): failure %u\n", (unsigned)rc);
                return;
        }

        for (;;) {
                // Skip the \\?\ prefix and remove the trailing backslash.
                const size_t Index = wcslen(volumeName) - 1;

                if (volumeName[0] != L'\\' || volumeName[1] != L'\\' ||  volumeName[2] != L'?' || volumeName[3] != L'\\' || volumeName[Index] != L'\\') {
                        OutputW(L"FindFirstVolume/FindNextVolume(): bad path: <%ls>\n", volumeName);
                        break;
                }

                volumeName[Index] = L'\0';      // temporarily removal of slash; QueryDosDeviceW requirement
                charCount = QueryDosDeviceW(&volumeName[4], deviceName, _countof(deviceName));
                volumeName[Index] = L'\\';

                if (charCount == 0) {
                        const DWORD rc = GetLastError();
                        OutputW(L"QueryDosDevice(): failure %u\n", (unsigned)rc);
                        break;
                }

                OutputW(L" Device: %ls\n", deviceName);
                if (overbose)
                        OutputW(L"  Volume: %ls\n", volumeName);
                OutputW(L"  Paths:\n");

                DisplayVolume(volumeName);

                // Next volume.
                if (! FindNextVolumeW(findHandle, volumeName, _countof(volumeName))) {
                        const DWORD rc = GetLastError();
                        if (rc != ERROR_NO_MORE_FILES) {
                                OutputW(L"FindNextVolume(): failure %u\n", (unsigned)rc);
                                break;
                        }
                        break;
                }
                OutputW(L"\n");
        }

        FindVolumeClose(findHandle);
}


static void
DisplayVolume(const wchar_t *VolumeName)
{
        DWORD  charCount = MAX_PATH + 1;
        PWCHAR names = NULL;
        BOOL   success = FALSE;

        for (;;) {
                // Allocate a buffer to hold the paths.
                names = (PWCHAR) malloc(charCount * sizeof(WCHAR));
                if (NULL == names) {
                        OutputW(L"Memory error!\n");
                        return;
                }

                // Obtain for this volume.
                success = GetVolumePathNamesForVolumeNameW(VolumeName, names, charCount, &charCount);
                if (success) {
                        break;
                }

                if (GetLastError() != ERROR_MORE_DATA) {
                        break;
                }

                // Retry using new suggested size.
                free((void *) names);
                names = NULL;
        }

        if (success && names[0]) {
                // Display the various paths.
                const wchar_t *drive = NULL;

                for (drive = names; drive[0] != L'\0'; drive += wcslen(drive) + 1) {
                        StatDrive(drive);
                }
        }
        free((void *)names);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Enumerate network connections.
//

static void NetworkDisplayStruct(unsigned i, LPNETRESOURCEW lpnrLocal);

static BOOL WINAPI
EnumNetworkFunc(LPNETRESOURCEW lpnr)
{
        const DWORD dwScope = (onetconnected ? RESOURCE_CONNECTED : RESOURCE_REMEMBERED);
                // CONNECTED -  Open connections.
                // REMEMBERED - PERSISTED, aka are connected at log-on.

        DWORD cbBuffer = 16384;                     // buffer size
        DWORD dwResult, dwResultEnum;
        DWORD cEntries = (DWORD)-1;                 // enumerate all possible entries
        LPNETRESOURCEW lpnrLocal = NULL;            // pointer to enumerated structures
        HANDLE hEnum = NULL;

        // Enumerate all currently connected resources.
        dwResult = WNetOpenEnumW(dwScope, RESOURCETYPE_DISK, 0, lpnr, &hEnum);
        if (dwResult != NO_ERROR) {
                OutputA("WnetOpenEnum: failure %u\n", (unsigned)dwResult);
                return FALSE;
        }

        if (NULL == (lpnrLocal = (LPNETRESOURCEW) GlobalAlloc(GPTR, cbBuffer))) {
                OutputA("EnumNetworkFunc: memory error\n");
                (void) WNetCloseEnum(hEnum);
                return FALSE;
        }

        do {
                ZeroMemory(lpnrLocal, cbBuffer);
                dwResultEnum = WNetEnumResourceW(hEnum, &cEntries, lpnrLocal, &cbBuffer);
                if (dwResultEnum == NO_ERROR) {
                        DWORD i;

                        for (i = 0; i < cEntries; ++i) {
                                LPNETRESOURCEW netResource = lpnrLocal + i;

                                NetworkDisplayStruct(i, netResource);
                                if (RESOURCEUSAGE_CONTAINER == (netResource->dwUsage & RESOURCEUSAGE_CONTAINER)) {
                                        // If the NETRESOURCE structure represents a container resource,
                                        // call the EnumerateFunc function recursively.
                                        EnumNetworkFunc(netResource);
                                }
                        }

                } else if (dwResultEnum != ERROR_NO_MORE_ITEMS) {
                        OutputA("WNetEnumResource: failure %u\n", (unsigned)dwResultEnum);
                        break;
                }

        } while (dwResultEnum != ERROR_NO_MORE_ITEMS);

        GlobalFree((HGLOBAL)lpnrLocal);

        dwResult = WNetCloseEnum(hEnum);
        if (dwResult != NO_ERROR) {
                OutputA("WNetCloseEnum: failure %u\n", (unsigned)dwResult);
                return FALSE;
        }
        return TRUE;
}


static void
NetworkDisplayStruct(unsigned i, LPNETRESOURCEW lpnrLocal)
{
        OutputA("  NETRESOURCE[%u]:\n", i);

        OutputA("    Scope:       ");
        switch (lpnrLocal->dwScope) {
        case (RESOURCE_CONNECTED):
                OutputA("connected\n");
                break;
        case (RESOURCE_GLOBALNET):
                OutputA("all resources\n");
                break;
        case (RESOURCE_REMEMBERED):
                OutputA("remembered\n");
                break;
        default:
                OutputA("unknown scope %u\n", (unsigned)(lpnrLocal->dwScope));
                break;
        }

        OutputA("    Type:        ");
        switch (lpnrLocal->dwType) {
        case (RESOURCETYPE_ANY):
                OutputA("any\n");
                break;
        case (RESOURCETYPE_DISK):
                OutputA("disk\n");
                break;
        case (RESOURCETYPE_PRINT):
                OutputA("print\n");
                break;
        default:
                OutputA("unknown type %u\n", (unsigned)(lpnrLocal->dwType));
                break;
        }

        OutputA("    DisplayType: ");
        switch (lpnrLocal->dwDisplayType) {
        case (RESOURCEDISPLAYTYPE_GENERIC):
                OutputA("generic\n");
                break;
        case (RESOURCEDISPLAYTYPE_DOMAIN):
                OutputA("domain\n");
                break;
        case (RESOURCEDISPLAYTYPE_SERVER):
                OutputA("server\n");
                break;
        case (RESOURCEDISPLAYTYPE_SHARE):
                OutputA("share\n");
                break;
        case (RESOURCEDISPLAYTYPE_FILE):
                OutputA("file\n");
                break;
        case (RESOURCEDISPLAYTYPE_GROUP):
                OutputA("group\n");
                break;
        case (RESOURCEDISPLAYTYPE_NETWORK):
                OutputA("network\n");
                break;
        default:
                OutputA("unknown display type %u\n", (unsigned)(lpnrLocal->dwDisplayType));
                break;
        }

        OutputA("    Usage:       0x%x = ", (unsigned)(lpnrLocal->dwUsage));
        if (lpnrLocal->dwUsage & RESOURCEUSAGE_CONNECTABLE)
                OutputA("connectable ");
        if (lpnrLocal->dwUsage & RESOURCEUSAGE_CONTAINER)
                OutputA("container ");
        if (lpnrLocal->dwUsage & RESOURCEUSAGE_NOLOCALDEVICE)
                OutputA("nonlocaldevice ");
        if (lpnrLocal->dwUsage & RESOURCEUSAGE_ATTACHED)
                OutputA("attached ");
        OutputA("\n");

        OutputW(L"    LocalName:   %ls\n", lpnrLocal->lpLocalName  ? lpnrLocal->lpLocalName  : L"");
        OutputW(L"    RemoteName:  %ls\n", lpnrLocal->lpRemoteName ? lpnrLocal->lpRemoteName : L"");
        OutputW(L"    Comment:     %ls\n", lpnrLocal->lpComment    ? lpnrLocal->lpComment    : L"");
        OutputW(L"    Provider:    %ls\n", lpnrLocal->lpProvider   ? lpnrLocal->lpProvider   : L"");

        if (lpnrLocal->dwType == RESOURCETYPE_DISK && lpnrLocal->lpLocalName) {
                if (lpnrLocal->lpLocalName[0] && lpnrLocal->lpLocalName[1] == ':') {
                        wchar_t drive[4] = { L"X:\\" };

                        drive[0] = lpnrLocal->lpLocalName[0];
                        OutputA("\n");
                        StatDrive(drive);
                }
        }

        OutputA("\n");
}


static void
EnumNetwork(void)
{
        OutputA("\nEnumNetworks:\n\n");
        EnumNetworkFunc((LPNETRESOURCEW) NULL);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Publish drive information.
//

static void
StatDrive(const wchar_t *drive)
{
#define MNAMELEN    90                          /* length of buffer for returned name */
#define MFSNAMELEN  16                          /* length of fs type name, including null */

        wchar_t volName[MNAMELEN] = { 0 }, fsName[MFSNAMELEN] = { 0 };
        DWORD maxLength = 0, fsFlags = 0;
        BOOL ready = 0, freespace = 0;

        if (otimestamp) {
                const ULONGLONG ms = GetTickCount64();
                const time_t now = time(NULL);
                char buffer[26];
                struct tm *tm;

                tm = localtime(&now);
                strftime(buffer, sizeof(buffer), "%H:%M:%S", tm);
                OutputA("  %s.%03u", buffer, ms % 1000);
        }

        Tally(0, NULL);
        OutputW(L"  %ls\t", drive);

        if (ovolumeinfo || ostatfs) {
                const ULONGLONG start = GetTickCount64();

                if (GetVolumeInformationW(drive,
                        volName, MNAMELEN,      /* VolumeName and size */
                        NULL, &maxLength, &fsFlags, fsName, MFSNAMELEN)) {
                    ready = TRUE;
                }

                Tally(start, "GetVolumeInformation");
                OutputW(L" <%ls>%*ls | fsflags:0x%08x, maxln:%4u",
                    fsName, _countof(fsName) - wcslen(fsName), L"", fsFlags, maxLength);
        }

        if (ostatfs && overbose) {
                const ULONGLONG start = GetTickCount64();
                wchar_t device[MAX_PATH], *ret = device;
                wchar_t disk[3] = { L"X:" };

                disk[0] = drive[0];
                if (QueryDosDeviceW(disk, device, _countof(device))) {
                        if (0 == wcsncmp(device, L"\\Device\\", 8)) {
                                ret = device + 8;
                        }
                } else {
                        ret = L"NA";
                }

                Tally(start, "QueryDosDevice");
                OutputW(L", [%-16.16ls]", ret);
        }

        if (odrivetype || ostatfs) {
                const ULONGLONG start = GetTickCount64();
                const UINT type = GetDriveTypeW(drive);

                Tally(start, "GetDriveType");
                OutputW(L", type:");

                switch (type) {
                case DRIVE_FIXED:
                    OutputW(L"Fixed,     ");
                    freespace = ready;
                    break;
                case DRIVE_REMOVABLE:
                    OutputW(L"Removable, ");
                    freespace = ready;
                    break;
                case DRIVE_CDROM:
                    OutputW(L"CDROM,     ");
                    freespace = ready;
                    break;
                case DRIVE_REMOTE:
                    OutputW(L"REMOTE,    ");
                    if (0 == wcscmp(fsName, L"9P")) {
                            freespace = ready;  /* WSL2 */
                    }
                    break;
                case DRIVE_RAMDISK:
                    OutputW(L"RamDisk,   ");
                    freespace = ready;
                    break;
                case DRIVE_UNKNOWN:
                    OutputW(L"Unknown,   ");
                    break;
                case DRIVE_NO_ROOT_DIR:
                    OutputW(L"Invalid,   ");
                    break;
                default:
                    OutputW(L"NA,        ");
                    break;
                }
        }

        if (oattributes || ostatfs) {
                const ULONGLONG start = GetTickCount64();
                const DWORD Attributes = GetFileAttributesW(drive);

                Tally(start, "GetFileAttributes");
                if (Attributes == INVALID_FILE_ATTRIBUTES) {
                        OutputW(L"flags:none   ");
                } else if (Attributes & FILE_ATTRIBUTE_READONLY) {
                        OutputW(L"flags:rdonly ");
                } else {
                        OutputW(L"flags:rdwr   ");
                }
        }

        if (ofreespace || (ostatfs && freespace)) {
                DWORD SectorsPerCluster = 0, BytesPerSector = 0, FreeClusters = 0, Clusters = 0;
                const ULONGLONG start = GetTickCount64();
                BOOL success = GetDiskFreeSpaceW(drive, &SectorsPerCluster, &BytesPerSector, &FreeClusters, &Clusters);

                Tally(start, "GetDiskFreeSpace");
                OutputW(L" | ");

                if (success) {
                        /* available */
                        HumanReadable("avail=",
                            (((uint64_t)SectorsPerCluster * BytesPerSector * FreeClusters) /*/ 1024*/));

                        /* total */
                        HumanReadable(", blocks=",
                            (((uint64_t)SectorsPerCluster * BytesPerSector * Clusters) /*/ 1024*/));

                        /* clusters */
                        HumanReadable(", free=", (size_t)(FreeClusters /*/ 10*/));
                        HumanReadable(", files=", (size_t)(Clusters /*/ 10*/));
                } else {
                        OutputW(L" free:n/a");
                }
        }

        Profile();
        OutputW(L"\n");
}


static void
HumanReadable(const char* label, uint64_t size)
{
        const char *units[] = { "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
        unsigned u = 0;

        while (size >= (1024 * 1024)) {
            size /= 1024;
            ++u;
        }

        OutputA("%s%8.3f %s", label, ((double)size / 1024), units[u]);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Execution profile

static unsigned tm_count;

static struct TimestampEvent {
        const char *label;
        ULONGLONG start, end;
} tm_events[16] = {0};


static void
Tally(ULONGLONG start, const char *label)
{
        if (0 == start) { // reset
                tm_count = 0;
                return;
        }

        if (tm_count < _countof(tm_events)) { // push profile event
                struct TimestampEvent *evt = tm_events + tm_count++;

                evt->label = label;
                evt->start = start;
                evt->end   = GetTickCount64();
        }
}


static void
Profile(void)
{
        if (tm_count) {
                const unsigned total = (unsigned)(tm_events[tm_count - 1].end - tm_events[0].start);
                unsigned tm, count = 0;

                OutputA(" | total:%ums", total);
                for (tm = 0; tm != tm_count; ++tm) {
                        const struct TimestampEvent *evt = tm_events + tm;
                        const unsigned slice = (unsigned)(evt->end - evt->start);

                        if (slice >= 5) {
                                OutputA("%s%s:%ums", (0 == count ? " [ " : ", "), evt->label, slice);
                                ++count;
                        }
                }
                if (count) OutputA(" ]");
        }
        tm_count = 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Command line support

static const char *
IsOption(const char *argv, const char *option)
{
        const size_t olen = strlen(option);
        if (strncmp(argv, option, olen) == 0) {
                return argv + olen;
        }
        return NULL;
}


static void
Usage(void)
{
        fprintf(stderr,
            "\n" \
            "volinfo [options] [attributes] <operation>\n" \
            "\n" \
            "   Query disk information using on the following operations:\n" \
            "\n" \
            "Operations:\n" \
            "   volumes -           Iterate by volume enumeration.\n" \
            "   network -           Iterate by network enumeration.\n" \
            "   drives -            Iterate published drive letters.\n" \
            "   all -               All available methods.\n" \
            "\n" \
            "Attributes:\n" \
            "   --volumeinfo        Volume information.\n" \
            "   --drivetype         Drive type.\n" \
            "   --attributes        Attributes.\n" \
            "   --freespace         Free space.\n" \
            "   --statfs            statfs, all attributes.\n" \
            "\n" \
            "Options:\n" \
            "   --time              Access timestamps.\n" \
            "   --verbose           Additional info.\n" \
            "   --netconnected      Network status (default: connected).\n" \
            "   or --netremembered.\n" \
            "   --help\n" \
            "\n");
}


static void
OutputA(const char *fmt, ...)
{
        HANDLE cout = GetStdHandle(STD_OUTPUT_HANDLE);
        char out[512];
        va_list ap;
        int len;

        va_start(ap, fmt);
        if ((len = vsnprintf(out, _countof(out), fmt, ap)) > (int)_countof(out)) {
                len = _countof(out);
        }
        WriteConsoleA(cout, out, len, NULL, NULL);
        va_end(ap);
}


static void
OutputW(const wchar_t *fmt, ...)
{
        HANDLE cout = GetStdHandle(STD_OUTPUT_HANDLE);
        wchar_t out[512];
        va_list ap;
        int len;

        va_start(ap, fmt);
        if ((len = vswprintf(out, _countof(out), fmt, ap)) > (int)_countof(out)) {
                len = _countof(out);
        }
        WriteConsoleW(cout, out, len, NULL, NULL);
        va_end(ap);
}

//end
