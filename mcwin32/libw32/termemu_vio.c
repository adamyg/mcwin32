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

/*
 * Notes:
 *   o 256 color mode available under both Legacy and Win10 enhanced console.
 *   o Use of non-monospaced fonts are not advised unless UNICODE characters are required.
 *   o Neither wide nor combined characters are fully addressed.
 */

#if defined(HAVE_CONFIG_H)
#include "w32config.h"
#endif

#if !defined(WINDOWS_MEAN_AND_LEAN)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#elif (_WIN32_WINNT < 0x601)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif
#undef WINVER
#define WINVER _WIN32_WINNT

#if !defined(WINDOWS_MEAN_AND_LEAN)
#define WINDOWS_MEAN_AND_LEAN
#define PSAPI_VERSION               1           // EnumProcessModules and psapi.dll
#include <windows.h>
#endif
#include <psapi.h>
#endif

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>

#define TERMEMU_VIO_SOURCE                      /* source level interface */
#include "termemu_vio.h"                        /* public interface */

#if defined(_DEBUG) && !defined(DO_TRACE_LOG)
#define DO_TRACE_LOG
#endif
#define WIN32_CONSOLEEXT                        /* extended console */
#define WIN32_CONSOLE256                        /* enable 256 color console support */

#if defined(__WATCOMC__)
#pragma disable_message(124)                    /* Comparison result always 0 / 1 */
#endif

#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "Psapi.lib")

#if defined(WIN32_CONSOLEEXT) && defined(WIN32_CONSOLE256)
#define WIN32_COLORS                256
#else
#undef  WIN32_CONSOLE256
#define WIN32_COLORS                16
#endif

#if !defined(TRACE_LOG)
#if defined(DO_TRACE_LOG)
static void                         vio_trace(const char *, ...);
#define TRACE_LOG(__x)              vio_trace __x;
#else
#define TRACE_LOG(__x)
#endif //DO_TRACE_LOG
#endif //TRACE_LOG

#define MAXCOLORS                   256
#define MAXOBJECTS                  256
#define MASKOBJECTS                 0xff

#ifndef CP_UTF8
#define CP_UTF8                     65001       /* UTF-8 translation */
#endif

#define ISACS                       0x1000000   /* ACS mapped character */

#if (defined(_MSC_VER) && (_MSC_VER < 1500)) && \
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

static const uint32_t   acs_characters[128] = { /* alternative character map to Unicode */

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

struct rgb {                                    // Red/Green/Blue color reference.
    unsigned char       red;
    unsigned char       green;
    unsigned char       blue;
};

struct sline {
    unsigned            flags;
    WCHAR_INFO *        text;
};

typedef struct copyoutctx {
    int                 active;
    int                 cursormode;
    int                 codepagemode;           // code-page to restore.
    HDC                 devicecontext;          // active device context handle.
    CONSOLE_CURSOR_INFO cursorinfo;             // current cursor state.
    RECT                devicerect;             // current display area.
    UINT                codepage;               // current code page.
}  copyoutctx_t;

#define SWAPFGBG(__f, __b) \
        { int t_color = __f; __f = __b; __b = t_color; }

#define SWAPRGB(__f, __b) \
        { COLORREF t_color = __f; __f = __b; __b = t_color; }

static int              vio_init(void);
static COORD            vio_size(HANDLE console, int *rows, int *cols);
static void             vio_profile(int rebuild);
static BOOL             vio_setsize(int rows, int cols);
static void             vio_reset(void);
static void             vio_setcursor(int col, int row);
static int              rgb_search(const int maxval, const COLORREF rgb);

static BOOL             IsVirtualConsole(int *depth);
static BOOL             IsConsole2(void);

static uint32_t         unicode_remap(uint32_t ch);

static __inline void    WCHAR_BUILD(const uint32_t ch, const struct WCHAR_COLORINFO *color, WCHAR_INFO *ci);
static __inline BOOL    WCHAR_COMPARE(const WCHAR_INFO *c1, const WCHAR_INFO *c2);
static __inline unsigned WCHAR_UPDATE(WCHAR_INFO *cursor, const uint32_t ch, const struct WCHAR_COLORINFO *color);

struct attrmap;
static int              parse_color(const char *color, const char *defname, const struct attrmap *map, int *attr);
static int              parse_true_color(const char *color, COLORREF *rgb, int *attr);
static int              parse_attributes(const char *attr);
static void             check_activecolors(void);
static __inline int     winnormal(const int color);
static __inline int     vtnormal(const int color);

static void             CopyIn(unsigned pos, unsigned cnt, WCHAR_INFO *image);
static void             CopyOut(copyoutctx_t *ctx, unsigned offset, unsigned len, unsigned flags);
static void             CopyOutLegacy(copyoutctx_t* ctx, unsigned pos, unsigned cnt, unsigned flags);
#if defined(WIN32_CONSOLEEXT)
#if defined(WIN32_CONSOLE256)
static void             CopyOutEx(copyoutctx_t *ctx, unsigned pos, unsigned cnt, unsigned flags);
#define WIN32_CONSOLEVIRTUAL
#if defined(WIN32_CONSOLEVIRTUAL)
static void             CopyOutVirtual(copyoutctx_t *ctx, size_t pos, size_t cnt, unsigned flags);
#endif  //WIN32_CONSOLEVIRTUAL
#endif  //WIN32_CONSOLE256
static void             UnderOutEx(copyoutctx_t *ctx, unsigned pos, unsigned cnt);
static void             StrikeOutEx(copyoutctx_t *ctx, unsigned pos, unsigned cnt);
#endif  //WIN32_CONSOLEEXT

#if defined(WIN32_CONSOLE256)
static int              consolefontset(int height, int width, const char *facename);
static void             consolefontsenum(void);
static HFONT            consolefontcreate(int height, int width, int weight, int italic, const char *facename);
#endif

static void             ImageSave(HANDLE console, unsigned pos, unsigned cnt);
static void             ImageRestore(HANDLE console, unsigned pos, unsigned cnt);

#define FACENAME_MAX    64
#define FONTS_MAX       64

typedef DWORD  (WINAPI *GetNumberOfConsoleFonts_t)(void);
typedef BOOL   (WINAPI *GetConsoleFontInfo_t)(HANDLE, BOOL, DWORD, CONSOLE_FONT_INFO *);
typedef BOOL   (WINAPI *GetConsoleFontInfoEx_t)(HANDLE, BOOL, DWORD, CONSOLE_FONT_INFOEX *);
typedef BOOL   (WINAPI *GetCurrentConsoleFontEx_t)(HANDLE, BOOL, CONSOLE_FONT_INFOEX *);
typedef BOOL   (WINAPI *SetConsoleFont_t)(HANDLE, DWORD);
typedef BOOL   (WINAPI *SetCurrentConsoleFontEx_t)(HANDLE, BOOL, CONSOLE_FONT_INFOEX *);
typedef BOOL   (WINAPI *GetConsoleScreenBufferInfoEx_t)(HANDLE, CONSOLE_SCREEN_BUFFER_INFOEX *);

static struct {
    CHAR_INFO *         image;
    CONSOLE_CURSOR_INFO cursorinfo;
    COORD               cursorcoord;
    int                 rows;
    int                 cols;
} vio_state;

static struct {                                 /* Video state */
    //  Resource handles
    //
    BOOL                clocal;                 // Local console handle.
    HANDLE              chandle;                // Console handle.
    HANDLE              whandle;                // Underlying window handle.
    HFONT               fnHandle;               // Current normal font.
    HFONT               fbHandle;               // Current bold font.
    HFONT               fiHandle;               // Current italic font.
    HFONT               fbiHandle;              // Current bold + italic font.
    HFONT               ffHandle;               // Current faint font.

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
    int                 envtest;                /* One-shot environment tests */
    int                 notruecolor;            /* Disable true-color decoding */
    int                 isvirtualconsole;       /* Virtual console mode (1=detected, 2=enabled. 3=enabled+restore) */
    DWORD               oldConsoleMode;         /* Previous console mode */
    unsigned            oldConsoleCP;

    int                 maximised;              /* Maximised status */
    int                 maximised_oldrows;
    int                 maximised_oldcols;

    CONSOLE_CURSOR_INFO cinfo;
    COORD               ccoord;

    int                 displaymode;            /* Display mode (0=Normal, 1=Full) */
    int                 rows, cols;             /* Screen size */
    int                 wcols, wrows;           /* Physical window size */

    ULONG               size;                   /* Screen buffer size, in character cells */
    WCHAR_INFO *        image;                  /* Screen working image */
    WCHAR_INFO *        oimage;                 /* Screen output image; double buffered image */
    CHAR_INFO *         oshadow;                /* Black & white/16 color shadow image */
    CHAR_INFO *         iimage;                 /* Temporary working screen image */
    unsigned            codepage;               /* Font code page */
    unsigned            maxcolors;              /* Maximum colors supported (16 or 256) */
    unsigned            activecolors;           /* Active colors (16 or 256) */

    struct WCHAR_COLORINFO c_color;             /* Current color 'attribute' */
    struct WCHAR_COLORINFO c_attrs[MAXOBJECTS]; /* Attribute objects */
    const char *        c_names[MAXOBJECTS];    /* Object names */

    int                 console_rgb16;          /* RGB16 is available */
    COLORREF            rgb16[16];              /* Console's color settings */

    COLORREF            rgb256[256+2];          /* 256-color (plus fg/bg) to RGB color map */
    BYTE                color256to16[256+2];    /* 256-color to win-16-color map */
#define COLIDX_FOREGROUND   256
#define COLIDX_BACKGROUND   257

    int                 c_state;                /* Cursor state */
    DWORD               c_size;                 /* Cursor size/mask */
    int                 c_attr;                 /* Cursor attribute */
    int                 c_row, c_col;           /* Cursor row/col */

#define TOUCHED             0x01
#define TRASHED             0x02
    unsigned            c_trashed;              /* Trashed signal */
    struct sline        c_screen[VIO_MAXROWS];  /* Screen lines */
} vio;

static const struct rgb rgb_colors256[256] = {
#include "w32_colors256.h"
    };

static const BYTE       win2vt[16] = {
    /*
     *  map windows-console to its vt/xterm/SLMSG color.
     */                               //              IRGB
    VT_COLOR_BLACK,                   // BLACK        0000
    VT_COLOR_BLUE,                    // BLUE         0001
    VT_COLOR_GREEN,                   // GREEN        0010
    VT_COLOR_CYAN,                    // AQUA         0011
    VT_COLOR_RED,                     // RED          0100
    VT_COLOR_MAGENTA,                 // PURPLE       0101
    VT_COLOR_YELLOW,                  // BROWN        0110
    VT_COLOR_LIGHT_GREY,              // WHITE        0111
    VT_COLOR_DARK_GREY,               // GRAY         1000
    VT_COLOR_LIGHT_BLUE,              // BRIGHTBLUE   1001
    VT_COLOR_LIGHT_GREEN,             // BRIGHTGREEN  1010
    VT_COLOR_LIGHT_CYAN,              // BRIGHTAQUA   1011
    VT_COLOR_LIGHT_RED,               // BRIGHTRED    1100
    VT_COLOR_LIGHT_MAGENTA,           // BRIGHTPURPLE 1101
    VT_COLOR_LIGHT_YELLOW,            // BRIGHTBROWN  1110
    VT_COLOR_WHITE                    // BRIGHTWHITE  1111
    };

static const BYTE       vt2win[16] = {
    /*
     *  map vt/xterm to its windows-console color.
     */
    WIN_COLOR_BLACK,
    WIN_COLOR_RED,
    WIN_COLOR_GREEN,
    WIN_COLOR_BROWN,
    WIN_COLOR_BLUE,
    WIN_COLOR_PURPLE,
    WIN_COLOR_AQUA,
    WIN_COLOR_LIGHTGREY,
    WIN_COLOR_DARKGREY,
    WIN_COLOR_BRIGHTRED,
    WIN_COLOR_BRIGHTGREEN,
    WIN_COLOR_BRIGHTBROWN,
    WIN_COLOR_BRIGHTBLUE,
    WIN_COLOR_BRIGHTPURPLE,
    WIN_COLOR_BRIGHTAQUA,
    WIN_COLOR_WHITE
    };

struct attrmap {
    const char *        name;
    WORD                win;
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

#if (0)
static const struct localecodepage {
    /*
     *  Code Page Bitfields; see FONTSIGNATURE and LOCALESIGNATURE structures.
     */
    unsigned    bit;
    unsigned    codepage;
    const char *description;

} localecodepages[] = {
    { 0,    1252,       "Latin 1" },
    { 1,    1250,       "Latin 2: Central Europe" },
    { 2,    1251,       "Cyrillic" },
    { 3,    1253,       "Greek" },
    { 4,    1254,       "Turkish" },
    { 5,    1255,       "Hebrew" },
    { 6,    1256,       "Arabic" },
    { 7,    1257,       "Baltic" },
    { 8,    1258,       "Vietnamese" },
        // 9 - 15   Reserved for ANSI.
    { 16,   874,        "Thai" },
    { 17,   932,        "Japanese, Shift-JIS" },
    { 18,   936,        "Simplified Chinese (PRC, Singapore)" },
    { 19,   949,        "Korean Unified Hangul Code (Hangul TongHabHyung Code)" },
    { 20,   950,        "Traditional Chinese (Taiwan; Hong Kong SAR, PRC)" },
    { 21,   1361,       "Korean (Johab)" },
        // 22 - 29  Reserved for alternate ANSI and OEM.
        // 30 - 31  Reserved by system.
        // 32 - 46  Reserved for OEM.
    { 47,   1258,       "Vietnamese" },
    { 48,   869,        "Modern Greek" },
    { 49,   866,        "Russian" },
    { 50,   865,        "Nordic" },
    { 51,   864,        "Arabic" },
    { 52,   863,        "Canadian French" },
    { 53,   862,        "Hebrew" },
    { 54,   861,        "Icelandic" },
    { 55,   860,        "Portuguese" },
    { 56,   857,        "Turkish" },
    { 57,   855,        "Cyrillic; primarily Russian" },
    { 58,   852,        "Latin 2" },
    { 59,   775,        "Baltic" },
    { 60,   737,        "Greek; formerly 437G" },
    { 61,   708,        "Arabic; ASMO 708" },
    { 61,   720,        "Arabic; ASMO 708" },
    { 62,   850,        "Multilingual Latin 1" },
    { 63,   437,        "US" },
    };
#endif


static void
vio_trace(const char *fmt, ...)
{
#if defined(_DEBUG)
    char debug[1024];
    int len1, len2, len;
    va_list ap;

    va_start(ap, fmt);
    len1 = _snprintf(debug, sizeof(debug), "%lu: vio:", (unsigned long)GetTickCount());
    len2 = _vsnprintf(debug + len1, (sizeof(debug) - 2) - len1, fmt, ap);
    if (len2 < 0 /*msvc overflow*/ || (len = len1 + len2) >= sizeof(debug) - 2)
        len = sizeof(debug)-2;
    if (debug[len-1] != '\n') debug[len] = '\n', debug[len+1] = 0;
    OutputDebugStringA(debug);
    va_end(ap);
#else   //_DEBUG
    (void)fmt;
#endif  //_DEBUG
}


static int
vio_init(void)
{
    CONSOLE_SCREEN_BUFFER_INFO sbi = {0};
    HANDLE chandle = GetStdHandle(STD_OUTPUT_HANDLE);
    unsigned fontprofile = 0;
    int rows = 0, cols = 0;
    RECT rect = {0};
    int ret = 0;

    assert((int)WIN_COLOR_FOREGROUND == (int)VT_COLOR_FOREGROUND);
    assert((int)WIN_COLOR_BACKGROUND == (int)VT_COLOR_BACKGROUND);

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
        vio.chandle = chandle;
        vio.c_state = -1;
        ++fontprofile;                          // new handle.
    }

    //  Screen sizing
    //
    vio_size(vio.chandle, &rows, &cols);        // buffer size.

    if (fontprofile || vio.cols != cols || vio.rows != rows) {
        const WCHAR_INFO *oimage;
        int l;

        if (fontprofile) {                      // initial console.
            const struct rgb *rgb = rgb_colors256;
            int color;

            vio.maxcolors = WIN32_COLORS;       // 16 or 256
            for (color = 0; color < 256; ++color, ++rgb) {
                const COLORREF ref = RGB(rgb->red, rgb->green, rgb->blue);

                vio.rgb256[color] = ref;        // BBGGRR, default colour table
                vio.color256to16[color] = vt2win[rgb_search(16, ref)];
            }
            vio.whandle = GetConsoleWindow();   // underlying console window handle
            ++fontprofile;
        }

        assert(vio.chandle);
        assert(vio.whandle);

        vio.size = rows * cols;                 // buffer size, in elements
        oimage = vio.image;
        vio.image = (WCHAR_INFO *)calloc(vio.size, sizeof(WCHAR_INFO));
        if (oimage) {                           // screen has resized
            const WCHAR_INFO wblank = {{0, FOREGROUND_INTENSITY}, ' '};
            const int screencols = vio.cols;
            const int cnt =
                (cols > screencols ? screencols : cols) * sizeof(WCHAR_INFO);
            int r, c;

            for (r = 0; r < rows; ++r) {
                if (r < vio.rows) {             // copy old image
                    memcpy(vio.image + (r*cols), oimage + (r*screencols), cnt);
                }
                                                // blank new cells
                if ((c = (r >= vio.rows ? 0 : screencols)) < cols) {
                    WCHAR_INFO *p = vio.image + (r*cols) + c;
                    do {
                        *p++ = wblank;
                    } while (++c < cols);
                }
            }
            free((void *)oimage);
        }

        free(vio.oimage);
        vio.oimage  = (WCHAR_INFO *)calloc(vio.size, sizeof(WCHAR_INFO));
        free(vio.oshadow);
        vio.oshadow = (CHAR_INFO *)calloc(vio.size, sizeof(CHAR_INFO));
        free(vio.iimage);
        vio.iimage  = (CHAR_INFO *)calloc(vio.size, sizeof(CHAR_INFO));

        vio.c_trashed = 1;
        vio.rows = rows;
        vio.cols = cols;

        vio_goto(vio.c_row, vio.c_col);         // limit current cursor

        for (l = 0; l < rows; ++l) {
            vio.c_screen[l].flags = 0;
            vio.c_screen[l].text = vio.image + (l * cols);
        }

        if (NULL == oimage) {
            CopyIn(0, vio.size, vio.image);     // populate image
        }

        (void) GetConsoleCursorInfo(vio.chandle, &vio.cinfo);
        vio.ccoord = sbi.dwCursorPosition;

        if (! fontprofile) {
            vio_profile(FALSE);                 // font profile
        }

        ret = 1;                                // change detected
    }

    //  Physical window size
    //
    GetWindowRect(GetConsoleWindow(), &rect);
    if (vio.wcols != (rect.right - rect.left) ||
                vio.wrows != (rect.bottom - rect.top)) {
        vio.wcols = (rect.right - rect.left);
        vio.wrows = (rect.bottom - rect.top);
        ++fontprofile;
        ret = 1;                                // change detected
    }

    //  Possible font change
    //
    if (fontprofile) {
        vio_profile(TRUE);
    }

    return ret;     //1=change-detected|0=no-change.
}


static COORD
vio_size(HANDLE console, int *rows, int *cols)
{
    const COORD home = {0, 0};

    if (console) {
        CONSOLE_SCREEN_BUFFER_INFO sbi = {0};

        if (0 == vio.cols && vio.whandle) {
            vio_profile(FALSE);
        }

        if (GetConsoleScreenBufferInfo(console, &sbi)) {
            int nrows, ncols;

            nrows = (sbi.srWindow.Bottom - sbi.srWindow.Top) + 1;
            ncols = (sbi.srWindow.Right - sbi.srWindow.Left) + 1;

            if (nrows < VIO_MINROWS) {
                nrows = VIO_MINROWS;
            } else if (nrows > VIO_MAXROWS) {
                nrows = VIO_MAXROWS;
            }

            if (ncols < VIO_MINCOLS) {
                ncols = VIO_MINCOLS;            // Note: shouldn't occur, console limits ~6 to accommodate def buttons.
            } else if (ncols > VIO_MAXCOLS) {
                ncols = VIO_MAXCOLS;
            }

            *rows = nrows;
            *cols = ncols;

            return sbi.dwCursorPosition;
        }

        *rows = 0;
        *cols = 0;

    } else {
        *rows = 25;
        *cols = 80;
    }

    return home;
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
    if (! vio.envtest) {                        // one-shot
        int depth = 0;

        if (GetModuleHandleA("ConEmuHk.dll") ||
                GetModuleHandleA("ConEmuHk64.dll")) {
            printf("Running under ConEmu, disabling 256 support\n");
            vio.maxcolors = 16;

        } else if (IsVirtualConsole(&depth)) {
#if defined(WIN32_CONSOLEVIRTUAL)
            if (depth > 16) {
                printf("Running under a virtual console, enabling 256/true-color support\n");
                vio.isvirtualconsole = 1;
                vio.maxcolors = 256;
            } else {
                printf("Running under a preliminary virtual console, disabling 256 support\n");
                vio.maxcolors = 16;
            }
#else
            printf("Running under a virtual console, disabling 256 support\n");
            vio.maxcolors = 16;
#endif  //WIN32_CONSOLEVIRTUAL

        } else if (IsConsole2()) {
            printf("Running under Console2, disabling 256 support\n");
            vio.maxcolors = 16;

        }
        vio.envtest = 1;
    }
#endif  //WIN32_CONSOLEEXT && WIN32_CONSOLE256

    //  Undocumented/dynamic function bindings
    //
    if (! vio.dynamic) {
        HMODULE hMod;

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
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
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif
        vio.dynamic = TRUE;
    }

    //  SetConsoleOutputCP()
    //      UTF7                = 0xfde8    65000
    //      UTF8                = 0xfde9    65001
    //      UTF16               = 0x4b0     1200
    //      UTF16_BIGENDIAN     = 0x4b1     1201
    //      UTF32               =           12000
    //      UTF32_BIGENDIAN     =           12001
    //
    //  EnumSystemCodePages()
    //      test for support
    //
    if (0 == (vio.codepage = GetConsoleOutputCP())) {
        vio.codepage = 437;
    }

#if (0)
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
#endif  //XXX

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
                vio.rgb16[c] = csbix.ColorTable[c];
            }
            vio.console_rgb16 = 1;              // RGB16 is available
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
                vio.fcfamily  = -1;
                wcstombs(vio.fcfacename, cfix.FaceName, sizeof(vio.fcfacename) /*bytes*/);
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
                (vio.fcflags & FCNRASTER) ? "Raster" : "Unicode"))

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

            if (count) {
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
                    wcstombs(t_facename, cursor->FaceName, sizeof(t_facename) /*bytes*/);
                    TRACE_LOG(("  %2d: %2u x %2u, %4u, %4u, <%s>\n", (int)cursor->nFont, \
                        (unsigned)cursor->dwFontSize.X, (unsigned)cursor->dwFontSize.Y, \
                        (unsigned)cursor->FontFamily, (unsigned)cursor->FontWeight, t_facename))
                }
