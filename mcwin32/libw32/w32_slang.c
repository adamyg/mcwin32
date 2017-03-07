/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 slang emulation.
 *
 * Notes:
 *   o Extended 256 color mode is experimental/work in progress.
 *   o Use of non-monospaced fonts are not advised unless UNICODE
 *        characters are required.
 *   o Neither wide nor combined characters are fully addressed.
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT                0x601
#endif
#include "win32_internal.h"

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>

#include "slang.h"
#include "w32_trace.h"

#include "unicode_cp437.h"

#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")

#define WIN32_CONSOLEEXT                        /* extended console */
#define WIN32_CONSOLE256                        /* enable 256 color console support */

#if defined(WIN32_CONSOLEEXT) && defined(WIN32_CONSOLE256)
#define WIN32_COLORS                256
#else
#undef  WIN32_CONSOLE256
#define WIN32_COLORS                16
#endif

// #define DO_TRACE_LOG
#if defined(DO_TRACE_LOG)
extern void w32_trace(const char *, ...);
#define TRACE_LOG(_x) w32_trace _x;
#else
#define TRACE_LOG(_x)
#endif

#define MAXROWS                     500
#define MAXCOLORS                   256

#define UNICODE_HI_SURROGATE_START  0xD800
#define UNICODE_HI_SURROGATE_END    0xDBFF

#define UNICODE_LO_SURROGATE_START  0xDC00
#define UNICODE_LO_SURROGATE_END    0xDFFF

#define UNICODE_REPLACE             0xfffd      /* replacement character */
#define UNICODE_REPLACECTRL         0x1a        /* substitute */
#define UNICODE_REPLACEFULL         0xff1f      /* full-width '?' */
#define UNICODE_MAX                 0x10ffff

#define ISACS                       0x1000000   /* ACS mapped character */

#if ((defined(_MSC_VER) && (_MSC_VER < 1500)) || defined(__MINGW32__)) && \
            !defined(CONSOLE_OVERSTRIKE)
#pragma pack(push, 1)

typedef struct _CONSOLE_FONT_INFOEX {
    ULONG           cbSize;
    DWORD           nFont;
    COORD           dwFontSize;
    UINT            FontFamily;
    UINT            FontWeight;
    WCHAR           FaceName[LF_FACESIZE];
} CONSOLE_FONT_INFOEX, *PCONSOLE_FONT_INFOEX;

typedef struct _CONSOLE_SCREEN_BUFFER_INFOEX {
    ULONG           cbSize;
    COORD           dwSize;
    COORD           dwCursorPosition;
    WORD            wAttributes;
    SMALL_RECT      srWindow;
    COORD           dwMaximumWindowSize;
    WORD            wPopupAttributes;
    BOOL            bFullscreenSupported;
    COLORREF        ColorTable[16];
} CONSOLE_SCREEN_BUFFER_INFOEX, *PCONSOLE_SCREEN_BUFFER_INFOEX;

#if defined(__MINGW32__)                        /* missing */
COORD WINAPI        GetConsoleFontSize(HANDLE hConsoleOutput, DWORD nFont);
BOOL WINAPI         GetCurrentConsoleFont(HANDLE hConsoleOutput, BOOL bMaximumWindow,
                        PCONSOLE_FONT_INFO lpConsoleCurrentFont);
#endif
#pragma pack(pop)
#endif  /*_CONSOLE_FONT_INFOEX*/

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
LIBW32_API int SLtt_Use_Ansi_Colors     = 1;    /* full color support */
LIBW32_API int SLtt_Term_Cannot_Scroll  = 0;
LIBW32_API int SLtt_Term_Cannot_Insert  = 0;
LIBW32_API int SLtt_Try_Termcap         = 0;

static     int SLtt_Maximised           = -1;
static     int SLtt_OldScreen_Rows      = -1;
static     int SLtt_OldScreen_Cols      = -1;
static     int SLtt_True_Color          = 0;

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

