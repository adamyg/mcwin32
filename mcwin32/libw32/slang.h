#ifndef LIBW32_SLANG_H_INCLUDED
#define LIBW32_SLANG_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <slang.h> partial implementation
 *
 * Copyright (c) 2007, 2012 - 2022 Adam Young.
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
#include <stdarg.h>

__BEGIN_DECLS

#define HAS_ACS_AS_PCCHARS      1

#define SLang_TT_Baud_Rate      19000

#define SLANG_VERSION           30000           /* interface version only; needs confirming */
#define SLANG_VERSION_STRING    "3.0.0 libw32"

LIBW32_API extern const int     SLang_Version;
    /*
     *  Used to test for specific Slang features, currently only true color support.
     *
     *  true-color was added versions:
     *      64bit   20301   slang-2.3.1
     *	    32bit   30000   slang-3 (projected)
     */

typedef unsigned char          SLsmg_Char_Type;
typedef uint32_t               SLtt_Char_Type;
typedef uint32_t               SLwchar_Type;

LIBW32_API extern int          SLsmg_Display_Eight_Bit;

LIBW32_API extern int          SLtt_Screen_Rows;
LIBW32_API extern int          SLtt_Screen_Cols;
LIBW32_API extern int          SLtt_Ignore_Beep;
LIBW32_API extern int          SLtt_Use_Ansi_Colors;
LIBW32_API extern int          SLtt_Try_Termcap;
#define                        SLtt_Has_Alt_Charset 1  /* FIXME */

 /*
  * not implemented
  *     global: SLsmg_Newline_Behavior
  *
#define SLSMG_NEWLINE_IGNORED   0               -* default *-
#define SLSMG_NEWLINE_MOVES     1               -* moves to next line, column 0 *-
#define SLSMG_NEWLINE_SCROLLS   2               -* moves but scrolls at bottom of screen *-
#define SLSMG_NEWLINE_PRINTABLE 3               -* prints as ^J *-
  */

#define SLTT_BOLD_MASK          0x01000000UL    /* Bold */
#define SLTT_BLINK_MASK         0x02000000UL    /* Blinking (non implemented) */
#define SLTT_ULINE_MASK         0x04000000UL    /* Underline */
#define SLTT_REV_MASK           0x08000000UL    /* Reverse */
#define SLTT_ITALIC_MASK        0x10000000UL    /* Italic (4.8.14) */
#define SLTT_ALTC_MASK          0x20000000UL    /* Alternative Character */
#define SLTT_ATTRIBUTE          0x80000000UL    /* Internal attribute, otherwise native */

/* SLtt_Ignore_Beep options */
#define SLTT_BEEP_AUDIBLE       0x01
#define SLTT_BEEP_FLASH         0x02
#define SLTT_BEEP_INVERT        0x04
#define SLTT_BEEP_LEGACY        0x80

/* VT100-compatible symbols -- box chars */
#define XTERM_ACS_ULCORNER      'l'
#define XTERM_ACS_LLCORNER      'm'
#define XTERM_ACS_URCORNER      'k'
#define XTERM_ACS_LRCORNER      'j'
#define XTERM_ACS_RTEE          'u'
#define XTERM_ACS_LTEE          't'
#define XTERM_ACS_BTEE          'v'
#define XTERM_ACS_TTEE          'w'
#define XTERM_ACS_HLINE         'q'
#define XTERM_ACS_VLINE         'x'
#define XTERM_ACS_PLUS          'n'

/* VT100-compatible symbols -- other */
#define XTERM_ACS_S1            'o'
#define XTERM_ACS_S9            's'
#define XTERM_ACS_DIAMOND       '`'
#define XTERM_ACS_CKBOARD       'a'
#define XTERM_ACS_DEGREE        'f'
#define XTERM_ACS_PLMINUS       'g'
#define XTERM_ACS_BULLET        '~'

/* SysV curses */
#define XTERM_ACS_LARROW        ','
#define XTERM_ACS_RARROW        '+'
#define XTERM_ACS_DARROW        '.'
#define XTERM_ACS_UARROW        '-'
#define XTERM_ACS_BOARD         'h'
#define XTERM_ACS_LANTERN       'i'
#define XTERM_ACS_BLOCK         '0'

/* Undocumented SysV symbols */
#define XTERM_ACS_S3            'p'
#define XTERM_ACS_S7            'r'
#define XTERM_ACS_LEQUAL        'y'
#define XTERM_ACS_GEQUAL        'z'
#define XTERM_ACS_PI            '{'
#define XTERM_ACS_NEQUAL        '|'
#define XTERM_ACS_STERLING      '}'

#define SLSMG_COLOR_BLACK       0
#define SLSMG_COLOR_RED         1
#define SLSMG_COLOR_GREEN       2
#define SLSMG_COLOR_BROWN       3
#define SLSMG_COLOR_BLUE        4
#define SLSMG_COLOR_MAGENTA     5
#define SLSMG_COLOR_CYAN        6
#define SLSMG_COLOR_LGRAY       7
#define SLSMG_COLOR_GRAY        8

