#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_fopen_c, "$Id: w32_fopen.c,v 1.4 2025/07/23 15:04:07 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 system io functionality
 *
 *  fopen
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x501)
#undef _WIN32_WINNT                             /* XP+ */
#define _WIN32_WINNT 0x0501
#endif

#if defined(__MINGW32__)
#undef  _WIN32_VER
#define _WIN32_VER _WIN32_WINNT
#endif

#include "win32_internal.h"
#include "win32_misc.h"
#include "win32_io.h"

static const char dev_null[] = "/dev/null";
static const char dev_stdin[] = "/dev/stdin";
static const char dev_stdout[] = "/dev/stdout";
static const char dev_stderr[] = "/dev/stderr";

static int   W32OpenModeA(const char *mode);
static int   W32OpenModeW(const wchar_t *mode);

static FILE *W32OpenStreamA(const char *path, const char *mode);
static FILE *W32OpenStreamW(const wchar_t *path, const wchar_t *mode);


/*
//  NAME
//      fopen - open a stream
//
//  SYNOPSIS
//
//      #include <stdio.h>
//
//      FILE *fopen(const char *path, const char *mode);
//
//  DESCRIPTION
//
//      The fopen() function shall open the file whose pathname is the string pointed to by filename,
//      and associates a stream with it.
//
//      The mode argument points to a string. If the string is one of the following,
//      the file shall be opened in the indicated mode. Otherwise, the behavior is undefined.
//
//        "r" or "rb"
//          - Open file for reading.
//
//        "w" or "wb"
//          - Truncate to zero length or create file for writing.
//
//        "a" or "ab"
//           - Append; open or create file for writing at end-of-file.
//
//        "r+" or "rb+" or "r+b"
//           - Open file for update (reading and writing).
//
//        "w+" or "wb+" or "w+b"
//           - Truncate to zero length or create file for update.
//
//        "a+" or "ab+" or "a+b"
//           - Append; open or create file for update, writing at end-of-file.
//
//      Opening a file with read mode (r as the first character in the mode argument) shall fail
//      if the file does not exist or cannot be read.
//
//      Opening a file with append mode (a as the first character in the mode argument) shall
//      cause all subsequent writes to the file to be forced to the then current end-of-file,
//      regardless of intervening calls to fseek().
//
//      When a file is opened with update mode ( '+' as the second or third character in the mode
//      argument), both input and output may be performed on the associated stream. However, the
//      application shall ensure that output is not directly followed by input without an
//      intervening call to fflush() or to a file positioning function (fseek(), fsetpos(), or
//      rewind()), and input is not directly followed by output without an intervening call to a
//      file positioning function, unless the input operation encounters end-of-file.
//
//      When opened, a stream is fully buffered if and only if it can be determined not to refer
//      to an interactive device. The error and end-of-file indicators for the stream shall be
//      cleared.
//
//      If mode is "w", "wb", "a", "ab", "w+", "wb+", "w+b", "a+", "ab+", or "a+b", and the file
//      did not previously exist, upon successful completion, the fopen() function shall mark for
//      update the 'st_atime', 'st_ctime', and 'st_mtime' fields of the file and the 'st_ctime'
//      and 'st_mtime' fields of the parent directory.
//
//      If mode is "w", "wb", "w+", "wb+", or "w+b", and the file did previously exist, upon successful
//      completion, fopen() shall mark for update the 'st_ctime' and 'st_mtime' fields of the file.
//      The fopen() function shall allocate a file descriptor as open() does.
//
//      After a successful call to the fopen() function, the orientation of the stream shall be
//      cleared, the encoding rule shall be cleared, and the associated mbstate_t object shall be
//      set to describe an initial conversion state.
//
//      The largest value that can be represented correctly in an object of type off_t shall be
//      established as the offset maximum in the open file description.
//
//  RETURN VALUE
//
//      Upon successful completion, fopen() shall return a pointer to the object controlling the stream.
//      Otherwise, a null pointer shall be returned, and errno shall be set indicate the error.
//
//  ERRORS
//
*/

FILE *
w32_fopen(const char *path, const char *mode)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (path && mode) {
            wchar_t wpath[WIN32_PATH_MAX], wmode[32];

            if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
                if (w32_utf2wc(mode, wmode, _countof(wmode)) > 0) {
                    return w32_fopenW(wpath, wmode);
                }
            }
            return NULL;
        }
    }
#endif  //UTF8FILENAMES

    return w32_fopenA(path, mode);
}


FILE *
w32_fopenA(const char *path, const char *mode)
{
    char symbuf[WIN32_PATH_MAX];
    FILE *file = NULL;
    int flags, fd, ret = 0;

    if (NULL == path || NULL == mode) {
        errno = EFAULT;
        return NULL;
    }

    if ((flags = W32OpenModeA(mode)) == -1) {
        errno = EINVAL;
        return NULL;
    }

    if (0 == w32_iostricmpA(path, dev_null)) {
        path = "NUL";                           // redirect

    } else if (0 == w32_iostricmpA(path, dev_stdin)) {
        if ((fd = w32_osfdup(GetStdHandle(STD_INPUT_HANDLE), flags)) != -1) {
            return fdopen(fd, mode);
        }

    } else if (0 == w32_iostricmpA(path, dev_stdout)) {
        if ((fd = w32_osfdup(GetStdHandle(STD_OUTPUT_HANDLE), flags)) != -1) {
            return fdopen(fd, mode);
        }

    } else if (0 == w32_iostricmpA(path, dev_stderr)) {
        if ((fd = w32_osfdup(GetStdHandle(STD_ERROR_HANDLE), flags)) != -1) {
            return fdopen(fd, mode);
        }

    } else if (w32_resolvelinkA(path, symbuf, _countof(symbuf), &ret) == NULL) {
        if (ret < 0) {
           if ((mode[0] != 'r') &&              // attempt creation
                    (ret == -ENOTDIR || ret == -ENOENT)) {
                ret = 0;
            } else {
                errno = -ret;
            }
        }
    } else {
        path = symbuf;                          // follow link
    }

    if (ret < 0 || (file = W32OpenStreamA(path, mode)) == NULL) {
        if (ENOTDIR == errno || ENOENT == errno) {
            if (path != symbuf &&               // component error, expand embedded shortcut
                w32_expandlinkA(path, symbuf, _countof(symbuf), SHORTCUT_COMPONENT)) {
                file = W32OpenStreamA(symbuf, mode);
            }
        }
    }

    return file;
}


