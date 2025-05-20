/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 Slang Screen Management (SLsmg) function emulation.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT                0x501       // Windows XP
#endif
#ifndef WINVER
#define WINVER                      _WIN32_WINNT
#endif
#include "win32_internal.h"
#define PSAPI_VERSION               1           // EnumProcessModules and psapi.dll
#include <psapi.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>

#include "slang.h"

#define TERMEMU_VIO_SOURCE                      /* private interface */
#define TERMEMU_VIO_STATIC                      /* static binding */
#include "termemu_vio.c"

#pragma comment(lib, "Winmm.lib")               /* PlaySound() */

#include "unicode_cp437.h"

#define UNICODE_HI_SURROGATE_START  0xD800
#define UNICODE_HI_SURROGATE_END    0xDBFF

#define UNICODE_LO_SURROGATE_START  0xDC00
#define UNICODE_LO_SURROGATE_END    0xDFFF

#define UNICODE_REPLACE             0xfffd      /* replacement character */
#define UNICODE_REPLACECTRL         0x1a        /* substitute */
#define UNICODE_REPLACEFULL         0xff1f      /* full-width '?' */
#define UNICODE_MAX                 0x10ffff

/*LIBW32_VAR*/ const int SLang_Version      = SLANG_VERSION;

/*
 *  also see libw32.def; variables must be explicitly exported.
 */
/*LIBW32_VAR*/ int SLsmg_Tab_Width          = 8;
/*LIBW32_VAR*/ int SLsmg_Display_Eight_Bit  = 256;
/*LIBW32_VAR*/ int SLsmg_Display_Alt_Chars  = 0;
/*LIBW32_VAR*/ int SLsmg_Newline_Behavior   = 0;
/*LIBW32_VAR*/ int SLsmg_Backspace_Moves    = 0;

/*LIBW32_VAR*/ int SLtt_Screen_Rows         = 0;
/*LIBW32_VAR*/ int SLtt_Screen_Cols         = 0;
/*LIBW32_VAR*/ int SLtt_Ignore_Beep         = SLTT_BEEP_AUDIBLE; /* default(1), beep() */
/*LIBW32_VAR*/ int SLtt_Use_Ansi_Colors     = -1;   /* full color support */
/*LIBW32_VAR*/ int SLtt_True_Color          = 0;    /* extension */
/*LIBW32_VAR*/ int SLtt_Term_Cannot_Scroll  = 0;
/*LIBW32_VAR*/ int SLtt_Term_Cannot_Insert  = 0;
/*LIBW32_VAR*/ int SLtt_Try_Termcap         = 0;

#if (UNUSED)
static const uint32_t   acs_oem[128] = {        /* alternative character map to OEM/CP437 */
     /*
      * NUL     SOH     STX     ETX     EOT     ENQ     ACK     BEL
      * BS      HT      NL      VT      NP      CR      SO      SI
      * DLE     DC1     DC2     DC3     DC4     NAK     SYN     ETB
      * CAN     EM      SUB     ESC     FS      GS      RS      US
      */
        0x00,   0x01,   0x02,   0x03,   0x04,   0x05,   0x06,   0x07,
        0x08,   0x09,   0x0a,   0x0b,   0x0c,   0x0d,   0x0e,   0x0f,
        0x10,   0x11,   0x12,   0x13,   0x14,   0x15,   0x16,   0x17,
        0x18,   0x19,   0x1a,   0x1b,   0x1c,   0x1d,   0x1e,   0x1f,

     /*
      * SP      !       "       #       $       %       &       '
      * (       )       *       +       ,       -       .       /
      * 0       1       2       3       4       5       6       7
      * 8       9       :       ;       <       =       >       ?
      * @       A       B       C       D       E       F       G
      * H       I       J       K       L       M       N       O
      * P       Q       R       S       T       U       V       W
      * X       Y       Z       [       \       ]       ^       _
      * `       a       b       c       d       e       f       g
      * h       i       j       k       l       m       n       o
      * p       q       r       s       t       u       v       w
      * x       y       z       {       |       }       ~       DEL
      */
        ' ',    '!',    '"',    '#',    '$',    '%',    '&',    '\'',
        '(',    ')',    '*',    0x1a,   0x1b,   0x18,   0x19,   '/',
        0xdb,   '1',    '2',    '3',    '4',    '5',    '6',    '7',
        '8',    '9',    ':',    ';',    '<',    '=',    '>',    '?',
        '@',    '+',    '+',    '+',    '+',    0xce,   '+',    '+',
        '+',    '+',    0xd9,   0xbf,   0xda,   0xc0,   0xc5,   'O',
        'P',    0xc4,   0xcd,   'S',    0xc3,   0xb4,   0xc2,   0xc1,
        0xb3,   0xba,   'Z',    '[',    '\\',   ']',    '^',    '_',
        0x04,   0xb1,   'b',    'c',    'd',    'e',    0xf8,   0xf1,
        0xb0,   '#',    0xd9,   0xbf,   0xda,   0xc0,   0xc5,   '~',
        '-',    0xc4,   '-',    '_',    0xc3,   0xb4,   0xc1,   0xc2,
        0xb3,   0xf3,   0xf2,   0xe3,   '!',    0x9c,   0xf9,   0x7f,
        };