static const uint32_t   acs_unicode[128] = {    /* alternative character map to unicode */

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

static void             vio_init(void);
static void             vio_profile(int rebuild);
static void             vio_setsize(int rows, int cols);
static void             vio_reset(void);
static void             vio_setcursor(int col, int row);
static int              rgb_search(int maxval, const struct rgbvalue *rgb);

static uint32_t         acs_lookup(uint32_t ch);
static uint32_t         unicode_map(uint32_t ch);
static int              compute_clip(int coord, int n, int start, int end, int *coordmin, int *coordmax);

static __inline void    CHAR_BUILD(uint32_t ch, int color, CHAR_INFO *ci);
static __inline BOOL    CHAR_COMPARE(const CHAR_INFO *c1, const CHAR_INFO *c2);
static __inline int     CHAR_UPDATE(CHAR_INFO *cursor, unsigned ch, int color);

static void             write_char(SLwchar_Type ch, unsigned cnt);
static void             write_string(const char *str, unsigned cnt);

static void             CopyIn(unsigned pos, unsigned cnt);
static void             CopyOut(unsigned offset, unsigned len);
#if defined(WIN32_CONSOLEEXT)
#if defined(WIN32_CONSOLE256)
static void             CopyOutEx(unsigned pos, unsigned cnt);
#endif
static void             UnderOutEx(unsigned pos, unsigned cnt);
#endif

#if defined(WIN32_CONSOLE256)
static int              consolefontset(int height, int width, const char *facename);
static void             consolefontsenum(void);
static HFONT            consolefontcreate(int height, int width, int weight, int italic, const char *facename);
static BOOL             GetConsoleFontInfoEx(HANDLE chandle, BOOL flag, DWORD count, CONSOLE_FONT_INFOEX *cfix);
#endif

static const void *     utf8_decode_raw(const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
static const void *     utf8_decode_safe(const void *src, const void *cpend, int32_t *cooked);

static int              parse_color(const char *name, const char *defname, const struct attrmap *map, int *attr);
static int              parse_true_color(const char *color, COLORREF *rgb, int *attr);
static int              parse_attributes(const char *attr);

struct rgbvalue {
    unsigned char       red;
    unsigned char       green;
    unsigned char       blue;
};

struct color16 {
    unsigned char       fg;
    unsigned char       bg;
};

struct color256 {
    unsigned            flags;
#define COLOR256_BACKGROUND_TRUECOLOR   0x01
#define COLOR256_FOREGROUND_TRUECOLOR   0x02
    unsigned char       fg;
    unsigned char       bg;
    COLORREF            fg_rgb;
    COLORREF            bg_rgb;
};

struct sline {
    unsigned            flags;
    CHAR_INFO *         text;
};

#define FACENAME_MAX    64
#define FONTS_MAX       64

typedef DWORD  (WINAPI *GetNumberOfConsoleFonts_t)(void);
typedef BOOL   (WINAPI *GetConsoleFontInfo_t)(HANDLE, BOOL, DWORD, CONSOLE_FONT_INFO *);
typedef BOOL   (WINAPI *GetConsoleFontInfoEx_t)(HANDLE, BOOL, DWORD, CONSOLE_FONT_INFOEX *);
typedef BOOL   (WINAPI *GetCurrentConsoleFontEx_t)(HANDLE, BOOL, CONSOLE_FONT_INFOEX *);
typedef BOOL   (WINAPI *SetConsoleFont_t)(HANDLE, DWORD);
typedef BOOL   (WINAPI *SetCurrentConsoleFontEx_t)(HANDLE, BOOL, CONSOLE_FONT_INFOEX *);
typedef BOOL   (WINAPI *GetConsoleScreenBufferInfoEx_t)(HANDLE, CONSOLE_SCREEN_BUFFER_INFOEX *);

static struct {                                 /* Video state */
    //  Resource handles
    //
    BOOL                clocal;                 // Local console handle.
    HANDLE              chandle;                // Console handle.
    HANDLE              whandle;                // Underlying window handle.
    HFONT               fnHandle;               // Current normal font.
    HFONT               fbHandle;               // Current bold font.
    HFONT               fiHandle;               // Current italic font.

    //  Dynamic bindings
    //
    BOOL                getDynamic;             // Dynamic lookup status.
    GetNumberOfConsoleFonts_t GetNumberOfConsoleFonts;
    GetConsoleFontInfo_t GetConsoleFontInfo;
    GetConsoleFontInfoEx_t GetConsoleFontInfoEx;
    GetCurrentConsoleFontEx_t GetCurrentConsoleFontEx;
    GetConsoleScreenBufferInfoEx_t GetConsoleScreenBufferInfoEx;
    SetConsoleFont_t SetConsoleFont;
    SetCurrentConsoleFontEx_t SetCurrentConsoleFontEx;

    //  Font information
    //
    //      Family:
    //          54      Consolas/Lucida Console
    //          48      Terminal
    //      Weight:
    //          400     Normal
    //          700     Bold
    //
    int32_t             fcwidth;
    int32_t             fcheight;
    int32_t             fcfamily;
    int32_t             fcweight;
    uint32_t            fcflags;                /* FCNxxx */
    char                fcfacename[LF_FACESIZE+1];
    struct fcname {
        const char *    name;
#define FCNPRO              0x0001              /* Proportional display qualities */
#define FCNPRO5             (FCNPRO|0x02)       /* Proportional with 50% width scaling */
#define FCNPRO2             (FCNPRO|0x04)       /* Proportional 2/3 width scaled */
#define FCNUNC              0x0100              /* Unicode */
#define FCNTRUETYPE         0x1000
#define FCNRASTER           0x2000
#define FCNDEVICE           0x4000
        uint32_t        flags;
        uint32_t        available;
    } fcnames[FACENAME_MAX];

    int                 fontindex;
    int                 fontnumber;
    CONSOLE_FONT_INFOEX fonts[FONTS_MAX];

    //  General information
    //
    int                 inited;
    int                 dynamic;                /* Dynamic bindings */

    CONSOLE_CURSOR_INFO cinfo;
    COORD               ccoord;

    int                 displaymode;            /* Diplay mode (0=Normal,1=Full) */
    int                 rows, cols;             /* Screen size */

    ULONG               size;                   /* Screen buffer size */
    CHAR_INFO *         image;                  /* Screen image */
    CHAR_INFO *         oimage;                 /* Backing image */
    CHAR_INFO *         oshadow;                /* Black&white shadow backing image */
    unsigned            codepage;               /* Font code page */
    unsigned            maxcolors;              /* Maximum colors supported (16 or 256) */
    unsigned            activecolors;           /* Active colors (16 or 256) */
    const uint32_t *    acsmap;                 /* Alternative character map */

    SLtt_Char_Type      c_color;                /* Current color 'attribute' */
    struct color16      c_colors[MAXCOLORS];    /* 16color attributes */
    struct color256     c_colors256[MAXCOLORS]; /* 256color attributes */

    WORD                c_attrs[MAXCOLORS];     /* Attributes */
#define VIO_UNDERLINE       0x0100
#define VIO_BOLD            0x0200
#define VIO_BLINK           0x0400
#define VIO_REVERSE         0x0800
#define VIO_ITALIC          0x1000
#define VIO_SPECIAL         0x2000
#define VIO_ALTCHAR         0x4000
    COLORREF            rgb256[256];            /* 256-color to RGB color map */
    BYTE                color256to16[256];      /* 256-color to win-16-color map */

    int                 c_row, c_col;           /* Cursor row/col */
#define TOUCHED             0x01
#define TRASHED             0x02

    unsigned            c_trashed;              /* Trashed signal */
    struct sline        c_screen[MAXROWS];      /* Screen lines */
} vio;

enum wincols {
    WINCOL_BLACK = 0,
    WINCOL_BLUE,
    WINCOL_GREEN,
    WINCOL_AQUA,
    WINCOL_RED,
    WINCOL_PURPLE,
    WINCOL_BROWN,
    WINCOL_WHITE,
    WINCOL_GRAY,
    WINCOL_BRIGHTBLUE,
    WINCOL_BRIGHTGREEN,
    WINCOL_BRIGHTAQUA,
    WINCOL_BRIGHTRED,
    WINCOL_BRIGHTPURPLE,
    WINCOL_BRIGHTBROWN,
    WINCOL_BRIGHTWHITE
    };

static const struct rgbvalue rgb_colors256[256] = {
#include "w32_colors256.h"
    };

static const WORD       win2xterm[16] = {
    /*
     *  Map windows-console to internal/xterm colors.
     */                                         //              IRGB
    SLSMG_COLOR_BLACK,                          // BLACK        0000
    SLSMG_COLOR_BLUE,                           // BLUE         0001
    SLSMG_COLOR_GREEN,                          // GREEN        0010
    SLSMG_COLOR_CYAN,                           // AQUA         0011
    SLSMG_COLOR_RED,                            // RED          0100
    SLSMG_COLOR_MAGENTA,                        // PURPLE       0101
    SLSMG_COLOR_BROWN,                          // BROWN        0110
    SLSMG_COLOR_LGRAY,                          // WHITE        0111
    SLSMG_COLOR_GRAY,                           // GRAY         1000
    SLSMG_COLOR_BRIGHT_BLUE,                    // BRIGHTBLUE   1001
    SLSMG_COLOR_BRIGHT_GREEN,                   // BRIGHTGREEN  1010
    SLSMG_COLOR_BRIGHT_CYAN,                    // BRIGHTAQUA   1011
    SLSMG_COLOR_BRIGHT_RED,                     // BRIGHTRED    1100
    SLSMG_COLOR_BRIGHT_MAGENTA,                 // BRIGHTPURPLE 1101
    SLSMG_COLOR_BRIGHT_BROWN,                   // BRIGHTBROWN  1110
    SLSMG_COLOR_BRIGHT_WHITE                    // BRIGHTWHITE  1111
    };

static const BYTE       xterm2win[16] = {
    /*
     *  Map internal/xterm to windows-console colors.
     */
    WINCOL_BLACK,
    WINCOL_RED,
    WINCOL_GREEN,
    WINCOL_BROWN,
    WINCOL_BLUE,
    WINCOL_PURPLE,
    WINCOL_AQUA,
    WINCOL_WHITE,
    WINCOL_GRAY,
    WINCOL_BRIGHTRED,
    WINCOL_BRIGHTGREEN,
    WINCOL_BRIGHTBROWN,
    WINCOL_BRIGHTBLUE,
    WINCOL_BRIGHTPURPLE,
    WINCOL_BRIGHTAQUA,
    WINCOL_BRIGHTWHITE
    };

struct attrmap {
    const char *        name;
    WORD                value;
    };

static const struct attrmap win16_background[] = {
    /*
     *  Windows color16 background colour map.
     */
    { "black",          0 },
    { "red",            BACKGROUND_RED },
    { "green",          BACKGROUND_GREEN  },
    { "brown",          BACKGROUND_RED | BACKGROUND_GREEN },
    { "blue",           BACKGROUND_BLUE },
    { "magenta",        BACKGROUND_BLUE | BACKGROUND_RED },
    { "cyan",           BACKGROUND_BLUE | BACKGROUND_GREEN },
    { "lightgray",      BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE },
    { "gray",           BACKGROUND_INTENSITY },
    { "brightred",      BACKGROUND_INTENSITY | BACKGROUND_RED },
    { "brightgreen",    BACKGROUND_INTENSITY | BACKGROUND_GREEN  },
    { "yellow",         BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN },
    { "brightblue",     BACKGROUND_INTENSITY | BACKGROUND_BLUE },
    { "brightmagenta",  BACKGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_RED },
    { "brightcyan",     BACKGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_GREEN },
    { "white",          BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE },
    { NULL }
    };

static const struct attrmap win16_foreground[] = {
    /*
     *  Windows color16 foreground colour map.
     */
    { "black",          0 },
    { "red",            FOREGROUND_RED },
    { "green",          FOREGROUND_GREEN  },
    { "brown",          FOREGROUND_RED | FOREGROUND_GREEN },
    { "blue",           FOREGROUND_BLUE },
    { "magenta",        FOREGROUND_BLUE | FOREGROUND_RED },
    { "cyan",           FOREGROUND_BLUE | FOREGROUND_GREEN },
    { "lightgray",      FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
    { "gray",           FOREGROUND_INTENSITY },
    { "brightred",      FOREGROUND_INTENSITY | FOREGROUND_RED },
    { "brightgreen",    FOREGROUND_INTENSITY | FOREGROUND_GREEN  },
    { "yellow",         FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN },
    { "brightblue",     FOREGROUND_INTENSITY | FOREGROUND_BLUE },
    { "brightmagenta",  FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_RED},
    { "brightcyan",     FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN },
    { "white",          FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
    { NULL }
    };


static const struct localecodepage {
    /*
     *  Code Page Bitfields; see FONTSIGNATURE and LOCALESIGNATURE structures.
     */
    unsigned    bit;
    unsigned    codepage;
    const char *description;

} localecodepages[] = {
    { 0,    1252,   "Latin 1" },
    { 1,    1250,   "Latin 2: Central Europe" },
    { 2,    1251,   "Cyrillic" },
    { 3,    1253,   "Greek" },
    { 4,    1254,   "Turkish" },
    { 5,    1255,   "Hebrew" },
    { 6,    1256,   "Arabic" },
    { 7,    1257,   "Baltic" },
    { 8,    1258,   "Vietnamese" },
        // 9 - 15   Reserved for ANSI
    { 16,   874,    "Thai" },
    { 17,   932,    "Japanese, Shift-JIS" },
    { 18,   936,    "Simplified Chinese (PRC, Singapore)" },
    { 19,   949,    "Korean Unified Hangul Code (Hangul TongHabHyung Code)" },
    { 20,   950,    "Traditional Chinese (Taiwan; Hong Kong SAR, PRC)" },
    { 21,   1361,   "Korean (Johab)" },
        // 22 - 29  Reserved for alternate ANSI and OEM
        // 30 - 31  Reserved by system.
        // 32 - 46  Reserved for OEM
    { 47,   1258,   "Vietnamese" },
    { 48,   869,    "Modern Greek" },
    { 49,   866,    "Russian" },
    { 50,   865,    "Nordic" },
    { 51,   864,    "Arabic" },
    { 52,   863,    "Canadian French" },
    { 53,   862,    "Hebrew" },
    { 54,   861,    "Icelandic" },
    { 55,   860,    "Portuguese" },
    { 56,   857,    "Turkish" },
    { 57,   855,    "Cyrillic; primarily Russian" },
    { 58,   852,    "Latin 2" },
    { 59,   775,    "Baltic" },
    { 60,   737,    "Greek; formerly 437G" },
    { 61,   708,    "Arabic; ASMO 708" },
    { 61,   720,    "Arabic; ASMO 708" },
    { 62,   850,    "Multilingual Latin 1" },
    { 63,   437,    "US" },
    };


static void
vio_init(void)
{
    CONSOLE_SCREEN_BUFFER_INFO sbinfo;
    HANDLE chandle = GetStdHandle(STD_OUTPUT_HANDLE);
    int rows, cols;

    //  Console handle,
    //      when stdout has been redirected, create a local console.
    //
    if (NULL == vio.chandle) {
        if (chandle == INVALID_HANDLE_VALUE ||
                GetFileType(chandle) != FILE_TYPE_CHAR) {
            SECURITY_ATTRIBUTES sa = {0};

            sa.nLength = sizeof(sa);
            sa.lpSecurityDescriptor = NULL;
            sa.bInheritHandle = TRUE;           // inherited
            chandle = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_WRITE | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, 0);
            vio.clocal = TRUE;
        }
    }

    //  Screen sizing
    //
    GetConsoleScreenBufferInfo(chandle, &sbinfo);

    rows = 1 + sbinfo.srWindow.Bottom - sbinfo.srWindow.Top;
    cols = 1 + sbinfo.srWindow.Right - sbinfo.srWindow.Left;

    if (rows >= MAXROWS) {
        rows = (MAXROWS - 1);
    }

    if (vio.chandle != chandle || vio.cols != cols || vio.rows != rows) {
        const CHAR_INFO *oimage;
        int l;

        if (NULL == vio.chandle) {
            const struct rgbvalue *rgb = rgb_colors256;
            int color;

            vio.maxcolors = WIN32_COLORS;       // 16 or 256

            for (color = 0; color < 256; ++color, ++rgb) {

                vio.rgb256[color] =             // default colour table
                    RGB(rgb->red, rgb->green, rgb->blue);

                vio.color256to16[color] =
                    xterm2win[rgb_search(16, rgb)];
            }

            vio.whandle = GetConsoleWindow();   // underlying console window handle
            vio.chandle = chandle;
        }

        vio_profile(0);                         // font profile

        vio.size = rows * cols;

        oimage = vio.image;
        vio.image = malloc(vio.size * sizeof(CHAR_INFO));

        if (oimage) {                           // screen has resized
            const CHAR_INFO blank = {' ', FOREGROUND_INTENSITY};
            const int screencols = vio.cols;
            int r, c, cnt =
                (cols > screencols ? screencols : cols);

            for (r = 0; r < rows; ++r) {

                if (r < vio.rows) {             // copy oldimage
                    memcpy(vio.image + (r*cols), oimage + (r*screencols), cnt * sizeof(CHAR_INFO));
                }
                                                // blank new cells
                if ((c = (r >= vio.rows ? 0 : screencols)) < cols) {
                    CHAR_INFO *p = vio.image + (r*cols) + c;
                    do {
                        *p++ = blank;
                    } while (++c < cols);
                }
            }

            free((void *)oimage);
        }

        free(vio.oimage);
        vio.oimage  = (CHAR_INFO *)calloc(vio.size, sizeof(CHAR_INFO));
        free(vio.oshadow);
        vio.oshadow = (CHAR_INFO *)calloc(vio.size, sizeof(CHAR_INFO));

        vio.c_trashed = 1;
        vio.rows = rows;
        vio.cols = cols;

        for (l = 0; l < rows; ++l) {
            vio.c_screen[l].flags = 0;
            vio.c_screen[l].text = vio.image + (l * cols);
        }

        if (NULL == oimage) {
            CopyIn(0, vio.size);                // populate image
        }

        GetConsoleCursorInfo(vio.chandle, &vio.cinfo);
        vio.ccoord = sbinfo.dwCursorPosition;
    }

    SLtt_Screen_Rows = vio.rows;
    SLtt_Screen_Cols = vio.cols;
}


static void
vio_profile(int rebuild)
{
    HANDLE chandle = vio.chandle;
    DWORD consolemode = 0;

    //  Basic
    //
    assert(chandle);
    assert(vio.whandle);
    TRACE_LOG(("vio_profile()\n"))

    vio.displaymode =                           // 0=Normal or 1=Full-screen mode
        (GetConsoleDisplayMode(&consolemode) && consolemode ? 1 : 0);

    //  Color support
    //
#if defined(WIN32_CONSOLEEXT) && defined(WIN32_CONSOLE256)
    if (GetModuleHandleA("ConEmuHk.dll") ||
            GetModuleHandleA("ConEmuHk64.dll")) {
        printf("Running under ConEmu, disabling 256 support\n");
        vio.maxcolors = 16;
    }
#endif

    //  Undocumented/dynamic function bindings
    //
    if (! vio.dynamic) {
        HMODULE hMod;

        if (0 != (hMod = GetModuleHandleA("Kernel32.dll"))) {
                                                // resolve
            vio.GetConsoleFontInfo =
                (GetConsoleFontInfo_t) GetProcAddress(hMod, "GetConsoleFontInfo");
            vio.GetConsoleFontInfoEx =
                (GetConsoleFontInfoEx_t) GetProcAddress(hMod, "GetConsoleFontInfoEx");
            vio.GetNumberOfConsoleFonts =
                (GetNumberOfConsoleFonts_t) GetProcAddress(hMod, "GetNumberOfConsoleFonts");
            vio.GetCurrentConsoleFontEx =
                (GetCurrentConsoleFontEx_t) GetProcAddress(hMod, "GetCurrentConsoleFontEx");
            vio.GetConsoleScreenBufferInfoEx =
                (GetConsoleScreenBufferInfoEx_t) GetProcAddress(hMod, "GetConsoleScreenBufferInfoEx");
            vio.SetConsoleFont =
                (SetConsoleFont_t) GetProcAddress(hMod, "SetConsoleFont");
            vio.SetCurrentConsoleFontEx =
                (SetCurrentConsoleFontEx_t) GetProcAddress(hMod, "SetCurrentConsoleFontEx");

            TRACE_LOG(("Console Functions\n"))
            TRACE_LOG(("  GetConsoleFontInfo:           %p\n", vio.GetConsoleFontInfo))
            TRACE_LOG(("  GetConsoleFontInfoEx:         %p\n", vio.GetConsoleFontInfoEx))
            TRACE_LOG(("  GetNumberOfConsoleFonts:      %p\n", vio.GetNumberOfConsoleFonts))
            TRACE_LOG(("  GetCurrentConsoleFontEx:      %p\n", vio.GetCurrentConsoleFontEx))
            TRACE_LOG(("  GetConsoleScreenBufferInfoEx: %p\n", vio.GetConsoleScreenBufferInfoEx))
            TRACE_LOG(("  SetConsoleFont:               %p\n", vio.SetConsoleFont))
            TRACE_LOG(("  SetCurrentConsoleFontEx:      %p\n", vio.SetCurrentConsoleFontEx))
        }
        vio.dynamic = TRUE;
    }

    //  CodePage
    //
    TRACE_LOG(("CodePage: %u\n", (unsigned)GetConsoleOutputCP()))
    if (0 == (vio.codepage = GetConsoleOutputCP())) {
        vio.codepage = 437;                     // default windows/dos code page.
    }

#if defined(DO_TRACE_LOG) && defined(_MSC_VER)  // FIXME: OWC missing definitions.
    {   LOCALESIGNATURE locSig;                 // locale information
        if (GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT,
                    LOCALE_FONTSIGNATURE, (LPWSTR)&locSig, (sizeof(locSig) / sizeof(WCHAR)))) {

            TRACE_LOG(("FontSignature:\n"))
            TRACE_LOG(("  Usb[%08x, %08x, %08x, %08x]\n", (unsigned)locSig.lsUsb[0], (unsigned)locSig.lsUsb[1], (unsigned)locSig.lsUsb[2], (unsigned)locSig.lsUsb[3]))

            TRACE_LOG(("  CsbDefault[%08x, %08x]\n", (unsigned)locSig.lsCsbDefault[0], (unsigned)locSig.lsCsbDefault[1]))
            {   const uint64_t bits = ((uint64_t)locSig.lsCsbDefault[0]) | (((uint64_t)locSig.lsCsbDefault[1]) << 32);
                const struct localecodepage *cursor = localecodepages,
                    *end = cursor + (sizeof(localecodepages)/sizeof(localecodepages[0]));

                for (;cursor < end; ++cursor)
                    if ((((uint64_t)1) << cursor->bit) & bits) {
                        TRACE_LOG(("   %2d] %4u '%s'", cursor->bit, cursor->codepage, cursor->description))
                    }
            }

            TRACE_LOG(("  CsbSupported[%08x, %08x]\n", (unsigned)locSig.lsCsbSupported[0], (unsigned)locSig.lsCsbSupported[1]))
            {   const uint64_t bits = ((uint64_t)locSig.lsCsbSupported[0]) | (((uint64_t)locSig.lsCsbSupported[1]) << 32);
                const struct localecodepage *cursor = localecodepages,
                    *end = cursor + (sizeof(localecodepages)/sizeof(localecodepages[0]));

                for (;cursor < end; ++cursor)
                    if ((((uint64_t)1) << cursor->bit) & bits) {
                        TRACE_LOG(("   %2d] %4u '%s'", cursor->bit, cursor->codepage, cursor->description))
                    }
            }
        }
    }
#endif  //DO_TRACE_LOG

    //  Font configuration
    //
    if (NULL == vio.fcnames[0].name) {
        consolefontsenum();
    }

    if (0 == vio.fontnumber || rebuild) {

        // dynamic colors
        if (vio.GetConsoleScreenBufferInfoEx) { // vista+
            CONSOLE_SCREEN_BUFFER_INFOEX csbix = {0};
            WORD c;

            csbix.cbSize = sizeof(csbix);
            vio.GetConsoleScreenBufferInfoEx(chandle, &csbix);
            TRACE_LOG(("Console Colors (BBGGRR)\n"))
            for (c = 0; c < 16; ++c) {
                TRACE_LOG(("  [%2u] 0x%06x\n", c, (unsigned)csbix.ColorTable[c]))
                vio.rgb256[win2xterm[c]] = csbix.ColorTable[c];
            }
        }

        // current fonts
        if (vio.GetCurrentConsoleFontEx) {
            CONSOLE_FONT_INFOEX cfix = {0};

            cfix.cbSize = sizeof(cfix);
            if (vio.GetCurrentConsoleFontEx(chandle, FALSE, &cfix)) {
                COORD coord;

                vio.fontindex = cfix.nFont;
                coord = GetConsoleFontSize(chandle, cfix.nFont);
                vio.fcheight  = coord.Y;        // cfix.dwFontSize.Y;
                vio.fcwidth   = coord.X;        // cfix.dwFontSize.X;
                vio.fcfamily  = cfix.FontFamily;
                vio.fcweight  = cfix.FontWeight;
                wcstombs(vio.fcfacename, cfix.FaceName, sizeof(vio.fcfacename));
            }

        } else {
            CONSOLE_FONT_INFO cfi = {0};

            if (GetCurrentConsoleFont(chandle, FALSE, &cfi)) {
                COORD coord;

                vio.fontindex = cfi.nFont;
                coord = GetConsoleFontSize(chandle, cfi.nFont);
                vio.fcheight  = coord.Y;        // cfi.dwFontSize.Y;
                vio.fcwidth   = coord.X;        // cfi.dwFontSize.X;
                vio.fcfamily  = -1;
                vio.fcweight  = -1;
                vio.fcfacename[0] = 0;          // Note: GetTextFace() is 'System'

            } else {
                vio.fontindex = -1;             // full screen
                vio.fcheight  = 16;
                vio.fcwidth   =  8;
                vio.fcweight  = -1;
                vio.fcfacename[0] = 0;
            }
        }

        TRACE_LOG(("Current Font: Idx:%d, %dx%d, Family:%d, Weight:%d, Mode:%d, Name:<%s> (%s)\n",
            vio.fontindex, vio.fcwidth, vio.fcheight,
                vio.fcfamily, vio.fcweight, vio.displaymode, vio.fcfacename,
                (vio.fcflags & FCNRASTER) ? "raster" : "unicode"))
        vio.acsmap = (vio.fcflags & FCNRASTER ? acs_oem : acs_unicode);

        // available fonts
        vio.fontnumber = -1;
        if (vio.GetNumberOfConsoleFonts && vio.GetConsoleFontInfo) {
            CONSOLE_FONT_INFOEX *fonts = vio.fonts;
            CONSOLE_FONT_INFO fi[FONTS_MAX + 1] = {0};
            DWORD count;

            if ((count = vio.GetNumberOfConsoleFonts()) > FONTS_MAX) {
                count = FONTS_MAX;
            }

            if (vio.GetConsoleFontInfoEx) {     // extension
                fonts->cbSize = sizeof(vio.fonts);
                if (vio.GetConsoleFontInfoEx(chandle, 0, count, fonts)) {
                    vio.fontnumber = count;
                }
                                                // xp+
            } else if (vio.GetConsoleFontInfo(chandle, 0, count, fi)) {
                DWORD f; for (f = 0; f < count; ++f, ++fonts) {
                    fonts->nFont = fi[f].nFont;
                    fonts->dwFontSize = GetConsoleFontSize(chandle, fi[f].nFont);
                }
                vio.fontnumber = count;
            }

            if (vio.fontnumber) {
                //
                //  Generally the first entry represents a <Terminal> entry,
                //  with secondary elements representing True-Type fonts.
                //
                //    Idx    W x  H   Fam   Wgt  Facename
                //      0:   4 x  6,    0,    0, <Terminal>
                //      1:   6 x  8,    0,    0, <Terminal>
                //      2:   8 x  8,    0,    0, <Terminal>
                //      3:  16 x  8,    0,    0, <Terminal>
                //      4:   5 x 10,    0,    0, <Consolas> [Normal]
                //      5:   5 x 10,    0,    0, <Consolas> [Bold]
                //      6:   5 x 12,    0,    0, <Terminal>
                //      7:   7 x 12,    0,    0, <Terminal>
                //      8:   8 x 12,    0,    0, <Terminal>
                //      9:  16 x 12,    0,    0, <Terminal>
                //      10:  8 x 16,    0,    0, <Terminal>
                //      11:  8 x 16,    0,    0, <Terminal>
                //      12: 12 x 16,    0,    0, <Terminal>
                //      13:  8 x 18,    0,    0, <Terminal>
                //      14:  8 x 18,    0,    0, <Terminal>
                //      15: 10 x 18,    0,    0, <Terminal>
                //
                //  Note: The font table is console session specific.
                //
#if defined(DO_TRACE_LOG)
                CONSOLE_FONT_INFOEX *cursor = vio.fonts;
                char t_facename[32] = {0};
                DWORD f;

                TRACE_LOG(("Console Facenames (%u)\n", (unsigned)count))
                TRACE_LOG(("  Idx  W x  H   Fam   Wgt  Facename\n"))
                for (f = 0; f < count; ++f, ++cursor) {
                    wcstombs(t_facename, cursor->FaceName, sizeof(t_facename));
                    TRACE_LOG(("  %2d: %2u x %2u, %4u, %4u, <%s>\n", (int)cursor->nFont, \
                        (unsigned)cursor->dwFontSize.X, (unsigned)cursor->dwFontSize.Y, \
                        (unsigned)cursor->FontFamily, (unsigned)cursor->FontWeight, t_facename))
                }
#endif
            }
        }
    }
}


static BOOL
GetConsoleFontInfoEx(HANDLE chandle, BOOL flag, DWORD count, CONSOLE_FONT_INFOEX *cfix)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi = {0};
    CONSOLE_FONT_INFOEX cfi = {0};
    WINDOWPLACEMENT wp = {0};
    HANDLE whandle = vio.whandle;
    DWORD fontNumber, nFont;
    char facename[64] = {0};

    cfi.cbSize = sizeof(cfi);
    if (! vio.GetNumberOfConsoleFonts ||
            ! vio.GetCurrentConsoleFontEx ||
            ! vio.SetConsoleFont ||
            ! vio.GetCurrentConsoleFontEx(chandle, FALSE, &cfi) ||
            0 == (fontNumber = vio.GetNumberOfConsoleFonts())) {
        TRACE_LOG(("GetConsoleFontInfoEx(n/a)\n"))
        return FALSE;
    }

    wp.length = sizeof(wp);
    GetWindowPlacement(whandle, &wp);
    GetConsoleScreenBufferInfo(chandle, &csbi);
    SendMessage(whandle, WM_SETREDRAW, FALSE, 0);
    ShowWindow(whandle, SW_MINIMIZE);

    TRACE_LOG(("GetConsoleFontInfoEx (%u of %u)\n", (unsigned)count, (unsigned)fontNumber))
    TRACE_LOG(("  Idx   Wx H   Family Weight FontName\n"))

    if (fontNumber > count) {
        fontNumber = count;
    }

    for (nFont = 0; nFont < fontNumber; ++nFont, ++cfix) {

        if (vio.SetConsoleFont(chandle, nFont)) {
            cfix->cbSize = sizeof(*cfix);
            vio.GetCurrentConsoleFontEx(chandle, flag, cfix);
        }

        wcstombs(facename, (const void *)cfix->FaceName, sizeof(facename));
        TRACE_LOG(("  [%2u] %2ux%2u %6u %6u  <%s>\n",
        (unsigned)nFont, (unsigned)cfix->dwFontSize.X, (unsigned)cfix->dwFontSize.Y,
                (unsigned)cfix->FontFamily, (unsigned)cfix->FontWeight, facename));
    }

    vio.SetConsoleFont(chandle, cfi.nFont);
    SetConsoleWindowInfo(chandle, TRUE, &csbi.srWindow);
    SetConsoleCursorPosition(chandle, csbi.dwCursorPosition);
    ShowWindow(whandle, SW_NORMAL);
    SendMessage(whandle, WM_SETREDRAW, TRUE, 0);

    return fontNumber;
}


static void
vio_setsize(int rows, int cols)
{
    const int orows = vio.rows, ocols = vio.cols;
    HANDLE chandle = vio.chandle;
    SMALL_RECT rect = {0, 0, 0, 0};
    COORD msize, nbufsz;
    int bufwin = FALSE;

    msize = GetLargestConsoleWindowSize(chandle);

    if (rows <= 0) rows = orows;                // current
    else if (rows >= msize.Y) rows = msize.Y-1; // limit

    if (cols <= 0) cols = ocols;                // current
    else if (cols >= msize.X) cols = msize.X-1; // limit

    rect.Top    = 0;
    rect.Bottom = (SHORT)(rows - 1);
    rect.Left   = 0;
    rect.Right  = (SHORT)(cols - 1);

    nbufsz.Y    = (SHORT)rows;
    nbufsz.X    = (SHORT)cols;

    if (orows <= rows) {
        if (ocols <= cols) {                    // +cols, +rows
            bufwin = TRUE;

        } else {                                // -cols, -rows
            SMALL_RECT nwinsz = {0, 0, (SHORT)(cols - 1), (SHORT)(orows - 1)};
            SetConsoleWindowInfo(chandle, TRUE, &nwinsz);
            bufwin = TRUE;
        }
    } else {
        if (ocols <= cols) {                    // +cols, -rows
            SMALL_RECT nwinsz = {0, 0, (SHORT)(ocols - 1), (SHORT)(rows- 1)};
            SetConsoleWindowInfo(chandle, TRUE, &nwinsz);
            bufwin = TRUE;

        } else {                                // -cols, -rows
            SetConsoleWindowInfo(chandle, TRUE, &rect);
            SetConsoleScreenBufferSize(chandle, nbufsz);
        }
    }

    if (bufwin) {                               // set buffer and window
        SetConsoleScreenBufferSize(chandle, nbufsz);
        SetConsoleWindowInfo(chandle, TRUE, &rect);
    }
}


static void
vio_reset(void)
{
    if (!vio.inited) return;

    vio.cols = vio.rows = 0;
    if (vio.image) {
        free(vio.oshadow); vio.oshadow = NULL;
        free(vio.oimage); vio.oimage = NULL;
        free(vio.image); vio.image = NULL;
    }
    vio.inited = 0;
}


static void
vio_setcursor(int col, int row)
{
    COORD coord;
    coord.X = col;
    coord.Y = row;
    SetConsoleCursorPosition(vio.chandle, coord);
}


static int
rgb_search(int maxval, const struct rgbvalue *rgb)
{
    const int red   = (int)rgb->red;
    const int green = (int)rgb->green;
    const int blue  = (int)rgb->blue;

    const struct rgbvalue *table = rgb_colors256;
    double smallest = 100000000.0;
    int color = 0, i;

    for (i = 0; i < maxval; ++i) {
        double distance, tmp;

        tmp = red   - (int)table->red;
        distance  = tmp * tmp;
        tmp = green - (int)table->green;
        distance += tmp * tmp;
        tmp = blue  - (int)table->blue;
        distance += tmp * tmp;
        if (distance < smallest) {
            smallest = distance;
            color = i;
        }
        ++table;
    }
    return color;
}


int
SLsmg_init_smg(void)
{
    if (vio.inited) {
        vio_reset();
    } else {
        const char *ct = getenv ("COLORTERM");
        if (ct) {
            SLtt_Use_Ansi_Colors = 1;           // implied
            if (0 == strcmp(ct, "truecolor") || 0 == strcmp(ct, "24bit")) {
                SLtt_True_Color = 1;            // true color support requested.
            }
        }
    }
    vio_init();
    SLtt_set_color(0, NULL, "lightgray", "black");
    vio.inited = 1;
    return 0;
}


int
SLsmg_reinit_smg(void)
{
	if (!vio.inited) return -1;
    vio_init();
    vio.c_trashed = 1;
    return 0;
}


void
SLsmg_reset_smg(void)
{
    if (1 == SLtt_Maximised) {
        vio_setsize(SLtt_OldScreen_Rows, SLtt_OldScreen_Cols);
        ShowWindow(vio.whandle, /*SW_RESTORE*/ SW_NORMAL);
        SLtt_Maximised = 0;
    }
    vio_reset();
}


void
SLsmg_refresh(void)
{
    int cnt, l;

    if (vio.inited == 0) return;

	if (vio.c_trashed)
		vio_setcursor(0, 0);					/* home console (and visible buffer) */
    for (l = 0, cnt = vio.c_trashed; l < vio.rows; ++l)
        if (vio.c_trashed || vio.c_screen[l].flags) {
			if (0 == ++cnt)
                vio_setcursor(0, 0);
            CopyOut(vio.cols * l, vio.cols);
            vio.c_screen[l].flags = 0;
        }
    vio.c_trashed = 0;
    vio_setcursor(vio.c_col, vio.c_row);        /* true cursor position */
}


static void
CopyIn(unsigned pos, unsigned cnt)
{
    const int rows = vio.rows, cols = vio.cols;
    COORD is = {0}, ic = {0};
    SMALL_RECT wr = {0};
    DWORD rc;

    assert(pos < vio.size);
    assert(0 == (pos % cols));
    assert((pos + cnt) <= vio.size);

    wr.Left   = 0;                              /* src. screen rectangle */
    wr.Right  = (SHORT)(cols - 1);
    wr.Top    = (SHORT)(pos / cols);
    wr.Bottom = (SHORT)((pos + (cnt - 1)) / cols);

    is.Y      = vio.rows - wr.Top;              /* size of image */
    is.X      = vio.cols;

    ic.X      = 0;                              /* top left src cell in image */
    ic.Y      = 0;

    rc = ReadConsoleOutputW(vio.chandle,        /* read in image */
            vio.image + pos, is, ic, &wr);

    if (0 == rc && ERROR_NOT_ENOUGH_MEMORY == GetLastError()) {
        if (cnt >= (unsigned)(cols * 4)) {      /* sub-divide request */
            const int cnt2 = cols * ((cnt / cols) / 2);

            CopyIn(pos, cnt2);
            CopyIn(pos + cnt2, cnt - cnt2);
            return;
        }
    }

    {CHAR_INFO *cursor, *end;
    for (cursor = vio.image + pos, end = cursor + cnt; cursor < end; ++cursor) {
        cursor->Attributes &= ~VIO_SPECIAL;     /* clear special  */
    }}
}


/*  Function;           Attributes16
 *      Build a 16-color CHAR_INFO attribute; being a native window attribute specification.
 *
 *  Parameters
 *      attributes - Internal VIO CHAR_INFO cell attributes.
 *
 *  Returns:
 *      16-color CHAR_INF attributes.
 */
static __inline WORD
Attributes16(WORD attributes)
{
    if (VIO_SPECIAL & attributes) {
        /*
         *  internal attribute,
         *      map the fg/bg attributes to 16-color counterparts.
         */
        if ((VIO_SPECIAL|0xff) == attributes) {
            return FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;

        } else {
            const uint8_t color = (uint8_t)(attributes & 0x0ff);
            WORD nattr;

            nattr  = win16_foreground[ vio.c_colors[color].fg ].value;
            nattr |= win16_background[ vio.c_colors[color].bg ].value;
            if (((nattr & 0xf0) >> 4) == (nattr & 0x0f)) {
                nattr = FOREGROUND_INTENSITY;
            }
            return nattr|(attributes & 0xff00)|vio.c_attrs[color];
        }
    }
    return attributes;  /* windows native */
}


/*  Function;           AttributesExt
 *      Build an extended CHAR_INFO attribute; being a value-added internal attribute
 *      specification with any associated free attributes.
 *
 *  Parameters
 *      attributes - Internal VIO CHAR_INFO cell attributes.
 *
 *  Returns:
 *      Internal color CHAR_INF attributes.
 */
static __inline WORD
AttributesExt(WORD attributes)
{
    if (VIO_SPECIAL & attributes) {
        /*
         *  internal attribute,
         *      value-add color specific attributes.
         */
        return (attributes|vio.c_attrs[attributes & 0xff]);
    }
    return attributes;  /* windows native */
}


/*  Function;           AttributesShadow
 *      Build a 16-color CHAR_INFO attribute based on the 256-color to 16-color
 *      table; being a native window attribute specification.
 *
 *  Parameters
 *      attributes - Internal VIO CHAR_INFO cell attributes.
 *
 *  Returns:
 *      16-color CHAR_INF attributes.
 */
static __inline WORD
AttributesShadow(WORD attributes)
{
    if (VIO_SPECIAL & attributes) {
        /*
         *  internal attribute,
         *      map the fg/bg 256-color attributes to thier 16-color window counterparts.
         */
        const BYTE color = (BYTE)(attributes & 0x0ff),
            fg = vio.color256to16[ vio.c_colors256[color].fg ],
            bg = vio.color256to16[ vio.c_colors256[color].bg ];

        if ((VIO_SPECIAL|0xff) == attributes || (fg == bg)) {
            return FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
        }
        return (WORD)(fg|(bg << 4));
    }
    return attributes;  /* windows native */
}


/*  Function;           CopyOut
 *      Copy out to video memory.
 *
 *  Parameters
 *      pos - Starting offset in video cells from top left.
 *      cnt - Video cell count.
 *
 *  Returns:
 *      nothing.
 */
static void
CopyOut(unsigned pos, unsigned cnt)
{
    const unsigned activecolors = (vio.displaymode ? 16 : vio.activecolors);
    const int rows = vio.rows, cols = vio.cols;
    CHAR_INFO *oimage = vio.oimage + pos,
        *oshadow = vio.oshadow + pos;
    CHAR_INFO *cursor, *end;
    HANDLE chandle = vio.chandle;
    COORD is = {0}, ic = {0};
    SMALL_RECT wr = {0};
    unsigned underline = 0;

    assert(pos < vio.size);
    assert(0 == (pos % cols));
    assert((pos + cnt) <= vio.size);

    if (vio.maxcolors > 16) {                   /* build output images */
        for (cursor = vio.image + pos, end = cursor + cnt; cursor < end; ++cursor) {
            /*
             *  Primary image and optional shadow image,
             *      shadow is written to the console when running in 256 colour mode.
             */
            const WCHAR UnicodeChar = cursor->Char.UnicodeChar;
            WORD Attributes = cursor->Attributes;

            if ((Attributes & (VIO_SPECIAL|VIO_UNDERLINE)) == (VIO_SPECIAL|VIO_UNDERLINE)) {
                ++underline;
            }

            if (activecolors > 16) {
                Attributes = AttributesExt(Attributes);
                oshadow->Attributes = AttributesShadow(Attributes);
                oshadow->Char.UnicodeChar = UnicodeChar;
                ++oshadow;

            } else {
                Attributes = Attributes16(Attributes);

            }

            oimage->Attributes = Attributes;
            oimage->Char.UnicodeChar = UnicodeChar;
            ++oimage;
        }

    } else {
        for (cursor = vio.image + pos, end = cursor + cnt; cursor < end; ++cursor) {
            /*
             *  Primary image.
             */
            oimage->Attributes = Attributes16(cursor->Attributes);
            oimage->Char.UnicodeChar = cursor->Char.UnicodeChar;
            ++oimage;
        }
    }

    wr.Left   = 0;                              /* src. screen rectangle */
    wr.Right  = (SHORT)(cols - 1);
    wr.Top    = (SHORT)(pos / cols);
    wr.Bottom = (SHORT)((pos + (cnt - 1)) / cols);

    is.Y      = vio.rows - wr.Top;              /* size of image */
    is.X      = vio.cols;

    ic.X      = 0;                              /* top left src cell in image */
    ic.Y      = 0;

#if defined(WIN32_CONSOLEEXT)
#if defined(WIN32_CONSOLE256)
    if (activecolors > 16) {
        //                                      // update console buffer
        //  cursor-get
        //  cursor-hide
        //  updates-disable (stop screen update flicker)
        //      console-write-std
        //  update-enable
        //  console-write-extended
        //  cursor-show
        //
        CONSOLE_CURSOR_INFO cinfo = {0};
        BOOL omode;

        GetConsoleCursorInfo(chandle, &cinfo);
        if (0 != (omode = cinfo.bVisible)) {
            cinfo.bVisible = FALSE;             // hide cursor
            SetConsoleCursorInfo(chandle, &cinfo);
        }
                                                // flush changes, disable updates
        SendMessage(vio.whandle, WM_SETREDRAW, FALSE, 0);
        WriteConsoleOutputW(chandle, vio.oshadow + pos, is, ic, &wr);
        SendMessage(vio.whandle, WM_SETREDRAW, TRUE, 0);

        CopyOutEx(pos, cnt);                    // export text
        if (0 != (cinfo.bVisible = omode)) {    // restore cursor
            SetConsoleCursorInfo(chandle, &cinfo);
        }

    } else {
        WriteConsoleOutputW(chandle, vio.oimage + pos, is, ic, &wr);
    }
#endif  //CONSOLE256

    if (underline) {
        UnderOutEx(pos, cnt);                   // underline region
    }

#else   //CONSOLEEXT
    WriteConsoleOutputW(chandle, vio.oimage + pos, is, ic, &wr);

#endif
}


#if defined(WIN32_CONSOLE256)
/*private*/
/*  Function:           SameAttribute
 *      Determine whether the same attribute, ignore win16_foreground colors for spaces
 *      allowing correct italic display.
 */
static __inline int
IsSpace(const DWORD ch)
{
    switch (ch) {
    case ' ':           // SPACE
    case 0x00A0:        // NO-BREAK SPACE
    case 0x2000:        // EN QUAD
    case 0x2001:        // EM QUAD
    case 0x2002:        // EN SPACE
    case 0x2003:        // EM SPACE
    case 0x2004:        // THREE-PER-EM SPACE
    case 0x2005:        // FOUR-PER-EM SPACE
    case 0x2006:        // SIX-PER-EM SPACE
    case 0x2007:        // FIGURE SPACE
    case 0x2008:        // PUNCTUATION SPACE
    case 0x2009:        // THIN SPACE
    case 0x200A:        // HAIR SPACE
    case 0x200B:        // ZERO WIDTH SPACE
    case 0x202F:        // NARROW NO-BREAK SPACE
        return 1;
    }
    return 0;
}


static __inline int
SameAttributes(const CHAR_INFO cell, const WORD Attributes)
{
    if (IsSpace(cell.Char.UnicodeChar)) {
        return (vio.c_colors256[cell.Attributes & 0xff].bg == vio.c_colors256[Attributes & 0xff].bg);
    }
    return ((cell.Attributes & ~VIO_UNDERLINE) == (Attributes & ~VIO_UNDERLINE));
}


/*private*/
/*  Function:           RGB256
 *      Derive the RGB color256 specifications from the specified attributes.
 *
 *  Parameters:
 *      attributes - Internal VIO CHAR_INFO cell attributes.
 *      fg - Foreground COLORREF value.
 *      bg - Background COLORREF value.
 *
 *  Returns:
 *      nothing.
 */
static __inline void
RGB256(WORD attributes, COLORREF *fg, COLORREF *bg)
{
    if (VIO_SPECIAL & attributes) {
        /*
         *  internal attribute
         */
        if ((VIO_SPECIAL|0xff) == attributes) {
            *fg = vio.rgb256[ SLSMG_COLOR_LGRAY ];
            *bg = vio.rgb256[ SLSMG_COLOR_BLACK ];

        } else {
            const uint8_t obj = (uint8_t)(attributes & 0xff);

            if (vio.c_colors256[obj].flags & COLOR256_FOREGROUND_TRUECOLOR) {
                *fg = vio.c_colors256[obj].fg_rgb;
            } else {
                *fg = vio.rgb256[ vio.c_colors256[obj].fg ];
            }

            if (vio.c_colors256[obj].flags & COLOR256_BACKGROUND_TRUECOLOR) {
                *bg = vio.c_colors256[obj].bg_rgb;
            } else {
                *bg = vio.rgb256[ vio.c_colors256[obj].bg ];
            }

            if (*fg == *bg) {
                *fg = vio.rgb256[ SLSMG_COLOR_LGRAY ];
                *bg = vio.rgb256[ SLSMG_COLOR_BLACK ];
            }
        }

    } else {
        /*
         *  system attribute
         */
        *fg = vio.rgb256[  attributes & 0x0f ];
        *bg = vio.rgb256[ (attributes & 0xf0) >> 4 ];
    }
}


/*private*/
/*  Function:           CopyOutEx
 *      Extended export characters within the specified region to the console
 *      window. Underline is ignored, this attribute is addressed elsewhere.
 *
 *  Parameters:
 *      pos - Starting offset in video cells from top left.
 *      cnt - Video cell count.
 *
 *  Returns:
 *      nothing.
 */
static void
CopyOutEx(size_t pos, size_t cnt)
{
    const CHAR_INFO *cursor = vio.oimage + pos, *end = cursor + cnt;
    HANDLE whandle = vio.whandle;
    const int cols = vio.cols;
    float fcwidth, fcheight;                    // proportional sizing
    int row = pos / cols;
    RECT rect = {0};
    WCHAR text[1024];                           // ExtTextOut limit 8192
    HFONT oldfont;
    HDC wdc;

    assert(pos < vio.size);
    assert(0 == (pos % cols));
    assert((pos + cnt) <= vio.size);

    wdc = GetDC(whandle);                       // client area DC
    GetClientRect(whandle, &rect);
    fcwidth = (float)rect.right / vio.cols;
    fcheight = (float)rect.bottom / vio.rows;

    if (! vio.fnHandle) {                       // allocate font handle
        consolefontset(-1, -1, vio.fcfacename);
        if (! vio.fnHandle) {
            return;
        }
    }
    oldfont = SelectObject(wdc, vio.fnHandle);  // base font

    do {
        int start = -1, col = 0, len = 0;
        WORD attributes = 0;                    // attribute accum

        while (col < cols) {
            do {
                const CHAR_INFO cell = *cursor++;

                if (start >= 0) {
                    if (SameAttributes(cell, attributes)) {
                        text[len] = cell.Char.UnicodeChar;
                        ++col;
                        if (++len >= (int)(sizeof(text)/sizeof(text[0]))-1) {
                            break;              // flush
                        }
                        continue;
                    } else {
                        --cursor;
                        break;                  // flush
                    }
                }
                text[0] = cell.Char.UnicodeChar;
                attributes = cell.Attributes;
                start = col++;
                len = 1;
            } while (col < cols);

            if (start >= 0) {                   // write text
                COLORREF bg, fg;

                RGB256(attributes, &fg, &bg);
                if (vio.fbHandle || vio.fiHandle) {
                    HFONT fhandle = vio.fnHandle;

                    if (((VIO_BOLD|VIO_BLINK) & attributes) && vio.fbHandle) {
                        fhandle = vio.fbHandle;

                    } else if ((VIO_ITALIC & attributes) && vio.fiHandle) {
                        fhandle = vio.fiHandle;

                    }
                    SelectObject(wdc, fhandle);
                }
                text[len] = 0;

                if (FCNPRO & vio.fcflags) {
                    //
                    //  Variable width font,
                    //      display character-by-character.
                    //
                    const int left = (int)(fcwidth * start);
                    const int top = (int)(fcheight * row);
                    HPEN oldbrush, oldpen;
                    const WCHAR *ch = text;
                    int idx;

                    oldbrush = SelectObject(wdc, CreateSolidBrush(bg));
                    oldpen = SelectObject(wdc, CreatePen(PS_SOLID, 0, bg));
                    Rectangle(wdc, left, top, left + (int)(fcwidth * len), top + (int)fcheight);
                    oldpen = SelectObject(wdc, oldpen);
                    oldbrush = SelectObject(wdc, oldbrush);
                    DeleteObject(oldpen);
                    DeleteObject(oldbrush);

                    SetBkColor(wdc, bg);
                    SetTextColor(wdc, fg);
                    SetTextAlign(wdc, GetTextAlign(wdc) | TA_CENTER);
                    for (idx = 0; idx < len; ++idx) {
                        const WCHAR t_ch = *ch++;

                        if (t_ch && !IsSpace(t_ch)) {
                            ExtTextOutW(wdc, left + (int)(fcwidth * (0.5 + idx)), top, ETO_OPAQUE, NULL, &t_ch, 1, NULL);
                        }
                    }

                } else {
                    //
                    //  Fixed width font.
                    //
                    const int left = vio.fcwidth * start,
                        top = vio.fcheight * row;

                    SetBkColor(wdc, bg);
                    SetTextColor(wdc, fg);
                    ExtTextOutW(wdc, left, top, ETO_OPAQUE, NULL, text, len, NULL);
                }
                start = -1;
            }
        }
        ++row;

    } while (cursor < end);

    SelectObject(wdc, oldfont);                 // restore original font (normally system).
    DeleteDC(wdc);                              // release device resources.
}
#endif  //CONSOLE256


#if defined(WIN32_CONSOLEEXT)
/*private*/
/*  Function:           UnderOutEx
 *      Underline characters within the specified region.
 *
 *  Parameters
 *      pos - Starting offset in video cells from top left.
 *      cnt - Video cell count.
 *
 *  Returns:
 *      nothing.
 */
static void
UnderOutEx(size_t pos, size_t cnt)
{
    const CHAR_INFO *cursor = vio.image + pos, *end = cursor + cnt;
    const int cols = vio.cols;
    float fcwidth, fcheight;                    // proportional sizing
    RECT rect = {0};
    int row = pos / cols;
    HDC wdc;

    if (vio.maxcolors < 256) {
        return;
    }

    wdc = GetDC(vio.whandle);                   // client area DC
    GetClientRect(vio.whandle, &rect);
    fcwidth = (float)rect.right / vio.cols;
    fcheight = (float)rect.bottom / vio.rows;

    do {
        int start = -1, col = 0;
        unsigned fg = 0x01;

        while (col < cols) {
            while (col < cols) {
                const CHAR_INFO cell = *cursor++;

                if (VIO_UNDERLINE & cell.Attributes) {
                    if (start < 0) {            // inside
                        fg = vio.c_colors256[cell.Attributes & 0xff].fg;
                        start = col;
                    }
                } else if (start >= 0) {
                    ++col;
                    break;                      // flush
                }
                ++col;
            }

            if (start >= 0) {                   // underline current extent
                const int y = (int)(fcheight * (row + 1)) - 1,
                        x = (int)(fcwidth * start);
                HPEN oldpen;

                oldpen = SelectObject(wdc, CreatePen(PS_SOLID, 0, vio.rgb256[fg]));
                MoveToEx(wdc, x, y, NULL);
                LineTo(wdc, x + (int)(fcwidth * (col - (start + 1))), y);
                oldpen = SelectObject(wdc, oldpen);
                DeleteObject(oldpen);
                start = -1;
            }
        }
        ++row;

    } while (cursor < end);

    DeleteDC(wdc);
}
#endif  //CONSOLEEXT


static const struct fcname *
fcnfind(const char *name)
{
    struct fcname *fcname;
    unsigned names;

    for (names = 0, fcname = vio.fcnames; fcname->name; ++names, ++fcname) {
        if (0 == _stricmp(name, fcname->name)) {
            return fcname;
        }
    }
    return NULL;
}


static int
fcnpush(const char *name, unsigned flags)
{
    struct fcname *fcname;
    unsigned names;

    for (names = 0, fcname = vio.fcnames; fcname->name; ++names, ++fcname) {
        if (0 == _stricmp(name, fcname->name)) {
            return -1;
        }
    }
    if (names < ((sizeof(vio.fcnames)/sizeof(vio.fcnames[0]))-1)) {
        fcname->name = name;
        fcname->flags = flags;
        return names;
    }
    return -1;
}


static int CALLBACK
enumFontFamExProc(const LOGFONTA *lpelfe, const TEXTMETRICA *unused1, DWORD FontType, LPARAM unused2)
{
    struct fcname *fcname;
    for (fcname = vio.fcnames; fcname->name; ++fcname) {
        if (0 == _stricmp(lpelfe->lfFaceName, fcname->name)) {
            fcname->flags |=
                (TRUETYPE_FONTTYPE == FontType ? FCNTRUETYPE :
                   RASTER_FONTTYPE == FontType ? FCNRASTER :
                   DEVICE_FONTTYPE == FontType ? FCNDEVICE : 0);
            ++fcname->available;
            break;
        }
    }

    return TRUE;                                // next
}



static void
consolefontsenum(void)
{
    LOGFONTA logFont = {0};
    unsigned names = 0;
    HKEY hKey;
    HDC wdc;

    //  Console font list, as seen on console properties
    //
    TRACE_LOG(("Console Fonts Reg\n"))

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Console\\TrueTypeFont",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD rc, cValues = 0;

        if (ERROR_SUCCESS ==
                (rc = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &cValues, NULL, NULL, NULL, NULL))) {
            char valueName[128];
            BYTE data[64];
            DWORD i;

            for (i = 0, rc = ERROR_SUCCESS; i < cValues; ++i) {
                DWORD type, cchValueName = sizeof(valueName),
                    cchData = (sizeof(data) - 1);

                valueName[0] = '\0';
                if (ERROR_SUCCESS ==
                        (rc = RegEnumValueA(hKey, i, valueName, &cchValueName, NULL, &type, data, &cchData))) {
                    data[cchData] = 0;
                    TRACE_LOG(("  %02u: %s=<%s>\n", names, valueName, data))
                    if (REG_SZ == type && '0' == valueName[0]) {
                        //
                        //  0       Lucida Console
                        //  00      Consolas
                        //  9xx     ... others ...
                        //
                        fcnpush(_strdup((const char *)data), 0);
                        ++names;
                    }
                }
            }
        }
        RegCloseKey(hKey);
    }

    if (0 == names) {                           // default, iso-8859-x coverage
        fcnpush("Lucida Console", 0);
    }

    //  Alternatives see:
    //
    //      <charmap.exe> for font details
    //
    //      <http://www.microsoft.com/typography/TrueTypeProperty21.mspx>
    //          Font properties extension, version 2.30, allows
    //
    //      References:
    //          http://www.alanwood.net/unicode
    //          http://www.lowing.org/fonts/
    //              Good lists of available Unicode fonts.
    //          https://github.com/chrissimpkins/codeface
    //              Typefaces for Source Code Beautification.
    //
    //      Fonts:
    //          http://www.proggyfonts.net/
    //          http://www.levien.com/type/myfonts/inconsolata.html
    //          http://terminus-font.sourceforge.net/ and https://files.ax86.net/terminus-ttf/
    //
    fcnpush("Classic Console", FCNUNC);         // clean mon-spaced font, http://webdraft.hu/fonts/classic-console/

    fcnpush("DejaVu Sans Mono", FCNUNC);        // nice mono-spaced font, https://dejavu-fonts.github.io/

    fcnpush("FreeMono", FCNUNC);                // GNU Freefont project (8x16 or better advised)

    fcnpush("Quivira", FCNUNC|FCNPRO2);         // www.quivira-font.com
    fcnpush("TITUS Cyberbit Basic", FCNUNC|FCNPRO2);
    fcnpush("Marin", FCNUNC|FCNPRO2);

    fcnpush("Arial Unicode MS", FCNUNC|FCNPRO5);// 'most complete' standard windows unicode font (Office)

    fcnpush("Courier New", FCNUNC|FCNPRO);      // next most complete font, yet monospaced.

    fcnpush("Consolas", 0);                     // "Consolas Font Pack" for Microsoft Visual Studio.

    fcnpush("Terminal", FCNRASTER);             // implied rastor font.


    //  Determine availability
    //
    logFont.lfCharSet = DEFAULT_CHARSET;
    wdc = GetDC(vio.whandle);
    EnumFontFamiliesExA(wdc, &logFont, enumFontFamExProc, 0, 0);
    ReleaseDC(vio.whandle, wdc);

    //  Trace results
    //
