/* -*- mode: c; indent-width: 4; -*- */
/*
   WIN32 tty/tty-win32 implementation.

   Copyright (C) 2012
   The Free Software Foundation, Inc.

   Written by: Adam Young 2012 - 2022

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <config.h>

#include "libw32.h"
#include "libw32/termemu_vio.h"                 /* vio driver */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib/global.h"
#include "lib/strutil.h"

#include "lib/tty/tty.h"
#include "lib/tty/tty-slang.h"
#include "lib/tty/tty-internal.h"
#include "lib/tty/color-internal.h"
#include "lib/tty/mouse.h"                      /* use_mouse_p */
#include "lib/tty/key.h"                        /* console_alert_mode */
#include "lib/tty/color.h"                      /* tty_use_256colors(); */
#include "lib/tty/win.h"

#include "src/consaver/cons.saver.h"            /* CONSOLE_xxx */

#include "win32_key.h"

static int tty_mouse_enabled = 0;               /* mouse mode; true/false */
static wchar_t original_title[512];             /* original title */

int reset_hp_softkeys = 0;                      /* command line argument */


void
tty_init (gboolean mouse_enable, gboolean is_xterm)
{
    if (NULL == getenv("TERM")) {
        (void) putenv("TERM=dos");
    }

    if (NULL == getenv("COLORTERM")) {
#if (VERSION_3 >= 27)
        if (tty_use_256colors(NULL)) {          /* TODO: command line, max colors. */
#else
        if (tty_use_256colors()) {
#endif
            (void) putenv("COLORTERM=24bit");   /* allow true-color skins */
        } else {
            (void) putenv("COLORTERM=16");
        }
    }

//  if (mc_global.tty.ugly_line_drawing) {
//      SLtt_Has_Alt_Charset = 0;
//  }

    key_mouse_mode (mouse_enable);
    SLsmg_init_smg ();

    if ((COLS < 10) || (LINES < 5) || (COLS > VIO_MAXCOLS) || (LINES > VIO_MAXROWS)) {
        fprintf (stderr,
                 _("Screen size %dx%d is not supported; limit 500x500.\n"
                   "Check console properties.\n"), COLS, LINES);
        exit (EXIT_FAILURE);
    }

//  SLtt_Blink_Mode = (0 != tty_use_256colors());
    SLsmg_touch_screen();
}


void
tty_shutdown (void)
{
    SLsmg_reset_smg();
}


void
tty_change_screen_size (void)
{
    SLsmg_reinit_smg();

#ifdef ENABLE_SUBSHELL
    if (mc_global.tty.use_subshell) {
        tty_resize (mc_global.tty.subshell_pty);
    }
#endif
}


void
tty_set_title (const char *title)
{
    if (title) {
        WCHAR t_title[512] = {0};
        if (MultiByteToWideChar(CP_UTF8, 0, title, -1, t_title, _countof(t_title)) > 0) {
            SetConsoleTitleW(t_title);
        } else {
            SetConsoleTitleA(title);
        }
    }
}


void
tty_reset_prog_mode (void)
{
    SLsmg_reinit_smg();
    SLsmg_touch_screen();
    if (0 == original_title[0]) {
        GetConsoleTitleW(original_title, _countof(original_title));
    }
    SetConsoleTitleA("Midnight Commander (" VERSION ")");
    key_prog_mode();
}


void
tty_reset_shell_mode (void)
{
    if (original_title[0]) {
        SetConsoleTitleW(original_title);
    }
    SLsmg_touch_screen();
    key_shell_mode();
}


void
tty_raw_mode (void)
{
}


void
tty_noraw_mode (void)
{
}


void
tty_noecho (void)
{
}


int
tty_flush_input (void)
{
    return 0;
}


void
tty_keypad (gboolean set)
{
}


void
tty_nodelay (gboolean set)
{
}


int
tty_baudrate (void)
{
    return SLang_TT_Baud_Rate;
}


int
tty_reset_screen (void)
{
    return 0;
}


void
tty_touch_screen (void)
{
    SLsmg_touch_lines (0, LINES);
    SLsmg_touch_screen ();
}


void
tty_gotoyx (int y, int x)
{
    SLsmg_gotorc (y, x);
}


void
tty_getyx (int *py, int *px)
{
    *py = SLsmg_get_row();
    *px = SLsmg_get_column();
}


void
tty_draw_hline (int y, int x, int ch, int len)
{
    int x1;

    if (y < 0 || y >= LINES || x >= COLS)
        return;

    x1 = x;

    if (x < 0) {
        len += x;
        if (len <= 0)
            return;
        x = 0;
    }

    if (ch == ACS_HLINE)
        ch = mc_tty_frm[MC_TTY_FRM_HORIZ];
    if (ch == 0)
        ch = ACS_HLINE;

    SLsmg_gotorc (y, x);

    if (ch == ACS_HLINE) {
        SLsmg_draw_hline (len);
    } else {
        while (len-- != 0) {
            tty_print_char (ch);
        }
    }
    SLsmg_gotorc (y, x1);
}


void
tty_draw_vline (int y, int x, int ch, int len)
{
    int y1;

    if (x < 0 || x >= COLS || y >= LINES)
        return;

    y1 = y;

    if (y < 0) {
        len += y;
        if (len <= 0)
            return;
        y = 0;
    }

    if (ch == ACS_VLINE)
        ch = mc_tty_frm[MC_TTY_FRM_VERT];
    if (ch == 0)
        ch = ACS_VLINE;

    SLsmg_gotorc (y, x);

    if (ch == ACS_VLINE) {
        SLsmg_draw_vline (len);
    } else {
        int pos = 0;

        while (len-- != 0) {
            SLsmg_gotorc (y + pos, x);
            tty_print_char (ch);
            pos++;
        }
    }
}


void
tty_fill_region (int y, int x, int rows, int cols, unsigned char ch)
{
    SLsmg_fill_region (y, x, rows, cols, ch);
}


void // 4.8.27+
tty_colorize_area (int y, int x, int rows, int cols, int color)
{
    if (use_colors) {
        SLsmg_set_color_in_region (color, y, x, rows, cols);
    }
}


void
tty_set_alt_charset (gboolean alt_charset)
{
    SLsmg_set_char_set ((int) alt_charset);
}


void
tty_display_8bit (gboolean what)
{
    SLsmg_Display_Eight_Bit = (what ? 128 : 160);
}


void
tty_print_char (int c)
{
    SLsmg_write_char ((int)((unsigned int) c));
}


void
tty_print_alt_char (int c, gboolean single)
{
    int alt = -1;

    switch (c) {
    case ACS_VLINE:
        alt = mc_tty_frm[single ? MC_TTY_FRM_VERT : MC_TTY_FRM_DVERT];
        break;
    case ACS_HLINE:
        alt = mc_tty_frm[single ? MC_TTY_FRM_HORIZ : MC_TTY_FRM_DHORIZ];
        break;
    case ACS_LTEE:
        alt = mc_tty_frm[single ? MC_TTY_FRM_LEFTMIDDLE : MC_TTY_FRM_DLEFTMIDDLE];
        break;
    case ACS_RTEE:
        alt = mc_tty_frm[single ? MC_TTY_FRM_RIGHTMIDDLE : MC_TTY_FRM_DRIGHTMIDDLE];
        break;
    case ACS_ULCORNER:
        alt = mc_tty_frm[single ? MC_TTY_FRM_LEFTTOP : MC_TTY_FRM_DLEFTTOP];
        break;
    case ACS_LLCORNER:
        alt = mc_tty_frm[single ? MC_TTY_FRM_LEFTBOTTOM : MC_TTY_FRM_DLEFTBOTTOM];
        break;
    case ACS_URCORNER:
        alt = mc_tty_frm[single ? MC_TTY_FRM_RIGHTTOP : MC_TTY_FRM_DRIGHTTOP];
        break;
    case ACS_LRCORNER:
        alt = mc_tty_frm[single ? MC_TTY_FRM_RIGHTBOTTOM : MC_TTY_FRM_DRIGHTBOTTOM];
        break;
    case ACS_PLUS:
        alt = mc_tty_frm[MC_TTY_FRM_CROSS];
        break;
    default:
        SLsmg_write_char ((unsigned int) c);
        return;
    }

    if (alt < 0 || c == alt) {
        SLsmg_draw_object (SLsmg_get_row(), SLsmg_get_column(), c);
    } else {
        SLsmg_write_char ((unsigned int) alt);
    }
}


void
tty_print_anychar (int c)
{
    if (c > 255) {
        SLsmg_write_char ((int) ((unsigned int) c));
    } else {
        if (c < 0 || !isprint((int)c)) {
            c = '.';
        }
        SLsmg_write_char ((int) ((unsigned int) c));
    }
}


void
tty_print_string (const char *s)
{
    SLsmg_write_string ((char *) str_term_form (s));
}


void
tty_printf (const char *fmt, ...)
{
    va_list args;

    va_start (args, fmt);
    SLsmg_vprintf (fmt, args);
    va_end (args);
}


char *
tty_tgetstr (const char *cap)
{
    return "";
}


void
tty_refresh (void)
{
    SLsmg_refresh();
}


int
mc_tty_normalize_lines_char (const char *str)
{
    static const struct mc_tty_lines_struct {
        const char *line;
        int         line_code;
    } lines_codes[] = {
        { "\342\224\214", SLSMG_ULCORN_CHAR },
        { "\342\224\220", SLSMG_URCORN_CHAR },
        { "\342\224\224", SLSMG_LLCORN_CHAR },
        { "\342\224\230", SLSMG_LRCORN_CHAR },
        { "\342\224\234", SLSMG_LTEE_CHAR   },
        { "\342\224\244", SLSMG_RTEE_CHAR   },
        { "\342\224\254", SLSMG_UTEE_CHAR   },
        { "\342\224\264", SLSMG_DTEE_CHAR   },
        { "\342\224\200", SLSMG_HLINE_CHAR  },
        { "\342\224\202", SLSMG_VLINE_CHAR  },
        { "\342\224\274", SLSMG_PLUS_CHAR   },
        { NULL, 0 }
        };
    char *str2;
    int res;

    if (NULL == str || !*str)
        return ' ';

    for (res = 0; lines_codes[res].line; res++) {
        if (strcmp (str, lines_codes[res].line) == 0) {
            return lines_codes[res].line_code;
        }
    }

    str2 = mc_tty_normalize_from_utf8 (str);
    res = g_utf8_get_char_validated (str2, -1);
    if (res < 0) {
        res = (unsigned char) str2[0];
    }
    g_free (str2);
    return res;
}


void
tty_beep (void)
{
    SLtt_Ignore_Beep = console_alert_mode;
    SLtt_beep();
}


/*
 *  cons.handler.c support
 */

void
show_console_contents_win32 (
    int starty, unsigned char begin_line, unsigned char end_line )
{
    SLtt_restore();
    SLsmg_touch_screen();
}


void
handle_console_win32 (console_action_t action)
{
    switch (action) {
    case CONSOLE_INIT:
        break;
    case CONSOLE_DONE:
        break;
    case CONSOLE_SAVE:
        SLtt_save();
        break;
    case CONSOLE_RESTORE:
        SLtt_restore();
        SLsmg_touch_screen();
        break;
    default:
        break;
    }
}


/*
 *  tty-internal.c, 4.8.24+
 */

void
tty_create_winch_pipe (void)
{
}


void
tty_destroy_winch_pipe (void)
{
}

/*end*/