#endif

static void             set_position(int row, int col);
static int              cliptoarena(int coord, int n, int start, int end, int *coordmin, int *coordmax);
static void             write_char(SLwchar_Type ch, unsigned cnt);
static void             write_string(const char *str, unsigned cnt);
static void             invert_region(int top_row, int bot_row);

static const void *     utf8_decode_raw(const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
static const void *     utf8_decode_safe(const void *src, const void *cpend, int32_t *cooked);


/*
 *  SLsmg_init_smg ---
 *      Initialise the SLsmg routines.
 **/
int
SLsmg_init_smg(void)
{
    if (-1 == SLtt_Use_Ansi_Colors) {
        const char *ct = getenv("COLORTERM");
        if (ct) {
            SLtt_Use_Ansi_Colors = 1;           // implied
            if (0 == strcmp(ct, "truecolor") || 0 == strcmp(ct, "24bit")) {
                SLtt_True_Color = 1;            // true color support requested.
            }
        }
        vio_config_truecolor(SLtt_True_Color);  // true-color decoder support
        TRACE_LOG(("init_smg(ansi:%d, true:%d)", SLtt_Use_Ansi_Colors, SLtt_True_Color))
    }
    vio_open(&SLtt_Screen_Rows, &SLtt_Screen_Cols);
    SLtt_set_color(0, NULL, "lightgray", "black");
    return 0;
}


/*
 *  SLsmg_reinit_smg ---
 *      Re-initialise the SLsmg routines.
 **/
int
SLsmg_reinit_smg(void)
{
    SLtt_Use_Ansi_Colors = -1;                  // reimport environment settings.
    SLtt_True_Color = 0;
    SLsmg_init_smg();
    return 0;
}


/*
 *  SLsmg_reset_smg ---
 *      Reset the SLsmg routines; closes the virtual terminal, releasing resources
 *      and restores the terminal to it default state.
 **/
void
SLsmg_reset_smg(void)
{
    vio_close();
}


/*
 *  SLsmg_refresh ---
 *      Update the physical display; flushing any changes to the virtual display to the terminal.
 **/
void
SLsmg_refresh(void)
{
    vio_flush();
}


void
SLsmg_togglesize(void)
{
    vio_toggle_size(&SLtt_Screen_Rows, &SLtt_Screen_Cols);
}


/*
 *  SLsmg_set_char_set ---
 *      xxx
 **/
void
SLsmg_set_char_set(int alt_charset)
{
    SLsmg_Display_Alt_Chars = alt_charset;
}


/*
 *  SLsmg_get_char_set ---
 *      xxx
 **/
int
SLsmg_get_char_set(void)
{
    return SLsmg_Display_Alt_Chars;
}


/*
 *  SLtt_initialize ---
 *      Returns 0 if all goes well, -1 if terminal capabilities cannot be deduced,
 *      or -2 if terminal cannot position the cursor.
 **/
int
SLtt_initialize(const char *term)
{
    (void) term;
    SLsmg_init_smg();
    return 0;
}


/*
 *  SLtt_save ---
 *      Save the current screen image.
 **/
void
SLtt_save(void)
{
    vio_save_lines(1);
}


/*
 *  SLtt_restore ---
 *      Restore the previously saved screen image.
 **/
void
SLtt_restore(void)
{
    vio_restore();
}


void
SLtt_restore_lines(int top, int bottom, int to)
{
    vio_restore_lines(top, bottom, to);
}


/*
 *  SLsmg_touch_lines ---
 *      Mark the stated lines as trashed; requiring them to redrawn.
 **/
void
SLsmg_touch_lines(int row, unsigned n)
{
    if (vio.inited) {
        int i, r1, r2;

        if (1 == cliptoarena(row, (int)n, 0, vio.rows, &r1, &r2)) {
            for (i = r1; i < r2; ++i) {
                vio.c_screen[i].flags |= TRASHED;
            }
        }
    }
}


/*
 *  SLsmg_touch_screen ---
 *      Mark the screen as trashed; requiring them to redrawn.
 **/
void
SLsmg_touch_screen(void)
{
    ++vio.c_trashed;
}


/*
 *  SLtt_set_color ---
 *      Set the stated color object to the given foreground and background colors.
 **/
void
SLtt_set_color(int obj, const char *what, const char *fg, const char *bg)
{
    (void) vio_define_attr(obj, what, fg, bg);
}


/*
 *  SLtt_get_font ---
 *      Retrieve the current terminal font.
 **/
const char *
SLtt_get_font(char *buffer, size_t buflen)
{
    if (buffer && buflen) {                     /* user buffer? */
        int len;

        if ((len = _snprintf(buffer, buflen, "%s %dx%d",
                        vio.fcfacename, vio.fcwidth, vio.fcheight)) < 0 ||
                (size_t)len >= buflen) {
            buffer[buflen-1] = 0;               /* error/overflow */
        }
        return buffer;
    }
    return vio.fcfacename;
}


/*
 *  SLtt_add_color_attribute ---
 *      xxx
 **/
void
SLtt_add_color_attribute(int obj, SLtt_Char_Type attr)
{
    WORD nattr = 0;

    if (attr & SLTT_BOLD_MASK)   nattr |= VIO_BOLD;
    if (attr & SLTT_BLINK_MASK)  nattr |= VIO_BLINK;
    if (attr & SLTT_ULINE_MASK)  nattr |= VIO_UNDERLINE;
    if (attr & SLTT_REV_MASK)    nattr |= VIO_INVERSE;
    if (attr & SLTT_ITALIC_MASK) nattr |= VIO_ITALIC;
    if (attr & SLTT_ALTC_MASK)   nattr |= VIO_ALTCHARSET;
    vio_define_flags(obj, nattr);
    ++vio.c_trashed;
}


/*
 *  SLtt_set_mono ---
 *      xxx
 **/
void
SLtt_set_mono(int obj, char *name, SLtt_Char_Type c)
{
    (void) obj;
    (void) name;
    (void) c;
    assert(0);
}


/*
 *  SLtt_beep ---
 *      Audible bell.
 **/
static void
beep(int sync)
{
    if (0 == (SLTT_BEEP_LEGACY & SLtt_Ignore_Beep)) {
        PlaySoundA("SystemAsterisk", NULL, sync ? SND_SYNC : SND_ASYNC); // Windows 2000+
    } else {
        Beep(750, 120); // legacy
    }
}


void
SLtt_beep(void)
{
    int audible;

    if (0 == SLtt_Ignore_Beep)
        return;

    audible = (SLTT_BEEP_AUDIBLE & SLtt_Ignore_Beep);

    if (SLTT_BEEP_FLASH & SLtt_Ignore_Beep) { // flash title, optional sound
        FLASHWINFO fi = {0};
        fi.cbSize = sizeof(fi);
        fi.hwnd = vio.whandle;
        fi.dwFlags = FLASHW_ALL;
        fi.uCount = 1;
        fi.dwTimeout = 0;
        FlashWindowEx(&fi);
        if (audible) beep(FALSE);

    } else if (SLTT_BEEP_INVERT & SLtt_Ignore_Beep) { // invert last line, optional sound
        invert_region(vio.rows-1, vio.rows);
        vio_flush();
        if (audible) {
            beep(TRUE);
        } else {
            Sleep(200);
        }
        invert_region(vio.rows-1, vio.rows);
        vio_flush();

    } else { // default, sound
        beep(FALSE);
    }
}


/*
 *  SLtt_tgetnum ---
 *      Retrieve the terminal numeric configuration element.
 **/
int
SLtt_tgetnum(const char *key)
{
    if (key) {
        if (0 == strcmp(key, "Co")) {           // colors
            if (vio.inited)
                return vio.maxcolors;           // 16 or 256
            return WIN32_COLORS;                // assumed support
        }
    }
    return -1;
}


/*
 *  SLtt_tigetent ---
 *      Retrieve the terminal string configuration element.
 **/
char *
SLtt_tigetent(const char *key)
{
    (void) key;
    return NULL;
}


/*
 *  SLtt_tigetstr ---
 *      Retrieve the terminal string configuration element.
 **/
char *
SLtt_tigetstr(const char *a, char **b)
{
    (void) a;
    (void) b;
    return NULL;
}


/*
 *  SLsmg_gotorc ---
 *      Goto the specified row/column.
 **/
void
SLsmg_gotorc(int r, int c)
{
    vio_goto(r, c);
}


/*
 *  SLsmg_get_row ---
 *      Retrieve the current cursor row coordinate.
 **/
int
SLsmg_get_row(void)
{
    return vio_row();
}


/*
 *  SLsmg_get_column ---
 *      Retrieve the current cursor column coordinate.
 **/
int
SLsmg_get_column(void)
{
    return vio_column();
}


/*
 *  SLsmg_set_color ---
 *      Set the current color object.
 **/
void
SLsmg_set_color(int obj)
{
    assert(obj >= 0 && obj < MAXCOLORS);
    vio_set_colorattr(obj);
}


/*
 *  SLsmg_normal_video ---
 *      Set the current color to object 0.
 **/
void
SLsmg_normal_video(void)
{
    vio_set_colorattr(0);
}


static uint32_t
acs_lookup(uint32_t ch)
{
    if (ch <= 127) {                            // OEM or UNICODE selection required
//      ch = (vio.acsmap ? vio.acsmap[ ch ] : acs_characters[ ch ]) | ISACS;
        ch = acs_characters[ ch ] | ISACS;
    }
    return ch;
}


static void
set_position(int row, int col)
{
    vio.c_row = (row < 0 ? 0 : (row >= vio.rows ? vio.rows-1 : row));
    vio.c_col = (col < 0 ? 0 : (col >= vio.cols ? vio.cols-1 : col));
}


static int
cliptoarena(int coord, int n, int start, int end, int *coordmin, int *coordmax)
{
    int coord_max;

    if (n < 0) return 0;                        /* out-of-bounds */
    if (coord >= end) return 0;                 /* out-of-bounds */
    coord_max = coord + n;
    if (coord_max <= start) return 0;           /* out-of-bounds */
    if (coord < start) coord = start;
    if (coord_max >= end) coord_max = end;
    *coordmin = coord;
    *coordmax = coord_max;
    return 1;   /*success*/
}


static void
write_char(SLwchar_Type ch, unsigned cnt)
{
    const int width = vio_wcwidth(ch);
    WCHAR_INFO *cursor, *cend, text = { 0 };
    int row = vio.c_row, col = vio.c_col;
    unsigned flags = 0;

    assert(vio.inited);                         /* not initialised */
    assert(row >= 0 && row <  vio.rows);
    assert(col >= 0 && col <= vio.cols);        /* note: allow position off-screen +1 */

    if (0 == width)                             /* zero width character; ignore/todo */
        return;

    cursor  = vio.c_screen[ row ].text;
    cend    = cursor + vio.cols;
    cursor += col;

    WCHAR_BUILD(ch, &vio.c_color, &text);
    while (cnt-- > 0 && cursor < cend) {
        if (! WCHAR_COMPARE(cursor, &text)) {
            *cursor = text;
            flags |= TOUCHED;
        }

        ++cursor;
        ++col;

        if (width > 1) {                        /* NULL padding */
            WCHAR_INFO null = {0};
            null.Info = vio.c_color;
            *cursor++ = null;
            ++col;
        }
    }

    vio.c_screen[ row ].flags |= flags;
    assert(col >= 0 && col <= vio.cols);        /* note: allow position off-screen +1 */
    vio.c_col = col;
}


static void
write_string(const char *str, unsigned cnt)
{
    const WCHAR_COLORINFO *color = &vio.c_color;
    const char *send = str + cnt;
    WCHAR_INFO *cursor, *cend;
    unsigned flags = 0;
#if defined(SLSMG_NEWLINE_PRINTABLE)
    unsigned nl;
#endif
    unsigned char ch;
    int col;

#if defined(SLSMG_NEWLINE_SCROLLS)
top:                                            /* get here only on newline */
    nl = 0;
#endif
    if (vio.c_row >= vio.rows) {
        cursor = cend = NULL;                   /* off screen */
    } else {
        cursor = vio.c_screen[vio.c_row].text;
        cend = cursor + vio.cols;
    }
    col     = vio.c_col;
    cursor += col;
    flags   = 0;

    while (cursor < cend && str < send) {

        ch = (unsigned char) *str++;            /* next character */

        if ((ch >= ' ' && ch < 127) ||
                (ch >= (unsigned char) SLsmg_Display_Eight_Bit)) {

            const char *t_str;
            int32_t cooked;

            if ((t_str = utf8_decode_safe(str - 1, send, &cooked)) > str) {
                const int width = vio_wcwidth(cooked);

                str = t_str;

                if (0 == width)                 /* zero width character; ignore/todo */
                    continue;

                if (SLsmg_Display_Alt_Chars) cooked = acs_lookup(cooked);
                flags |= WCHAR_UPDATE(cursor, cooked, color);
                if (width > 1) {                /* wide-character, NUL padding */
                    flags |= WCHAR_UPDATE(++cursor, 0, color);
                    ++col;
                }

            } else {
                if (SLsmg_Display_Alt_Chars) ch = (unsigned char)acs_lookup(ch);
                flags |= WCHAR_UPDATE(cursor, ch, color);
            }

            ++cursor;
            ++col;

        } else if ((ch == '\t') && (SLsmg_Tab_Width > 0)) {
            int nexttab = SLsmg_Tab_Width - ((col + SLsmg_Tab_Width) % SLsmg_Tab_Width);
            WCHAR_INFO t_text = {0};

            WCHAR_BUILD(' ', color, &t_text);
            while (nexttab-- && cursor < cend) {
                if (! WCHAR_COMPARE(cursor, &t_text)) {
                    *cursor = t_text;
                    flags |= TOUCHED;
                }
                ++cursor;
                ++col;
            }

#if defined(SLSMG_NEWLINE_PRINTABLE)
        } else if ((ch == '\n') && (SLsmg_Newline_Behavior != SLSMG_NEWLINE_PRINTABLE)) {
            nl = 1;
            break;
#endif

        } else if ((ch == 0x8) && SLsmg_Backspace_Moves) {
            if (col > 0) {
                --cursor;
                --col;
            }

        } else {
            /*
             *  ~^[char]
             */
            if ((ch & 0x80) && cursor < cend) {
                flags |= WCHAR_UPDATE(cursor, '~', color);
                ch &= 0x7F;
                ++cursor;
                ++col;
            }

            if (cursor < cend) {
                flags |= WCHAR_UPDATE(cursor, '^', color);
                ++cursor;
                ++col;

                if (cursor < cend) {
                    flags |= WCHAR_UPDATE(cursor, (127 == ch ? '?' : ch + '@'), color);
                    ++cursor;
                    ++col;
                }
            }
        }
    }

    if (flags) vio.c_screen[vio.c_row].flags |= flags;
    assert(col >= 0 && col <= vio.cols);        /* note: allow final position off-screen */
    vio.c_col = col;

#if defined(SLSMG_NEWLINE_SCROLLS)
    if (0 == SLsmg_Newline_Behavior) {
        return;
    }

    if (! nl) {
        while (str < send) {
            if (*str++ == '\n') {
                break;
            }
        }
        if (str >= send) {
            return;
        }
    }

    vio.c_col = 0;
    if (++vio.c_row >= vio.c_rows) {
        if (SLsmg_Newline_Behavior == SLSMG_NEWLINE_SCROLLS) {
            scroll_up();
        }
    }
    goto top;
#endif
}


static void
invert_region(int top_row, int bot_row)
{
    if (bot_row > vio.rows) bot_row = vio.rows;
    while (top_row < bot_row) {
        WCHAR_INFO *cursor = vio.c_screen[top_row].text,
            *cend = cursor + vio.cols;

        vio.c_screen[top_row].flags |= TOUCHED|TRASHED;
        while (cursor < cend) {
            cursor->Info.Attributes ^= VIO_INVERSE;
            ++cursor;
        }
        ++top_row;
    }
}


/*
 *  SLsmg_printf ---
 *      Formatted output.
 **/
void
SLsmg_printf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    SLsmg_vprintf(fmt, ap);
    va_end(ap);
}


