/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 Slang Screen Management (SLsmg) function emulation.
 *
 * Copyright (c) 2007, 2012 - 2020 Adam Young.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT                0x601
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

#include "unicode_cp437.h"

#define UNICODE_HI_SURROGATE_START  0xD800
#define UNICODE_HI_SURROGATE_END    0xDBFF

#define UNICODE_LO_SURROGATE_START  0xDC00
#define UNICODE_LO_SURROGATE_END    0xDFFF

#define UNICODE_REPLACE             0xfffd      /* replacement character */
#define UNICODE_REPLACECTRL         0x1a        /* substitute */
#define UNICODE_REPLACEFULL         0xff1f      /* full-width '?' */
#define UNICODE_MAX                 0x10ffff

LIBW32_API const int SLang_Version      = SLANG_VERSION;

/*
 *  also see libw32.def; variables must be explicitly exported.
 */
LIBW32_API int SLsmg_Tab_Width          = 8;
LIBW32_API int SLsmg_Display_Eight_Bit  = 256;
LIBW32_API int SLsmg_Display_Alt_Chars  = 0;
LIBW32_API int SLsmg_Newline_Behavior   = 0;
LIBW32_API int SLsmg_Backspace_Moves    = 0;

LIBW32_API int SLtt_Screen_Rows         = 0;
LIBW32_API int SLtt_Screen_Cols         = 0;
LIBW32_API int SLtt_Ignore_Beep         = 0;
LIBW32_API int SLtt_Use_Ansi_Colors     = -1;   /* full color support */
LIBW32_API int SLtt_Term_Cannot_Scroll  = 0;
LIBW32_API int SLtt_Term_Cannot_Insert  = 0;
LIBW32_API int SLtt_Try_Termcap         = 0;

static int SLtt_True_Color              = 0;    // TODO

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

static int              cliptoarena(int coord, int n, int start, int end, int *coordmin, int *coordmax);
static void             write_char(SLwchar_Type ch, unsigned cnt);
static void             write_string(const char *str, unsigned cnt);

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
    vio_save();
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


/*
 *  SLsmg_touch_lines ---
 *      Mark the stated lines as trashed; requiring them to redrawn.
 **/
void
SLsmg_touch_lines(int row, unsigned int n)
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
    if (buffer && buflen > 0) {
        _snprintf(buffer, buflen, "%s %dx%d", vio.fcfacename, vio.fcwidth, vio.fcheight);
        buffer[buflen-1]=0;
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
    assert(0); //TODO
}


/*
 *  SLtt_beep ---
 *      xxx
 **/