FILE*
w32_fopenW(const wchar_t* path, const wchar_t* mode)
{
    wchar_t symbuf[WIN32_PATH_MAX];
    FILE *file = NULL;
    int flags, fd, ret = 0;

    if (NULL == path || NULL == mode) {
        errno = EFAULT;
        return NULL;
    }

    if ((flags = W32OpenModeW(mode)) == -1) {
        errno = EINVAL;
        return NULL;
    }

    if (0 == w32_iostricmpW(path, dev_null)) {
        path = L"NUL";                          // redirect

    } else if (0 == w32_iostricmpW(path, dev_stdin)) {
        if ((fd = w32_osfdup(GetStdHandle(STD_INPUT_HANDLE), flags)) != -1) {
            return _wfdopen(fd, mode);
        }

    } else if (0 == w32_iostricmpW(path, dev_stdout)) {
        if ((fd = w32_osfdup(GetStdHandle(STD_OUTPUT_HANDLE), flags)) != -1) {
            return _wfdopen(fd, mode);
        }

    } else if (0 == w32_iostricmpW(path, dev_stderr)) {
        if ((fd = w32_osfdup(GetStdHandle(STD_ERROR_HANDLE), flags)) != -1) {
            return _wfdopen(fd, mode);
        }

    } else if (w32_resolvelinkW(path, symbuf, _countof(symbuf), &ret) == NULL) {
        if (ret < 0) {
            if ((mode[0] != 'r') &&             // attempt creation
                    (ret == -ENOTDIR || ret == -ENOENT)) {
                ret = 0;
            } else {
                errno = -ret;
            }
        }

    } else {
        path = symbuf;                          // follow link
    }

    if (ret < 0 || (file = W32OpenStreamW(path, mode)) == NULL) {
        if (ENOTDIR == errno || ENOENT == errno) {
            if (path != symbuf &&               // component error, expand embedded shortcut
                w32_expandlinkW(path, symbuf, _countof(symbuf), SHORTCUT_COMPONENT)) {
                file = W32OpenStreamW(symbuf, mode);
            }
        }
    }

    return file;
}


static int
W32OpenModeA(const char *mode)
{
    int plus = 0, done = 0, flags = 0, fmode = 0;

    //
    //  r or rb             Open file for reading.
    //  w or wb             Truncate to zero length or create file for writing.
    //  a or ab             Append; open or create file for writing at end-of-file.
    //  r+ or rb+ or r+b    Open file for update (reading and writing).
    //  w+ or wb+ or w+b    Truncate to zero length or create file for update.
    //  a+ or ab+ or a+b    Append; open or create file for update, writing at end-of-file.
    //
    //  b=binary, otherwise t=text
    //

    switch (*mode++) {
    case 'r': flags |= _O_RDONLY; break;
    case 'w': flags |= _O_WRONLY|_O_TRUNC; break;
    case 'a': flags |= _O_WRONLY|_O_APPEND; break;
    default:
        return -1;
    }

    while (*mode && ! done) {
        switch (*mode++) {
        case '+': // update
            if (! plus) {
                flags &= ~(_O_RDONLY | _O_WRONLY);
                flags |= _O_RDWR;
                plus = 1;
            } else {
                done = 1;
            }
            break;
        case 't': // text
            if (! fmode) {
                flags |= _O_TEXT;
                fmode = 1;
            } else {
                done = 1;
            }
            break;
        case 'b': // binary
            if (! fmode) {
                flags |= _O_BINARY;
                fmode = 1;
            } else {
                done = 1;
            }
            break;
        default: // other; ignore
            done = 1;
            break;
        }
    }

    if (! fmode) { // default mode
#if defined(__WATCOMC__)
        flags |= _fmode;
#else
        if (0 == _get_fmode(&fmode)) {
            flags |= fmode;
        }
#endif
    }

    return flags;
}


static int
W32OpenModeW(const wchar_t *mode)
{
    unsigned length = 0;
    char amode[8] = {0};

    while (*mode && *mode < 0x7f && length != (_countof(amode) - 1)) {
        amode[length++] = (char) *mode++;       // import ascii
    }
    return W32OpenModeA(amode);
}


static FILE *
W32OpenStreamA(const char *path, const char *mode)
{
    char *expath;
    FILE *file;

    // open
    if (NULL != (expath = w32_extendedpathA(path))) {
        path = expath;                          // extended abs-path
    }

    if ((file = fopen(path, mode)) != NULL) {
        w32_fdregister(_fileno(file));
    }

    free((void *)expath);                       // release temporary

    return file;
}


static FILE *
W32OpenStreamW(const wchar_t *path, const wchar_t *mode)
{
    wchar_t *expath;
    FILE* file;

    // open
    if (NULL != (expath = w32_extendedpathW(path))) {
        path = expath;                          // extended abs-path
    }

    if ((file = _wfopen(path, mode)) != NULL) {
        w32_fdregister(_fileno(file));
    }

    free((void *)expath);                       // release temporary

    return file;
}

/*end*/