/*
 *  SLsmg_vprintf ---
 *      Formatted output.
 **/
void
SLsmg_vprintf(const char *fmt, va_list ap)
{
    char buf[4*1024];
    int len;

    if (0 == vio.inited) return;                /* not initialised */

    if ((len = _vsnprintf(buf, sizeof(buf), fmt, ap)) < 0 || len >= (int)sizeof(buf)) {
        len = sizeof(buf);                      /* error/overflow */
    }
    if (len > 0) {
        write_string(buf, (unsigned)len);       /* export */
    }
}


/*
 *  SLsmg_write_string ---
 *      Write a character string to the virtual terminal.
 **/
void
SLsmg_write_string(const char *s)
{
    if (0 == vio.inited) return;                /* not initialised */

    if (s) write_string(s, (unsigned)strlen(s));
}


/*
 *  SLsmg_write_nstring ---
 *      Write the character buffer, of length n characters, to the virtual terminal.
 **/
void
SLsmg_write_nstring(const char *s, unsigned n)
{
    if (0 == vio.inited) return;                /* not initialised */

    if (s && n) write_string(s, n);
}


/*
 *  SLsmg_write_char ---
 *      Write the character to the virtual terminal.
 **/
void
SLsmg_write_char(SLwchar_Type ch)
{
    if (0 == vio.inited) return;                /* not initialised */

    if (SLsmg_Display_Alt_Chars) {
        ch = acs_lookup(ch);
    }
    write_char(ch, 1);
}


