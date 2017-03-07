/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 interface support
 *
 * Copyright (c) 2007, 2012 - 2017 Adam Young.
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


int
w32_getsysdir(
    int id, char *buf, int maxlen )
{
    char szPath[ MAX_PATH ];
    HRESULT hres;
    int len;

    switch (id) {
    case SYSDIR_TEMP:
        id = CSIDL_INTERNET_CACHE;
        break;
    default:
        return -1;
    }

    hres = SHGetSpecialFolderPath(NULL, szPath, id, FALSE);
    if (SUCCEEDED(hres) && 
            (len = (int)strlen(szPath)) <= maxlen) {
        (void) strcpy(buf, (const char *)szPath);
        return len;
    }            
    return (-1);
}


const char *
w32_selectfolder( 
    const char *strMessage, char *szBuffer)
{
    char const * Result = NULL;
    BROWSEINFO BrowseInfo;
    LPITEMIDLIST pList;

    /* Throw display dialog */
    memset(&BrowseInfo, 0, sizeof(BrowseInfo));
    BrowseInfo.hwndOwner = NULL;                /* XXX */
    BrowseInfo.pszDisplayName = szBuffer;
    BrowseInfo.lpszTitle = strMessage;
    BrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
    pList = SHBrowseForFolder(&BrowseInfo);

    /* Convert from MIDLISt to real string path */
    if (pList != NULL) {
        SHGetPathFromIDList(pList, szBuffer);
        CoTaskMemFree(pList);
        Result = szBuffer;
    }
    return (Result);
}
