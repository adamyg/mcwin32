#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_sysdir_c,"$Id: w32_sysdir.c,v 1.8 2021/11/30 13:06:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 interface support
 *
 * Copyright (c) 2007, 2012 - 2021 Adam Young.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==end==
 */

#include "win32_internal.h"
#include "win32_misc.h"

#include <shlobj.h>                             /* SHXxxxx functions */
#include <shlguid.h>
#include <unistd.h>


LIBW32_API int
w32_getsysdir(int id, char *buf, int maxlen)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t wpath[WIN32_PATH_MAX];

        if (w32_getsysdirW(id, wpath, _countof(wpath)) > 0) {
            return w32_wc2utf(wpath, buf, maxlen);
        }
        return -1;
    }
#endif

    return w32_getsysdirA(id, buf, maxlen);
}


static int
map_csidl(int id)
{
    // https://docs.microsoft.com/en-us/windows/win32/shell/csidl
    switch (id) {
    case SYSDIR_TEMP:
        return CSIDL_INTERNET_CACHE;
    case SYSDIR_WINDOWS:
        return CSIDL_WINDOWS;
    case SYSDIR_SYSTEM:
        return CSIDL_SYSTEM;
    case SYSDIR_PROGRAM_FILES:
        return CSIDL_PROGRAM_FILES;
    }
    return -1;
}


LIBW32_API int
w32_getsysdirA(int id, char *buf, int maxlen)
{
    char t_path[ MAX_PATH ], *path = buf;
    HRESULT hres;
    int len;

    if (NULL == buf || maxlen < 4 ||
            -1 == (id = map_csidl(id))) {
        return -1;
    }

    if (maxlen < MAX_PATH) path = t_path;
    hres = SHGetSpecialFolderPathA(NULL, path, id, FALSE);
    if (SUCCEEDED(hres)) {
        len = (int)strlen(path);
        if (path == buf) {                      // direct
            return len;
        } else if (len < maxlen) {              // indirect
            (void) strcpy(buf, (const char *)t_path);
            return len;
        }
    }
    return -1;
}


LIBW32_API int
w32_getsysdirW(int id, wchar_t *buf, int maxlen)
{
    wchar_t t_path[ MAX_PATH ], *path = buf;
    HRESULT hres;
    int len;

    if (NULL == buf || maxlen < 4 ||
            -1 == (id = map_csidl(id))) {
        return -1;
    }

    if (maxlen < MAX_PATH) path = t_path;
    hres = SHGetSpecialFolderPathW(NULL, path, id, FALSE);
    if (SUCCEEDED(hres)) {
        len = (int)wcslen(path);
        if (path == buf) {                      // direct
            return len;
        } else if (len < maxlen) {              // indirect
            (void) wcscpy(buf, (const wchar_t *)t_path);
            return len;
        }
    }
    return -1;
}


LIBW32_API const char *
w32_selectfolder(const char *message, char *buffer, int buflen)
{
    if (NULL == message || NULL == buffer || buflen < MAX_PATH) {
        return NULL;
    }

#if defined(UTF8FILENAMES)
     if (w32_utf8filenames_state()) {
        wchar_t wpath[MAX_PATH + 1], *wmessage;
        const wchar_t *wresult;
        char *result = NULL;

        if (NULL != (wmessage = w32_utf2wca(message, NULL))) {
            if (NULL != (wresult = w32_selectfolderW(wmessage, wpath, _countof(wpath)))) {
                if (w32_wc2utf(wresult, buffer, buflen) > 0) {
                    result = buffer;
                }
            }
            free(wmessage);
        }
        return result;
    }
#endif

    return w32_selectfolderA(message, buffer, buflen);
}


LIBW32_API const char *
w32_selectfolderA(const char *message, char *buffer, int buflen)
{
    const char *result = NULL;
    BROWSEINFOA bi;
    LPITEMIDLIST pl;

    if (NULL == message || NULL == buffer || buflen < MAX_PATH) {
        return NULL;
    }

    /* Throw display dialog */
    memset(&bi, 0, sizeof(bi));
    bi.hwndOwner = NULL;
    bi.pszDisplayName = buffer;
    bi.lpszTitle = message;
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    pl = SHBrowseForFolderA(&bi);

    /* Convert from MIDLISt to real string path */
    if (pl != NULL) {
        SHGetPathFromIDListA(pl, buffer);
        CoTaskMemFree(pl);
        result = buffer;
    }
    return result;
}


LIBW32_API const wchar_t *
w32_selectfolderW(const wchar_t *message, wchar_t *buffer, int buflen)
{
    const wchar_t *result = NULL;
    BROWSEINFOW bi;
    LPITEMIDLIST pl;

    if (NULL == message || NULL == buffer || buflen < MAX_PATH) {
        return NULL;
    }

    /* Throw display dialog */
    memset(&bi, 0, sizeof(bi));
    bi.hwndOwner = NULL;
    bi.pszDisplayName = buffer;
    bi.lpszTitle = message;
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    pl = SHBrowseForFolderW(&bi);

    /* Convert from MIDLISt to real string path */
    if (pl != NULL) {
        SHGetPathFromIDListW(pl, buffer);
        CoTaskMemFree(pl);
        result = buffer;
    }
    return result;
}

/*end*/
