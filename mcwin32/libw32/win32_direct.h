#ifndef LIBW32_DIRECT_H_INCLUDED
#define LIBW32_DIRECT_H_INCLUDED

/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 2021 - 2022, Adam Young.
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

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#define DIR_MAGIC               0x57333264  // W32d
#define DIR_WMAGIC              0x57333244  // W32D
#define DIR_UMAGIC              0x57333255  // W32U

#define DIR_FISHPF              0x0001
#define DIR_FISUTF8             0x0002
#define DIR_FHAVESTATS          0x0040
    //#define DIR_FDYNAMICREAD        0x1000      // TODO

#define PATH_SEP                '/'
#define PATH_SEP_STR            "/"
#define PATH_SEP2               '\\'
#define PATH_SEP_STR2           "\\"
#define IS_PATH_SEP(c)          ((c) == PATH_SEP || (c) == PATH_SEP2)

DIR *                   w32_dir_alloc(void);
void                    w32_dir_free(DIR *dp);
int                     w32_dir_identifier(void);

typedef int (*unc_push_t)(void *data, const wchar_t *);

int                     w32_unc_iterateA(const char *servername, unc_push_t push, void *data);
int                     w32_unc_iterateW(const wchar_t *servername, unc_push_t push, void *data);

DIR *                   w32_unc_opendirA(const char *dirname);
DIR *                   w32_unc_opendirW(const wchar_t *dirname);
struct dirent *         w32_unc_readdirA(DIR *dp);
struct dirent *         w32_unc_readdirW(DIR *dp);
int                     w32_unc_closedir(DIR *dp);

int                     w32_unc_validA(const char *path);
int                     w32_unc_validW(const wchar_t *path);

int                     w32_unc_rootA(const char *path, int *length);
int                     w32_unc_rootW(const wchar_t *path, int *length);

__END_DECLS

#endif /*LIBW32_DIRECT_H_INCLUDED*/
