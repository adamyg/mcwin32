//  $Id: mcsignature.cpp,v 1.10 2025/04/22 18:24:21 cvsuser Exp $
//
//  mcsignature - manifest generation tool.
//

#include "sign/signtoolshim.h"

#include "hosturls.inc"
#include "../buildinfo.h"

int
main(int argc, char *argv[])
{
    const char *hosturl = hosturl1;

    struct SignToolArgs args = {0};

    args.progname = "mcsignature";
    args.progtitle = "Midnight Commander, manifest generator (" VERSION "." BUILD_NUMBER ")";

    args.hosturl = hosturl1;
    args.hosturlalt = hosturl2;

    return SignToolShim(argc, argv, &args);
}

//end