#endif  //DO_TRACE_LOG
            }
        }
    }
}


static DWORD        // move to w32_process.c
w32_GetParentProcessId()
{
    typedef struct _PROCESS_BASIC_INFORMATION { // undocumented
        LONG_PTR    ExitStatus;
        LONG_PTR    PebBaseAddress;
        LONG_PTR    AffinityMask;
        LONG_PTR    BasePriority;
        ULONG_PTR   UniqueProcessId;
        LONG_PTR    InheritedFromUniqueProcessId;
    } PROCESS_BASIC_INFORMATION;

    typedef LONG (WINAPI *NtQueryInformationProcess_t)(HANDLE ProcessHandle, ULONG ProcessInformationClass,
                        PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    NtQueryInformationProcess_t NtQueryInformationProcess =
            (NtQueryInformationProcess_t)GetProcAddress(LoadLibraryA("NTDLL.DLL"), "NtQueryInformationProcess");
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif

    if (NtQueryInformationProcess) {
        PROCESS_BASIC_INFORMATION pbi = {0};
        ULONG ulSize = 0;

        if (NtQueryInformationProcess(GetCurrentProcess(), 0, &pbi, sizeof(pbi), &ulSize) >= 0 && ulSize == sizeof(pbi)) {
            return (DWORD)pbi.InheritedFromUniqueProcessId;
        }
    }
    return (DWORD)-1;
}


#if defined(__MINGW32__)
typedef struct _OSVERSIONINFOW RTL_OSVERSIONINFOW, *PRTL_OSVERSIONINFOW;
typedef DWORD (WINAPI *fnRtlGetVersion_t)(PRTL_OSVERSIONINFOW);

#elif defined(__WATCOMC__)
typedef struct _OSVERSIONINFOW RTL_OSVERSIONINFOW, *PRTL_OSVERSIONINFOW;
#endif

#if !defined(__MINGW32__)
typedef NTSTATUS (WINAPI *fnRtlGetVersion_t)(PRTL_OSVERSIONINFOW);
#endif

DWORD WINAPI GetModuleFileNameExA(HANDLE hProcess,HMODULE hModule,LPSTR lpFilename,DWORD nSize);
BOOL WINAPI EnumProcessModules(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded);


static BOOL
w32_RtlGetVersion(RTL_OSVERSIONINFOW *rovi)
{
#define STATUS_SUCCESS  (0x00000000)

    HMODULE hMod = GetModuleHandleA("ntdll.dll");
    if (hMod) {
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
        fnRtlGetVersion_t cb = (fnRtlGetVersion_t)GetProcAddress(hMod, "RtlGetVersion");
        if (cb) {
            rovi->dwOSVersionInfoSize = sizeof(*rovi);
            if (STATUS_SUCCESS == cb(rovi)) {
                return TRUE;
            }
        }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif
    }
    memset(rovi, 0, sizeof(*rovi));
    return FALSE;
}


static BOOL
IsVirtualConsole(int *depth)
{
#if !defined(ENABLE_VIRTUAL_TERMINAL_PROCESSING)
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    // When writing with WriteFile or WriteConsole, characters are parsed for VT100 and similar control
    // character sequences that control cursor movement, color/font mode, and other operations that can
    // also be performed via the existing Console APIs.
#endif
#if !defined(DISABLE_NEWLINE_AUTO_RETURN)
#define DISABLE_NEWLINE_AUTO_RETURN 0x0008
    // When writing with WriteFile or WriteConsole, this adds an additional state to end-of-line wrapping
    // that can delay the cursor move and buffer scroll operations.
#endif
#if !defined(ENABLE_INSERT_MODE)
#define ENABLE_INSERT_MODE 0x0020
    // When enabled, text entered in a console window will be inserted at the current cursor location and
    // all text following that location will not be overwritten.
    // When disabled, all following text will be overwritten.
    // To enable this mode, use ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS.
    // To disable this mode, use ENABLE_EXTENDED_FLAGS without this flag.
#endif
#if !defined(ENABLE_QUICK_EDIT_MODE)
#define ENABLE_QUICK_EDIT_MODE 0x0040
    // This flag enables the user to use the mouse to select and edit text.
    // To enable this mode, use ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS.
    // To disable this mode, use ENABLE_EXTENDED_FLAGS without this flag.
#endif
#if !defined(ENABLE_EXTENDED_FLAGS)
#define ENABLE_EXTENDED_FLAGS 0x0080
    // Required to enable or disable extended flags. See ENABLE_INSERT_MODE and ENABLE_QUICK_EDIT_MODE.
#endif

    RTL_OSVERSIONINFOW rovi = {0};
    HANDLE chandle = vio.chandle;
    DWORD mode;

    //  #include <VersionHelpers.h>
    //  if (! IsWindows10OrGreater()) {
    //        // BOOL WINAPI IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor);
    //     return FALSE;
    //  }

    if (w32_RtlGetVersion(&rovi)) {             // Note: GetVersionEx() only returns the compatibly details.
        const int t_depth =                     // Build 14931+
            (rovi.dwBuildNumber >= 14931 ? 256 : 16);

        if (GetConsoleMode(chandle, &mode)) {   // Windows 10 virtual console test.
            if (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) {
                *depth = t_depth;
                return TRUE;
            } else if (SetConsoleMode(chandle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                (void) SetConsoleMode(chandle, mode);
                *depth = t_depth;
                return TRUE;
            }
        }
    }
    return FALSE;
}


static BOOL
IsConsole2(void)
{
    DWORD  parentID = w32_GetParentProcessId();
    HANDLE hProcess = (parentID != (DWORD) -1 ?
                OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, parentID) : 0);
    BOOL console2 = FALSE;
    unsigned i;

    if (hProcess) {                             // parent process handle.
        HMODULE hMods[1024] = {0};
        char szModName[MAX_PATH];
        DWORD cbNeeded = 0;
                                                // get a list of all the modules in this process.
        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
            for (i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
                const DWORD modLength = GetModuleFileNameExA(hProcess, hMods[i], szModName, sizeof(szModName));
                if (modLength) {
                    if (modLength >= 16 &&      // consolehook.dll; assume Console2 or similar.
                            0 == stricmp(szModName + modLength - 16, "\\consolehook.dll")) {
                        console2 = TRUE;
                        break;
                    }
                }
            }
        }
        CloseHandle(hProcess);
    }
    return console2;
}