/*
 *  SLsmg_draw_object ---
 *      Draw an object from the alternate character set at row 'r' and column 'c'.
 *      The object is really a character from the alternate character set and may be specified using one of the following constants:
 *
 *          SLSMG_HLINE_CHAR        Horizontal line.
 *          SLSMG_VLINE_CHAR        Vertical line.
 *          SLSMG_ULCORN_CHAR       Upper left corner.
 *          SLSMG_URCORN_CHAR       Upper right corner.
 *          SLSMG_LLCORN_CHAR       Lower left corner.
 *          SLSMG_LRCORN_CHAR       Lower right corner.
 *          SLSMG_CKBRD_CHAR        Checkboard character.
 *          SLSMG_RTEE_CHAR         Right Tee.
 *          SLSMG_LTEE_CHAR         Left Tee.
 *          SLSMG_UTEE_CHAR         Up Tee.
 *          SLSMG_DTEE_CHAR         Down Tee.
 *          SLSMG_PLUS_CHAR         Plus or Cross character.
 **/
void
SLsmg_draw_object(int row, int col, SLwchar_Type object)
{
    if (0 == vio.inited) return;                /* not initialised */

    set_position(row, col);
    write_char(acs_lookup(object), 1);
}


/*
 *  SLsmg_draw_bline ---
 *      Draws a horizontal line of length len on the virtual display.
 *      The position of the virtual cursor is left at the end of the line.
 **/
