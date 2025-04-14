// $Id: getopt.cpp,v 1.3 2025/04/12 18:01:04 cvsuser Exp $
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
const wchar_t      *woptarg;                    /* argument associated with option */


#define OPTBADCH    (int)'?'

int
Getopt(int nargc, char **nargv, const char *ostr)
{
#define OPTEMSG     ""
#define OPTERR(__s) \
        fputs(*nargv, stderr); fputs(__s, stderr); fputc(optopt, stderr); fputc('\n', stderr); \
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

#undef OPTEMSG
#undef OPTERR

    return(optopt);                             /* dump back option letter */
}


int
Getopt(int nargc, wchar_t **nargv, const char *ostr)
{
#define LOPTEMSG        L""
#define LOPTERR(__s) \
        fputws(*nargv, stderr); fputws(__s, stderr); fputwc(optopt, stderr); fputwc('\n', stderr); \
        return (OPTBADCH);

    static const wchar_t *place = LOPTEMSG;     /* option letter processing */
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
        LOPTERR(L": illegal option -- ");
    }

    if (*++oli != ':') {                        /* don't need argument */
        woptarg = NULL;
        if (!*place) ++optind;

    } else {                                    /* need an argument */
        if (*place) {
            woptarg = place;                    /* no white space */

        } else if (nargc <= ++optind) {         /* no arg */
            place = LOPTEMSG;
            LOPTERR(L": option requires an argument -- ");

        } else {
            woptarg = nargv[optind];            /* white space */
        }

        place = LOPTEMSG;
        ++optind;
    }

#undef LOPTEMSG
#undef LOPTERR

    return(optopt);                             /* dump back option letter */
}

}   // namespace Updater

