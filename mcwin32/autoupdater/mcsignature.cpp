//  $Id: mcsignature.cpp,v 1.7 2025/03/09 13:43:38 cvsuser Exp $
//
//  AutoUpdater: Manifest generation tool.
//

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string>
#include <iostream>
#include <time.h>
#include <windows.h>

#include <signature.h>

#include "../buildinfo.h"
#include "getopt.h"

#pragma comment(lib, "Version.lib")

static void                 Usage();
static const char *         Basename(const char *name);
static const char *         ExeVersion(const char *executable, char *version, size_t versize);

static const char *         x_progname;


//  Function: Main
//      Application entry.
//
int
main(int argc, char *argv[])
{
    const char *version = NULL,
#if defined(_M_AMD64)           // x64; XXX as channel?
            *hosturl = "https://sourceforge.net/projects/mcwin32/files/mcwin32x64.manifest/download";
#else
            *hosturl = "https://sourceforge.net/projects/mcwin32/files/mcwin32.manifest/download";
#endif
    const char *exename = NULL;
    int ch;

    x_progname = Basename(argv[0]);
    while (-1 != (ch = Updater::Getopt(argc, argv, "V:E:H:h"))) {
        switch (ch) {
        case 'V':   /* application version */
            version = Updater::optarg;
            break;
        case 'E':   /* executable name */
            exename = Updater::optarg;
            break;
        case 'H':   /* host URL template */
            hosturl = Updater::optarg;
            break;
        case 'h':
        default:
            Usage();
            break;
        }
    }

    if (version && exename) {
        std::cerr << "\n" <<
            x_progname << ": -V and -E are mutually exclusive options." << std::endl;
        Usage();
    }

    argv += Updater::optind;
    if ((argc -= Updater::optind) < 1) {
        std::cerr << "\n" <<
            x_progname << ": expected arguments <input> [<output>]" << std::endl;
        Usage();

    } else if (argc > 2) {
        std::cerr << "\n" <<
            x_progname << ": unexpected arguments '" << argv[2] << "' ..." << std::endl;
        Usage();
    }

    char exeversion[64] = {0};
    const char *inputname = argv[0], *outputname = argv[1];
    size_t inputlen;

    if ((inputlen = strlen(inputname)) < 5 || _stricmp(inputname + (inputlen-4), ".exe")) {
        std::cerr << "\n" <<
            x_progname << ": <input> should reference an installer exe image." << std::endl;
        Usage();
    }

    if (outputname && 0 == strcmp(inputname, outputname)) {
        std::cerr << "\n" <<
            x_progname << ": <input> and <output> names must be different." << std::endl;
        Usage();
    }

    if (NULL == version) version = VERSION "." BUILD_NUMBER; // default.
    if (exename) { // optional executable name.
        if (NULL != ExeVersion(exename, exeversion, sizeof(exeversion))) {
            version = exeversion;
        }
    }

    sign_manifest(inputname, version, hosturl);
    return 0;
}


//  Function: Usage
//      Echo the command line usage and exit.
//
//  Parameters:
//      none
//
//  Returns:
//      n/a
//
static void
Usage()
{
    std::cerr <<
        "\n"\
        "Autoupdater manifest signature generator               version 1.01\n"\
        "\n"\
        "   mcsignature [options] <input> [<output>]\n"\
        "\n"\
        "Options:\n"\
        "   -H <manifest>       HostURL template.\n"\
        "   -V <version>        Version label.\n"\
        "\n"\
        "Arguments:\n"\
        "   input               Name of the input file.\n"\
        "   output              Optional name of the results output file.\n"\
        "\n" << std::endl;
    exit(3);
}


//  Function: Basename
//      Retrieve the file basename from the specified file path.
//
static const char *
Basename(const char *filename)
{
    const char *name;
    return (NULL != (name = strrchr(filename, '/')))
                || (NULL != (name = strrchr(filename, '\\'))) ? name + 1 : filename;
}


//  Function: ExeVersion
//      Retrieve the executable version information.
//
static const char *
ExeVersion(const char *executable, char *version, size_t versize)
{
    char sub_block[2] = { '\\', '\0' };
    char path[MAX_PATH] = {0};
    void *vi = NULL, *sb = NULL;
    DWORD visz, dummy;
    UINT sbsz;

    // determine the size of the version info
    if (! SearchPathA(NULL, executable, NULL, sizeof(path), path, NULL)) {
        std::cerr << "Cannot find <" << executable << ">\n";
        return NULL;
    }

    if (0 == (visz = GetFileVersionInfoSize(path, &dummy))) {
        switch (GetLastError()) {
        case ERROR_RESOURCE_TYPE_NOT_FOUND:
            std::cerr << "<" << path << "> does not contain version info; this is probably an executable\n";
            break;
        default:
            std::cerr << "GetFileVersionInfoSize() failed : " << GetLastError() << "\n";
            break;
        }
        return NULL;
    }

    if (NULL == (vi = (void *)GlobalAlloc(GMEM_FIXED, visz))) {
        std::cerr << "Out of memory\n";
        return NULL;
    }

    // retrieve the version info
    if (! GetFileVersionInfo(path, 0, visz, vi)) {
        std::cerr << "GetFileVersionInfo() failed : " << GetLastError() << "\n";
        return NULL;
    }

    // extract the VS_FIXEDFILEINFO from the version info
    if (! VerQueryValueA(vi, sub_block, &sb, &sbsz)) {
        std::cerr << "VerQueryValue() failed : " << GetLastError() << "\n";
        return NULL;
    }

    const VS_FIXEDFILEINFO *ffi = (const VS_FIXEDFILEINFO *)sb;
    if (ffi->dwProductVersionMS && ffi->dwProductVersionLS) {
        _snprintf(version, versize, "%u.%u.%u.%u",
            (HIWORD(ffi->dwProductVersionMS) & 0xFF), (LOWORD(ffi->dwProductVersionMS) & 0xFF),
            (HIWORD(ffi->dwProductVersionLS) & 0xFF), (LOWORD(ffi->dwProductVersionLS) & 0xFF));
    } else {
        version = NULL;
    }

    GlobalFree(vi);
    return version;
}

/*end*/