static BOOL
vio_setsize(int rows, int cols)
{
    const int orows = vio.rows, ocols = vio.cols;
    HANDLE chandle = vio.chandle;
    SMALL_RECT rect = { 0, 0, 0, 0 };
    COORD msize, nbufsz;
    int bufwin = FALSE;

    msize = GetLargestConsoleWindowSize(chandle);

    if (vio.isvirtualconsole) {
        //
        //  GetLargestConsoleWindowSize() reports an incorrectly scaled value.
        //  It uses the current screen resolution (in pixels) and divides by an invalid font size.
        //
        return FALSE;                           // not-supported
    }

    if (rows <= 0) {
        rows = orows;                           // current
    } else {
        if (rows >= msize.Y) {
            rows = msize.Y - 1;                 // limit
        }
        if (rows >= VIO_MAXROWS) {
            rows = VIO_MAXROWS;
        }
    }

    if (cols <= 0) {
        cols = ocols;                           // current
    } else {
        if (cols >= msize.X) {
            cols = msize.X - 1;                 // limit
        }
        if (cols >= VIO_MAXCOLS) {
            cols = VIO_MAXCOLS;
        }
    }

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

    return TRUE;
}


static void
vio_reset(void)
{
    if (0 == vio.inited) return;                /* uninitialised */

    if (vio.image) {
        free(vio.iimage); vio.iimage = NULL;
        free(vio.oshadow); vio.oshadow = NULL;
        free(vio.oimage); vio.oimage = NULL;
        free(vio.image); vio.image = NULL;
    }
    vio.c_col  = vio.c_row = 0;
    vio.cols   = vio.rows = 0;
    vio.inited = 0;
}


static void
vio_setcursor(int col, int row)
{
    COORD coord;
    coord.X = (SHORT)col;
    coord.Y = (SHORT)row;
    SetConsoleCursorPosition(vio.chandle, coord);
}