void
SLsmg_draw_hline(int cnt)
{
    const uint32_t ch = acs_lookup(SLSMG_HLINE_CHAR);
    const int endcol = vio.c_col + cnt;
    int cmin, cmax;

    if (0 == vio.inited) return;                /* not initialised */
    if (cnt <= 0) return;                       /* invalid count */

    if (vio.c_row < 0 || vio.c_row >= vio.rows ||
            (0 == cliptoarena(vio.c_col, cnt, 0, vio.cols, &cmin, &cmax))) {
        vio.c_col = endcol;
        return;
    }

    if (cmax > cmin) {
        vio.c_col = cmin;
        write_char(ch, cmax - cmin);
    }

    vio.c_col = endcol;
}


/*
 *  SLsmg_draw_vline ---
 *      Draws a vertical line of length len on the virtual display.
 *      The position of the virtual cursor is left at the end of the line.
 **/
void
SLsmg_draw_vline(int cnt)
{
    const uint32_t ch = acs_lookup(SLSMG_VLINE_CHAR);
    const int endrow = vio.c_row + cnt, col = vio.c_col;
    int rmin, rmax;

    if (0 == vio.inited) return;                /* not initialised */
    if (cnt <= 0) return;                       /* invalid count */

    if (col < 0 || col >= vio.cols ||
            (0 == cliptoarena(vio.c_row, cnt, 0, vio.rows, &rmin, &rmax))) {
        vio.c_row = endrow;
        return;
    }

    for (vio.c_row = rmin; vio.c_row < rmax; ++vio.c_row) {
        vio.c_col = col;
        write_char(ch, 1);
    }

    vio.c_row = endrow;
}