#define SLSMG_COLOR_BRIGHT_RED      9
#define SLSMG_COLOR_BRIGHT_GREEN    10
#define SLSMG_COLOR_BRIGHT_BROWN    11
#define SLSMG_COLOR_BRIGHT_BLUE     12
#define SLSMG_COLOR_BRIGHT_MAGENTA  13
#define SLSMG_COLOR_BRIGHT_CYAN     14
#define SLSMG_COLOR_BRIGHT_WHITE    15

#define SLSMG_HLINE_CHAR        XTERM_ACS_HLINE
#define SLSMG_VLINE_CHAR        XTERM_ACS_VLINE
#define SLSMG_ULCORN_CHAR       XTERM_ACS_ULCORNER
#define SLSMG_URCORN_CHAR       XTERM_ACS_URCORNER
#define SLSMG_LLCORN_CHAR       XTERM_ACS_LLCORNER
#define SLSMG_LRCORN_CHAR       XTERM_ACS_LRCORNER
#define SLSMG_RTEE_CHAR         XTERM_ACS_RTEE
#define SLSMG_LTEE_CHAR         XTERM_ACS_LTEE
#define SLSMG_UTEE_CHAR         XTERM_ACS_TTEE
#define SLSMG_DTEE_CHAR         XTERM_ACS_BTEE
#define SLSMG_PLUS_CHAR         XTERM_ACS_PLUS
#define SLSMG_CKBRD_CHAR        XTERM_ACS_CKBOARD
#define SLSMG_DIAMOND_CHAR      XTERM_ACS_DIAMOND
#define SLSMG_DEGREE_CHAR       XTERM_ACS_DEGREE
#define SLSMG_PLMINUS_CHAR      XTERM_ACS_PLMINUS
#define SLSMG_BULLET_CHAR       XTERM_ACS_BULLET
#define SLSMG_LARROW_CHAR       XTERM_ACS_LARROW
#define SLSMG_RARROW_CHAR       XTERM_ACS_RARROW
#define SLSMG_DARROW_CHAR       XTERM_ACS_DARROW
#define SLSMG_UARROW_CHAR       XTERM_ACS_UARROW
#define SLSMG_BOARD_CHAR        XTERM_ACS_BOARD
#define SLSMG_BLOCK_CHAR        XTERM_ACS_BLOCK

LIBW32_API int              SLsmg_init_smg (void);
LIBW32_API int              SLsmg_reinit_smg (void);
LIBW32_API void             SLsmg_reset_smg (void);
LIBW32_API void             SLsmg_togglesize (void);

LIBW32_API void             SLsmg_refresh (void);
LIBW32_API void             SLsmg_gotorc (int, int);
LIBW32_API void             SLsmg_set_color (int);
LIBW32_API void             SLsmg_set_char_set (int alt_charset);
LIBW32_API int              SLsmg_get_char_set (void);
LIBW32_API void             SLsmg_write_char (SLtt_Char_Type);
LIBW32_API void             SLsmg_write_string (const char *);
LIBW32_API void             SLsmg_write_nstring (const char *s, unsigned n);
LIBW32_API void             SLsmg_printf (const char *, ...);
LIBW32_API void             SLsmg_vprintf (const char *fmt, va_list);
LIBW32_API void             SLsmg_normal_video (void);
LIBW32_API void             SLsmg_touch_lines (int, unsigned);
LIBW32_API void             SLsmg_touch_screen (void);
LIBW32_API void             SLsmg_draw_object (int, int, SLwchar_Type);
LIBW32_API void             SLsmg_draw_box (int, int, unsigned, unsigned);
LIBW32_API void             SLsmg_draw_vline (int cnt);
LIBW32_API void             SLsmg_draw_hline (int cnt);
LIBW32_API int              SLsmg_get_row (void);
LIBW32_API int              SLsmg_get_column (void);
LIBW32_API void             SLsmg_fill_region (int, int, unsigned, unsigned, SLwchar_Type);
LIBW32_API void             SLsmg_set_color_in_region (int color, int r, int c, unsigned dr, unsigned dc);
LIBW32_API void             SLsmg_forward (int n);

LIBW32_API int              SLtt_initialize (const char *term);
LIBW32_API void             SLtt_save (void);
LIBW32_API void             SLtt_restore (void);
//  LIBW32_API int              SLtt_set_font (const char *font);
LIBW32_API const char *     SLtt_get_font (char *buffer, size_t buflen);
LIBW32_API void             SLtt_set_color (int, const char *, const char *, const char *);
LIBW32_API void             SLtt_set_mono (int, char *, SLtt_Char_Type);
LIBW32_API void             SLtt_add_color_attribute (int, SLtt_Char_Type);

//  LIBW32_API void             SLtt_write_string (const char *);
LIBW32_API void             SLtt_beep (void);
//  LIBW32_API void             SLtt_normal_video (void);

LIBW32_API int              SLtt_tgetnum (const char *);
LIBW32_API char *           SLtt_tigetent (const char *);
LIBW32_API char *           SLtt_tigetstr (const char *, char **);

__END_DECLS

#endif /*LIBW32_SLANG_H_INCLUDED*/