#if defined(DO_TRACE_LOG)
    {   unsigned names = 0;
        struct fcname *fcname;

        TRACE_LOG(("Console Fonts\n"))
        for (fcname = vio.fcnames; fcname->name; ++names, ++fcname) {
            TRACE_LOG(("  [%02u] 0x%04x <%s> %s\n", names,
                fcname->flags, fcname->name, (fcname->available ? " (*)" : "")))
        }
    }
#endif
}


//  References:
//      http://blogs.msdn.com/b/oldnewthing/archive/2007/05/16/2659903.aspx
//      http://support.microsoft.com/kb/247815
//
//  To change the default console font family, apply the following Windows NT / Windows 2000 registry hack.
//
//      Hive:   HKEY_CURRENT_USER
//      Key:    Console
//      Name:   FontFamily
//      Type:   REG_DWORD
//      Value:  0   Don't care, any True Type
//      Value:  10  Roman
//      Value:  20  Swiss
//      Value:  30  Modern
//      Value:  40  Script
//      Value:  50  Decorative
//
static int
consolefontset(int height, int width, const char *facename)
{
    HANDLE whandle = vio.whandle;
    HFONT fnHandle = 0, fiHandle = 0, fbHandle = 0, fHandleOrg = 0;
    int faceindex = 0;
    const struct fcname *fcname = NULL;
    char t_facename[LF_FACESIZE + 1] = {0};
    TEXTMETRIC tm = {0};
    RECT rect = {0};
    HDC wdc;
    SIZE size;

    wdc = GetDC(whandle);
    GetClientRect(whandle, &rect);

    if (-1 == height) {                         // -1, implied
        height = (int)((float)rect.bottom/vio.rows);
    }

    if (-1 == width) {
        width = (int)((float)rect.right/vio.cols);
    }

    // match
    do {
#define WEIGHT_REGULAR  FW_REGULAR
#define WEIGHT_BOLD     FW_MEDIUM

        //  Select by name.
        //
        if (facename && *facename) {
            fcname = fcnfind(facename);
            fnHandle = consolefontcreate(height, width, WEIGHT_REGULAR, FALSE, facename);
            if (fnHandle) {
                fiHandle = consolefontcreate(height, width, WEIGHT_REGULAR, TRUE, facename);
                fbHandle = consolefontcreate(height, width, WEIGHT_BOLD, FALSE, facename);
            }
        }

        //  Select first available.
        //
        if (! fnHandle) {
            while (1) {                         // test availablity
                fcname = vio.fcnames + faceindex;

                if (NULL == (facename = fcname->name)) {
                    break;
                }

                if (fcname->available) {
                    const uint32_t flags = fcname->flags;
                    int t_width = width;

                    if (FCNPRO2 & flags) {      // resize variable width, 2/3
                        t_width = (int)ceil(((float)width / 3) * 2);

                    } else if (FCNPRO5 & flags) { // resize variable width, 45%
                        t_width = (int)ceil(((float)width / 100) * 45);
                    }

                    fnHandle = consolefontcreate(height, t_width, WEIGHT_REGULAR, FALSE, facename);
                    if (fnHandle) {
                        fiHandle = consolefontcreate(height, t_width, WEIGHT_REGULAR, TRUE, facename);
                        fbHandle = consolefontcreate(height, t_width, WEIGHT_BOLD, FALSE, facename);
                        break;
                    }
                }
                ++faceindex;
            }
            facename = NULL;
        }

        if (! fnHandle) {
            TRACE_LOG(("Console Font: <n/a>, width:-1, height:-1\n"))
            return -1;                          // unsuccessful
        }

        // size characters
        t_facename[0] = 0;
        fHandleOrg = SelectObject(wdc, fnHandle);
        GetTextExtentPoint32A(wdc, "[", 1, &size);
        GetTextMetrics(wdc, &tm);
        GetTextFaceA(wdc, sizeof(t_facename), t_facename);
        SelectObject(wdc, fHandleOrg);
        if (vio.fnHandle) {
            if (vio.fiHandle) DeleteObject(vio.fiHandle);
            if (vio.fbHandle) DeleteObject(vio.fbHandle);
            DeleteObject(vio.fnHandle);
        }

        vio.fcwidth  = (int)size.cx;            // tm.tmMaxCharWidth
        vio.fcheight = (int)size.cy;            // tm.tmHeight
        vio.fiHandle = fiHandle;
        vio.fbHandle = fbHandle;
        vio.fnHandle = fnHandle;

#undef  WEIGHT_REGULAR
#undef  WEIGHT_BOLD

        // font glyphs
#if defined(DO_TRACE_LOG)
        {   DWORD dwGlyphSize, r;

            if ((dwGlyphSize = GetFontUnicodeRanges(wdc, NULL)) > 0) {
                GLYPHSET *glyphsets;
                                                // TODO -- not supported mapping
                if (NULL != (glyphsets = calloc(dwGlyphSize, 1))) {
                    glyphsets->cbThis = dwGlyphSize;
                    TRACE_LOG(("Font UNICODE Support\n"))
                    if (GetFontUnicodeRanges(wdc, glyphsets) > 0) {
                        TRACE_LOG(("  flAccel: %u\n", glyphsets->flAccel))
                        TRACE_LOG(("  total:   %u\n", glyphsets->cGlyphsSupported))
                        for (r = 0; r < glyphsets->cRanges; ++r) {
                            TRACE_LOG(("  range:   0x%04x-0x%04x\n", \
                                glyphsets->ranges[r].wcLow, glyphsets->ranges[r].wcLow+glyphsets->ranges[r].cGlyphs))
                        }
                    }
                    free(glyphsets);
                }
            }
        }
#endif  /*DO_TRACE_LOG*/

        break;

    } while (1);

    ReleaseDC(whandle, wdc);

    // apply result
    strncpy(vio.fcfacename, (const char *)t_facename, sizeof(vio.fcfacename));
    if (fcname) {
        vio.fcflags = fcname->flags;
    } else {
        vio.fcflags =
            (0 == stricmp("Terminal", vio.fcfacename) ? FCNRASTER : 0);
    }
    vio.fcheight = height;
    vio.fcwidth  = width;
    vio.acsmap   = (vio.fcflags & FCNRASTER ? acs_oem : acs_unicode);

    TRACE_LOG(("Console Font: <%s>, %dx%d (%s)\n", vio.fcfacename, \
        vio.fcwidth, vio.fcheight, (vio.fcflags & FCNRASTER) ? "raster" : "unicode"))
    return 0;
}


