/* -*- mode: c; tabs: 4 -*- */
#ifndef WIN32_SYS_UTSNAME_H_INCLUDED
#define WIN32_SYS_UTSNAME_H_INCLUDED

/* -*- mode: c; indent-width: 4; -*- */
/*
 *
 * ==end==
 */

#define _UTSNAME_LENGTH             64

struct utsname {
    char        sysname[_UTSNAME_LENGTH];
    char        nodename[_UTSNAME_LENGTH];
    char        release[_UTSNAME_LENGTH];
    char        version[_UTSNAME_LENGTH];
    char        machine[_UTSNAME_LENGTH];
};

int             uname( struct utsname *buf );

#endif  /*WIN32_SYS_UTSNAME_H_INCLUDED*/
