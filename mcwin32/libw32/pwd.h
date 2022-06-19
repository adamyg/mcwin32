#ifndef LIBW32_PWD_H_INCLUDED
#define LIBW32_PWD_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_pwd_h,"$Id: pwd.h,v 1.11 2022/06/08 09:51:43 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <pwd.h> implementation
 *
 *      pwd.h - password structure
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 * or (at your option) any later version.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 */

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */
#include <sys/utypes.h>                         /* uid_t */
#include <stddef.h>                             /* size_t */

__BEGIN_DECLS

/*
 *  The <pwd.h> header shall provide a definition for struct passwd,
 *  which shall include at least the following members:
 *
 *      char    *pw_name            User's login name.
 *      uid_t    pw_uid             Numerical user ID.
 *      gid_t    pw_gid             Numerical group ID.
 *      char    *pw_dir             Initial working directory.
 *      char    *pw_shell           Program to use as shell.
 */

struct passwd {
    const char *        pw_name;
    const char *        pw_passwd;
    int                 pw_uid;
    int                 pw_gid;
    const char *        pw_age;
    const char *        pw_comment;
    const char *        pw_gecos;
    const char *        pw_dir;
    const char *        pw_shell;
    long                pw_audid;
    int                 pw_audflg;
};

LIBW32_API struct passwd *getpwuid(int);
LIBW32_API struct passwd *getpwnam(const char *);

LIBW32_API void         setpwent(void);
LIBW32_API struct passwd *getpwent(void);
LIBW32_API void         endpwent(void);
LIBW32_API int          getpwent_r(struct passwd *, char *, size_t, struct passwd **);

LIBW32_API int          getpwnam_r(const char *, struct passwd *, char *, size_t, struct passwd **);
LIBW32_API int          getpwuid_r(uid_t, struct passwd *, char *, size_t, struct passwd **);

__END_DECLS

#endif /*LIBW32_PWD_H_INCLUDED*/