static HFONT
consolefontcreate(int height, int width, int weight, int italic, const char *facename)
{
    const BOOL isTerminal =                     // special terminal/raster support.
        (0 == strcmp(facename, "Terminal"));

    HFONT hFont = CreateFontA(
        height, width,                          // logic (device dependent pixels) height and width
        0, 0, weight,
        (italic ? TRUE : FALSE),                // italic, underline, strikeout
            FALSE,
            FALSE,
        (isTerminal ?
            OEM_CHARSET : ANSI_CHARSET),        // DEFAULT, ANSI, OEM ...
        (isTerminal ?
            OUT_RASTER_PRECIS : OUT_TT_PRECIS),
        CLIP_DEFAULT_PRECIS,                    // default clipping behavior.
        ANTIALIASED_QUALITY,                    // PROOF
        FF_MODERN,                              // DECORATIVE, DONTCARE, MODERN, ROMAN, SCRIPT, SWISS
        facename);

    TRACE_LOG(("Create Font: <%s> %dx%d, Weight:%d, Italic:%d (%u)\n", \
        facename, width, height, weight, italic, (unsigned)hFont))

    return hFont;
}


/*
 *  SLtt_initialize ---
 *      Returns 0 if all goes well, -1 if terminal capabilities cannot be deduced,
 *      or -2 if terminal cannot position the cursor.
 */
