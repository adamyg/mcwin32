/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 slang emulation
 *
 * Copyright (c) 2007, 2012, Adam Young.
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

#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "slang.h"
#include "w32_trace.h"

#define MAXROWS                     500
#define MAXCOLOURS                  256

#define SLSMSG_ALT_CHARS            22
#define SLSMSG_ALT_BASE             0xFDD0

//#define SLSMG_NEWLINE_PRINTABLE   0x01
//#define SLSMG_NEWLINE_SCROLLS     0x02

#define UNICODE_HI_SURROGATE_START  0xD800
#define UNICODE_HI_SURROGATE_END    0xDBFF

#define UNICODE_LO_SURROGATE_START  0xDC00
#define UNICODE_LO_SURROGATE_END    0xDFFF

#define UNICODE_REPLACE             0xfffd      /* replacement character */
#define UNICODE_REPLACECTRL         0x1a        /* substitute */
#define UNICODE_REPLACEFULL         0xff1f      /* full-width '?' */
#define UNICODE_MAX                 0x10ffff

int SLsmg_Tab_Width                 = 8;
int SLsmg_Display_Eight_Bit         = 256;
int SLsmg_Display_Alt_Chars         = 0;
int SLsmg_Newline_Behavior          = 0;
int SLsmg_Backspace_Moves           = 0;

int SLtt_Screen_Rows                = 0;    
int SLtt_Screen_Cols                = 0;
int SLtt_Ignore_Beep                = 0;
int SLtt_Use_Ansi_Colors            = 1;        // full color support       
int SLtt_Try_Termcap                = 0;

static const uint32_t   acs_characters[128] = { /* alternative character map to unicode */

     /*
      * NUL     SOH     STX     ETX     EOT     ENQ     ACK     BEL     
      * BS      HT      NL      VT      NP      CR      SO      SI
      * DLE     DC1     DC2     DC3     DC4     NAK     SYN     ETB     
      * CAN     EM      SUB     ESC     FS      GS      RS      US
      */
        0,      1,      2,      3,      4,      5,      6,      7,      
        8,      9,      10,     11,     12,     13,     14,     15,
        16,     17,     18,     19,     20,     21,     22,     23,     
        24,     25,     26,     27,     28,     29,     30,     31,
    
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
        '(',    ')',    '*',    0x2192, 0x2190, 0x2191, 0x2193, '/',    
        0x2588, '1',    '2',    '3',    '4',    '5',    '6',    '7', 
        '8',    '9',    ':',    ';',    '<',    '=',    '>',    '?', 
        '@',    'A',    'B',    'C',    'D',    'E',    'F',    'G', 
        'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O', 
        'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
        'X',    'Y',    'Z',    '[',    '\\',   ']',    '^',    '_',
        0x2666, 0x2592, 'b',    'c',    'd',    'e',    0x00b0, 0x00b1, 
        0x2591, 0x00a4, 0x2518, 0x2510, 0x250c, 0x2514, 0x253c, 0x23ba, 
        0x23bb, 0x2500, 0x23bc, 0x23bd, 0x251c, 0x2524, 0x2534, 0x252c, 
        0x2502, 0x2264, 0x2265, 0x03c0, 0x2260, 0x00a3, 0x00b7, 127
        };


static uint32_t         alt_lookup(uint32_t ch);
static int              compute_clip(int coord, int n, int start, int end, int *coordmin, int *coordmax);

static void             BUILD_CHAR(unsigned ch, int color, CHAR_INFO *ci);
static BOOL             CMP_CHAR(const CHAR_INFO *c1, const CHAR_INFO *c2);

static void             write_char(SLwchar_Type ch, unsigned cnt);
static void             write_string(const char *str, unsigned cnt);

static void             Copyin(unsigned pos, unsigned cnt);
static void             Copyout(unsigned offset, unsigned len);

