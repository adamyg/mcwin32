#ifndef LIBW32_SLANG_H_INCLUDED
#define LIBW32_SLANG_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <slang.h> implementation
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
#include <stdarg.h>

__BEGIN_DECLS

#define HAS_ACS_AS_PCCHARS      1

#define SLang_TT_Baud_Rate      19000

typedef unsigned char   SLsmg_Char_Type;
typedef uint32_t        SLtt_Char_Type;
typedef uint32_t        SLwchar_Type;

extern int              SLsmg_Display_Eight_Bit;
extern int              SLtt_Try_Termcap;
extern int              SLtt_Screen_Rows;
extern int              SLtt_Screen_Cols;
extern int              SLtt_Ignore_Beep;
extern int              SLtt_Use_Ansi_Colors;
#define                 SLtt_Has_Alt_Charset 1

#define SLTT_BOLD_MASK	        0x01000000UL
#define SLTT_BLINK_MASK	        0x02000000UL
#define SLTT_ULINE_MASK	        0x04000000UL
#define SLTT_REV_MASK	        0x08000000UL
#define SLTT_ALTC_MASK          0x10000000UL
#define __SLTT_ATTR_MASK        0x1F000000UL

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

extern int              SLsmg_init_smg (void);
extern int              SLsmg_reinit_smg (void);
extern void             SLsmg_reset_smg (void);
extern void             SLsmg_togglesize (void);

extern void             SLsmg_refresh (void);
extern void             SLsmg_gotorc (int, int);
extern void             SLsmg_set_color (int);
extern void             SLsmg_set_char_set (int alt_charset);
extern int              SLsmg_get_char_set (void);
extern void             SLsmg_write_char (SLtt_Char_Type);
extern void             SLsmg_write_string (const char *);
extern void             SLsmg_printf (const char *, ...);
extern void             SLsmg_vprintf (const char *fmt, va_list);
extern void             SLsmg_normal_video (void);
extern void             SLsmg_touch_lines (int, unsigned int);
extern void             SLsmg_touch_screen (void);
extern void             SLsmg_draw_object (int, int, SLwchar_Type);
extern void             SLsmg_draw_box (int, int, unsigned int, unsigned int);
extern void             SLsmg_draw_vline (int cnt);
extern void             SLsmg_draw_hline (int cnt);
extern int              SLsmg_get_row (void);
extern int              SLsmg_get_column (void);
extern void             SLsmg_fill_region (int, int, unsigned int, unsigned int, SLwchar_Type);

extern void             SLtt_set_color (int, const char *, const char *, const char *);
extern void             SLtt_set_mono (int, char *, SLtt_Char_Type);
extern void             SLtt_add_color_attribute (int, SLtt_Char_Type);

extern void             SLtt_write_string (const char *);
extern void             SLtt_beep (void);
extern void             SLtt_normal_video (void);

extern int              SLtt_tgetnum (const char *);
extern char *           SLtt_tigetent (const char *);
extern char *           SLtt_tigetstr (const char *, char **);

__END_DECLS

#endif /*LIBW32_SLANG_H_INCLUDED*/