int
SLtt_initialize(const char *term)
{
    (void) term;
    SLsmg_init_smg();
    return 0;
}


static int
parse_color(const char *color, const char *defname, const struct attrmap *map, int *attr)
{
    char t_name[128] = {0};
    const char *a;
    int c, col = -1;

    //
    //  color[;attribute[;...]]
    //
    if (!color || !*color) color = defname;      // undefined, apply default

    //  optional trailing attributes
    if (NULL != (a = strchr(color, ';'))) {
        int len = a - color;

        strncpy(t_name, color, sizeof(t_name)-1);// remove attribute component.
        if (len < sizeof(t_name)) t_name[len] = 0;
        color = t_name;

        *attr = parse_attributes(a);
    }

    //  non-optional color
    while (' ' == *color || '\t' == *color) ++color;
	if (0 == sscanf(color, "color%u", &col)) {   // extension
		int defcolor = -1;

		if (!defname || !*defname) 
            defname = "white";

		for (c = 0; map[c].name; ++c) {          // search color map
			if (0 == stricmp(color, map[c].name)) {
				col = c;
				break;     //done
			} else if (0 == stricmp(defname, map[c].name)) {
				defcolor = c;
			}
        }

		if (-1 == col) {   //unknown or default
			col = defcolor;                      // apply default. 
		}
    }

    return col;
}


