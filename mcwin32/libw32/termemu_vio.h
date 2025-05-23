#ifndef TERMEMU_VIO_H_INCLUDED
#define TERMEMU_VIO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(termemu_vio_h,"$Id: termemu_vio.h,v 1.14 2025/05/20 12:17:30 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * libtermemu console driver
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
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
 */

#include <stdarg.h>
#include <unistd.h>

enum wincols {
    WIN_COLOR_BACKGROUND = -3,
    WIN_COLOR_MIN = WIN_COLOR_BACKGROUND,
    WIN_COLOR_FOREGROUND = -2,
    WIN_COLOR_INVALID = -1,
    WIN_COLOR_BLACK = 0,
    WIN_COLOR_BLUE,
    WIN_COLOR_GREEN,
    WIN_COLOR_AQUA,
    WIN_COLOR_RED,
    WIN_COLOR_PURPLE,
    WIN_COLOR_BROWN,
    WIN_COLOR_LIGHTGREY,
    WIN_COLOR_DARKGREY,
    WIN_COLOR_BRIGHTBLUE,
    WIN_COLOR_BRIGHTGREEN,
    WIN_COLOR_BRIGHTAQUA,
    WIN_COLOR_BRIGHTRED,
    WIN_COLOR_BRIGHTPURPLE,
    WIN_COLOR_BRIGHTBROWN,
    WIN_COLOR_WHITE,
    WIN_COLOR_NUM
    };

enum vt_colors {
    VT_COLOR_BACKGROUND = -3,
    VT_COLOR_MIN = VT_COLOR_BACKGROUND,
    VT_COLOR_FOREGROUND = -2,
    VT_COLOR_INVALID = -1,
    VT_COLOR_BLACK = 0,
    VT_COLOR_RED,
    VT_COLOR_GREEN,
    VT_COLOR_YELLOW,
    VT_COLOR_BLUE,
    VT_COLOR_MAGENTA,
    VT_COLOR_CYAN,
    VT_COLOR_LIGHT_GREY,
    VT_COLOR_DARK_GREY,
    VT_COLOR_LIGHT_RED,
    VT_COLOR_LIGHT_GREEN,
    VT_COLOR_LIGHT_YELLOW,
    VT_COLOR_LIGHT_BLUE,
    VT_COLOR_LIGHT_MAGENTA,
    VT_COLOR_LIGHT_CYAN,
    VT_COLOR_WHITE,
    VT_COLOR_NUM,
    };

#define VIO_ALTCHARSET      0x0100
#define VIO_UNDERLINE       0x0200
#define VIO_BOLD            0x0400
#define VIO_BLINK           0x0800
#define VIO_INVERSE         0x1000
#define VIO_ITALIC          0x2000
#define VIO_STRIKE          0x4000
#define VIO_FAINT           0x8000
#define VIO_ATTRIBUTES      0xff00
#define VIO_MINCOLS         12
#define VIO_MINROWS         3
#define VIO_MAXCOLS         1024
#define VIO_MAXROWS         500

#if defined(TERMEMU_VIO_SOURCE)
typedef struct WCHAR_COLORINFO {                // color information.
#define VIO_FOBJECT         0x0001                  // object reference.
#define VIO_FNORMAL         0x0002                  // normal foreground/background.
#define VIO_F16             0x0004                  // vt/xterm 16 color palette.
#define VIO_F256            0x0008                  // vt/xterm 256 color palette (fg and/or bg).
#define VIO_FRGB            0x0010                  // RGB otherwise vt/xterm (fg and/or bg).
    WORD                Flags;
    WORD                Attributes;
    short               fg, bg;                 // WIN/VT/16 and VT/256
    COLORREF            fgrgb, bgrgb;           // RGB.
} WCHAR_COLORINFO;

