#ifndef LIBW32_PWD_H_INCLUDED
#define LIBW32_PWD_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <pwd.h> implementation
 *
 *      pwd.h - password structure
 *
 * Copyright (c) 2007, 2012 - 2015 Adam Young.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * The Midnight Commander is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ==end==
 */

#include <sys/cdefs.h>

__BEGIN_DECLS

/*
 *  The <pwd.h> header shall provide a definition for struct passwd,
 *   which shall include at least the following members:
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

struct passwd *         getpwent(void);
struct passwd *         getpwuid(int);
struct passwd *         getpwnam(const char *);
void                    setpwent(void);
void                    endpwent(void);

//  int                 getpwnam_r(const char *, struct passwd *, char *, size_t, struct passwd **);
//  int                 getpwuid_r(uid_t, struct passwd *, char *, size_t, struct passwd **);

__END_DECLS

#endif /*LIBW32_PWD_H_INCLUDED*/