static int
parse_true_color(const char *color, COLORREF *rgb, int *attr)
{
    struct rgbvalue t_rgb;
    unsigned char v[6];
    int i;

    //
    //  #RRGGBB[;attribute[;...]]
    //
    while (' ' == *color || '\t' == *color) ++color;
    if ('#' != *color++)
        return -1;                              // missing marker

    for (i = 0; i < 6; ++i) {
        const char ch = *color++;
        if (ch >= '0' && ch <= '9') v[i] = ch - '0';
        else if (ch >= 'A' && ch <= 'F') v[i] = 10 + (ch - 'A');
        else if (ch >= 'a' && ch <= 'f') v[i] = 10 + (ch - 'a');
        else return -1;                         // invalid hex digit
    }

    while (' ' == *color || '\t' == *color) ++color;
    if (*color && *color != ';') {
        return -1;                              // invalid terminator
    }

    t_rgb.red   = (v[0] << 4) | v[1];
    t_rgb.green = (v[2] << 4) | v[3];
    t_rgb.blue  = (v[4] << 4) | v[5];

    *rgb  = RGB(t_rgb.red, t_rgb.green, t_rgb.blue);

    *attr = parse_attributes(color);            // optional trailing attribute(s)
    *attr |= VIO_SPECIAL;                       // denote special processing

    return rgb_search(16, &t_rgb);              // return 16-color equiv
}


