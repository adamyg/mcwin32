//  $Id: mcupdater.cpp,v 1.13 2025/04/22 18:24:21 cvsuser Exp $
//
//  Midnight Commander AutoUpdater command line.
//

#include "update/updatetoolshim.h"

#include "hosturls.inc"
#include "public_version1.inc"

#include "../buildinfo.h"

//  Function: Main
//      Application entry.
//
//  Returns:
//      0  - No check performed.
//      1  - Up-to-date.
//      2  - Installed.
//      3  - Update available.
//      99 - Usage
//

int
main(int argc, char *argv[])
{
    struct UpdateToolArgs args = {0};

    args.progname = "mcupdater";
    args.progtitle = "Midnight Commander, updater (" VERSION "." BUILD_NUMBER ")";

    args.appname = "Midnight Commander";
    args.version = VERSION "." BUILD_NUMBER;

    args.hosturl = hosturl1;
    args.hosturlalt = hosturl2;
    args.publickey = public_key_base64;
    args.keyversion = key_version;

    return UpdateToolShim(argc, argv, &args);
}

//end
