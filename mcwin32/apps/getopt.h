#ifndef MCGETOPT_H_INCLUDED
#define MCGETOPT_H_INCLUDED
// $Id: getopt.h,v 1.2 2025/04/12 18:01:04 cvsuser Exp $
//
//  getopt() implementation
//

#include <wchar.h>

namespace Updater {

extern int          optind,                     /* index into parent argv vector */
                    optopt;                     /* character checked for validity */
extern const char  *optarg;                     /* argument associated with option */
extern const wchar_t *woptarg;                  /* argument associated with option */

extern int          Getopt(int nargc, char **nargv, const char *ostr);
extern int          Getopt(int nargc, wchar_t **nargv, const char *ostr);


} //namespace Updater

#endif  /*MCGETOPT_H_INCLUDED*/