static int
parse_attributes(const char *attr)
{
    int attributes = 0;

    while (attr) {                              // foreach(attribute)
        while (';' == *attr || ' ' == *attr || '\t' == *attr)
            ++attr;                             // leading whitespace

        if (*attr) {
            if (0 == strncmp(attr, "bold", 4)) {
                attributes |= VIO_BOLD;
            } else if (0 == strncmp(attr, "blink", 5)) {
                attributes |= VIO_BLINK;
            } else if (0 == strncmp(attr, "underline", 9)) {
                attributes |= VIO_UNDERLINE;
            } else if (0 == strncmp(attr, "italic", 6)) {
                attributes |= VIO_ITALIC;
            }
        }
        attr = strchr(attr, ';');               // next
    }
    return attributes;
}


void
SLtt_set_color(int obj, const char *what, const char *fg, const char *bg)
{
    COLORREF brgb = 0, frgb = 0;
    int bcolor = 0, fcolor = 0;
    int battr = 0, fattr = 0;
    unsigned flags = 0;

    (void) what;                                // TODO

    if (obj < 0 || obj >= MAXCOLORS) {
        assert(0);
        return;
    }

    if (SLtt_True_Color &&
            (fcolor = parse_true_color(fg, &frgb, &fattr)) >= 0) {
        flags |= COLOR256_FOREGROUND_TRUECOLOR;
    } else {
        fcolor = parse_color(fg, "lightgray", win16_foreground, &fattr);
    }

    if (SLtt_True_Color &&
            (bcolor = parse_true_color(bg, &brgb, &battr)) >= 0) {
        flags |= COLOR256_BACKGROUND_TRUECOLOR;
    } else {
        bcolor = parse_color(bg, "black", win16_background, &battr);
    }

    vio.c_colors[obj].fg = fcolor & 0xf;
    vio.c_colors[obj].bg = bcolor & 0xf;
    vio.c_colors256[obj].flags  = flags;
    vio.c_colors256[obj].fg     = fcolor;
    vio.c_colors256[obj].bg     = bcolor;
    vio.c_colors256[obj].fg_rgb = frgb;
    vio.c_colors256[obj].bg_rgb = brgb;
    vio.c_attrs[obj] = fattr|battr;

    vio.activecolors = 16;
    if (vio.maxcolors > 16) {
        for (obj = 0; obj < MAXCOLORS; ++obj) {
            if ((vio.c_colors256[obj].flags &   // true-color or >16 color
                    (COLOR256_FOREGROUND_TRUECOLOR|COLOR256_BACKGROUND_TRUECOLOR)) ||                                                
                (vio.c_colors256[obj].fg & 0xf0) ||
                    (vio.c_colors256[obj].bg & 0xf0)) {
                vio.activecolors = 256;         // enable 256+ driver
                break;
            }
        }
    }
}


