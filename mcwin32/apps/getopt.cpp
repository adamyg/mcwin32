// $Id: getopt.cpp,v 1.2 2025/04/09 10:11:21 cvsuser Exp $
//
//  getopt() implementation
//

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "getopt.h"

#if defined(__WATCOMC__)
#pragma disable_message(391)                    /* assignment found in boolean expression */
#endif

namespace Updater {

int                 optind = 1,                 /* index into parent argv vector */
                    optopt;                     /* character checked for validity */
const char         *optarg;                     /* argument associated with option */

int
Getopt(int nargc, char **nargv, const char *ostr)
{
#define OPTBADCH        (int)'?'
#define OPTEMSG         ""
#define OPTERR(s) \
        fputs(*nargv,stderr);fputs(s,stderr); \
        fputc(optopt,stderr);fputc('\n',stderr); \
        return(OPTBADCH);

    static const char *place = OPTEMSG;         /* option letter processing */
    const char *oli;                            /* option letter list index */

    if (!*place) {                              /* update scanning pointer */
        if (optind >= nargc || *(place = nargv[optind]) != '-' || !*++place) {
            return(EOF);
        }

        if (*place == '-') {                    /* found "--" */
            ++optind;
            return EOF;
        }
    }
                                                /* option letter okay? */
    if ((optopt = (int)*place++) == (int)':' ||
                NULL == (oli = strchr(ostr,optopt))) {
        if (!*place) ++optind;
        OPTERR(": illegal option -- ");
    }

    if (*++oli != ':') {                        /* don't need argument */
        optarg = NULL;
        if (!*place) ++optind;

    } else {                                    /* need an argument */
        if (*place) {
            optarg = place;                     /* no white space */

        } else if (nargc <= ++optind) {         /* no arg */
            place = OPTEMSG;
            OPTERR(": option requires an argument -- ");

        } else {
            optarg = nargv[optind];             /* white space */
        }

        place = OPTEMSG;
        ++optind;
    }

#undef OPTBADCH
#undef OPTEMSG
#undef OPTERR

    return(optopt);                             /* dump back option letter */
}

}   // namespace Updater