void
SLtt_beep(void)
{
    if (! SLtt_Ignore_Beep) {
        Beep(2048, 500);
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
            return WIN32_COLORS;                // assumeed support
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
    return NULL;
}


/*
 *  SLtt_tigetstr ---
 *      Retrieve the terminal string configuration element.
 **/
char *
SLtt_tigetstr(const char *a, char **b)
{
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
 *      Retrieve the current cursor row corrdinate.
 **/
int
SLsmg_get_row(void)
{
    return vio_row();
}


/*
 *  SLsmg_get_column ---
 *      Retrieve the current cursor column corrdinate.
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
    assert(obj >= 0);
    assert(obj < MAXCOLORS);
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
//      assert(vio.acsmap);
//      ch = vio.acsmap[ ch ] | ISACS;
        ch = acs_characters[ ch ] | ISACS;
    }
    return ch;
}


#if (XXX)
static uint32_t
unicode_lookup(const struct unicode_table *table, const size_t count, const uint32_t ch)
{
    if (table && count) {
        size_t first = 0, last = count - 1;

        while (first <= last) {
            const size_t middle = (first + last) / 2;
            const struct unicode_table *elm = table + middle;
            const uint32_t unicode = elm->unicode;

            if (ch == unicode) {
                return elm->ch;

            } else if (ch > unicode) {
                first = middle + 1;
            } else {
                last  = middle - 1;
            }
        }
    }
    return ch;
}
#endif  //XXX


#if (XXX)
static uint32_t
unicode_map(uint32_t ch)
{
    if (ch > 0x7f) {
        //
        //  OEM mapping
        //
        if (ISACS & ch) {
            ch &= ~ISACS;                       // already mapped

        } else if (acs_oem == vio.acsmap) {
            //
            //  Terminal font,
            //      limited character support, re-map to cp437.
            //
            //  TODO:
            //      terminal is generally based on the code-page 437, yet an alternative code-page (like 850)
            //      might be in use; determine best method to discover, related to SystemLocale.
            //
            //      The following are related:
            //          HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Console\RasterFont
            //              woafont         app850.fon
            //
            //          HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\GRE_Initialize\SmallFont
            //              OEMFONT.FONT    [vga850.fon | vgaoem.fon etc]
            //
            ch = unicode_lookup(unicode_cp437, (sizeof(unicode_cp437)/sizeof(unicode_cp437[0])), ch);
        }

        //
        //  Remap characters not available on standard console fonts.
        //    o Small box characters.
        //    o Misc
        //
        if (ch > 0xff) {
            switch (ch) {
            case 0x22C5:        // DOT OPERATOR
                ch = 0xB7;
                break;
            case 0x2715:        // MULTIPLICATION X
                ch = 0x2573;    // 'x'
                break;
            case 0x25B4:        // BLACK UP-POINTING SMALL TRIANGLE
                ch = 0x25B2;    // BLACK UP-POINTING TRIANGLE
                break;
            case 0x25B8:        // BLACK RIGHT-POINTING SMALL TRIANGLE
                ch = 0x25BA;    // BLACK RIGHT-POINTING POINTER
                break;
            case 0x25BE:        // BLACK DOWN-POINTING SMALL TRIANGLE
                ch = 0x25BC;    // BLACK DOWN-POINTING TRIANGLE
                break;
            case 0x25C2:        // BLACK LEFT-POINTING SMALL TRIANGLE
                ch = 0x25C4;    // BLACK LEFT-POINTING POINTER
                break;
            default:
                break;
            }
        }
    }
    return ch;
}
#endif  //XXX


static int
cliptoarena(int coord, int n, int start, int end, int *coordmin, int *coordmax)
{
    int coord_max;

    if (n < 0) return 0;
    if (coord >= end) return 0;
    coord_max = coord + n;
    if (coord_max <= start) return 0;
    if (coord < start) coord = start;
    if (coord_max >= end) coord_max = end;
    *coordmin = coord;
    *coordmax = coord_max;
    return 1;
}


static void
write_char(SLwchar_Type ch, unsigned cnt)
{
    WCHAR_INFO *cursor, *cend, text = {0};
    int row = vio.c_row, col = vio.c_col;
    unsigned flags = 0;

    if (0 == vio.inited) return;

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
    }
    vio.c_screen[ row ].flags |= flags;
    vio.c_col = col;
}


static void
write_string(const char *str, unsigned cnt)
{
    const WCHAR_COLORINFO *color = &vio.c_color;
    const char *send = str + cnt;
    WCHAR_INFO *cursor, *cend;
    unsigned flags = 0, nl;
    unsigned char ch;
    int col;

#if defined(SLSMG_NEWLINE_SCROLLS)
top:                                            /* get here only on newline */
#endif
    nl = 0;
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
                str = t_str;

                if (SLsmg_Display_Alt_Chars) cooked = acs_lookup(cooked);
                if (WCHAR_UPDATE(cursor, cooked, color)) {
                    flags |= TOUCHED;
                }
            } else {
                if (SLsmg_Display_Alt_Chars) ch = (unsigned char)acs_lookup(ch);
                if (WCHAR_UPDATE(cursor, ch, color)) {
                    flags |= TOUCHED;
                }
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
            if (ch & 0x80) {
                if (WCHAR_UPDATE(cursor, '~', color)) {
                    flags |= TOUCHED;
                }
                ch &= 0x7F;
                ++cursor;
                ++col;
            }

            if (cursor < cend) {
                if (WCHAR_UPDATE(cursor, '^', color)) {
                    flags |= TOUCHED;
                }
                ++cursor;
                ++col;

                if (cursor < cend) {
                    if (WCHAR_UPDATE(cursor, (127 == ch ? '?' : ch + '@'), color)) {
                        flags |= TOUCHED;
                    }
                    ++cursor;
                    ++col;
                }
            }
        }
    }

    vio.c_screen[vio.c_row].flags |= flags;
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

    ++vio.c_row;
    vio.c_col = 0;
    if (vio.c_row >= vio.c_rows) {
        if (SLsmg_Newline_Behavior == SLSMG_NEWLINE_SCROLLS) {
            scroll_up();
        }
    }
    goto top;
#endif
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

    if (0 == vio.inited) return;
    len = _vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
    assert(len >= 0 && len < sizeof(buf));
    if (len > 0) {
        buf[len] = 0; write_string(buf, len);
    }
}


/*
 *  SLsmg_write_string ---
 *      Write a character string to the virtual terminal.
 **/
void
SLsmg_write_string(const char *s)
{
    if (0 == vio.inited) return;
    if (s) write_string(s, strlen(s));
}


/*
 *  SLsmg_write_nstring ---
 *      Write the character buffer, of length n characters, to the virtual terminal.
 **/
void
SLsmg_write_nstring(const char *s, unsigned n)
{
    if (0 == vio.inited) return;
    if (s && n) write_string(s, n);
}


/*
 *  SLsmg_write_char ---
 *      Write the character to the virtual terminal.
 **/
void
SLsmg_write_char(SLwchar_Type ch)
{
    if (0 == vio.inited) return;
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
SLsmg_draw_object(int r, int c, SLwchar_Type object)
{
    if (0 == vio.inited) return;
    vio.c_row = r;
    vio.c_col = c;
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

    if (0 == vio.inited || cnt <= 0) return;

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

    if (0 == vio.inited || cnt <= 0) return;

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
SLsmg_draw_box(int r, int c, unsigned int dr, unsigned int dc)
{
    if (0 == vio.inited || !dr || !dc) return;

    vio.c_row = r; vio.c_col = c;
    dr--; dc--;
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

    if (0 == vio.inited || !nr || !nc) return;

    if (1 == cliptoarena(r, nr, 0, vio.rows, &rmin, &rmax) &&
            1 == cliptoarena(c, nc, 0, vio.cols, &cmin, &cmax)) {
        WCHAR_INFO text = {0};
        int i;

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