/*
 *  SLsmg_draw_box ---
 *      Draw a box using the SLsmg_draw_box uses the SLsmg_draw_hline and SLsmg_draw_vline functions.
 *      The box's upper left corner is placed at row 'r' and column 'c'.
 *      The width and length of the box is specified by 'dc' and 'dr', respectively.
 **/
void
SLsmg_draw_box(int r, int c, unsigned dr, unsigned dc)
{
    if (0 == vio.inited) return;                /* not initialised */

    if (r > vio.rows || c > vio.cols) return;   /* out-of-bounds */
    if (0 == dr || 0 == dc) return;             /* zero size */

    vio.c_row = r; vio.c_col = c;

    --dr; --dc;
    SLsmg_draw_hline(dc);
    SLsmg_draw_vline(dr);
    vio.c_row = r; vio.c_col = c;
    SLsmg_draw_vline(dr);
    SLsmg_draw_hline(dc);

    SLsmg_draw_object(r, c, SLSMG_ULCORN_CHAR);
    SLsmg_draw_object(r, c + (int) dc, SLSMG_URCORN_CHAR);
    SLsmg_draw_object(r + (int) dr, c, SLSMG_LLCORN_CHAR);
    SLsmg_draw_object(r + (int) dr, c + (int) dc, SLSMG_LRCORN_CHAR);

    vio.c_row = r; vio.c_col = c;
}


