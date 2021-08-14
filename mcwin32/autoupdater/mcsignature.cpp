//  $Id: mcsignature.cpp,v 1.1 2021/08/14 09:31:31 cvsuser Exp $
//
//  AutoUpdater: Manifest generation tool.
//

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <string>
#include <iostream>
#include <time.h>
#include <windows.h>

#include <signature.h>

#include "../buildinfo.h"
#include "getopt.h"


static void                 Usage();
static const char *         Basename(const char *name);

static const char *         x_progname;


//  Function: Main
//      Application entry.
//
int
main(int argc, char *argv[])
{
    const char *version = VERSION "." BUILD_NUMBER,
            *hosturl = "https://sourceforge.net/projects/mcwin32/files/mcwin32.manifest/download";
    int ch;

    x_progname = Basename(argv[0]);
    while (-1 != (ch = Updater::Getopt(argc, argv, "V:H:h"))) {
        switch (ch) {
        case 'V':   /* application version */
            version = Updater::optarg;
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

    const char *inputname = argv[0],
            *outputname = argv[1];
    size_t inputlen;

    if ((inputlen = strlen(inputname)) < 5 || _stricmp(inputname + (inputlen-4), ".exe")) {
        std::cerr << "\n" <<
            x_progname << ": <input> should reference an installer exe image\n";
        Usage();
    }

    if (outputname && 0 == strcmp(inputname, outputname)) {
        std::cerr << "\n" <<
            x_progname << ": <input> and <output> names must be different\n";
        Usage();
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
        "   grsignature [options] <input> [<output>]\n"\
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
//
static const char *
Basename(const char *filename)
{
    const char *name;
    return (NULL != (name = strrchr(filename, '/')))
                || (NULL != (name = strrchr(filename, '\\'))) ? name + 1 : filename;
}

/*end*/