static int
rgb_search(const int maxval, const COLORREF rgb)
{
    const int red   = GetRValue(rgb);
    const int green = GetGValue(rgb);
    const int blue  = GetBValue(rgb);

    const struct rgb *table = rgb_colors256;
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


static uint32_t
unicode_remap(uint32_t ch)
{
    if (ch > 0x255) {
        //
        //  Remap characters not available on standard console fonts.
        //    o Small box characters.
        //    o Misc
        //
        switch (ch) {
        case 0x22C5:        // DOT OPERATOR
            ch = 0x00B7;
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
    return ch;
}


static void
CopyIn(unsigned pos, unsigned cnt, WCHAR_INFO *image)
{
    const int /*rows = vio.rows,*/ cols = vio.cols;
    COORD is = {0}, ic = {0};
    SMALL_RECT wr = {0};
    DWORD rc;

    assert(image);
    assert(pos < vio.size);                     /* starting position within window */
    assert((pos + cnt) <= vio.size);            /* end position within window */
    assert(cnt && 0 == (pos % cols));           /* row aligned */

    if (!image) return;                         /* missing destination */

    wr.Left   = 0;                              /* src. screen rectangle */
    wr.Right  = (SHORT)(cols - 1);
    wr.Top    = (SHORT)(pos / cols);
    wr.Bottom = (SHORT)((pos + (cnt - 1)) / cols);

    is.Y      = (SHORT)(vio.rows - wr.Top);     /* size of image */
    is.X      = (SHORT)(vio.cols);

    ic.X      = 0;                              /* top left src cell in image */
    ic.Y      = 0;

    rc = ReadConsoleOutputW(vio.chandle,        /* read in image; temporary input buffer */
            vio.iimage + pos, is, ic, &wr);

    if (0 == rc && ERROR_NOT_ENOUGH_MEMORY == GetLastError()) {
        if (cnt > ((unsigned)cols * 2)) {       /* sub-divide request (max 8k) */
            const int cnt2 = (cnt / (cols * 2)) * cols;

            CopyIn(pos, cnt2, image);
            CopyIn(pos + cnt2, cnt - cnt2, image);
            return;
        }
    }

    /* import native window attributes */
    {   const CHAR_INFO *icursor = vio.iimage + pos;
        WCHAR_INFO *cursor = image + pos, *end = cursor + cnt;

        (void) memset(cursor, 0, sizeof(WCHAR_INFO) * cnt);
        for (; cursor < end; ++cursor, ++icursor) {
            cursor->Info.Flags = 0;
            cursor->Info.Attributes = icursor->Attributes;
            cursor->Char.UnicodeChar = icursor->Char.UnicodeChar;
            cursor->Info.fg = -1;
            cursor->Info.bg = -1;
        }
    }
}


/*  Function;           AttributesShadow
 *      Build a 16-color CHAR_INFO attribute based on the 256-color to 16-color
 *      table; being a native window attribute specification.
 *
 *  Parameters
 *      attributes - Internal WCHAR_INFO cell attributes.
 *
 *  Returns:
 *      16-color CHAR_INF0 attributes.
 **/
static __inline WORD
AttributesShadow(const struct WCHAR_COLORINFO *color)
{
    WORD flags = color->Flags;

    if (flags) {
        if (VIO_FOBJECT & flags) {              // attribute object, redirect
            color = &vio.c_attrs[color->Attributes & MASKOBJECTS];
            flags = color->Flags;
        }

        if (flags) {
            /*
             *  internal attribute,
             *      map the fg/bg 256-color attributes to their 16-color window counterparts.
             */
            WORD fg, bg;

            fg = vio.color256to16[ color->fg ];
            bg = vio.color256to16[ color->bg ];

            if (fg == bg) {                     // default, normal
                return FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
            }

            return (WORD)((fg)|(bg << 4));      // foreground low-4, background high-4
        }
    }

    return (WORD)(color->Attributes & 0xff);    // windows native
}


/*  Function;           CopyOut
 *      Copy out to video memory.
 *
 *  Parameters
 *      pos - Starting offset in video cells from top left.
 *      cnt - Video cell count.
 *      flags - Line management flags.
 *
 *  Returns:
 *      nothing.
 **/
static void
CopyOutInit(copyoutctx_t *ctx)
{
    (void) memset(ctx, 0, sizeof(*ctx));
    ctx->cursormode = -1;
    ctx->codepagemode = -1;
    ctx->active = 42;
}


static void
CopyOutFinal(copyoutctx_t *ctx)
{
    assert(42 == ctx->active);
    if (42 != ctx->active) return;

    if (-1 != ctx->cursormode) {                /* restore cursor. */
        if (0 != (ctx->cursorinfo.bVisible = ctx->cursormode)) {
            (void) SetConsoleCursorInfo(vio.chandle, &ctx->cursorinfo);
        }
    }

    if (-1 != ctx->codepagemode) {              /* restore code page. */
        (void) SetConsoleOutputCP(ctx->codepage);
    }

    if (ctx->devicecontext) {                   /* release device context handle */
        (void) DeleteDC(ctx->devicecontext);
        ctx->devicecontext = 0;
    }

    ctx->active = 0;                            /* complete */
}


static void
CopyOut(copyoutctx_t* ctx, unsigned pos, unsigned cnt, unsigned flags)
{
#if defined(WIN32_CONSOLEVIRTUAL)
    //
    //  windows 10+ virtual console.
    if (vio.isvirtualconsole) {
        CopyOutVirtual(ctx, pos, cnt, flags);
        return;
    }
#endif  //WIN32_CONSOLEVIRTUAL

    //
    //  legacy modes
    CopyOutLegacy(ctx, pos, cnt, flags);
}


static void
CopyOutLegacy(copyoutctx_t *ctx, unsigned pos, unsigned cnt, unsigned flags)
{
    const unsigned activecolors = (vio.displaymode || 0 == vio.activecolors ? 16 : vio.activecolors);
    const int /*rows = vio.rows,*/ cols = vio.cols;
    HANDLE chandle = vio.chandle;
    COORD is = {0}, ic = {0};
    SMALL_RECT wr = {0};
    unsigned underline = 0, strike = 0,         /* special attribute counts */
        mcnt = vio.c_trashed|(flags & TRASHED), /* modified cell count */
        wcnt = 0;                               /* wide-character count */
    WORD attr;

    assert(pos < vio.size);                     /* starting position within window */
    assert((pos + cnt) <= vio.size);            /* end position within window */
    assert(0 == (pos % cols));                  /* row aligned */

    {   const WCHAR_INFO *cursor, *end;         /* build output images */
        CHAR_INFO *oshadow = vio.oshadow + pos;

        for (cursor = vio.image + pos, end = cursor + cnt; cursor < end; ++cursor, ++oshadow) {
            /*
             *  Primary image and shadow image,
             *      shadow is written to console when running 256 colour mode.
             */
            if (cursor->Info.Flags) {           /* extended */
                if (cursor->Info.Attributes & VIO_UNDERLINE) {
                    ++underline;
                }
                if (cursor->Info.Attributes & VIO_STRIKE) {
                    ++strike;
                }
            }

            attr = AttributesShadow(&cursor->Info);
            if (oshadow->Attributes != attr ||
                        oshadow->Char.UnicodeChar != cursor->Char.UnicodeChar) {
                const WCHAR wch = (WCHAR)cursor->Char.UnicodeChar;
                if (0 == (oshadow->Char.UnicodeChar = wch)) {
                    ++wcnt;
                }
                oshadow->Attributes = attr;
                ++mcnt;
            }
        }
    }

    wr.Left   = 0;                              /* src. screen rectangle */
    wr.Right  = (SHORT)(cols - 1);
    wr.Top    = (SHORT)(pos / cols);
    wr.Bottom = (SHORT)((pos + (cnt - 1)) / cols);

    is.Y      = (SHORT)(vio.rows - wr.Top);     /* size of image */
    is.X      = (SHORT)(vio.cols);

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
        if (-1 == ctx->cursormode) {            // hide cursor, if visible
            GetConsoleCursorInfo(chandle, &ctx->cursorinfo);
            if (0 != (ctx->cursormode = ctx->cursorinfo.bVisible)) {
                ctx->cursorinfo.bVisible = FALSE;
                (void) SetConsoleCursorInfo(chandle, &ctx->cursorinfo);
            }
        }

        if (mcnt) {
            const BOOL isvisible = IsWindowVisible(vio.whandle);

            if (isvisible) {                    // flush changes, disable updates
                SendMessage(vio.whandle, WM_SETREDRAW, FALSE, 0);
                WriteConsoleOutputW(chandle, vio.oshadow + pos, is, ic, &wr);
                SendMessage(vio.whandle, WM_SETREDRAW, TRUE, 1);
                    // Note: If the application sends the WM_SETREDRAW message to a hidden window,
                    //  the window becomes visible (that is, the operating system adds the WS_VISIBLE style to the window).

            } else {
                WriteConsoleOutputW(chandle, vio.oshadow + pos, is, ic, &wr);
            }
        }

        CopyOutEx(ctx, pos, cnt, flags);        // export text

    } else {
        if (mcnt) {
            if (wcnt) {
                CHAR_INFO wcbuf[VIO_MAXCOLS], *wcend;

                const CHAR_INFO *cursor = vio.oshadow + pos, *end;
                const SHORT Bottom = wr.Bottom;

                while (wr.Top <= Bottom) {      // export line-by-line
                    SHORT nchrs = 0;

                    for (wcend = wcbuf, end = cursor + cols; cursor < end; ++cursor) {
                        if (cursor->Char.UnicodeChar) {
                            *wcend++ = *cursor; // omit padding
                            ++nchrs;
                        }
                    }

                    wr.Bottom = wr.Top;
                    is.Y = (SHORT)(vio.rows - wr.Top);
                    is.X = nchrs;

                    WriteConsoleOutputW(chandle, wcbuf, is, ic, &wr);

                    ++wr.Top;
                }

            } else {
                WriteConsoleOutputW(chandle, vio.oshadow + pos, is, ic, &wr);
            }
        }
    }
#endif  //CONSOLE256

    if (underline) {
        UnderOutEx(ctx, pos, cnt);              // underline region
    }

    if (strike) {
        StrikeOutEx(ctx, pos, cnt);             // overstrike region
    }

#else   //CONSOLEEXT
    WriteConsoleOutputW(chandle, vio.oshadow + pos, is, ic, &wr);

#endif
}


#if defined(WIN32_CONSOLE256)
/*private*/
/*  Function:           IsSpace
 *      Test whether the stated character is a 'space'.
 **/
static __inline int
IsSpace(const DWORD ch)
{
    // Note: In the Unicode standard, U+200B and U+FEFF are not included in the table of space characters,
    //  as they have no width and are not supposed to have any visible glyph.
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
    case 0x205F:        // MEDIUM MATHEMATICAL SPACE
    case 0x3000:        // IDEOGRAPHIC SPACE
    case 0xFEFF:        // ZERO WIDTH NO - BREAK SPACE
        return 1;
    }
    return 0;
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
 **/
static __inline void
COLOR256(const struct WCHAR_COLORINFO *color, COLORREF *fg, COLORREF *bg)
{
    COLORREF t_fg = (COLORREF)-1, t_bg = (COLORREF)-1;
    WORD flags = color->Flags;

    if (VIO_FOBJECT & flags) {                  // attribute object, redirect
        color = &vio.c_attrs[color->Attributes & MASKOBJECTS];
        flags = color->Flags;
    }

    if (flags) {
        /*
         *  internal attribute,
         *      map the fg/bg 256-color attributes to their 16-color window counterparts.
         */
        assert(color->fg >= 0 && color->fg <= COLIDX_BACKGROUND);
        assert(color->bg >= 0 && color->bg <= COLIDX_BACKGROUND);

        if (VIO_FRGB & flags) {                 // RGB, fg and/or bg
            t_fg = ((COLORREF)-1 != color->fgrgb ?
                        color->fgrgb : vio.rgb256[ color->fg ]);
            t_bg = ((COLORREF)-1 != color->bgrgb ?
                        color->bgrgb : vio.rgb256[ color->bg ]);

        } else {                                // NORMAL|16|256
            t_fg = vio.rgb256[ color->fg ];
            t_bg = vio.rgb256[ color->bg ];
        }

    } else {
        /*
         *  windows native attribute.
         */
        const WORD Attributes = color->Attributes;
        if (vio.console_rgb16) {
            t_fg = vio.rgb16[  Attributes & 0x0f ];
            t_bg = vio.rgb16[ (Attributes & 0xf0) >> 4 ];
        } else {
            t_fg = vio.rgb256[ win2vt[ Attributes & 0x0f] ];
            t_bg = vio.rgb256[ win2vt[(Attributes & 0xf0) >> 4] ];
        }
    }

    if (t_fg == t_bg) {                         // default, normal
        t_fg = vio.rgb256[ COLIDX_FOREGROUND ];
        t_bg = vio.rgb256[ COLIDX_BACKGROUND ];
        if (t_fg == t_bg) {                     // fall-back
            t_bg = vio.rgb256[ VT_COLOR_BLACK ];
            t_fg = vio.rgb256[ VT_COLOR_LIGHT_GREY ];
        }
    }
    *fg = t_fg; *bg = t_bg;
}


static __inline int
SameCell(const WCHAR_INFO *c1, const WCHAR_INFO *c2)
{
    return (c1->Char.UnicodeChar == c2->Char.UnicodeChar &&
        c1->Info.Flags == c2->Info.Flags &&
        c1->Info.Attributes == c2->Info.Attributes &&
        c1->Info.fg == c2->Info.fg &&
        c1->Info.bg == c2->Info.bg &&
        c1->Info.fgrgb == c2->Info.fgrgb &&
        c1->Info.bgrgb == c2->Info.bgrgb);
}


static __inline int
SameAttributesFGBG(const WCHAR_INFO *cell, const struct WCHAR_COLORINFO *info, const COLORREF fg, const COLORREF bg, const WORD viomask)
{
    COLORREF cfg, cbg;

    if ((cell->Info.Attributes & viomask) !=
             (info->Attributes & viomask)) {
        return 0;                               // type-face change.
    }

    COLOR256(&cell->Info, &cfg, &cbg);          // cell colors.
    if (IsSpace(cell->Char.UnicodeChar)) {
         return (cbg == bg);                    // background only.
    }
    return (cfg == fg && cbg == bg);            // foreground and background.
}


static __inline int
SameAttributesBG(const WCHAR_INFO *cell, const struct WCHAR_COLORINFO *info, const COLORREF bg, const WORD viomask)
{
    COLORREF cfg, cbg;

    if ((cell->Info.Attributes & viomask) !=
        (info->Attributes & viomask)) {
        return 0;                               // type-face change.
    }

    COLOR256(&cell->Info, &cfg, &cbg);          // cell colors.
    return (cbg == bg);                         // background only.
}


/*private*/
/*  Function:           CopyOutEx
 *      Extended export characters within the specified region to the console window.
 *      Note: Underline and over-strike attributes are ignored, these are handled separately.
 *
 *  Parameters:
 *      pos - Starting offset in video cells from top left.
 *      cnt - Video cell count.
 *      flags - Line management flags.
 *
 *  Returns:
 *      nothing.
 **/
static void
CopyOutEx(copyoutctx_t *ctx, unsigned pos, unsigned cnt, unsigned flags)
{
    const WCHAR_INFO *cursor = vio.image + pos, *end = cursor + cnt;
    const int rows = vio.rows, cols = vio.cols;
    float fcwidth, fcheight;                    // proportional sizing
    int row = (int)(pos / cols);
    WCHAR wcbuf[1024],                          // ExtTextOut limit 8192
        *wcend = wcbuf + (sizeof(wcbuf)/sizeof(wcbuf[0]));
    HFONT oldfont;
    HDC wdc;

    assert(pos < vio.size);
    assert(0 == (pos % cols));
    assert((pos + cnt) <= vio.size);

    if (0 == (wdc = ctx->devicecontext)) {      // client area DC
        ctx->devicecontext = GetDC(vio.whandle);
        (void) GetClientRect(vio.whandle, &ctx->devicerect);
        wdc = ctx->devicecontext;
    }
    fcwidth = (float)ctx->devicerect.right / cols;
    fcheight = (float)ctx->devicerect.bottom / rows;

    if (! vio.fnHandle) {                       // allocate font handle
        consolefontset(-1, -1, vio.fcfacename);
        if (! vio.fnHandle) {
            return;
        }
    }
    oldfont = SelectObject(wdc, vio.fnHandle);  // base font

    do {    // foreach(row)
        WCHAR_INFO *ocursor = vio.oimage + (row * cols);
        struct WCHAR_COLORINFO info = {0};      // accumulator
        COLORREF bg = (COLORREF)-1, fg = (COLORREF)-1; // current colors
        WCHAR *text = NULL;
        int col = 0;

        while (col < cols) {
            int start = -1;

            do {
                const WCHAR_INFO cell = *cursor++;

                if (0 == cell.Char.UnicodeChar) {
                    ocursor[col++] = cell;      // update out image
                    continue;                   // NUL, padding
                }

                if (start >= 0) {               // attribute run
                    if (SameAttributesFGBG(&cell, &info, fg, bg, VIO_BOLD|VIO_BLINK|VIO_ITALIC|VIO_FAINT|VIO_INVERSE)) {
                        ocursor[col++] = cell;  // update out image
                        *text = (WCHAR)cell.Char.UnicodeChar;
                        if (++text >= wcend)
                            break;              // flush
                        continue;
                    } //else, attribute change
                    --cursor;
                    break;                      // flush
                }

                if (0 == (flags & TRASHED) &&
                        SameCell(&cell, ocursor + col)) {
                    ++col;
                    continue;                   // up-to-date
                }
                ocursor[col] = cell;            // update out image

                // start of new draw arena
                start = col++;
                text  = wcbuf;
                info  = cell.Info;
                COLOR256(&info, &fg, &bg);
                if (info.Attributes & VIO_INVERSE) {
                    SWAPRGB(fg, bg);
                }

                if (start > 0) {                // if previous is space, also redraw; address font cell draw bleeding.
                    const WCHAR_INFO backcell = cursor[-2];

                    if ((IsSpace(cell.Char.UnicodeChar) &&
                            SameAttributesFGBG(&backcell, &info, fg, bg, VIO_BOLD|VIO_BLINK|VIO_ITALIC|VIO_FAINT|VIO_INVERSE)) ||
                        (IsSpace(backcell.Char.UnicodeChar) &&
                            SameAttributesBG(&backcell, &info, bg, VIO_BOLD|VIO_BLINK|VIO_ITALIC|VIO_FAINT|VIO_INVERSE))) {
                        *text++ = (WCHAR)backcell.Char.UnicodeChar;
                        --start;
                    }
                }
                *text++ = (WCHAR)cell.Char.UnicodeChar;

            } while (col < cols);

            if (start >= 0) {                   // write text

                if (vio.fbHandle || vio.fiHandle) {
                    HFONT fhandle = vio.fnHandle;
                    if (((VIO_BOLD | VIO_BLINK) & info.Attributes) && vio.fbHandle) {
                        fhandle = vio.fbHandle;
                        if ((VIO_ITALIC & info.Attributes) && vio.fbiHandle) {
                            fhandle = vio.fbiHandle;
                        }
                    } else if ((VIO_ITALIC & info.Attributes) && vio.fiHandle) {
                        fhandle = vio.fiHandle;
                    } else if ((VIO_FAINT & info.Attributes) && vio.ffHandle) {
                        fhandle = vio.ffHandle;
                    }
                    SelectObject(wdc, fhandle);
                }

//              if (FCNPRO & vio.fcflags) {
                {   //
                    //  Variable width font,
                    //      display character-by-character.
                    //
                    const int left = (int)(fcwidth * start);
                    const int top = (int)(fcheight * row);
                    HPEN oldbrush, oldpen;
                    const WCHAR *otext = wcbuf;
                    unsigned idx = 0;

                    oldbrush = SelectObject(wdc, CreateSolidBrush(bg));
                    oldpen = SelectObject(wdc, CreatePen(PS_SOLID, 0, bg));
                    Rectangle(wdc, left, top, left + (int)(fcwidth * (text - wcbuf)), top + (int)fcheight);
                    oldpen = SelectObject(wdc, oldpen);
                    oldbrush = SelectObject(wdc, oldbrush);
                    DeleteObject(oldpen);
                    DeleteObject(oldbrush);

                    SetBkColor(wdc, bg);
                    SetTextColor(wdc, fg);
                    SetTextAlign(wdc, GetTextAlign(wdc) | TA_CENTER);
                    for (idx = 0; otext < text;) {
                        WCHAR t_ch = *otext++;

                        if (t_ch) {             // omit padding
                            if (! IsSpace(t_ch)) {
                                ExtTextOutW(wdc, left + (int)(fcwidth * (0.5 + idx)), top, ETO_OPAQUE, NULL, &t_ch, 1, NULL);
                            }
                            ++idx;
                        }
                    }

//              } else {
//                  //
//                  //  Fixed width font,
//                  //      in theory safe as a single export; yet not all fonts are equal in their abilities.
//                  //
//                  const int left = vio.fcwidth * start,
//                      top = vio.fcheight * row;
//
//                  SetBkColor(wdc, bg);
//                  SetTextColor(wdc, fg);
//                  SetTextAlign(wdc, GetTextAlign(wdc) | TA_CENTER);
//                  ExtTextOutW(wdc, left, top, ETO_OPAQUE, NULL, textbuf, text - textbuf, NULL);
                }
            }
        }
        ++row;

    } while (cursor < end);

    SelectObject(wdc, oldfont);
}
#endif  //WIN32_CONSOLE256


#if defined(WIN32_CONSOLEVIRTUAL)
/*  Function:           CopyOutVirtual
 *      Virtual console drivers.
 *
 *  Reference:
 *      https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
 */
static void
CopyOutVirtual(copyoutctx_t *ctx, size_t pos, size_t cnt, unsigned flags)
{
    const WCHAR_INFO *cursor = vio.image + pos, *end = cursor + cnt;
    HANDLE chandle = vio.chandle;
    const int cols = vio.cols;
    int row = (int)(pos / cols);

    WCHAR wcbuf[2 * 1024],                      // ExtTextOut limit
        *wcend = wcbuf + ((sizeof(wcbuf) / sizeof(wcbuf[0])) - 128);
    char u8buf[sizeof(wcbuf) * 4];

    assert(pos < vio.size);
    assert(0 == (pos % cols));
    assert((pos + cnt) <= vio.size);
    assert(vio.isvirtualconsole);

    if (1 == vio.isvirtualconsole) {            // enable Windows 10 virtual console
        const UINT cp = GetConsoleOutputCP();
        DWORD mode = 0;

        vio.isvirtualconsole = 2;               // post initialisation state

        if (GetConsoleMode(chandle, &mode)) {
            vio.oldConsoleMode = mode;
            vio.oldConsoleCP = cp;

            //
            // virtual terminal mode
            if (0 == (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                //
                //  enable virtual terminal processing
                mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
                            /*| ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT |*/
                vio.isvirtualconsole = 3;       // update/restore

            } else {
                //
                //  virtual terminal processing active, disable newline
                //  and retain/inherit other settings.
                if (0 == (mode & DISABLE_NEWLINE_AUTO_RETURN)) {
                    mode |= DISABLE_NEWLINE_AUTO_RETURN;
                    vio.isvirtualconsole = 3;   // update/restore
                }
            }

            if (3 == vio.isvirtualconsole) {
                (void) SetConsoleMode(chandle, mode);
            }
        }
    }

#define L_VTESC     L"\x1b"
#define L_VTCSI     L"\x1b["

    do {    // foreach(row)
        WCHAR_INFO *ocursor = vio.oimage + (row * cols);
        struct WCHAR_COLORINFO info = {0};      // accumulator
        COLORREF bg = (COLORREF)-1, fg = (COLORREF)-1; // current colors
        WCHAR *wctext = NULL;
        int col = 0;                            // current column

        while (col < cols) {
            int start = -1;                     // start of change

            do {
                const WCHAR_INFO cell = *cursor++;

                if (0 == cell.Char.UnicodeChar) {
                    ocursor[col++] = cell;      // update out image
                    continue;                   // NUL, padding
                }

                //  ESC[<y>; <x> H              CUP, Cursor Position *Cursor moves to <x>; <y> coordinate within the view-port, where <x> is the column of the <y> line.
                //
                if (-1 == start) {
                    if (0 == (flags & TRASHED) &&
                            SameCell(&cell, ocursor + col)) {
                        ++col;
                        continue;               // up-to-date
                    }
                    wctext = wcbuf + wsprintfW(wcbuf, L_VTCSI L"%u;%uH", row + 1, col + 1);
                    start = col;

                } else {                        // attribute run
                    if (SameAttributesFGBG(&cell, &info, fg, bg, VIO_UNDERLINE|VIO_BOLD|VIO_BLINK|VIO_INVERSE)) {
                        ocursor[col++] = cell;  // update out image
                        *wctext++ = (WCHAR)cell.Char.UnicodeChar;
                        continue;
                    }

                    //else, attribute change.
                }

                //  ESC[<n> m                   SGR, Set Graphics Rendition, Set the format of the screen and text as specified by <n>.
                //
                //      0                       Default, returns all attributes to the default state prior to modification.
                //
                //      1                       Bold / Bright Applies brightness / intensity flag to foreground color.
                //      4                       Underline
                //      7                       Negative; swaps foreground and background colors.
                //
                //      38; 2; <r>; <g>; <b>    Set foreground color to RGB value specified in <r>, <g>, <b> parameters.
                //      48; 2; <r>; <g>; <b>    Set background color to RGB value specified in <r>, <g>, <b> parameters.
                //      38; 5; <s>              Set foreground color to <s> index in 88 or 256 color table.
                //      48; 5; <s>              Set background color to <s> index in 88 or 256 color table.
                //
                ocursor[col++] = cell;          // update out image.

                info = cell.Info;
                COLOR256(&info, &fg, &bg);

                wctext += wsprintfW(wctext, L_VTCSI L"0m" L_VTCSI L"38;2;%u;%u;%um" L_VTCSI L"48;2;%u;%u;%um",
                                GetRValue(fg), GetGValue(fg), GetBValue(fg), GetRValue(bg), GetGValue(bg), GetBValue(bg));

                {   const WORD Attributes = info.Attributes;
                    if (Attributes) {           // special attributes.
                        if (Attributes & VIO_UNDERLINE)           wctext += wsprintfW(wctext, L_VTCSI L"4m");
                        if (Attributes & (VIO_BOLD | VIO_BLINK))  wctext += wsprintfW(wctext, L_VTCSI L"1m");
                        if (Attributes & VIO_INVERSE)             wctext += wsprintfW(wctext, L_VTCSI L"7m");
                      //if (Attributes & VIO_ITALIC)
                      //if (Attributes & VIO_STRIKE)
                      //if (Attributes & VIO_FAINT)
                    }
                }

                *wctext++ = (WCHAR)cell.Char.UnicodeChar;

            } while (col < cols && wctext < wcend);

            if (start >= 0) {                   // write text.
                const int size =
                    WideCharToMultiByte(CP_UTF8, 0, wcbuf, (int)(wctext - wcbuf), u8buf, sizeof(u8buf), NULL, NULL);

                if (CP_UTF8 != ctx->codepagemode && CP_UTF8 != vio.oldConsoleMode) {
                    (void) SetConsoleOutputCP(CP_UTF8);
                        // If the current font is a fixed - pitch Unicode font, SetConsoleOutputCP changes the mapping of the character values into the glyph set of the font,
                        // rather than loading a separate font each time it is called. This affects how extended characters (ASCII value greater than 127) are displayed in a console window.
                        // However, if the current font is a raster font, SetConsoleOutputCP() does not affect how extended characters are displayed.
                    ctx->codepagemode = vio.oldConsoleMode;
                }

                (void) WriteFile(chandle, u8buf, size, NULL, 0);
            }
        }
        ++row;

    } while (cursor < end);
}
#endif  //WIN32_CONSOLEVIRTUAL


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
 **/
static void
UnderOutEx(copyoutctx_t *ctx, unsigned pos, unsigned cnt)
{
    const WCHAR_INFO *cursor = vio.image + pos, *end = cursor + cnt;
    const int rows = vio.rows, cols = vio.cols;
    float fcwidth, fcheight;                    // proportional sizing
    int row = (int)(pos / cols);
    HDC wdc;

    if (vio.maxcolors < 256) {
        return;
    }

    if (0 == (wdc = ctx->devicecontext)) {      // client area DC
        ctx->devicecontext = GetDC(vio.whandle);
        (void) GetClientRect(vio.whandle, &ctx->devicerect);
        wdc = ctx->devicecontext;
    }
    fcwidth = (float)ctx->devicerect.right / cols;
    fcheight = (float)ctx->devicerect.bottom / rows;

    do {
        int start = -1, col = 0;
        COLORREF fg = 0, bg = 0;

        while (col < cols) {
            start = -1;
            do {
                const WCHAR_INFO cell = *cursor++;

                if (VIO_UNDERLINE & cell.Info.Attributes) {
                    if (start < 0) {            // inside
                        COLOR256(&cell.Info, &fg, &bg);
                        start = col;
                    }
                } else if (start >= 0) {
                    ++col;
                    break;                      // flush
                }
                ++col;
            } while (col < cols);

            if (start >= 0) {                   // underline current extent
                const int y = (int)(fcheight * (row + 1)) - 1,
                        x = (int)(fcwidth * start);
                HPEN oldpen;

                oldpen = SelectObject(wdc, CreatePen(PS_SOLID, 0, fg));
                MoveToEx(wdc, x, y, NULL);
                LineTo(wdc, x + (int)(fcwidth * (col - (start + 1))), y);
                oldpen = SelectObject(wdc, oldpen);
                DeleteObject(oldpen);
            }
        }
        ++row;

    } while (cursor < end);
}


/*private*/
/*  Function:           StrikeOutEx
 *      Over-strike characters within the specified region.
 *
 *  Parameters
 *      pos - Starting offset in video cells from top left.
 *      cnt - Video cell count.
 *
 *  Returns:
 *      nothing.
 **/
static void
StrikeOutEx(copyoutctx_t *ctx, unsigned pos, unsigned cnt)
{
    const WCHAR_INFO *cursor = vio.image + pos, *end = cursor + cnt;
    const int rows = vio.rows, cols = vio.cols;
    float fcwidth, fcheight;                    // proportional sizing.
    int row = (int)(pos / cols);
    HDC wdc;

    if (vio.maxcolors < 256) {
        return;
    }

    if (0 == (wdc = ctx->devicecontext)) {      // client area DC
        ctx->devicecontext = GetDC(vio.whandle);
        (void)GetClientRect(vio.whandle, &ctx->devicerect);
        wdc = ctx->devicecontext;
    }
    fcwidth = (float)ctx->devicerect.right / cols;
    fcheight = (float)ctx->devicerect.bottom / rows;

    do {
        int start = -1, col = 0;
        COLORREF fg = 0, bg = 0;

        while (col < cols) {
            start = -1;
            do {
                const WCHAR_INFO cell = *cursor++;

                if (VIO_STRIKE & cell.Info.Attributes) {
                    if (start < 0) {            // inside.
                        COLOR256(&cell.Info, &fg, &bg);
                        start = col;
                    }
                } else if (start >= 0) {
                    ++col;
                    break;                      // flush.
                }
                ++col;
            } while (col < cols);

            if (start >= 0) {                   // underline current extent.
                const int y = (int)((fcheight * (row + 1)) - (fcheight / 2)),
                        x = (int)(fcwidth * start);
                HPEN oldpen;

                oldpen = SelectObject(wdc, CreatePen(PS_SOLID, 0, fg));
                MoveToEx(wdc, x, y, NULL);
                LineTo(wdc, x + (int)(fcwidth * (col - (start + 1))), y);
                oldpen = SelectObject(wdc, oldpen);
                DeleteObject(oldpen);
            }
        }
        ++row;

    } while (cursor < end);
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
enumFontFamExProc(const LOGFONTA *lpelfe, const TEXTMETRICA * unused1, DWORD FontType, LPARAM unused2)
{
    struct fcname *fcname;

    __CUNUSED(unused1)
    __CUNUSED(unused2)

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

    if (0 == names) {                           // default, iso-8859-x coverage.
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
    //          https://github.com/powerline/fonts
    //          http://www.proggyfonts.net/
    //          http://www.levien.com/type/myfonts/inconsolata.html
    //          http://terminus-font.sourceforge.net/ and https://files.ax86.net/terminus-ttf/
    //
    fcnpush("Classic Console", FCNUNC);         // clean mono-spaced font, http://webdraft.hu/fonts/classic-console/

    fcnpush("DejaVu Sans Mono", FCNUNC);        // nice mono-spaced font, dejavu-fonts.org

    fcnpush("FreeMono", FCNUNC);                // GNU Freefont project (8x16 or better advised)

    fcnpush("Quivira", FCNUNC|FCNPRO2);         // www.quivira-font.com
    fcnpush("TITUS Cyberbit Basic", FCNUNC|FCNPRO2);
    fcnpush("Marin", FCNUNC|FCNPRO2);

    fcnpush("Arial Unicode MS", FCNUNC|FCNPRO5);// 'most complete' standard windows Unicode font (Office)

    fcnpush("Courier New", FCNUNC|FCNPRO);      // next most complete font, yet mono-spaced.

    fcnpush("Consolas", 0);                     // "Consolas Font Pack" for Microsoft Visual Studio.

    fcnpush("Terminal", FCNRASTER);             // implied raster font.

    //  Determine availability
    //
    logFont.lfCharSet = DEFAULT_CHARSET;
    wdc = GetDC(vio.whandle);
    EnumFontFamiliesExA(wdc, &logFont, enumFontFamExProc, 0, 0);
    ReleaseDC(vio.whandle, wdc);

    //  Trace results
    //
#if defined(DO_TRACE_LOG)
    {   struct fcname *fcname = 0;
        unsigned cnt = 0;

        TRACE_LOG(("Console Fonts\n"))
        for (fcname = vio.fcnames; fcname->name; ++cnt, ++fcname) {
            TRACE_LOG(("  [%02u] 0x%04x <%s> %s\n", cnt,
                fcname->flags, fcname->name, (fcname->available ? " (*)" : "")))
        }
    }
#endif  //DO_TRACE_LOG
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
    HFONT fnHandle = 0, fiHandle = 0, fbHandle = 0,
        fbiHandle = 0, ffHandle = 0, fHandleOrg = 0;
    int faceindex = 0;
    const struct fcname *fcname = NULL;
    char t_facename[LF_FACESIZE + 1] = {0};
    TEXTMETRIC tm = {0};
    RECT rect = {0};
    HDC wdc;
    SIZE size;

    wdc = GetDC(whandle);
    GetClientRect(whandle, &rect);

    if (-1 == height) {                         // -1, implied.
        height = (int)((float)rect.bottom/vio.rows);
    }

    if (-1 == width) {
        width = (int)((float)rect.right/vio.cols);
    }

    // match
    do {
#define WEIGHT_FAINT    FW_ULTRALIGHT
//  #define WEIGHT_REGULAR  FW_REGULAR          // personal settings; TODO configuration option.
//  #define WEIGHT_BOLD     FW_SEMIBOLD
#define WEIGHT_REGULAR  FW_MEDIUM               // closer match to standard console.
#define WEIGHT_BOLD     FW_BOLD

        //  Select by name.
        //
        if (facename && *facename) {
            fcname = fcnfind(facename);
            fnHandle = consolefontcreate(height, width, WEIGHT_REGULAR, FALSE, facename);
            if (fnHandle) {                     // bold, italic, bold+italic, faint
                fbHandle  = consolefontcreate(height, width, WEIGHT_BOLD,    FALSE, facename);
                fiHandle  = consolefontcreate(height, width, WEIGHT_REGULAR, TRUE,  facename);
                fbiHandle = consolefontcreate(height, width, WEIGHT_BOLD,    TRUE,  facename);
                ffHandle  = consolefontcreate(height, width, WEIGHT_FAINT,   FALSE, facename);
            }
        }

        //  Select first available.
        //
        if (! fnHandle) {
            while (1) {                         // test availability.
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
                    if (fnHandle) {             // bold, italic, bold+italic, faint.
                        fbHandle  = consolefontcreate(height, t_width, WEIGHT_BOLD,    FALSE, facename);
                        fiHandle  = consolefontcreate(height, t_width, WEIGHT_REGULAR, TRUE,  facename);
                        fbiHandle = consolefontcreate(height, t_width, WEIGHT_BOLD,    TRUE,  facename);
                        ffHandle  = consolefontcreate(height, t_width, WEIGHT_FAINT,   FALSE, facename);
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
            if (vio.ffHandle) DeleteObject(vio.ffHandle);
            if (vio.fbiHandle) DeleteObject(vio.fbiHandle);
            if (vio.fiHandle) DeleteObject(vio.fiHandle);
            if (vio.fbHandle) DeleteObject(vio.fbHandle);
            DeleteObject(vio.fnHandle);
        }

        vio.fcwidth  = (int)size.cx;            // tm.tmMaxCharWidth
        vio.fcheight = (int)size.cy;            // tm.tmHeight
        vio.ffHandle = ffHandle;
        vio.fbiHandle = fbiHandle;
        vio.fiHandle = fiHandle;
        vio.fbHandle = fbHandle;
        vio.fnHandle = fnHandle;

#undef  WEIGHT_FAINT
#undef  WEIGHT_REGULAR
#undef  WEIGHT_BOLD

        // font glyphs
#if defined(DO_TRACE_LOG)
        {   DWORD dwGlyphSize;

            if ((dwGlyphSize = GetFontUnicodeRanges(wdc, NULL)) > 0) {
                GLYPHSET *glyphsets;
                DWORD r;
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

    TRACE_LOG(("Console Font: <%s>, %dx%d (%s)\n", vio.fcfacename, \
        vio.fcwidth, vio.fcheight, (vio.fcflags & FCNRASTER) ? "raster" : "unicode"))

    return 0;
}


static HFONT
consolefontcreate(int height, int width, int weight, int italic, const char *facename)
{
    const BOOL isTerminal =                     // special terminal/raster support.
        (0 == strcmp(facename, "Terminal"));

    wchar_t wfacename[64] = {0};
    for (unsigned i = 0; i < (_countof(wfacename)-1) && facename[i]; ++i)
        wfacename[i] = (wchar_t)(facename[i]);

    HFONT hFont = CreateFontW(
        height, width -                         // logic (device dependent pixels) height and width.
            (italic ? 3 : (FW_BOLD == weight ? 1 : 0)),
        0, 0, weight,
        (italic ? TRUE : FALSE),                // italic, underline:FALSE, strikeout:FALSE.
            FALSE,
            FALSE,
    //  (isTerminal ? OEM_CHARSET : ANSI_CHARSET),
        ANSI_CHARSET,                           // DEFAULT (locale), ANSI, OEM ...
                        // DEFAULT, ANSI, BALTIC, CHINESEBIG5, EASTEUROPE, GB2312, GREEK, HANGUL,
                        // MAC_CHARSET, OEM, RUSSIAN, SHIFTJIS, TURKISH, VIETNAMESE
                                                // .. should we match the locale/codepage ?
        (isTerminal ? OUT_RASTER_PRECIS : OUT_TT_PRECIS),
        CLIP_DEFAULT_PRECIS,                    // default clipping behavior.
        (italic ? PROOF_QUALITY : ANTIALIASED_QUALITY),
        FIXED_PITCH | FF_MODERN,                // DECORATIVE, DONTCARE, MODERN, ROMAN, SCRIPT, SWISS
        wfacename);

    TRACE_LOG(("Create Font: <%s> %dx%d, Weight:%d, Italic:%d (%p)\n", \
        facename, width, height, weight, italic, hFont))

    return hFont;
}


static __inline void
WCHAR_BUILD(const uint32_t ch, const struct WCHAR_COLORINFO *info, WCHAR_INFO *ci)
{
    ci->Info = *info;

     if (ISACS & ch) {
         ci->Char.UnicodeChar = ch & 0xffffff;  // already mapped; mask character.

     } else {
         if (ci->Info.Attributes & VIO_ALTCHARSET) {
             if (ch > 0 && ch < (sizeof(acs_characters) / sizeof(acs_characters[0]))) {
                 ci->Char.UnicodeChar = acs_characters[ch];
                 return;
             }
         }
         ci->Char.UnicodeChar = unicode_remap(ch);
     }
}


static BOOL
WATTR_COMPARE(const WCHAR_INFO *c1, const struct WCHAR_COLORINFO *info2)
{
    if (c1->Info.Flags || info2->Flags) {
        return (c1->Info.Flags == info2->Flags &&
                    c1->Info.Attributes == info2->Attributes &&
                    c1->Info.fg == info2->fg &&
                    c1->Info.bg == info2->bg &&
                    c1->Info.fgrgb == info2->fgrgb &&
                    c1->Info.bgrgb == info2->bgrgb);
    }
    return (c1->Info.Attributes == info2->Attributes);
}


static BOOL
WCHAR_COMPARE(const WCHAR_INFO *c1, const WCHAR_INFO *c2)
{
    if (c1->Char.UnicodeChar == c2->Char.UnicodeChar) {
        return WATTR_COMPARE(c1, &c2->Info);
    }
    return FALSE;
}


static __inline unsigned
WCHAR_UPDATE(WCHAR_INFO *cursor, const uint32_t ch, const struct WCHAR_COLORINFO *info)
{
    WCHAR_INFO text;

    WCHAR_BUILD(ch, info, &text);
    if (WCHAR_COMPARE(cursor, &text)) {
        return 0;                               // up-to-date
    }
    *cursor = text;
    return TOUCHED;
}


static int
parse_color(const char *color, const char *defname, const struct attrmap *map, int *attr)
{
    char t_name[128] = {0};
    const char *a;
    int c, col = -1;

    //  color[;attribute[;...]]
    //
    if (!defname || !*defname)                   // undefined; assume white/black.
        defname = (map == win16_foreground ? "white" : "black");

    if (!color || !*color) color = defname;      // undefined, apply default.

    //  optional trailing attributes
    if (NULL != (a = strchr(color, ';'))) {
        const int len = (int)(a - color);

        strncpy(t_name, color, sizeof(t_name)-1);// remove attribute component.
        if (len < (int)sizeof(t_name))
            t_name[len] = 0;
        color = t_name;

        *attr = parse_attributes(a);
    }

    //  non-optional color
    while (' ' == *color || '\t' == *color) ++color;
    if (0 == sscanf(color, "color%u", &col)) {   // extension
        for (c = 0; map[c].name; ++c) {          // search color map
            if (0 == stricmp(color, map[c].name)) {
                return map[c].win; //done

            } else if (0 == stricmp(defname, map[c].name)) {
                col = map[c].win;  //apply default
            }
        }
    }
    return col;
}


static int
parse_true_color(const char *color, COLORREF *rgb, int *attr)
{
    unsigned char red, green, blue;
    unsigned char v[6];
    int i;

    if (vio.notruecolor)
        return -1;                              // disabled

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

    red   = (v[0] << 4) | v[1];
    green = (v[2] << 4) | v[3];
    blue  = (v[4] << 4) | v[5];
    *rgb  = RGB(red, green, blue);

    *attr = parse_attributes(color);            // optional trailing attribute(s)

    return rgb_search(16, *rgb);                // return 16-color equiv
}


static int
parse_attributes(const char *attr)
{
    int attributes = 0;

    while (attr) {                              // foreach(attribute)
        while (';' == *attr || ' ' == *attr || '\t' == *attr) {
            ++attr;                             // leading whitespace
        }

        if (*attr) {
            if (0 == _strnicmp(attr, "bold", 4)) {
                attributes |= VIO_BOLD;
            } else if (0 == _strnicmp(attr, "blink", 5)) {
                attributes |= VIO_BLINK;
            } else if (0 == _strnicmp(attr, "underline", 9)) {
                attributes |= VIO_UNDERLINE;
            } else if (0 == _strnicmp(attr, "italic", 6)) {
                attributes |= VIO_ITALIC;
            } else if (0 == _strnicmp(attr, "inverse", 7) || 0 == _strnicmp(attr, "reverse", 7)) {
                attributes |= VIO_INVERSE;
            } else if (0 == _strnicmp(attr, "strike", 6)) {
                attributes |= VIO_STRIKE;
            } else if (0 == _strnicmp(attr, "faint", 5)) {
                attributes |= VIO_FAINT;
            }
        }
        attr = strchr(attr, ';');               // next
    }
    return attributes;
}


/*
 *  check_activecolors ---
 *      Verify the current active colors, calling after an attribute-object change.
 **/
static void
check_activecolors(void)
{
    vio.activecolors = 16;                      // by default 16
    if (vio.maxcolors > 16) {                   // supported colors > 16; scan objects.
        const struct WCHAR_COLORINFO *attr, *end;

        for (attr = vio.c_attrs, end = attr + MAXOBJECTS; attr < end; ++attr) {
            if (attr->Flags & (VIO_F256|VIO_FRGB)) {
                vio.activecolors = 256;         // enable 256 driver
                break;
            }
        }
    }
}


/*  Function:       winnormal
 *      Apply color foreground/background defaults for WIN_COLOR specifications.
 *
 *  Parameters
 *      color - WIN_COLOR value.
 *
 *  Returns:
 *      Color index value with defaults applied.
 **/
static __inline int
winnormal(const int color)
{
    if (color == VT_COLOR_FOREGROUND) return COLIDX_FOREGROUND;
    if (color == VT_COLOR_BACKGROUND) return COLIDX_BACKGROUND;
    return win2vt[color & 0xf];
}


/*  Function:       vtnormal
 *      Apply color foreground/background defaults for VT_COLOR specifications.
 *
 *  Parameters
 *      color - VT_COLOR value.
 *
 *  Returns:
 *      Color index value with defaults applied.
 **/
static __inline int
vtnormal(const int color)
{
    if (color == VT_COLOR_FOREGROUND) return COLIDX_FOREGROUND;
    if (color == VT_COLOR_BACKGROUND) return COLIDX_BACKGROUND;
    return color & 0xff;
}


/*
 *  vio_save ---
 *      Save the buffer screen state; for later restoration via via_restore()
 **/
LIBVIO_API void
vio_save(void)
{
    HANDLE console = (vio.inited ? vio.chandle : GetStdHandle(STD_OUTPUT_HANDLE));
    COORD cursor, home = {0, 0};
    int rows, cols;

    /*
     *  Size arena
     */
    cursor = vio_size(console, &rows, &cols);

    if (!vio_state.image ||                     // initial or size change
            vio_state.rows != rows || vio_state.cols != cols) {
        CHAR_INFO *nimage;

        if (rows <= 0 || cols <= 0 ||
                NULL == (nimage = calloc(rows * cols, sizeof(CHAR_INFO)))) {
            return;
        }
        free(vio_state.image);                  // release previous; if any
        vio_state.image = nimage;
        vio_state.rows = rows;
        vio_state.cols = cols;
    }

    /*
     *  Save cursor and image
     */
    vio_state.cursorcoord = cursor;
    GetConsoleCursorInfo(console, &vio_state.cursorinfo);
    SetConsoleCursorPosition(console, home);    // home cursor.

    ImageSave(console, 0, rows * cols);         // read image.
}


static void
ImageSave(HANDLE console, unsigned pos, unsigned cnt)
{
    const int rows = vio_state.rows, cols = vio_state.cols;
    COORD is = {0}, ic = {0};
    SMALL_RECT wr = {0};
    DWORD rc;

    assert(pos < (unsigned)(rows * cols));
    assert(cnt && 0 == (pos % cols));
    assert((pos + cnt) <= (unsigned)(rows * cols));

    wr.Left   = 0;                              // src. screen rectangle.
    wr.Right  = (SHORT)(cols - 1);
    wr.Top    = (SHORT)(pos / cols);
    wr.Bottom = (SHORT)((pos + (cnt - 1)) / cols);

    is.Y      = (SHORT)(rows - wr.Top);         // size of image.
    is.X      = (SHORT)(cols);

    ic.X      = 0;                              // top left src cell in image.
    ic.Y      = 0;
                                                // read in image.
    rc = ReadConsoleOutputW(console, vio_state.image + pos, is, ic, &wr);

    if (0 == rc && ERROR_NOT_ENOUGH_MEMORY == GetLastError()) {
        if (cnt > ((unsigned)cols * 2)) {       // sub-divide request (max 8k).
            const int cnt2 = (cnt / (cols * 2)) * cols;

            ImageSave(console, pos, cnt2);
            ImageSave(console, pos + cnt2, cnt - cnt2);
        }
    }
}


/*
 *  vio_restore ---
 *      Restore the buffer; from a previous via_save()
 **/
LIBVIO_API void
vio_restore(void)
{
    HANDLE console = (vio.inited ? vio.chandle : GetStdHandle(STD_OUTPUT_HANDLE));
    COORD iHome = {0,0};
    CHAR_INFO * nimage;
    int rows = 0, cols = 0;

    if (NULL == vio_state.image)                // initialised?
        return;

    vio_size(console, &rows, &cols);
    if (0 == rows || 0 == cols)
        return;
                                                // resize
    if ((rows != vio_state.rows || cols != vio_state.cols) &&
            (nimage = calloc(rows * cols, sizeof(CHAR_INFO))) != NULL) {

        const CHAR_INFO blank = { {' '}, FOREGROUND_INTENSITY };
        const int cnt = (cols > vio_state.cols ? vio_state.cols : cols) * sizeof(CHAR_INFO);
        int r, c;

        for (r = 0; r < rows; ++r) {
            if (r < vio_state.rows) {           // merge previous image.
                memcpy(nimage + (r * cols),
                    (const void *)(vio_state.image + (r * vio_state.cols)), cnt);
            }
                                                // blank new cells.
            if ((c = (r >= vio_state.rows ? 0 : vio_state.cols)) < cols) {
                CHAR_INFO *p = nimage + (r * cols) + c;
                do {
                    *p++ = blank;
                } while (++c < cols);
            }
        }

        free((void *)vio_state.image);          // replace image.
        vio_state.image = nimage;
        vio_state.rows = rows;
        vio_state.cols = cols;
    }

    /*
     *  Restore image and cursor
     */
    SetConsoleCursorPosition(console, iHome);   // home cursor.

    ImageRestore(console, 0, rows * cols);      // write out image.

                                                // original cursor
    SetConsoleCursorPosition(console, vio_state.cursorcoord);
    SetConsoleCursorInfo(console, &vio_state.cursorinfo);
}


static void
ImageRestore(HANDLE console, unsigned pos, unsigned cnt)
{
    const int rows = vio_state.rows, cols = vio_state.cols;
    COORD is = {0}, ic = {0};
    SMALL_RECT wr = {0};
    DWORD rc;

    assert(pos < (unsigned)(rows * cols));
    assert(0 == (pos % cols));
    assert(cnt && 0 == (cnt % rows));
    assert((pos + cnt) <= (unsigned)(rows * cols));

    wr.Left   = 0;                              // src. screen rectangle.
    wr.Right  = (SHORT)(cols - 1);
    wr.Top    = (SHORT)(pos / cols);
    wr.Bottom = (SHORT)((pos + (cnt - 1)) / cols);

    is.Y      = (SHORT)(vio.rows - wr.Top);     // size of image.
    is.X      = (SHORT)vio.cols;

    ic.X      = 0;                              // top left src cell in image.
    ic.Y      = 0;
                                                // read in image.
    rc = WriteConsoleOutputW(console, vio_state.image + pos, is, ic, &wr);

    if (0 == rc && ERROR_NOT_ENOUGH_MEMORY == GetLastError()) {
        if (cnt > ((unsigned)cols * 2)) {       // sub-divide request.
            const int cnt2 = (cnt / (cols * 2)) * cols;

            ImageRestore(console, pos, cnt2);
            ImageRestore(console, pos + cnt2, cnt - cnt2);
        }
    }
}


/*
 *  vio_screenbuffersize ---
 *      Retrieve the size of the screen buffer in lines, which may differ from the window screen.
 **/
LIBVIO_API int
vio_screenbuffersize(void)
{
    HANDLE console = (vio.inited ? vio.chandle : GetStdHandle(STD_OUTPUT_HANDLE));
    CONSOLE_SCREEN_BUFFER_INFO sbi = {0};
    int ret = -1;

    if (GetConsoleScreenBufferInfo(console, &sbi)) {
        ret = sbi.dwSize.X;
    }
    return ret;
}


/*
 *  vio_open ---
 *      Open the console driver.
 **/
LIBVIO_API int
vio_open(int *rows, int *cols)
{
    if (vio.inited) vio_reset();

    vio_init();
    vio_define_vtattr(0, VT_COLOR_LIGHT_GREY, VT_COLOR_BLACK, 0);
    vio_normal_video();
    vio.inited = 1;

    TRACE_LOG(("open(rows:%d, cols:%d)", vio.rows, vio.cols))
    if (rows) *rows = vio.rows;
    if (cols) *cols = vio.cols;

    return 0;
}


/*
 *  vio_close ---
 *      Close the console driver.
 **/
LIBVIO_API void
vio_close(void)
{
    if (0 == vio.inited) return;                /* uninitialized */

    TRACE_LOG(("close(rows:%d, cols:%d)", vio.rows, vio.cols))
    if (vio.maximised) {
        vio_setsize(vio.maximised_oldrows, vio.maximised_oldcols);
        if (vio.isvirtualconsole) {
            if (3 == vio.isvirtualconsole) {    /* restore? */
                const DWORD mode = vio.oldConsoleMode;

                if (mode) {
                    (void) SetConsoleMode(vio.chandle, mode);
                }
                if (vio.oldConsoleCP) {
                    (void) SetConsoleOutputCP(vio.oldConsoleCP);
                }
            }
            vio.isvirtualconsole = 1;           /* reinitialize state */
        }
        ShowWindow(vio.whandle, /*SW_RESTORE*/ SW_NORMAL);
        vio.maximised = 0;
    }
    vio_reset();
}


/*
 *  vio_config_truecolor ---
 *      Configure whether true-color support should be available.
 **/
LIBVIO_API void
vio_config_truecolor(int truecolor)
{
    vio.notruecolor = (0 == truecolor);
}


/*
 *  vio_winch ---
 *      Determine whether there has been a change in screen-size.
 **/
LIBVIO_API int
vio_winch(int *nrows, int *ncols)
{
    int rows, cols, ret = 0;

    vio_size(vio.chandle, &rows, &cols);

    if (rows != vio.rows || cols != vio.cols) {
        TRACE_LOG(("winch(rows:%d->%d, cols:%d->%d)", vio.rows, rows, vio.cols, cols))
        vio_init();
        vio.c_trashed = 1;
        vio.inited = 1;
        ret = 1;
    }

    if (nrows) *nrows = vio.rows;
    if (ncols) *ncols = vio.cols;

    return ret;
}


/*
 *  vio_get_size ---
 *      Retrieve the current terminal size, in rows and columns.
 **/
LIBVIO_API void
vio_get_size(int *rows, int *cols)
{
    if (rows) *rows = vio.rows;
    if (cols) *cols = vio.cols;
}


/*
 *  vio_togglesize ---
 *      Toggle display screen, between original and maximised.
 **/
LIBVIO_API int
vio_toggle_size(int *rows, int *cols)
{
    int ret = 0;

    /* min/max */
    if (0 == vio.inited) return -1;             /* uninitialised */

    if (vio.maximised <= 0) {
        if (! vio_setsize(0xffff, 0xffff)) {
            return -1;                          /* not supported */
        }

        if (-1 == vio.maximised) {
            vio.maximised_oldrows = vio.rows;
            vio.maximised_oldcols = vio.cols;
        }

        vio.maximised = 1;
        ret = 1;

    } else {
        vio_setsize(vio.maximised_oldrows, vio.maximised_oldcols);
        vio.maximised = 0;
        ret = 0;
    }

    /* reinitialise */
    if (1 == vio.maximised) {
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
    if (rows) *rows = vio.rows;
    if (cols) *cols = vio.cols;
    return ret;
}


/*
 *  vio_cursor_show ---
 *      Make the cursor visible.
 **/
LIBVIO_API void
vio_cursor_show(void)
{
    if (vio.c_state > 0) {
        CONSOLE_CURSOR_INFO info;
        info.dwSize = vio.c_size;
        info.bVisible = TRUE;
        SetConsoleCursorInfo(vio.chandle, &info);
    }
}


/*
 *  vio_cursor_hide---
 *      Hide the cursor.
 **/
LIBVIO_API void
vio_cursor_hide(void)
{
    if (vio.c_state) {
        CONSOLE_CURSOR_INFO info;
        if (vio.c_state == -1) {
            GetConsoleCursorInfo(vio.chandle, &info);
            vio.c_size = info.dwSize;
        }
        info.dwSize = vio.c_size;
        info.bVisible = FALSE;
        SetConsoleCursorInfo(vio.chandle, &info);
        vio.c_state = 0;
    }
}


/*
 *  vio_cursor_state---
 *      Retrieve the current cursor state.
 **/
LIBVIO_API int
vio_cursor_state(void)
{
    return vio.c_state;
}


/*
 *  vio_goto ---
 *      Set the cursor position.
 **/
LIBVIO_API void
vio_goto(int row, int col)
{
    if (row >= vio.rows) row = vio.rows-1;
    vio.c_row = row;

    if (col >= vio.cols) col = vio.cols-1;
    vio.c_col = col;
}


/*
 *  vio_row ---
 *      Retrieve the current cursor row.
 **/
LIBVIO_API int
vio_row(void)
{
    return vio.c_row;
}


/*
 *  vio_col ---
 *      Retrieve the current cursor column.
 **/
LIBVIO_API int
vio_column(void)
{
    return vio.c_col;
}


LIBVIO_API int
vio_define_attr(int obj, const char *what, const char *fg, const char *bg)
{
    COLORREF frgb = (COLORREF)-1, brgb = (COLORREF)-1;
    int fcolor = 0, bcolor = 0, fattr = 0, battr = 0;
    unsigned flags = 0;

    assert(obj >= 0 && obj < MAXOBJECTS);
    if (obj < 0 || obj >= MAXOBJECTS)
        return -1;

    if ((fcolor = parse_true_color(fg, &frgb, &fattr)) >= 0) {
        flags |= VIO_FRGB;
    } else if ((fcolor = parse_color(fg, "lightgray", win16_foreground, &fattr)) >= 16) {
        flags |= VIO_F256;
    }

    if ((bcolor = parse_true_color(bg, &brgb, &battr)) >= 0) {
        flags |= VIO_FRGB;
    } else if ((bcolor = parse_color(bg, "black", win16_foreground /*background*/, &battr)) >= 16) {
        flags |= VIO_F256;
    }

//  if (flags & (VIO_F256|VIO_FRGB)) {
//      if (vio.maxcolors <= 16) {              // only apply, if supported.
//          return -1;
//      }
//  }

    if (VIO_INVERSE & (fattr|battr)) {          // apply inverse.
        fattr &= ~VIO_INVERSE;
        battr &= ~VIO_INVERSE;
        SWAPFGBG(fcolor, bcolor);
        SWAPRGB(frgb, brgb);
    }

    // convert any non-extended attributes
    if (flags & VIO_FRGB) {                     // RGB (fg and/or bg)
        if ((COLORREF)-1 == frgb && fcolor < 16) fcolor = win2vt[ fcolor ];
        if ((COLORREF)-1 == brgb && bcolor < 16) bcolor = win2vt[ bcolor ];

    } else if (flags & VIO_F256) {              // 256
        if (fcolor < 16) fcolor = win2vt[ fcolor ];
        if (bcolor < 16) bcolor = win2vt[ bcolor ];

    } else {                                    // native
        assert(0 == flags);
        assert(fcolor >= 0 && fcolor < 16);
        assert(bcolor >= 0 && bcolor < 16);
        fattr |=  fcolor & 0x0f;
        battr |= (bcolor & 0x0f) << 4;
        fcolor = win2vt[fcolor & 0x0f];
        bcolor = win2vt[bcolor & 0x0f];
    }

    // apply
    vio.c_attrs[obj].Flags = (WORD)flags;
    vio.c_attrs[obj].Attributes = (WORD)(fattr | battr);
    vio.c_attrs[obj].fg = (SHORT)fcolor;
    vio.c_attrs[obj].bg = (SHORT)bcolor;
    vio.c_attrs[obj].fgrgb = frgb;
    vio.c_attrs[obj].bgrgb = brgb;
    free((char *)vio.c_names[obj]);
    vio.c_names[obj] = (what ? strdup(what) : NULL);

    check_activecolors();
    return 0;
}


LIBVIO_API void
vio_define_winattr_native(int obj, uint16_t attributes)
{
    assert(obj >= 0 && obj < MAXOBJECTS);
    if (obj < 0 || obj >= MAXOBJECTS) return;

    vio.c_attrs[obj].Flags = 0;                 // native
    vio.c_attrs[obj].Attributes = attributes;
    vio.c_attrs[obj].fg = -1;
    vio.c_attrs[obj].bg = -1;
    vio.c_attrs[obj].fgrgb = (COLORREF)-1;
    vio.c_attrs[obj].bgrgb = (COLORREF)-1;
}


LIBVIO_API void
vio_define_winattr(int obj, int fg, int bg, uint16_t attributes)
{
    assert(obj >= 0 && obj < MAXOBJECTS);
    assert(fg >= WIN_COLOR_MIN && fg < WIN_COLOR_NUM);
    assert(bg >= WIN_COLOR_MIN && bg < WIN_COLOR_NUM);
    if (obj < 0 || obj >= MAXOBJECTS) return;

    if (VIO_INVERSE & attributes) {
        attributes &= ~VIO_INVERSE;
        SWAPFGBG(fg, bg);
    }

    if (fg < 0 || bg < 0) {                     // specials, dynamic
        vio.c_attrs[obj].Flags = VIO_F16;       // vt/xterm
        vio.c_attrs[obj].Attributes = attributes;
        vio.c_attrs[obj].fg = (short)winnormal(fg);
        vio.c_attrs[obj].bg = (short)winnormal(bg);

    } else {
        vio.c_attrs[obj].Flags = 0;             // native
        vio.c_attrs[obj].Attributes =
            (attributes & 0xff00) | (WORD)((fg)|(bg << 4));
        vio.c_attrs[obj].fg = -1;
        vio.c_attrs[obj].bg = -1;
    }
    vio.c_attrs[obj].fgrgb = (COLORREF)-1;
    vio.c_attrs[obj].bgrgb = (COLORREF)-1;
}


LIBVIO_API void
vio_define_vtattr(int obj, int fg, int bg, uint16_t attributes)
{
    assert(obj >= 0 && obj < MAXOBJECTS);
    assert(fg >= VT_COLOR_MIN && fg < MAXCOLORS);
    assert(bg >= VT_COLOR_MIN && bg < MAXCOLORS);
    if (obj < 0 || obj >= MAXOBJECTS) return;

    vio.c_attrs[obj].Flags = VIO_F16;           // vt/xterm
    if (fg >= 16 || bg >= 16) {
        vio.c_attrs[obj].Flags = VIO_F256;
        if (vio.maxcolors > 16) vio.activecolors = 256;
    }

    if (VIO_INVERSE & attributes) {
        attributes &= ~VIO_INVERSE;
        SWAPFGBG(fg, bg);
    }

    vio.c_attrs[obj].Attributes = attributes;
    vio.c_attrs[obj].fg = (short)vtnormal(fg);
    vio.c_attrs[obj].bg = (short)vtnormal(bg);
    vio.c_attrs[obj].fgrgb = (COLORREF)-1;
    vio.c_attrs[obj].bgrgb = (COLORREF)-1;
}


LIBVIO_API void
vio_define_rgbattr(int obj, int fg, int bg, uint16_t attributes)
{
    assert(obj >= 0 && obj < MAXOBJECTS);
    assert(fg >= 0 && fg < 0xfffffff);
    assert(bg >= 0 && bg < 0xffffff);
    if (obj < 0 || obj >= MAXOBJECTS) return;

    vio.c_attrs[obj].Flags = VIO_FRGB;          // true-color
    if (vio.maxcolors > 16) vio.activecolors = 256;

    if (VIO_INVERSE & attributes) {
        attributes &= ~VIO_INVERSE;
        SWAPFGBG(fg, bg);
    }

    vio.c_attrs[obj].Attributes = attributes;
    vio.c_attrs[obj].fg = (short)rgb_search(16, fg); // shadow colors (vt/xterm)
    vio.c_attrs[obj].bg = (short)rgb_search(16, bg);
    vio.c_attrs[obj].fgrgb = (COLORREF)fg;      // true-colors
    vio.c_attrs[obj].bgrgb = (COLORREF)bg;
}


LIBVIO_API void
vio_define_flags(int obj, uint16_t attributes)
{
    assert(obj >= 0 && obj < MAXOBJECTS);
    if (obj < 0 || obj >= MAXOBJECTS) return;
    vio.c_attrs[obj].Attributes &= 0x00ff;
    vio.c_attrs[obj].Attributes |= (attributes & 0xff00);
}


/*
 *  vio_set_colorattr ---
 *      Set the current terminal color, to the specified color-object.
 **/
LIBVIO_API void
vio_set_colorattr(int obj)
{
    assert(obj >= 0 && obj < MAXOBJECTS);
 // assert(vio.c_attrs[obj].Flags || vio.c_attrs[obj].Attributes); // active color-object?
    if (obj < 0 || obj >= MAXOBJECTS) return;

    vio.c_color.Flags = VIO_FOBJECT;            // redirect
    vio.c_color.Attributes = (WORD)obj;         // object index.
}


/*
 *  vio_set_wincolor_native ---
 *      Set the current terminal color, to the specified native windows console color-attribute.
 **/
LIBVIO_API void
vio_set_wincolor_native(uint16_t attributes)
{
    vio.c_color.Flags = 0;                      // native
    vio.c_color.Attributes = attributes;
    vio.c_color.fg = -1;
    vio.c_color.bg = -1;
    vio.c_color.fgrgb = (COLORREF)-1;
    vio.c_color.bgrgb = (COLORREF)-1;
}


/*
 *  vio_set_wincolor ---
 *      Set the current terminal color, foreground and background, to the specified windows console enumerated values.
 **/
LIBVIO_API void
vio_set_wincolor(int fg, int bg, uint16_t attributes)
{
    assert(fg >= WIN_COLOR_MIN && fg < WIN_COLOR_NUM);
    assert(bg >= WIN_COLOR_MIN && bg < WIN_COLOR_NUM);

    if (VIO_INVERSE & attributes) {
        attributes &= ~VIO_INVERSE;
        SWAPFGBG(fg, bg);
    }

    if (fg < 0 || bg < 0) {                     // specials, dynamic
        vio.c_color.Flags = VIO_F16;            // vt/xterm
        vio.c_color.Attributes = attributes;
        vio.c_color.fg = (short)winnormal(fg);
        vio.c_color.bg = (short)winnormal(bg);

    } else {
        vio.c_color.Flags = 0;                  // native
        vio.c_color.Attributes =
            (attributes & 0xff00) | (WORD)((fg)|(bg << 4));
        vio.c_color.fg = -1;
        vio.c_color.bg = -1;
    }
    vio.c_color.fgrgb = (COLORREF)-1;           // true-color (none)
    vio.c_color.bgrgb = (COLORREF)-1;
}


/*
 *  vio_set_vtcolor ---
 *      Set the current terminal color, foreground and background, to the specified vt/xterm color enumerated values.
 **/
LIBVIO_API void
vio_set_vtcolor(int fg, int bg, uint16_t attributes)
{
    assert(fg >= VT_COLOR_MIN && fg < MAXCOLORS);
    assert(bg >= VT_COLOR_MIN && bg < MAXCOLORS);

    vio.c_color.Flags = VIO_F16;
    if (fg >= 16 || bg >= 16) vio.c_color.Flags = VIO_F256;
    if (VIO_INVERSE & attributes) {
        attributes &= ~VIO_INVERSE;
        SWAPFGBG(fg, bg);
    }

    vio.c_color.Attributes = attributes;
    vio.c_color.fg = (short)vtnormal(fg);       // primary colors (vt/xterm)
    vio.c_color.bg = (short)vtnormal(bg);
    vio.c_color.fgrgb = (COLORREF)-1;           // true-colors (none)
    vio.c_color.bgrgb = (COLORREF)-1;
}


/*
 *  vio_set_rgbcolor ---
 *      Set the current terminal color, foreground and background, to the specified RGB color values.
 **/
LIBVIO_API void
vio_set_rgbcolor(int32_t fg, int32_t bg, uint16_t attributes)
{
    assert(fg >= 0 && fg <= 0xffffff);
    assert(bg >= 0 && bg <= 0xffffff);

    vio.c_color.Flags = VIO_FRGB;
    if (vio.maxcolors > 16) vio.activecolors = 256;
    if (VIO_INVERSE & attributes) {
        attributes &= VIO_INVERSE;
        SWAPFGBG(fg, bg);
    }

    vio.c_color.Attributes = attributes;
    vio.c_color.fg = (short)rgb_search(16, fg); // shadow colors (vt/xterm)
    vio.c_color.bg = (short)rgb_search(16, bg);
    vio.c_color.fgrgb = (COLORREF)fg;           // true-colors
    vio.c_color.bgrgb = (COLORREF)bg;
}


/*
 *  vio_set_winforeground ---
 *      Set the default terminal foreground color, to the specified windows console color enumerated value.
 **/
LIBVIO_API void
vio_set_winforeground(int color, int32_t rgb /*optional, otherwise -1*/)
{
    assert(color >= 0 && color < WIN_COLOR_NUM);

    color = win2vt[ color & 0x0f ];
    vio.color256to16[ COLIDX_FOREGROUND ] = vio.color256to16[ color ];
    vio.rgb256[ COLIDX_FOREGROUND ] = (rgb < 0 ? vio.rgb256[ color ] : (COLORREF)rgb);
}


/*
 *  vio_set_winbackground ---
 *      Set the default terminal background color, to the specified windows console color enumerated value.
 **/
LIBVIO_API void
vio_set_winbackground(int color, int32_t rgb /*optional, otherwise -1*/)
{
    assert(color >= 0 && color < WIN_COLOR_NUM);

    color = win2vt[ color & 0x0f ];
    vio.color256to16[ COLIDX_BACKGROUND ] = vio.color256to16[ color ];
    vio.rgb256[ COLIDX_BACKGROUND ] = (rgb < 0 ? vio.rgb256[ color ] : (COLORREF)rgb);
}


/*
 *  vio_set_vtforeground ---
 *      Set the default terminal foreground color, to the specified vt/xterm color enumerated value.
 **/
LIBVIO_API void
vio_set_vtforeground(int color, int32_t rgb /*optional, otherwise -1*/)
{
    assert(color >= 0 && color < 256);

    color &= 0xff;
    vio.color256to16[ COLIDX_FOREGROUND ] = vio.color256to16[ color ];
    vio.rgb256[ COLIDX_FOREGROUND ] = (rgb < 0 ? vio.rgb256[ color ] : (COLORREF)rgb);
}


/*
 *  vio_set_vtbackground ---
 *      Set the default terminal background color, to the specified vt/xterm color enumerated value.
 **/
LIBVIO_API void
vio_set_vtbackground(int color, int32_t rgb /*optional, otherwise -1*/)
{
    assert(color >= 0 && color < 256);

    color &= 0xff;
    vio.color256to16[ COLIDX_BACKGROUND ] = vio.color256to16[ color ];
    vio.rgb256[ COLIDX_BACKGROUND ] = (rgb < 0 ? vio.rgb256[ color ] : (COLORREF)rgb);
}


/*
 *  vio_normal_video ---
 *      Select the default terminal color.
 **/
LIBVIO_API void
vio_normal_video(void)
{
    vio.c_color.Flags = VIO_FNORMAL;
    vio.c_color.Attributes = 0;
    vio.c_color.fg = COLIDX_FOREGROUND;         /* implied indexes */
    vio.c_color.bg = COLIDX_BACKGROUND;
    vio.c_color.fgrgb = (COLORREF)-1;
    vio.c_color.bgrgb = (COLORREF)-1;
}


/*
 *  vio_flush ---
 *      Flush virtual display changes to the console.
 **/
LIBVIO_API void
vio_flush(void)
{
    copyoutctx_t ctx = {0};
    const int trashed = vio.c_trashed;
    int l, updates;

    if (0 == vio.inited) return;                /* uninitialised */

    CopyOutInit(&ctx);                          /* context initialisation */

    for (l = 0, updates = 0; l < vio.rows; ++l) {
        unsigned flags = vio.c_screen[l].flags;

        vio.c_screen[l].flags = 0;
        if (trashed) flags |= TRASHED;
        if (flags) {
            if (0 == updates++) {
                vio_setcursor(0, 0);            /* home console; on first update */
            }
            CopyOut(&ctx, vio.cols * l, vio.cols, flags);
        }
    }

    CopyOutFinal(&ctx);                         /* completion */

    if (0 == updates) {
        vio_setcursor(0, 0);                    /* home console; on first update */
    }
    vio_setcursor(vio.c_col, vio.c_row);        /* then ... set true cursor position */
            /* forces cursor to top of visible screen */
    vio.c_trashed = 0;
}


/*
 *  vio_atputc ---
 *      Write the specified character at the given coordinates onto the virtual display.
 **/
LIBVIO_API void
vio_atputc(int row, int col, unsigned ch, unsigned cnt)
{
    WCHAR_INFO *cursor, *cend, text;
    unsigned flags = 0;

    if (0 == vio.inited) return;                /* uninitialised */

    assert(row >= 0 && row < vio.rows);
    assert(col >= 0 && col < vio.cols);
    if (row < 0 || row >= vio.rows) return;
    if (col < 0 || col >= vio.cols) return;
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
        if (++col >= vio.cols) {
            vio.c_screen[ row ].flags |= flags;
            if (++row >= vio.rows) {
                row = vio.rows-1;               // scroll??
            }
            flags = 0;
            col = 0;
        }
    }
    vio.c_screen[ row ].flags |= flags;
}


/*
 *  vio_atputs ---
 *      Write the specified character string at the given coordinates onto the virtual display.
 **/
LIBVIO_API int
vio_atputs(int row, int col, const char *text)
{
    int cnt = 0;
    if (text) {
        while (*text) {
            vio_atputc(row, col++, *text++, 1);
            ++cnt;
        }
    }
    return cnt;
}


LIBVIO_API int
vio_atputsn(int row, int col, const char *text, unsigned len)
{
    int cnt = 0;
    if (text && len) {
        while (*text && len--) {
            vio_atputc(row, col++, *text++, 1);
            ++cnt;
        }
    }
    return cnt;
}


LIBVIO_API int
vio_atvprintf(int row, int col, const char *fmt, va_list ap)
{
    char buf[1024];
    int len;

    len = _vsnprintf(buf, sizeof(buf), fmt, ap);
    return vio_atputsn(row, col, buf, len);
}


LIBVIO_API int
vio_atprintf(int row, int col, const char *fmt, ...)
{
    va_list ap;
    int cnt;

    va_start(ap, fmt);
    cnt = vio_atvprintf(row, col, fmt, ap);
    va_end(ap);
    return cnt;
}


/*
 *  vio_putc ---
 *      Write the specified character at the current cursor position onto the virtual display.
 **/
LIBVIO_API void
vio_putc(unsigned ch, unsigned cnt, int move)
{
    WCHAR_INFO *cursor, *cend, text;
    int row = vio.c_row, col = vio.c_col;
    unsigned flags = 0;

    if (0 == vio.inited) return;                /* uninitialised */

    assert(row >= 0 && row < vio.rows);
    assert(col >= 0 && col < vio.cols);
    if (row < 0 || row >= vio.rows) return;
    if (col < 0 || col >= vio.cols) return;
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
        if (++col >= vio.cols) {
            vio.c_screen[ row ].flags |= flags;
            if (++row >= vio.rows) {
                row = vio.rows-1; //scroll??
            }
            flags = 0;
            col = 0;
        }
    }
    vio.c_screen[ row ].flags |= flags;
    if (move) {
        assert(row >= 0 && row < vio.rows);
        assert(col >= 0 && col < vio.cols);
        vio.c_col = col, vio.c_row = row;
    }
}


/* http://www.unicode.org/unicode/reports/tr11/
 *
 * Markus Kuhn -- 2007-05-26 (Unicode 5.0)
 *
 * Permission to use, copy, modify, and distribute this software
 * for any purpose and without fee is hereby granted. The author
 * disclaims all warranties with regard to this software.
 *
 * Latest version: http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 */

struct interval {
    int first;
    int last;
};


/* auxiliary function for binary search in interval table */
static int
bisearch(wchar_t ucs, const struct interval *table, int max)
{
    int min = 0;
    int mid;

    if (ucs < table[0].first || ucs > table[max].last)
        return 0;

    while (max >= min) {
        mid = (min + max) / 2;
        if (ucs > table[mid].last)
            min = mid + 1;
        else if (ucs < table[mid].first)
            max = mid - 1;
        else
            return 1;
    }

    return 0;
}


/* The following two functions define the column width of an ISO 10646
 * character as follows:
 *
 *    - The null character (U+0000) has a column width of 0.
 *
 *    - Other C0/C1 control characters and DEL will lead to a return
 *      value of -1.
 *
 *    - Non-spacing and enclosing combining characters (general
 *      category code Mn or Me in the Unicode database) have a
 *      column width of 0.
 *
 *    - SOFT HYPHEN (U+00AD) has a column width of 1.
 *
 *    - Other format characters (general category code Cf in the Unicode
 *      database) and ZERO WIDTH SPACE (U+200B) have a column width of 0.
 *
 *    - Hangul Jamo medial vowels and final consonants (U+1160-U+11FF)
 *      have a column width of 0.
 *
 *    - Spacing characters in the East Asian Wide (W) or East Asian
 *      Full-width (F) category as defined in Unicode Technical
 *      Report #11 have a column width of 2.
 *
 *    - All remaining characters (including all printable
 *      ISO 8859-1 and WGL4 characters, Unicode control characters,
 *      etc.) have a column width of 1.
 *
 * This implementation assumes that wchar_t characters are encoded
 * in ISO 10646.
 */

int vio_wcwidth(wchar_t ucs)
{
  /* sorted list of non-overlapping intervals of non-spacing characters */
  /* generated by "uniset +cat=Me +cat=Mn +cat=Cf -00AD +1160-11FF +200B c" */
  static const struct interval combining[] = {
    { 0x0300, 0x036F }, { 0x0483, 0x0486 }, { 0x0488, 0x0489 },
    { 0x0591, 0x05BD }, { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 },
    { 0x05C4, 0x05C5 }, { 0x05C7, 0x05C7 }, { 0x0600, 0x0603 },
    { 0x0610, 0x0615 }, { 0x064B, 0x065E }, { 0x0670, 0x0670 },
    { 0x06D6, 0x06E4 }, { 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED },
    { 0x070F, 0x070F }, { 0x0711, 0x0711 }, { 0x0730, 0x074A },
    { 0x07A6, 0x07B0 }, { 0x07EB, 0x07F3 }, { 0x0901, 0x0902 },
    { 0x093C, 0x093C }, { 0x0941, 0x0948 }, { 0x094D, 0x094D },
    { 0x0951, 0x0954 }, { 0x0962, 0x0963 }, { 0x0981, 0x0981 },
    { 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 }, { 0x09CD, 0x09CD },
    { 0x09E2, 0x09E3 }, { 0x0A01, 0x0A02 }, { 0x0A3C, 0x0A3C },
    { 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 }, { 0x0A4B, 0x0A4D },
    { 0x0A70, 0x0A71 }, { 0x0A81, 0x0A82 }, { 0x0ABC, 0x0ABC },
    { 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 }, { 0x0ACD, 0x0ACD },
    { 0x0AE2, 0x0AE3 }, { 0x0B01, 0x0B01 }, { 0x0B3C, 0x0B3C },
    { 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B43 }, { 0x0B4D, 0x0B4D },
    { 0x0B56, 0x0B56 }, { 0x0B82, 0x0B82 }, { 0x0BC0, 0x0BC0 },
    { 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 }, { 0x0C46, 0x0C48 },
    { 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 }, { 0x0CBC, 0x0CBC },
    { 0x0CBF, 0x0CBF }, { 0x0CC6, 0x0CC6 }, { 0x0CCC, 0x0CCD },
    { 0x0CE2, 0x0CE3 }, { 0x0D41, 0x0D43 }, { 0x0D4D, 0x0D4D },
    { 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 }, { 0x0DD6, 0x0DD6 },
    { 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A }, { 0x0E47, 0x0E4E },
    { 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 }, { 0x0EBB, 0x0EBC },
    { 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 },
    { 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E },
    { 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 }, { 0x0F90, 0x0F97 },
    { 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 },
    { 0x1032, 0x1032 }, { 0x1036, 0x1037 }, { 0x1039, 0x1039 },
    { 0x1058, 0x1059 }, { 0x1160, 0x11FF }, { 0x135F, 0x135F },
    { 0x1712, 0x1714 }, { 0x1732, 0x1734 }, { 0x1752, 0x1753 },
    { 0x1772, 0x1773 }, { 0x17B4, 0x17B5 }, { 0x17B7, 0x17BD },
    { 0x17C6, 0x17C6 }, { 0x17C9, 0x17D3 }, { 0x17DD, 0x17DD },
    { 0x180B, 0x180D }, { 0x18A9, 0x18A9 }, { 0x1920, 0x1922 },
    { 0x1927, 0x1928 }, { 0x1932, 0x1932 }, { 0x1939, 0x193B },
    { 0x1A17, 0x1A18 }, { 0x1B00, 0x1B03 }, { 0x1B34, 0x1B34 },
    { 0x1B36, 0x1B3A }, { 0x1B3C, 0x1B3C }, { 0x1B42, 0x1B42 },
    { 0x1B6B, 0x1B73 }, { 0x1DC0, 0x1DCA }, { 0x1DFE, 0x1DFF },
    { 0x200B, 0x200F }, { 0x202A, 0x202E }, { 0x2060, 0x2063 },
    { 0x206A, 0x206F }, { 0x20D0, 0x20EF }, { 0x302A, 0x302F },
    { 0x3099, 0x309A }, { 0xA806, 0xA806 }, { 0xA80B, 0xA80B },
    { 0xA825, 0xA826 }, { 0xFB1E, 0xFB1E }, { 0xFE00, 0xFE0F },
    { 0xFE20, 0xFE23 }, { 0xFEFF, 0xFEFF }, { 0xFFF9, 0xFFFB },
    { 0x10A01, 0x10A03 }, { 0x10A05, 0x10A06 }, { 0x10A0C, 0x10A0F },
    { 0x10A38, 0x10A3A }, { 0x10A3F, 0x10A3F }, { 0x1D167, 0x1D169 },
    { 0x1D173, 0x1D182 }, { 0x1D185, 0x1D18B }, { 0x1D1AA, 0x1D1AD },
    { 0x1D242, 0x1D244 }, { 0xE0001, 0xE0001 }, { 0xE0020, 0xE007F },
    { 0xE0100, 0xE01EF }
  };

  /* test for 8-bit control characters */
  if (ucs == 0)
    return 0;

  if (ucs < 32 || (ucs >= 0x7f && ucs < 0xa0))
    return -1;

  /* binary search in table of non-spacing characters */
  if (bisearch(ucs, combining, sizeof(combining) / sizeof(struct interval) - 1))
    return 0;

  /* if we arrive here, ucs is not a combining or C0/C1 control character */
  return 1 +
    (ucs >= 0x1100 &&
     (ucs <= 0x115f ||                    /* Hangul Jamo init. consonants */
      ucs == 0x2329 || ucs == 0x232a ||
      (ucs >= 0x2e80 && ucs <= 0xa4cf &&
       ucs != 0x303f) ||                  /* CJK ... Yi */
      (ucs >= 0xac00 && ucs <= 0xd7a3) || /* Hangul Syllables */
      (ucs >= 0xf900 && ucs <= 0xfaff) || /* CJK Compatibility Ideographs */
      (ucs >= 0xfe10 && ucs <= 0xfe19) || /* Vertical forms */
      (ucs >= 0xfe30 && ucs <= 0xfe6f) || /* CJK Compatibility Forms */
      (ucs >= 0xff00 && ucs <= 0xff60) || /* Full-width Forms */
      (ucs >= 0xffe0 && ucs <= 0xffe6)
#if defined(SIZEOF_WCHAR_T) && (SIZEOF_WCHAR_T > 2)
      || (ucs >= 0x20000 && ucs <= 0x2fffd)
      || (ucs >= 0x30000 && ucs <= 0x3fffd)
#endif
      ));
}

/*end*/