/*
 *  SLsmg_fill_region ---
 *      Fill a rectangular region with a character.
 *      The rectangle's upper left corner is at row 'r' and column 'c', and spans 'nr' rows and 'nc' columns.
 *      The position of the virtual cursor will be left at (r, c).
 **/
void
SLsmg_fill_region(int r, int c, unsigned nr, unsigned nc, SLwchar_Type ch)
{
    int rmin, rmax, cmin, cmax;

    if (0 == vio.inited) return;                /* not initialised */
    if (0 == nr || 0 == nc) return;             /* zero size */

    if (1 == cliptoarena(r, nr, 0, vio.rows, &rmin, &rmax) &&
            1 == cliptoarena(c, nc, 0, vio.cols, &cmin, &cmax)) {
        WCHAR_INFO text = {0};
        int i;

        assert(r >= 0 && r < vio.rows);
        assert(c >= 0 && c < vio.cols);
        assert(rmin >= 0 && rmax <= vio.rows);
        assert(cmin >= 0 && cmax <= vio.cols);

        WCHAR_BUILD(ch, &vio.c_color, &text);
        for (i = rmin; i < rmax; ++i) {
            WCHAR_INFO *cursor = vio.c_screen[i].text + cmin,
                *cend = vio.c_screen[i].text + cmax;
            unsigned flags = 0;

            while (cursor < cend) {
                if (! WCHAR_COMPARE(cursor, &text)) {
                    *cursor = text;
                    flags |= TOUCHED;
                }
                ++cursor;
            }
            vio.c_screen[i].flags |= flags;
        }

        vio.c_row = r; vio.c_col = c;
    }
}


/*
 *  SLsmg_fill_region ---
 *      Change the color of a specifed region
 *      The rectangle's upper left corner is at row 'r' and column 'c', and whose width and height is given by dc and dr, respectively.
 *      The position of the virtual cursor will be left at (r, c).
 **/
void
SLsmg_set_color_in_region (int color, int r, int c, unsigned dr, unsigned dc)
{
    int rmin, rmax, cmin, cmax;

    if (0 == vio.inited) return;                /* not initialised */
    if (0 == dr || 0 == dc) return;             /* zero size */

    if (1 == cliptoarena(r, dr, 0, vio.rows, &rmin, &rmax) &&
            1 == cliptoarena(c, dc, 0, vio.cols, &cmin, &cmax)) {
        int i;

        assert(r >= 0 && r < vio.rows);
        assert(c >= 0 && c < vio.cols);
        assert(rmin >= 0 && rmax <= vio.rows);
        assert(cmin >= 0 && cmax <= vio.cols);

        for (i = rmin; i < rmax; ++i) {
            WCHAR_INFO *cursor = vio.c_screen[i].text + cmin,
                *cend = vio.c_screen[i].text + cmax;
            unsigned flags = 0;

            while (cursor < cend) {
                if (! WATTR_COMPARE(cursor, &vio.c_color)) {
                    cursor->Info = vio.c_color;
                    flags |= TOUCHED;
                }
                ++cursor;
            }
            vio.c_screen[i].flags |= flags;
        }

        vio.c_row = r; vio.c_col = c;
    }
}


void
SLsmg_forward (int n)
{
    set_position (vio.c_row + n, vio.c_col);
}


static __inline int
utf8_illegal(const int32_t ch)
{
    const int32_t low16 = (0xffff & ch);

    if (0xfffe == low16 || 0xffff == low16 ||
            ch <= 0 || ch > UNICODE_MAX ||
            (ch >= UNICODE_HI_SURROGATE_START && ch <= UNICODE_LO_SURROGATE_END)) {
        return TRUE;
    }
    return FALSE;
}


static __inline int
utf8_overlong(const int32_t ch, const size_t length)
{
    if (ch <= 0x80) {
        if (1 != length) return TRUE;
    } else if (ch < 0x800) {
        if (2 != length) return TRUE;
    } else if (ch < 0x10000) {
        if (3 != length) return TRUE;
    } else if (ch < 0x200000) {
        if (4 != length) return TRUE;
    } else if (ch < 0x4000000) {
        if (5 != length) return TRUE;
    } else {
        if (6 != length) return TRUE;
    }
    return FALSE;
}


