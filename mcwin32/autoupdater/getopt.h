#ifndef UPGETOPT_H_INCLUDED
#define UPGETOPT_H_INCLUDED
// $Id: getopt.h,v 1.1 2021/08/14 09:31:31 cvsuser Exp $
//
//  getopt() implementation
//

namespace Updater {

extern int          optind,                     /* index into parent argv vector */
                    optopt;                     /* character checked for validity */
extern const char  *optarg;                     /* argument associated with option */

extern int          Getopt(int nargc, char **nargv, const char *ostr);

} //namespace Updater

#endif  /*UPGETOPT_H_INCLUDED*/