void
SLsmg_togglesize(void)
{
    /*
     *  min/max
     */
    if (!vio.inited) return;

    if (SLtt_Maximised <= 0) {
        if (-1 == SLtt_Maximised) {
            SLtt_OldScreen_Rows = SLtt_Screen_Rows;
            SLtt_OldScreen_Cols = SLtt_Screen_Cols;
        }
        vio_setsize(0xffff, 0xffff);
        SLtt_Maximised = 1;

    } else {
        vio_setsize(SLtt_OldScreen_Rows, SLtt_OldScreen_Cols);
        SLtt_Maximised = 0;
    }

    /*
     *  reinitialise
     */
    if (1 == SLtt_Maximised) {
        HWND hWnd = vio.whandle;
        RECT r;

        GetWindowRect(hWnd, &r);
        ShowWindow(hWnd, SW_MAXIMIZE);
        MoveWindow(hWnd, 0, 0, r.right, r.bottom, TRUE);
    } else {
        ShowWindow(vio.whandle, /*SW_RESTORE*/ SW_NORMAL);
    }
    vio_init();
    vio.c_trashed = 1;
}


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


void
SLtt_add_color_attribute(int obj, SLtt_Char_Type attr)
{
    WORD nattr = 0;

    if (obj < 0 || obj >= MAXCOLORS) {
        assert(0);
        return;
    }
    if (attr & SLTT_BOLD_MASK)   nattr |= VIO_BOLD;
    if (attr & SLTT_BLINK_MASK)  nattr |= VIO_BLINK;
    if (attr & SLTT_ULINE_MASK)  nattr |= VIO_UNDERLINE;
    if (attr & SLTT_REV_MASK)    nattr |= VIO_REVERSE;
    if (attr & SLTT_ITALIC_MASK) nattr |= VIO_ITALIC;
    if (attr & SLTT_ALTC_MASK)   nattr |= VIO_ALTCHAR;
    vio.c_attrs[obj] = nattr;
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
        if (0 == strcmp(key, "Co")) {           // colors
			if (vio.inited)
                 return vio.maxcolors;          // 16 or 256
			return WIN32_COLORS;                // assumeed support
        }
    }
    return -1;
}


char *
SLtt_tigetent(const char *key)
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
    assert(color < MAXCOLORS);
    vio.c_color = color;
}


void
SLsmg_normal_video(void)
{
    vio.c_color = 0;
}


static uint32_t
acs_lookup(uint32_t ch)
{
    if (ch <= 127) {                            // OEM or UNICODE selection required
        assert(vio.acsmap);
        ch = vio.acsmap[ ch ] | ISACS;
    }
    return ch;
}


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


static uint32_t
unicode_map(uint32_t ch)
{
    if (ch > 0x7f) {
        //
        //  OEM mapping
        //
        if (ISACS & ch) {
            ch &= ~ISACS;       // already mapped

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


static __inline void
CHAR_BUILD(uint32_t ch, int color, CHAR_INFO *ci)
{
    ci->Attributes =
        (color < 0 || color >= MAXCOLORS) ?
            (VIO_SPECIAL|0xff) : (VIO_SPECIAL|(color & 0xff));
    ci->Char.UnicodeChar = (WCHAR)unicode_map(ch);
}


static __inline BOOL
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
    cend    = cursor + vio.cols;
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
    if (vio.c_row >= vio.rows) {
        cursor = cend = NULL;
    } else {
        cursor = vio.c_screen[vio.c_row].text;
        cend = cursor + vio.cols;
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

                if (SLsmg_Display_Alt_Chars) cooked = acs_lookup(cooked);
                if (CHAR_UPDATE(cursor, cooked, color)) {
                    flags |= TOUCHED;
                }
            } else {
                if (SLsmg_Display_Alt_Chars) ch = (unsigned char)acs_lookup(ch);
                if (CHAR_UPDATE(cursor, ch, color)) {
                    flags |= TOUCHED;
                }
            }

            ++cursor;
            ++col;

        } else if ((ch == '\t') && (SLsmg_Tab_Width > 0)) {
            int nexttab = SLsmg_Tab_Width - ((col + SLsmg_Tab_Width) % SLsmg_Tab_Width);
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
        ch = acs_lookup(ch);
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

        if (1 == cliptoarena (row, (int) n, 0, vio.rows, &r1, &r2)) {
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
    write_char(acs_lookup(object), 1);
}


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
SLsmg_fill_region (int r, int c, unsigned nr, unsigned nc, SLwchar_Type ch)
{
    int rmin, rmax, cmin, cmax;

    if (1 == cliptoarena(r, nr, 0, vio.rows, &rmin, &rmax) &&
            1 == cliptoarena(c, nc, 0, vio.cols, &cmin, &cmax)) {

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