/*
 *  00000000-01111111  00-7F  0-127     Single-byte encoding (compatible with US-ASCII).
 *  10000000-10111111  80-BF  128-191   Second, third, or fourth byte of a multi-byte sequence.
 *  11000000-11000001  C0-C1  192-193   Overlong encoding: start of 2-byte sequence, but would encode a code point 127.
 *  11000010-11011111  C2-DF  194-223   Start of 2-byte sequence.
 *  11100000-11101111  E0-EF  224-239   Start of 3-byte sequence.
 *  11110000-11110100  F0-F4  240-244   Start of 4-byte sequence.
 *  11110101-11110111  F5-F7  245-247   Restricted by RFC 3629: start of 4-byte sequence for codepoint above 10FFFF.
 *  11111000-11111011  F8-FB  248-251   Restricted by RFC 3629: start of 5-byte sequence.
 *  11111100-11111101  FC-FD  252-253   Restricted by RFC 3629: start of 6-byte sequence.
 *  11111110-11111111  FE-FF  254-255   Invalid: not defined by original UTF-8 specification.
 */
static __inline const char *
utf8_decode(const void *src, const void *cpend, int32_t *result)
{
    register const unsigned char *t_src = (const unsigned char *)src;
    unsigned char ch;
    int32_t ret = 0;
    int remain;

    /*
    //  Bits    Last code point     Byte 1      Byte 2      Byte 3      Byte 4      Byte 5      Byte 6
    //  7       U+007F              0xxxxxxx
    //  11      U+07FF              110xxxxx    10xxxxxx
    //  16      U+FFFF              1110xxxx    10xxxxxx    10xxxxxx
    //  21      U+1FFFFF            11110xxx    10xxxxxx    10xxxxxx    10xxxxxx
    //  26      U+3FFFFFF           111110xx    10xxxxxx    10xxxxxx    10xxxxxx    10xxxxxx
    //  31      U+7FFFFFFF          1111110x    10xxxxxx    10xxxxxx    10xxxxxx    10xxxxxx    10xxxxxx
    */
    assert(src < cpend);
    ch = *t_src++;

    if (ch & 0x80) {
                                                /* C0-C1  192-193  Overlong encoding: start of 2-byte sequence. */
        if ((ch & 0xE0) == 0xC0) {              /* C2-DF  194-223  Start of 2-byte sequence. */
            remain = 1;
            ret = ch & 0x1F;

        } else if ((ch & 0xF0) == 0xE0) {       /* E0-EF  224-239  Start of 3-byte sequence. */
            remain = 2;
            ret = ch & 0x0F;

        } else if ((ch & 0xF8) == 0xF0) {       /* F0-F4  240-244  Start of 4-byte sequence. */
            remain = 3;
            ret = ch & 0x07;

        } else if ((ch & 0xFC) == 0xF8) {       /* F8-FB  248-251  Start of 5-byte sequence. */
            remain = 4;
            ret = ch & 0x03;

        } else if ((ch & 0xFE) == 0xFC) {       /* FC-FD  252-253  Start of 6-byte sequence. */
            remain = 5;
            ret = ch & 0x01;

        } else {                                /* invalid continuation (0x80 - 0xbf). */
            ret = -ch;
            goto done;
        }

        while (remain--) {
            if (t_src >= (const unsigned char *)cpend) {
                ret = -ret;
                goto done;
            }
            ch = *t_src++;
            if (0x80 != (0xc0 & ch)) {          /* invalid secondary byte (0x80 - 0xbf). */
                --t_src;
                ret = -ret;
                goto done;
            }
            ret <<= 6;
            ret |= (ch & 0x3f);
        }
    } else {
        ret = ch;
    }

done:;
    *result = ret;
    return (const void *)t_src;
}


static const void *
utf8_decode_raw(const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    int32_t result = 0;
    const char *ret = utf8_decode(src, cpend, &result);

    if (result <= 0 || utf8_illegal(result) ||
            (ret && utf8_overlong(result, ret - (const char *)src))) {
        *raw = (result < 0 ? -result : result);
        *cooked = UNICODE_REPLACE;              /* replacement character */
        return ret;
    }
    *cooked = *raw = result;
    return ret;
}


static const void *
utf8_decode_safe(const void *src, const void *cpend, int32_t *cooked)
{
    int32_t result = 0;
    const char *ret = utf8_decode(src, cpend, &result);

    if (result <= 0 || utf8_illegal(result) ||
            (ret && utf8_overlong(result, ret - (const char *)src))) {
        result = UNICODE_REPLACE;               /* replacement character */
    }
    *cooked = result;
    return ret;
}

/*end*/