static const void *     utf8_decode_raw(const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
static const void *     utf8_decode_safe(const void *src, const void *cpend, int32_t *cooked);

static struct {                         /* Video state */
    int                 inited;
    HANDLE              handle;
    CONSOLE_CURSOR_INFO cinfo;
    COORD               ccoord;
    int                 s_rows, s_cols; /* Video display */
    ULONG               size;
    CHAR_INFO *         image;
    int                 codepage;       /* Font code page */

    SLtt_Char_Type      c_colours[MAXCOLOURS];
    SLtt_Char_Type      c_color;
    int                 c_row, c_col;
#define TOUCHED             0x01
#define TRASHED             0x02
    int                 c_trashed;
    struct sline {
        unsigned            flags;
        CHAR_INFO *         text;
    }                   c_screen[MAXROWS];
    CHAR_INFO *         c_image;
} vio;


struct attrmap {                        /* attribute map local <> win */
    const char *        name;
    WORD                win;
};


static const struct attrmap background[] = {
    /* background colour map */
    { "black",          0 },
    { "blue",           BACKGROUND_BLUE },
    { "green",          BACKGROUND_GREEN  },
    { "cyan",           BACKGROUND_BLUE | BACKGROUND_GREEN },
    { "red",            BACKGROUND_RED },
    { "magenta",        BACKGROUND_BLUE | BACKGROUND_RED },
    { "brown",          BACKGROUND_RED | BACKGROUND_GREEN },
    { "gray",           BACKGROUND_INTENSITY },
    
    { "brightblue",     BACKGROUND_INTENSITY | BACKGROUND_BLUE },
    { "brightgreen",    BACKGROUND_INTENSITY | BACKGROUND_GREEN  },
    { "brightcyan",     BACKGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_GREEN },
    { "brightred",      BACKGROUND_INTENSITY | BACKGROUND_RED },
    { "brightmagenta",  BACKGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_RED},
    { "yellow",         BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN },
    { "lightgray",      BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE },
    
    { "white",          BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE },

    { "default",        0 },
    };


static const struct attrmap foreground[] = {
    /* foreground colour map */
    { "black",          0 },
    { "blue",           FOREGROUND_BLUE },
    { "green",          FOREGROUND_GREEN  },
    { "cyan",           FOREGROUND_BLUE | FOREGROUND_GREEN },
    { "red",            FOREGROUND_RED },
    { "magenta",        FOREGROUND_BLUE | FOREGROUND_RED },
    { "brown",          FOREGROUND_RED | FOREGROUND_GREEN },
    { "gray",           FOREGROUND_INTENSITY },
    
    { "brightblue",     FOREGROUND_INTENSITY | FOREGROUND_BLUE },
    { "brightgreen",    FOREGROUND_INTENSITY | FOREGROUND_GREEN  },
    { "brightcyan",     FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN },
    { "brightred",      FOREGROUND_INTENSITY | FOREGROUND_RED },
    { "brightmagenta",  FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_RED},
    { "yellow",         FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN },
    { "lightgray",      FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
    
    { "white",          FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },

    { "default",        FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE }
    };


static void
vio_init(void)
{
    CONSOLE_SCREEN_BUFFER_INFO sbinfo;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int rows, cols;

    GetConsoleScreenBufferInfo(hConsole, &sbinfo);

    rows = 1 + sbinfo.srWindow.Bottom - sbinfo.srWindow.Top;
    cols = 1 + sbinfo.srWindow.Right - sbinfo.srWindow.Left;

    if (rows >= MAXROWS)
        rows = (MAXROWS-1);

    if (vio.handle != hConsole || vio.s_cols != cols || vio.s_rows != rows) {
        const CHAR_INFO *oimage;
        int l;

        vio.handle = hConsole;
        vio.codepage = GetConsoleOutputCP();
        vio.size = rows * cols;

        oimage = vio.image;
        vio.image = malloc( vio.size * sizeof(CHAR_INFO) );
        
        if (oimage) {                           /* screen has resized */
            CHAR_INFO blank = {' ', FOREGROUND_INTENSITY};
            int r, c, cnt = 
                (cols > vio.s_cols ? vio.s_cols : cols) * sizeof(CHAR_INFO);

            for (r = 0; r < rows; r++) {
                if (r < vio.s_rows) {           /* copy oldimage */
                    memcpy(vio.image + (r*cols), oimage + (r*vio.s_cols), cnt);
                }

                                                /* blank new cells */
                if ((c = (r >= vio.s_rows ? 0 : vio.s_cols)) < cols) {
                    CHAR_INFO *p = vio.image + (r*cols) + c;
                    do {
                        *p++ = blank;
                    } while (++c < cols);
                }
            }
            free((void *)oimage);
        }

        vio.c_trashed = 1;
        vio.s_rows = rows;
        vio.s_cols = cols;
        SLtt_Screen_Rows = rows;
        SLtt_Screen_Cols = cols;

        for (l = 0; l < rows; l++) {
            vio.c_screen[l].flags = 0;
            vio.c_screen[l].text = vio.image + (l*cols);
        }

        if (oimage == NULL) {
            Copyin(0, vio.size);                /* populate image */
        }

        GetConsoleCursorInfo(vio.handle, &vio.cinfo);
        vio.ccoord = sbinfo.dwCursorPosition;
    }

                                                /* default colour */
    SLtt_set_color(0, NULL, "lightgray", "black");    
    SLtt_Screen_Rows = vio.s_rows;
    SLtt_Screen_Cols = vio.s_cols;
}


static void
vio_reset(void)
{
    if (!vio.inited) return;

    vio.s_cols = vio.s_rows = 0;
    if (vio.image) {
        free(vio.image);
        vio.image = NULL;
    }
    vio.inited = 0;
}


static void
vio_setcursor(int col, int row)
{
    COORD coord;

    coord.X = col;
    coord.Y = row;
    SetConsoleCursorPosition( vio.handle, coord );
}


int
SLsmg_init_smg(void)
{
    if (vio.inited) {
        vio_reset();
    }
    vio_init();
    vio.inited = 1;
    return 0;
}


int
SLsmg_reinit_smg(void)
{
    vio_init();
    vio.c_trashed = 1;
    vio.inited = 1;
    return 0;
}


void
SLsmg_reset_smg(void)
{
    vio_reset();
}


void
SLsmg_refresh(void)
{
    int l;

    if (vio.inited == 0) return;

    for (l = 0; l < vio.s_rows; ++l)
        if (vio.c_trashed || vio.c_screen[l].flags) {
            Copyout(vio.s_cols * l, vio.s_cols);
            vio.c_screen[l].flags = 0;
        }
    vio.c_trashed = 0;

    vio_setcursor(0, 0);                        /* home console */
    vio_setcursor(vio.c_col, vio.c_row);        /* then ... set true cursor position */
}


static void
Copyin(unsigned pos, unsigned cnt)
{
    const int rows = vio.s_rows, cols = vio.s_cols;
    COORD is = {0}, ic = {0};
    SMALL_RECT wr = {0};
    
    assert(pos < vio.size);
    assert(0 == (pos % cols));
    assert((pos + cnt) <= vio.size);

    wr.Left   = 0;                              /* src. screen rectangle */
    wr.Right  = (SHORT)(cols - 1);
    wr.Top    = (SHORT)(pos / cols);
    wr.Bottom = (SHORT)((pos + (cnt - 1)) / cols);

    is.Y      = vio.s_rows - wr.Top;            /* size of image */
    is.X      = vio.s_cols;
    
    ic.X      = 0;                              /* top left src cell in image */
    ic.Y      = 0;

    ReadConsoleOutputW(vio.handle,              /* read in image */
        vio.image + pos, is, ic, &wr);
}


static void
Copyout(unsigned pos, unsigned cnt)
{
    const int rows = vio.s_rows, cols = vio.s_cols;
    COORD is = {0}, ic = {0};
    SMALL_RECT wr = {0};

    assert(pos < vio.size);
    assert(0 == (pos % cols));
    assert((pos + cnt) <= vio.size);

    wr.Left   = 0;                              /* src. screen rectangle */
    wr.Right  = (SHORT)(cols - 1);
    wr.Top    = (SHORT)(pos / cols);
    wr.Bottom = (SHORT)((pos + (cnt - 1)) / cols);

    is.Y      = vio.s_rows - wr.Top;            /* size of image */
    is.X      = vio.s_cols;
    
    ic.X      = 0;                              /* top left src cell in image */
    ic.Y      = 0;

    WriteConsoleOutputW(vio.handle,             /* write out image */
        vio.image + pos, is, ic, &wr);
}


void
SLtt_set_color(
    int obj, const char *what, const char *fg, const char *bg)
{
    WORD attr = 0;
    unsigned b, f;

    (void) what;
    
    if (obj < 0 || obj >= MAXCOLOURS) {
        assert(0);
        return;
    }

    for (b = 0; b < (sizeof(background)/sizeof(background[0])); ++b)
        if (0 == stricmp(bg, background[b].name)) {
            attr |= background[b].win;
            break;
        }

    for (f = 0; f < (sizeof(foreground)/sizeof(foreground[0])); ++f)
        if (0 == stricmp(fg, foreground[f].name)) {
            attr |= foreground[f].win;
            break;
        }
    
    vio.c_colours[obj] =                        // black/black --> grey/black ... 
        (attr ? attr : FOREGROUND_INTENSITY);          
}


void
SLtt_add_color_attribute (int obj, SLtt_Char_Type attr)
{
    if (obj < 0 || obj >= MAXCOLOURS) {
        assert(0);
        return;
    }
    vio.c_colours[obj] |= (__SLTT_ATTR_MASK & attr);
}


void
SLtt_set_mono(int obj, char *name, SLtt_Char_Type c)
{
    assert(0);
}


void  
SLsmg_set_char_set(int alt_charset)
{
    SLsmg_Display_Alt_Chars = alt_charset;
}


int              
SLsmg_get_char_set(void)
{
    return SLsmg_Display_Alt_Chars;
}



void
SLtt_beep(void)
{
    if (! SLtt_Ignore_Beep) {
        Beep(2048, 500);
    }
}


int
SLtt_tgetnum(const char *key)
{
    if (key) {
        if (strcmp(key, "Co") == 0) {           // colors
            return 16;
        }
    }
    return -1;
}


char *
SLtt_tigetent (const char *key)
{
    return NULL;
}


char *
SLtt_tigetstr(const char *a, char **b)
{
    return NULL;
}


void
SLsmg_gotorc(int r, int c)
{
    vio.c_row = r;
    vio.c_col = c;
}


int
SLsmg_get_row(void)
{
    return vio.c_row;
}


int
SLsmg_get_column(void)
{
    return vio.c_col;
}


void
SLsmg_set_color(int color)
{
    assert(color >= 0);
    assert(color < MAXCOLOURS);
    vio.c_color = color;
}


void
SLsmg_normal_video(void)
{
    vio.c_color = 0;
}



static uint32_t
alt_lookup(uint32_t ch)
{
    if (ch <= 127) {
        ch = acs_characters[ch];
    }
    return ch;
}


static int
cliptoarena(
    int coord, int n, int start, int end, int *coordmin, int *coordmax)
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
CHAR_BUILD(unsigned ch, int color, CHAR_INFO *ci)
{
    WORD attr;

    if (color < 0 || color >= MAXCOLOURS ||
            (0 == (attr = (0xff & vio.c_colours[color])))) {
        attr = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
    }
    ci->Attributes = attr;
    ci->Char.UnicodeChar = ch;
}


static BOOL
CHAR_COMPARE(const CHAR_INFO *c1, const CHAR_INFO *c2)
{
    return (c1->Attributes == c2->Attributes && c1->Char.UnicodeChar == c2->Char.UnicodeChar);
}


static __inline int
CHAR_UPDATE(CHAR_INFO *cursor, unsigned ch, int color)
{
    CHAR_INFO text;
    
    CHAR_BUILD(ch, color, &text);
    if (! CHAR_COMPARE(cursor, &text)) {
        *cursor = text;
        return TRUE;
    }
    return FALSE;
}


static void
write_char(SLwchar_Type ch, unsigned cnt)
{
    CHAR_INFO *cursor, *cend, text;
    int row = vio.c_row, col = vio.c_col;
    unsigned flags = 0;

    if (0 == vio.inited) return;

    cursor  = vio.c_screen[ row ].text;
    cend    = cursor + vio.s_cols;
    cursor += col;
    CHAR_BUILD(ch, vio.c_color, &text);

    while (cnt-- > 0 && cursor < cend) {
        if (! CHAR_COMPARE(cursor, &text)) {
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
    const int color = vio.c_color;
    CHAR_INFO *cursor, *cend;
    const char *send = str + cnt;
    unsigned flags = 0, nl;
    unsigned char ch;
    int col;

#if defined(SLSMG_NEWLINE_SCROLLS)
top:                                            /* get here only on newline */
#endif
    nl = 0;
    if (vio.c_row >= vio.s_rows) {
        cursor = cend = NULL;
    } else {
        cursor = vio.c_screen[vio.c_row].text;
        cend = cursor + vio.s_cols;
    }
    col     = vio.c_col;
    cursor += col;
    flags   = vio.c_screen[vio.c_row].flags;

    while (cursor < cend && str < send) {

        ch = (unsigned char) *str++;            /* next character */

        if ((ch >= ' ' && ch < 127) ||
                (ch >= (unsigned char) SLsmg_Display_Eight_Bit)) {

            const char *t_str;
            int32_t cooked;

            if ((t_str = utf8_decode_safe(str - 1, send, &cooked)) > str) {
                str = t_str;

                if (cooked < 127 && SLsmg_Display_Alt_Chars)  {
                    ch = alt_lookup(ch);
                }

                if (CHAR_UPDATE(cursor, cooked, color)) {
                    flags |= TOUCHED;
                }
            } else {
                if (SLsmg_Display_Alt_Chars) {
                    ch = alt_lookup(ch);
                }
                if (CHAR_UPDATE(cursor, ch, color)) {
                    flags |= TOUCHED;
                }
            }

            ++cursor;
            ++col;

        } else if ((ch == '\t') && (SLsmg_Tab_Width > 0)) {
            int nexttab = 
                    SLsmg_Tab_Width - ((col + SLsmg_Tab_Width) % SLsmg_Tab_Width);
            CHAR_INFO t_text;

            CHAR_BUILD(' ', color, &t_text);
            while (nexttab-- && cursor < cend) {
                if (! CHAR_COMPARE(cursor, &t_text)) {
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
                if (CHAR_UPDATE(cursor, '~', color)) {
                    flags |= TOUCHED;
                }
                ch &= 0x7F;
                ++cursor;
                ++col;
            }

            if (cursor < cend) {
                if (CHAR_UPDATE(cursor, '^', color)) {
                    flags |= TOUCHED;
                }
                ++cursor;
                ++col;

                if (cursor < cend) {
                    if (CHAR_UPDATE(cursor, (127 == ch ? '?' : ch + '@'), color)) {
                        flags |= TOUCHED;
                    }
                    ++cursor;
                    ++col;
                }
            }
        }
    }

    vio.c_screen[vio.c_row].flags = flags;
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


void
SLsmg_write_char(SLwchar_Type ch)
{
    if (0 == vio.inited) return;
    if (SLsmg_Display_Alt_Chars) {
        ch = alt_lookup(ch);
    }
    write_char(ch, 1);
}


void
SLsmg_write_string(const char *s)
{
    if (0 == vio.inited) return;
    if (s) write_string(s, strlen(s));
}


void
SLsmg_vprintf(const char *fmt, va_list ap)
{
    char buf[1024];
    unsigned len;

    if (0 == vio.inited) return;
    len = _vsnprintf(buf, sizeof (buf), fmt, ap);
    if (len) write_string(buf, len);
}


void
SLsmg_printf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    SLsmg_vprintf(fmt, ap);
    va_end(ap);
}


void
SLsmg_touch_lines(int row, unsigned int n)
{
    if (vio.inited) {
        int i, r1, r2;

        if (1 == cliptoarena (row, (int) n, 0, vio.s_rows, &r1, &r2)) {
            for (i = r1; i < r2; ++i) {
                vio.c_screen[i].flags |= TRASHED;
            }
        }
    }
}


void
SLsmg_touch_screen(void)
{
    ++vio.c_trashed;
}


void
SLsmg_draw_object(int r, int c, SLwchar_Type object)
{
    if (0 == vio.inited) return;
    vio.c_row = r; 
    vio.c_col = c;
    write_char(alt_lookup(object), 1);
}


void
SLsmg_draw_hline(int cnt)
{
    const uint32_t ch = alt_lookup(SLSMG_HLINE_CHAR);
    const int endcol = vio.c_col + cnt;
    int cmin, cmax;

    if (0 == vio.inited || cnt <= 0) return;

    if (vio.c_row < 0 || vio.c_row >= vio.s_rows ||
            (0 == cliptoarena(vio.c_col, cnt, 0, vio.s_cols, &cmin, &cmax))) {
        vio.c_col = endcol;
        return;
    }

    if (cmax > cmin) {
        vio.c_col = cmin; 
        write_char(ch, cmax - cmin);
    }
    vio.c_col = endcol;
}


void
SLsmg_draw_vline(int cnt)
{
    const uint32_t ch = alt_lookup(SLSMG_VLINE_CHAR);
    const int endrow = vio.c_row + cnt, col = vio.c_col;
    int rmin, rmax;

    if (0 == vio.inited || cnt <= 0) return;

    if (col < 0 || col >= vio.s_cols ||
            (0 == cliptoarena(vio.c_row, cnt, 0, vio.s_rows, &rmin, &rmax))) {
        vio.c_row = endrow;
        return;
    }

    for (vio.c_row = rmin; vio.c_row < rmax; ++vio.c_row) {
        vio.c_col = col;
        write_char(ch, 1);
    }
    
    vio.c_row = endrow;
}


void
SLsmg_draw_box(int r, int c, unsigned int dr, unsigned int dc)
{
   if (vio.inited == 0) return;

   if (!dr || !dc) return;
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


void
SLsmg_fill_region (int r, int c, unsigned nr, unsigned nc, unsigned ch)
{
    int rmin, rmax, cmin, cmax;

    if (1 == cliptoarena(r, nr, 0, vio.s_rows, &rmin, &rmax) &&
            1 == cliptoarena(c, nc, 0, vio.s_cols, &cmin, &cmax)) {
    
        const int color = vio.c_color;

        for (r = rmin; r < rmax; ++r) {
            CHAR_INFO *text = vio.c_screen[r].text + cmin;

            for (c = cmin; c < cmax; ++c) {
                CHAR_BUILD(ch, color, text);
                ++text;
            }
            vio.c_screen[r].flags |= TRASHED;
        }
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