typedef struct WCHAR_INFO {                     // extended CHAR_INFO
    WCHAR_COLORINFO Info;
    union {
        unsigned        UnicodeChar;
        unsigned char   AsciiChar;
    } Char;
} WCHAR_INFO;
#endif  // TERMEMU_VIO_SOURCE

__BEGIN_DECLS

#if defined(TERMEMU_VIO_LOCAL)
#define LIBVIO_API /*local*/
#elif defined(TERMEMU_VIO_STATIC)
#define LIBVIO_API static /*private*/
#else
#define LIBVIO_API LIBW32_API /*otherwise inherit libw32 */
#endif

LIBVIO_API void             vio_save(void);
LIBVIO_API void             vio_restore(void);
LIBVIO_API void             vio_save_lines(int active);
LIBVIO_API void             vio_restore_lines(int top, int bottom, int to);
LIBVIO_API int              vio_screenbuffersize(void);

LIBVIO_API int              vio_open(int *rows, int *cols);
LIBVIO_API void             vio_close(void);
LIBVIO_API void             vio_config_truecolor(int truecolor);
LIBVIO_API int              vio_winch(int *rows, int *cols);
LIBVIO_API void             vio_get_size(int *rows, int *cols);
LIBVIO_API int              vio_toggle_size(int *rows, int *cols);

LIBVIO_API void             vio_goto(int row, int col);
LIBVIO_API void             vio_cursor_show(void);
LIBVIO_API void             vio_cursor_hide(void);
LIBVIO_API int              vio_cursor_state(void);
LIBVIO_API int              vio_row(void);
LIBVIO_API int              vio_column(void);

LIBVIO_API void             vio_define_winattr_native(int obj, uint16_t attributes);
LIBVIO_API void             vio_define_winattr(int obj, int fg, int bg, uint16_t attributes);
LIBVIO_API void             vio_define_vtattr(int obj, int fg, int bg, uint16_t attributes);
LIBVIO_API void             vio_define_rgbattr(int obj, int32_t fg, int32_t bg, uint16_t attributes);
LIBVIO_API int              vio_define_attr(int obj, const char *what, const char *fg, const char *bg);
LIBVIO_API void             vio_define_flags(int obj, uint16_t attributes);
LIBVIO_API void             vio_set_colorattr(int obj);

LIBVIO_API void             vio_set_wincolor_native(uint16_t attributes);
LIBVIO_API void             vio_set_wincolor(int fg, int bg, uint16_t attributes);
LIBVIO_API void             vio_set_vtcolor(int fg, int bg, uint16_t attributes);
LIBVIO_API void             vio_set_rgbcolor(int32_t fg, int32_t bg, uint16_t attributes);

LIBVIO_API void             vio_set_winforeground(int color, int32_t rgb /*optional, otherwise -1*/);
LIBVIO_API void             vio_set_winbackground(int color, int32_t rgb /*optional, otherwise -1*/);
LIBVIO_API void             vio_set_vtforeground(int color, int32_t rgb /*optional, otherwise -1*/);
LIBVIO_API void             vio_set_vtbackground(int color, int32_t rgb /*optional, otherwise -1*/);
LIBVIO_API void             vio_normal_video(void);
LIBVIO_API void             vio_atputc(int row, int col, unsigned ch, unsigned cnt);
LIBVIO_API int              vio_atputs(int row, int col, const char *text);
LIBVIO_API int              vio_atputsn(int row, int col, const char *text, unsigned len);
LIBVIO_API int              vio_atvprintf(int row, int col, const char *fmt, va_list ap);
LIBVIO_API int              vio_atprintf(int row, int col, const char *fmt, ...);
LIBVIO_API void             vio_putc(unsigned ch, unsigned cnt, int move);
LIBVIO_API void             vio_flush(void);

int vio_wcwidth(wchar_t ucs);

#if !defined(TERMEMU_VIO_SOURCE)
#undef LIBVIO_API
#endif

__END_DECLS

#endif //TERMEMU_VIO_H_INCLUDED
