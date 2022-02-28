/* -*- mode: c; indent-width: 4; -*- */
/*
   win32 tty/keys implementation

        #include "../lib/tty/key.h"

        void        init_key (void);
        void        init_key_input_fd (void);
        void        done_key (void);

        gboolean    define_sequence (int code, const char *seq, int action);

        long        lookup_key (const char *name, char **label);
        char *      lookup_key_by_code (const int keycode);

        void        add_select_channel (int fd, select_fn callback, void *info);
        void        delete_select_channel (int fd);
        void        remove_select_channel (int fd);

        void        channels_up (void);
        void        channels_down (void);

        void        load_xtra_key_defines (void);

        char *      learn_key (void);

        int         tty_get_event (struct Gpm_Event *event, gboolean redo_event, gboolean block);
        gboolean    is_idle (void);
        int         tty_getch (void);

        int         get_key_code (int nodelay);

        void        numeric_keypad_mode (void);
        void        application_keypad_mode (void);

        void        enable_bracketed_paste (void);
        void        disable_bracketed_paste (void);

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

#define _WIN32_WINNT 0x500
#include <config.h>

#include "libw32.h"
#include "libw32/termemu_vio.h"                 /* vio driver */

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include "win32_trace.h"

#if defined(__WATCOMC__)
_WCRTLINK extern char volatile __WD_Present;    /* debugger present? */
#if defined(_M_IX86)
extern void                 EnterDebugger(void);
#pragma aux EnterDebugger = "int 3"
#define TryEnterDebugger()  __TryEnterDebugger()
static  int __TryEnterDebugger() {
    if (__WD_Present) {
        EnterDebugger();
        return 1; //triggereds
    }
    return 0; //non-available
}
#endif
#endif  /*__WATCOMC__*/

#include "lib/global.h"

#include "lib/vfs/vfs.h"                        /* vfs_timeout_handler */
#include "lib/widget.h"                         /* mc_refresh() */

#include "lib/tty/color.h"                      /* tty_use_256colors(); */
#include "lib/tty/tty.h"
#include "lib/tty/mouse.h"
#include "lib/tty/key.h"
#include "lib/tty/win.h"

#include "src/args.h"                           /* mc_args__nomouse */
#if (VERSION_3 >= 27)
#include "src/filemanager/filemanager.h"        /* left/right panel */
#else
#include "src/filemanager/midnight.h"
#endif

extern gboolean             quit_cmd_internal (int quiet);
extern gboolean             confirm_exit;

extern void                 sigintr_set (int state);  /* tty.c */

#include "win32_key.h"


/*
 *  WIN32 Keyboard mapping table:
 *
 *  Notes:
 *      Enhanced keys for the IBM 101- and 102-key keyboards are the INS,
 *      DEL, HOME, END, PAGE UP, PAGE DOWN, and direction keys in the
 *      clusters to the left of the keypad; and the divide (/) and
 *      ENTER keys in the keypad.
 */

#define W32KEYS     ((int)(sizeof(w32Keys)/sizeof(w32Keys[0])))

static const struct {
    WORD            vk;                         /* windows virtual key code */
    int             mods;                       /* modifiers */
#define MOD_ALL             -1
#define MOD_ENHANCED        -2
#define MOD_FUNC            -3
    const char *    desc;                       /* description */
    int             key;                        /* interval key value */

} w32Keys[] = {
    { VK_PAUSE,         0,              "Complete",     ALT('\t') },
    { VK_F7,            KEY_M_ALT,      "FileFind",     ALT('?') },
    { VK_BACK,          MOD_ALL,        "Back",         KEY_BACKSPACE },
    { VK_TAB,           MOD_ALL,        "TAB",          '\t' },
    { VK_ESCAPE,        MOD_ALL,        "ESC",          ESC_CHAR },
    { VK_RETURN,        MOD_ALL,        "RETURN",       '\n' },
    { VK_PRIOR,         MOD_ALL,        "PRIOR",        KEY_PPAGE },
    { VK_NEXT,          MOD_ALL,        "NEXT",         KEY_NPAGE },
    { VK_END,           MOD_ALL,        "END",          KEY_END },
    { VK_HOME,          MOD_ALL,        "HOME",         KEY_HOME },
    { VK_LEFT,          MOD_ALL,        "LEFT",         KEY_LEFT },
    { VK_UP,            MOD_ALL,        "UP",           KEY_UP },
    { VK_RIGHT,         MOD_ALL,        "RIGHT",        KEY_RIGHT },
    { VK_DOWN,          MOD_ALL,        "DOWN",         KEY_DOWN },
    { VK_INSERT,        MOD_ALL,        "INSERT",       KEY_IC },
    { VK_DELETE,        MOD_ALL,        "DELETE",       KEY_DC },
    { VK_SUBTRACT,      MOD_ALL,        "-",            KEY_KP_SUBTRACT },
    { VK_MULTIPLY,      MOD_ALL,        "*",            KEY_KP_MULTIPLY },
    { VK_ADD,           MOD_ALL,        "+",            KEY_KP_ADD },
    { VK_F1,            MOD_FUNC,       "F1",           KEY_F(1) },
    { VK_F2,            MOD_FUNC,       "F2",           KEY_F(2) },
    { VK_F3,            MOD_FUNC,       "F3",           KEY_F(3) },
    { VK_F4,            MOD_FUNC,       "F4",           KEY_F(4) },
    { VK_F5,            MOD_FUNC,       "F5",           KEY_F(5) },
    { VK_F6,            MOD_FUNC,       "F6",           KEY_F(6) },
    { VK_F7,            MOD_FUNC,       "F7",           KEY_F(7) },
    { VK_F8,            MOD_FUNC,       "F8",           KEY_F(8) },
    { VK_F9,            MOD_FUNC,       "F9",           KEY_F(9) },
    { VK_F10,           MOD_FUNC,       "F10",          KEY_F(10) },
    { VK_F11,           MOD_FUNC,       "F11",          KEY_F(11) },
    { VK_F12,           MOD_FUNC,       "F12",          KEY_F(12) },
    { VK_F13,           MOD_FUNC,       "F13",          KEY_F(13) },
    { VK_F14,           MOD_FUNC,       "F14",          KEY_F(14) },
    { VK_F15,           MOD_FUNC,       "F15",          KEY_F(15) },
    { VK_F16,           MOD_FUNC,       "F16",          KEY_F(16) },
    { VK_F17,           MOD_FUNC,       "F17",          KEY_F(17) },
    { VK_F18,           MOD_FUNC,       "F18",          KEY_F(18) },
    { VK_F19,           MOD_FUNC,       "F19",          KEY_F(19) },
    { VK_F20,           MOD_FUNC,       "F20",          KEY_F(20) }
    };

/*
 *  source: lib/tty/key.c
 *
 *      This table is a mapping between names and the constants we use
 *      We use this to allow users to define alternate definitions for
 *      certain keys that may be missing from the terminal database.
 */
const key_code_name_t   key_name_conv_tab[] = {
    /*
     *  KEY_F(0) is not here, since we are mapping it to f10, so there is no reason
     *  to define f0 as well. Also, it makes Learn keys a bunch of problems :(
     */
    { KEY_F (1),        "f1",           N_("Function key 1"),       "F1" },
    { KEY_F (2),        "f2",           N_("Function key 2"),       "F2" },
    { KEY_F (3),        "f3",           N_("Function key 3"),       "F3" },
    { KEY_F (4),        "f4",           N_("Function key 4"),       "F4" },
    { KEY_F (5),        "f5",           N_("Function key 5"),       "F5" },
    { KEY_F (6),        "f6",           N_("Function key 6"),       "F6" },
    { KEY_F (7),        "f7",           N_("Function key 7"),       "F7" },
    { KEY_F (8),        "f8",           N_("Function key 8"),       "F8" },
    { KEY_F (9),        "f9",           N_("Function key 9"),       "F9" },
    { KEY_F (10),       "f10",          N_("Function key 10"),      "F10" },
    { KEY_F (11),       "f11",          N_("Function key 11"),      "F11" },
    { KEY_F (12),       "f12",          N_("Function key 12"),      "F12" },
    { KEY_F (13),       "f13",          N_("Function key 13"),      "F13" },
    { KEY_F (14),       "f14",          N_("Function key 14"),      "F14" },
    { KEY_F (15),       "f15",          N_("Function key 15"),      "F15" },
    { KEY_F (16),       "f16",          N_("Function key 16"),      "F16" },
    { KEY_F (17),       "f17",          N_("Function key 17"),      "F17" },
    { KEY_F (18),       "f18",          N_("Function key 18"),      "F18" },
    { KEY_F (19),       "f19",          N_("Function key 19"),      "F19" },
    { KEY_F (20),       "f20",          N_("Function key 20"),      "F20" },
    { KEY_BACKSPACE,    "backspace",    N_("Backspace key"),        "Backspace" },
    { KEY_END,          "end",          N_("End key"),              "End" },
    { KEY_UP,           "up",           N_("Up arrow key"),         "Up" },
    { KEY_DOWN,         "down",         N_("Down arrow key"),       "Down" },
    { KEY_LEFT,         "left",         N_("Left arrow key"),       "Left" },
    { KEY_RIGHT,        "right",        N_("Right arrow key"),      "Right" },
    { KEY_HOME,         "home",         N_("Home key"),             "Home" },
    { KEY_NPAGE,        "pgdn",         N_("Page Down key"),        "PgDn" },
    { KEY_PPAGE,        "pgup",         N_("Page Up key"),          "PgUp" },
    { KEY_IC,           "insert",       N_("Insert key"),           "Ins" },
    { KEY_DC,           "delete",       N_("Delete key"),           "Del" },
    { ALT ('\t'),       "complete",     N_("Completion/M-tab"),     "Meta-Tab" },
    { KEY_BTAB,         "backtab",      N_("Back Tabulation S-tab"), "Shift-Tab" },
    { KEY_KP_ADD,       "kpplus",       N_("+ on keypad"),          "+" },
    { KEY_KP_SUBTRACT,  "kpminus",      N_("- on keypad"),          "-" },
    { (int) '/',        "kpslash",      N_("Slash on keypad"),      "/" },
    { KEY_KP_MULTIPLY,  "kpasterisk",   N_("* on keypad"),          "*" },

    /* From here on, these won't be shown in Learn keys (no space) */
    { ESC_CHAR,         "escape",       N_("Escape key"),           "Esc" },
    { KEY_LEFT,         "kpleft",       N_("Left arrow keypad"),    "Left" },
    { KEY_RIGHT,        "kpright",      N_("Right arrow keypad"),   "Right" },
    { KEY_UP,           "kpup",         N_("Up arrow keypad"),      "Up" },
    { KEY_DOWN,         "kpdown",       N_("Down arrow keypad"),    "Down" },
    { KEY_HOME,         "kphome",       N_("Home on keypad"),       "Home" },
    { KEY_END,          "kpend",        N_("End on keypad"),        "End" },
    { KEY_NPAGE,        "kpnpage",      N_("Page Down keypad"),     "PgDn" },
    { KEY_PPAGE,        "kpppage",      N_("Page Up keypad"),       "PgUp" },
    { KEY_IC,           "kpinsert",     N_("Insert on keypad"),     "Ins" },
    { KEY_DC,           "kpdelete",     N_("Delete on keypad"),     "Del" },
    { (int) '\n',       "kpenter",      N_("Enter on keypad"),      "Enter" },
    { KEY_F (21),       "f21",          N_("Function key 21"),      "F21" },
    { KEY_F (22),       "f22",          N_("Function key 22"),      "F22" },
    { KEY_F (23),       "f23",          N_("Function key 23"),      "F23" },
    { KEY_F (24),       "f24",          N_("Function key 24"),      "F24" },
    { KEY_A1,           "a1",           N_("A1 key"), "A1" },
    { KEY_C1,           "c1",           N_("C1 key"), "C1" },

    /* Alternative label */
    { ESC_CHAR,         "esc",          N_("Escape key"), "Esc" },
    { KEY_BACKSPACE,    "bs",           N_("Backspace key"), "Backspace" },
    { KEY_IC,           "ins",          N_("Insert key"), "Ins" },
    { KEY_DC,           "del",          N_("Delete key"), "Del" },
    { (int) '+',        "plus",         N_("Plus"), "+" },
    { (int) '-',        "minus",        N_("Minus"), "-" },
    { (int) '*',        "asterisk",     N_("Asterisk"), "*" },
    { (int) '.',        "dot",          N_("Dot"), "." },
    { (int) '<',        "lt",           N_("Less than"), "<" },
    { (int) '>',        "gt",           N_("Great than"), ">" },
    { (int) '=',        "equal",        N_("Equal"), "=" },
    { (int) ',',        "comma",        N_("Comma"), "," },
    { (int) '\'',       "apostrophe",   N_("Apostrophe"), "\'" },
    { (int) ':',        "colon",        N_("Colon"), ":" },
    { (int) '!',        "exclamation",  N_("Exclamation mark"), "!" },
    { (int) '?',        "question",     N_("Question mark"), "?" },
    { (int) '&',        "ampersand",    N_("Ampersand"), "&" },
    { (int) '$',        "dollar",       N_("Dollar sign"), "$" },
    { (int) '"',        "quota",        N_("Quotation mark"), "\"" },
    { (int) '%',        "percent",      N_("Percent sign"), "%" },
    { (int) '^',        "caret",        N_("Caret"), "^" },
    { (int) '~',        "tilda",        N_("Tilda"), "~" },
    { (int) '`',        "prime",        N_("Prime"), "`" },
    { (int) '_',        "underline",    N_("Underline"), "_" },
    { (int) '_',        "understrike",  N_("Understrike"), "_" },
    { (int) '|',        "pipe",         N_("Pipe"), "|" },
    { (int) '(',        "lparenthesis", N_("Left parenthesis"), "(" },
    { (int) ')',        "rparenthesis", N_("Right parenthesis"), ")" },
    { (int) '[',        "lbracket",     N_("Left bracket"), "[" },
    { (int) ']',        "rbracket",     N_("Right bracket"), "]" },
    { (int) '{',        "lbrace",       N_("Left brace"), "{" },
    { (int) '}',        "rbrace",       N_("Right brace"), "}" },
    { (int) '\n',       "enter",        N_("Enter"), "Enter" },
    { (int) '\t',       "tab",          N_("Tab key"), "Tab" },
    { (int) ' ',        "space",        N_("Space key"), "Space" },
    { (int) '/',        "slash",        N_("Slash key"), "/" },
    { (int) '\\',       "backslash",    N_("Backslash key"), "\\" },
    { (int) '#',        "number",       N_("Number sign #"), "#" },
    { (int) '#',        "hash",         N_("Number sign #"), "#" },
    /* TRANSLATORS: Please translate as in "at sign" (@). */
    { (int) '@',        "at",           N_("At sign"), "@" },

    /* meta keys */
    { KEY_M_CTRL,       "control",      N_("Ctrl"),     "C" },
    { KEY_M_CTRL,       "ctrl",         N_("Ctrl"),     "C" },
    { KEY_M_ALT,        "meta",         N_("Alt"),      "M" },
    { KEY_M_ALT,        "alt",          N_("Alt"),      "M" },
    { KEY_M_ALT,        "ralt",         N_("Alt"),      "M" },
    { KEY_M_SHIFT,      "shift",        N_("Shift"),    "S" },

    { 0, NULL, NULL, NULL }
    };


static HANDLE           hConsoleIn;
static HANDLE           hConsoleOut;
static HANDLE           hWindow;
static HHOOK            hHook;

static int              consoleMouse = 1;
static DWORD            consoleMode = (DWORD) -1;
static DWORD            consoleOutMode = (DWORD) -1;
static DWORD            consoleAttribute = (DWORD) -1;

static int              disabled_channels;

static unsigned         ctrlbreak_triggered;
static unsigned         ctrlbreak_running;
static unsigned         ctrlc_triggered;

int                     mou_auto_repeat     = 100;          /* setup.c */
int                     double_click_speed  = 250;          /* setup.c */
gboolean                old_esc_mode        = 0;            /* setup.c */
int                     old_esc_mode_timeout = 1000000;     /* setup.c */
int                     use_8th_bit_as_meta = 1;            /* setup.c */

static enum { KEY_SORTNONE, KEY_SORTBYNAME, KEY_SORTBYCODE}
                        key_conv_tab_order  = KEY_SORTNONE;

static const size_t     key_conv_tab_size   = G_N_ELEMENTS (key_name_conv_tab) - 1;

static const key_code_name_t *
                        key_conv_tab_sorted[G_N_ELEMENTS (key_name_conv_tab) - 1];

typedef DWORD (WINAPI *CancelSynchronousIo_t)(HANDLE hThread);

static HANDLE           primary_thread;
static CancelSynchronousIo_t CancelSynchronousIoFn;

static DWORD WINAPI     CancelSynchronousIoImp(HANDLE hThread);

static BOOL __stdcall   CtrlHandler(DWORD fdwCtrlType);


/*
 *  runtime initialisation
 */
void
init_key (void)
{
    hConsoleIn = GetStdHandle (STD_INPUT_HANDLE);
    hConsoleOut = GetStdHandle (STD_OUTPUT_HANDLE);
    hWindow = GetConsoleWindow();

    mc_global.tty.console_flag = '\001';        /* console save/restore, toggle available */
    tty_reset_prog_mode ();

    if (! CancelSynchronousIoFn) {
        HINSTANCE hKernel32;
                                                /* thread handle; must duplicate as GetCurrentThread() is a pseudo handle */
        DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
            &primary_thread, 0 /*dwDesiredAccess*/, FALSE /*bInheritHandle*/, DUPLICATE_SAME_ACCESS);

                                                /* load CancelSynchronousIo() implementation */
        if (0 == (hKernel32 = LoadLibraryA("Kernel32")) ||
                0 == (CancelSynchronousIoFn =   /* Vista+ */
                        (CancelSynchronousIo_t) GetProcAddress(hKernel32, "CancelSynchronousIo"))) {
            CancelSynchronousIoFn = CancelSynchronousIoImp;
            if (hKernel32) FreeLibrary(hKernel32);
        }
    }
}


static DWORD WINAPI
CancelSynchronousIoImp (HANDLE hThread)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return 0;
}


void
init_key_input_fd (void)
{
}


void
done_key (void)
{
    tty_reset_shell_mode ();
}


gboolean
define_sequence (int code, const char *seq, int action)
{
    return FALSE;
}


void
key_mouse_mode (int mode)
{
    consoleMouse = mode;
}



#if !defined(ENABLE_VIRTUAL_TERMINAL_PROCESSING)
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#if !defined(DISABLE_NEWLINE_AUTO_RETURN)
#define DISABLE_NEWLINE_AUTO_RETURN 0x0008
#endif

void
key_prog_mode (void)
{
    if (hConsoleIn) {
        if ((DWORD)-1 == consoleMode) {         /* save */
            CONSOLE_SCREEN_BUFFER_INFO sbinfo = {sizeof(CONSOLE_SCREEN_BUFFER_INFO)};
            if (GetConsoleScreenBufferInfo(hConsoleOut, &sbinfo)) {
                consoleAttribute = sbinfo.wAttributes;
            }
            GetConsoleMode(hConsoleOut, &consoleOutMode);
            GetConsoleMode(hConsoleIn, &consoleMode);
        }

        if (consoleMouse) {                     /* mouse enabled? */
            SetConsoleMode(hConsoleIn, ENABLE_EXTENDED_FLAGS|\
                ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT|ENABLE_PROCESSED_INPUT);
                    // Note: Stating ENABLE_EXTENDED_FLAGS disables ENABLE_INSERT_MODE and/or ENABLE_QUICK_EDIT_MODE.
                    //  required for correct mouse operation; restored within key_shell_mode().
        } else {
            SetConsoleMode(hConsoleIn, ENABLE_WINDOW_INPUT|ENABLE_PROCESSED_INPUT);
        }
        SetConsoleCtrlHandler(CtrlHandler, TRUE);
    }
}


void
key_shell_mode (void)
{
    if (hConsoleIn) {
        if ((DWORD)-1 != consoleMode) {         /* restore */
            DWORD dwMode = 0;

            if (GetConsoleMode(hConsoleOut, &dwMode)) {
                DWORD dwNewMode = dwMode & ~DISABLE_NEWLINE_AUTO_RETURN;
                    // Filter DISABLE_NEWLINE_AUTO_RETURN; cmd default.
                    //   cmd seems to explicitly clear when executing in-built commands,
                    //   whereas apps this may not be the case; clear for consistency.

#if (VERSION_3 >= 27)
                if (tty_use_256colors(NULL)) {  /* enable VT100/ANSI */
#else
                if (tty_use_256colors()) {
#endif
                    dwNewMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                }
                if (dwNewMode != dwMode) {
                    (void) SetConsoleMode(hConsoleOut, dwNewMode);
                }
            }

//          if ((DWORD)-1 != consoleAttribute) { // ???
//              SetConsoleTextAttribute(hConsoleOut, consoleAttribute);
//          }
//          SetConsoleMode(hConsoleOut, consoleOutMode);

            SetConsoleMode(hConsoleIn, consoleMode);
        }
        SetConsoleCtrlHandler(NULL, TRUE);
    }
}


/*
 *  Key table support
 */

static int
key_code_comparator_by_name (const void *p1, const void *p2)
{
    const key_code_name_t *n1 = *(const key_code_name_t **) p1;
    const key_code_name_t *n2 = *(const key_code_name_t **) p2;

    return g_ascii_strcasecmp (n1->name, n2->name);
}


static int
key_code_comparator_by_code (const void *p1, const void *p2)
{
    const key_code_name_t *n1 = *(const key_code_name_t **) p1;
    const key_code_name_t *n2 = *(const key_code_name_t **) p2;

    return n1->code - n2->code;
}


static void
lookup_sort (const int order)
{
    register size_t i;

    if (order == key_conv_tab_order)
        return;

    for (i = 0; i < key_conv_tab_size; i++)
        key_conv_tab_sorted[i] = &key_name_conv_tab[i];

    if (order == KEY_SORTBYNAME) {
        qsort ((void *)key_conv_tab_sorted, key_conv_tab_size, sizeof(key_conv_tab_sorted[0]), &key_code_comparator_by_name);

    } else if (order == KEY_SORTBYCODE) {
        qsort ((void *)key_conv_tab_sorted, key_conv_tab_size, sizeof(key_conv_tab_sorted[0]), &key_code_comparator_by_code);
    }

    key_conv_tab_order = order;
}


static int
lookup_keyname (const char *name, int *idx)
{
    if (name[0] != '\0') {
        const key_code_name_t key = { 0, name, NULL, NULL };
        const key_code_name_t *keyp = &key;
        key_code_name_t **res;

        if (name[1] == '\0') {
            *idx = -1;
            return (int) name[0];
        }

        lookup_sort (KEY_SORTBYNAME);

        res = bsearch (&keyp, key_conv_tab_sorted, key_conv_tab_size,
                       sizeof (key_conv_tab_sorted[0]), key_code_comparator_by_name);

        if (res != NULL) {
            *idx = (int) (res - (key_code_name_t **) key_conv_tab_sorted);
            return (*res)->code;
        }
    }

    *idx = -1;
    return 0;
}


static gboolean
lookup_keycode (const long code, int *idx)
{
    if (code != 0) {
        const key_code_name_t key = { code, NULL, NULL, NULL };
        const key_code_name_t *keyp = &key;
        key_code_name_t **res;

        lookup_sort (KEY_SORTBYCODE);

        res = bsearch (&keyp, key_conv_tab_sorted, key_conv_tab_size,
                    sizeof (key_conv_tab_sorted[0]), key_code_comparator_by_code);

        if (res != NULL) {
            *idx = (int) (res - (key_code_name_t **) key_conv_tab_sorted);
            return TRUE;
        }
    }

    *idx = -1;
    return FALSE;
}


/*
 *  Return the code associated with the symbolic name keyname
 */
long
lookup_key (const char *name, char **label)
{
    char **lc_keys, **p;
    int k = -1;
    int key = 0;
    int lc_index = -1;

    int use_meta = -1;
    int use_ctrl = -1;
    int use_shift = -1;

    if (name == NULL)
        return 0;

    name = g_strstrip (g_strdup (name));
    p = lc_keys = g_strsplit_set (name, "-+ ", -1);
    g_free ((char *) name);

    while ((p != NULL) && (*p != NULL)) {
        if ((*p)[0] != '\0') {
            int idx;

            key = lookup_keyname (g_strstrip (*p), &idx);

            if (key == KEY_M_ALT) {
                use_meta  = idx;
            } else if (key == KEY_M_CTRL) {
                use_ctrl  = idx;
            } else if (key == KEY_M_SHIFT) {
                use_shift = idx;
            } else {
                k = key;
                lc_index = idx;
                break;
            }
        }

        p++;
    }

    g_strfreev (lc_keys);

    /* output */
    if (k <= 0)
        return 0;

    if (label != NULL) {
        GString *s;

        s = g_string_new ("");

        if (use_meta != -1) {
            g_string_append (s, key_conv_tab_sorted[use_meta]->shortcut);
            g_string_append_c (s, '-');
        }

        if (use_ctrl != -1) {
            g_string_append (s, key_conv_tab_sorted[use_ctrl]->shortcut);
            g_string_append_c (s, '-');
        }

        if (use_shift != -1) {
            if (k < 127) {
                g_string_append_c (s, (gchar) g_ascii_toupper ((gchar) k));
            } else {
                g_string_append (s, key_conv_tab_sorted[use_shift]->shortcut);
                g_string_append_c (s, '-');
                g_string_append (s, key_conv_tab_sorted[lc_index]->shortcut);
            }

        } else if (k < 128) {
            if ((k >= 'A') || (lc_index < 0) || (key_conv_tab_sorted[lc_index]->shortcut == NULL)) {
                g_string_append_c (s, (gchar) g_ascii_tolower ((gchar) k));
            } else {
                g_string_append (s, key_conv_tab_sorted[lc_index]->shortcut);
            }

        } else if ((lc_index != -1) && (key_conv_tab_sorted[lc_index]->shortcut != NULL)) {
            g_string_append (s, key_conv_tab_sorted[lc_index]->shortcut);

        } else {
            g_string_append_c (s, (gchar) g_ascii_tolower ((gchar) key));
        }

        *label = g_string_free (s, FALSE);
    }

    if (use_shift != -1) {
        if (k < 127 && k > 31)
            k = g_ascii_toupper ((gchar) k);
        else
            k |= KEY_M_SHIFT;
    }

    if (use_ctrl != -1) {
        if (/*k < 256*/ k < 0x1f ||
                (k >= 'a' && k <= 'z') || (k >= 'A' && k <= '^')) {
            k = XCTRL (k);                      /* ctrl-<a..z>, and ctrl-backslash */
        } else {
            k |= KEY_M_CTRL;                    /* ctrl-<fx>, ctrl-space etc */
        }
    }

    if (use_meta != -1)
        k = ALT (k);

    return (long) k;
}


char *
lookup_key_by_code (const int keycode)
{
    unsigned int k = keycode & ~KEY_M_MASK;     /* code without modifier */
    unsigned int mod = keycode & KEY_M_MASK;    /* modifier */

    int use_meta = -1;
    int use_ctrl = -1;
    int use_shift = -1;
    int key_idx = -1;

    GString *s;
    int idx;

    s = g_string_sized_new (8);

    if (lookup_keycode (k, &key_idx) || (k > 0 && k < 256)) {

        if (mod & KEY_M_ALT) {
            if (lookup_keycode (KEY_M_ALT, &idx)) {
                use_meta = idx;
                g_string_append (s, key_conv_tab_sorted[use_meta]->name);
                g_string_append_c (s, '-');
            }
        }

        if (mod & KEY_M_CTRL) {
            /* non printeble chars like a CTRL-[A..Z] */
            if (k < 32)
                k += 64;                        /* A...^, includes ESC & Ctrl-\ */

            if (lookup_keycode (KEY_M_CTRL, &idx)) {
                use_ctrl = idx;
                g_string_append (s, key_conv_tab_sorted[use_ctrl]->name);
                g_string_append_c (s, '-');
            }
        }

        if (mod & KEY_M_SHIFT) {
            if (lookup_keycode (KEY_M_ALT, &idx)) {
                use_shift = idx;
                if (k < 127) {
                    g_string_append_c (s, (gchar) g_ascii_toupper ((gchar) k));
                } else {
                    g_string_append (s, key_conv_tab_sorted[use_shift]->name);
                    g_string_append_c (s, '-');
                    g_string_append (s, key_conv_tab_sorted[key_idx]->name);
                }
            }

        } else if (k < 128) {
            if ((k >= 'A') || (key_idx < 0) || (key_conv_tab_sorted[key_idx]->name == NULL)) {
                g_string_append_c (s, (gchar) k);
            } else {
                g_string_append (s, key_conv_tab_sorted[key_idx]->name);
            }

        } else if ((key_idx != -1) && (key_conv_tab_sorted[key_idx]->name != NULL)) {
            g_string_append (s, key_conv_tab_sorted[key_idx]->name);

        } else {
            g_string_append_c (s, (gchar) keycode);
        }
    }

    return g_string_free (s, s->len == 0);
}


void
channels_down (void)
{
    ++disabled_channels;
}


void
channels_up (void)
{
    if (!disabled_channels) {
        fputs ("Error: channels_up called with disabled_channels = 0\n", stderr);
    }
    --disabled_channels;
}


void
load_xtra_key_defines (void)
{
}


char *
learn_key (void)
{
    return g_strdup ("key_escape");
}


/*
 *  enable keypad strings
 */
void
slang_keypad (int set)
{
    (void)set;
}


/*
 *  Returns a character read from stdin with appropriate interpretation. Also
 *  takes care of generated mouse events.
 *
 *  Returns
 *      EV_MOUSE if it is a mouse event
 *      EV_NONE if non-blocking or interrupt set and nothing was done
 */
static void
check_winch (int check_type)
{
    static char ofont[80];                      /* FIXME: clear */
    static RECT orect;

    if (0 == mc_global.tty.winch_flag) {
        HANDLE hConsole;
        CONSOLE_SCREEN_BUFFER_INFO sbinfo = {0};
        int  rows, cols, nfont = -1, resize = 0;
        char font[sizeof(ofont)];
        RECT rect;

        font[0] = 0;
        win32API_CALL_HANDLE(hConsole, GetStdHandle(STD_OUTPUT_HANDLE));
        win32API_CALL(GetConsoleScreenBufferInfo(hConsole, &sbinfo));
        rows = 1 + sbinfo.srWindow.Bottom - sbinfo.srWindow.Top;
        cols = 1 + sbinfo.srWindow.Right  - sbinfo.srWindow.Left;

        if (rows < VIO_MINROWS) rows = VIO_MINROWS;
        if (cols < VIO_MINCOLS) cols = VIO_MINCOLS;

        if (COLS != cols || LINES != rows) {    /* size change */
            resize = 1;

        } else if (2 == check_type) {           /* check for a possible font change */
            SLtt_get_font(font, sizeof(font));
            nfont = (ofont[0] && (0 != memcmp(font, ofont, sizeof(ofont))));
            win32API_CALL(GetWindowRect(GetConsoleWindow(), &rect));
            if ((orect.right - orect.left) != (rect.right - rect.left) ||
                    (orect.bottom - orect.top) != (rect.bottom - rect.top)) {
                if (orect.bottom) {
                    nfont = 1;
                }
            }
        }

        if (resize || 1 == nfont) {             /* change; cache current */
            ++mc_global.tty.winch_flag;
            if (-1 == nfont) {
                SLtt_get_font(font, sizeof(font));
                win32API_CALL(GetWindowRect(GetConsoleWindow(), &rect));
            }
            (void) memcpy(ofont, font, sizeof(ofont));
            orect = rect;
        }
    }
}


/*
 *  Returns:
 *      Returns the character-code read from the console, otherwise:
 *
 *        o EV_MOUSE - if it is a mouse event
 *        o 0EV_NONE - if non-blocking or interrupt set and nothing was done.
 */
int
tty_get_event(struct Gpm_Event *event, gboolean redo_event, gboolean block)
{
//  static int dirty = 2, clicks = 0;
    static int clicks = 0;

    assert(hConsoleIn);
    if (! hConsoleIn) {
        return 0;                               /* not active */
    }

//  if (block || (dirty >= 3) || is_idle()) {   /* act on winch_flag, otherwise tty_refresh() */
        // There seems to be a display update/key polling issue, whereby some interactive dialogs (copy/move etc)
        // request input prior to dialog rendering is completed; underlying cause/conditions are unclear.
        mc_refresh();
//      dirty = 1;
//  } else {
//      ++dirty; /* update every 3rd poll */
//  }

    if (mc_global.tty.winch_flag) {
        return EV_NONE;
    }

    while (1) {
        DWORD timeoutms = 0;                    /* non-blocking; by default */
        INPUT_RECORD k = {0};
        DWORD count = 0, rc;
        int c = EV_NONE;

#if defined(ENABLE_VFS)
        vfs_timeout_handler ();
#endif

        if (redo_event) {                       /* mouse repeat event */
            timeoutms = (mou_auto_repeat > 0 ? mou_auto_repeat : 100);

        } else if (block) {                     /* blocking request */
#if defined(ENABLE_VFS)
            const int seconds = vfs_timeouts();
            timeoutms = (seconds > 0 ? seconds * 1000 : INFINITE);
#else
            timeoutms = INFINITE;
#endif
        }

        sigintr_set (0);                        /* clear interrupt status */
        rc = WaitForSingleObject(hConsoleIn, timeoutms);

        if (ctrlc_triggered)  {
            ctrlc_triggered = ctrlbreak_triggered = 0;
            errno = EINTR;
            return EV_NONE;

        } else if (ctrlbreak_triggered) {       /* required??? */
            ctrlbreak_triggered = 0;
            if (0 == ctrlbreak_running) {
                const save_confirm_exit = confirm_exit;
                ++ctrlbreak_running;
                confirm_exit = TRUE;
                (void) quit_cmd_internal (FALSE);
                confirm_exit = save_confirm_exit;
                --ctrlbreak_running;
                errno = EINTR;
                return EV_NONE;
            }
        }

#define EV_VOID     -3                          // void event

        if (rc == WAIT_OBJECT_0 &&
                PeekConsoleInput(hConsoleIn, &k, 1, &count) && 1 == count) {

            c = EV_NONE;
            switch (k.EventType) {
            case KEY_EVENT:
                if (k.Event.KeyEvent.bKeyDown) {
                    c = get_key_code(1);        // read and translate key, nonblocking.
                    if (c == (KEY_M_SHIFT | '\n')) {
                        /*
                         *  <Shift-Return> - Toggle screen size.
                         */
                        ++mc_global.tty.winch_flag;
                        SLsmg_togglesize();
                        c = EV_VOID;            // consume
                    }
                } else {
                    (void) ReadConsoleInput(hConsoleIn, &k, 1, &count);
                    check_winch(1);
                    c = EV_VOID;                // consume
                }
                break;

            case MOUSE_EVENT:
                (void) ReadConsoleInput(hConsoleIn, &k, 1, &count);
                if (event && !mc_args__nomouse) {
                    /*
                     *  General mouse events:
                     *
                     *    GPM_MOVE   - A motion event, with all buttons up.
                     *    GPM_DRAG   - A motion event, but one or more buttons are kept pressed.
                     *    GPM_DOWN   - A button press event. The `buttons' field will report which buttons are pressed after the event.
                     *    GPM_UP     - A button release event. The `buttons' field will report which buttons are being released.
                     *                      Note that this is different from the previous case.
                     *
                     *  Theses bits are set at button-press, drag and button release events:
                     *
                     *    GPM_SINGLE - Used to identify a single press.
                     *    GPM_DOUBLE - Used to identify a double click (press, drag, release).
                     *    GPM_TRIPLE - Used to identify a triple click (press, drag, release).
                     */
                    event->x = k.Event.MouseEvent.dwMousePosition.X + 1;
                    event->y = k.Event.MouseEvent.dwMousePosition.Y + 1;
                    event->type = 0;
                    c = EV_MOUSE;

                    switch (k.Event.MouseEvent.dwButtonState) {
                    case 0:
                        event->buttons = 0;
                        break;
                    case FROM_LEFT_1ST_BUTTON_PRESSED:
                        event->buttons = GPM_B_LEFT;
                        break;
                    case RIGHTMOST_BUTTON_PRESSED:
                        event->buttons = GPM_B_RIGHT;
                        break;
                    default:
                        event->buttons = GPM_B_MIDDLE;
                        break;
                    }

                    switch (k.Event.MouseEvent.dwEventFlags) {
                    case 0:
                    case DOUBLE_CLICK:
                        if (event->buttons) {
                            clicks = (k.Event.MouseEvent.dwEventFlags == DOUBLE_CLICK ? 2 : 1);
                            event->type = GPM_DOWN | (2 == clicks ? GPM_DOUBLE : GPM_SINGLE);
                        } else {
                            event->type = GPM_UP   | (2 == clicks ? GPM_DOUBLE : GPM_SINGLE);
                        }
                        break;

                    case MOUSE_MOVED:
                        if (event->buttons) {
                            event->type = GPM_DRAG;
                        } else {
                            event->type = GPM_MOVE;
                        }
                        break;

#ifndef MOUSE_WHEELED
#define MOUSE_WHEELED   4   /*NT and Windows Me/98/95: This value is not supported.*/
#endif

                    case MOUSE_WHEELED:
                        c = (k.Event.MouseEvent.dwButtonState & 0xFF000000 ? KEY_NPAGE : KEY_PPAGE);
                        break;
                    }
                }
                check_winch(1);
                break;

            case WINDOW_BUFFER_SIZE_EVENT:
                (void) ReadConsoleInput(hConsoleIn, &k, 1, &count);
                check_winch(2);
                break;

            case FOCUS_EVENT:
                (void) ReadConsoleInput(hConsoleIn, &k, 1, &count);
                if (k.Event.FocusEvent.bSetFocus) {
#if defined(_MSC_VER)
                    if (! IsDebuggerPresent()) {
                        SLsmg_touch_screen();
                    }
#else
                    SLsmg_touch_screen();
#endif
                }
                check_winch(2);
                break;

            case MENU_EVENT:
                /*
                 *  font changes wont be reported, *unless* the window/buffer size is modified as a result,
                 *      as such force font checks for the number of input iterations.
                 */
                /*FALLTHRU*/

            default:
                (void) ReadConsoleInput(hConsoleIn, &k, 1, &count);
                break;
            }

        } else if (rc == WAIT_FAILED) {
            fprintf (stderr,
                "Console handle no longer valid, assuming EOF on stdin and exiting\n");
            fflush (stderr);
            Beep (600, 200);
            exit (1);
        }

        if (EV_NONE == c || EV_VOID == c) {
            if (redo_event)
                return EV_MOUSE;
            if (! block || tty_got_winch ()) {
                return EV_NONE;
            }
        } else {
            assert(EV_MOUSE == c || c > 0);
            return c;                           // reportable event
        }
    }

    /*NOTREACHED*/
    return EV_NONE;
}


/*  Function:           CtrlHandler
 *      Console control handler.
 *
 *   Description:
 *      When a CTRL_CLOSE_EVENT signal is received, the control handler returns TRUE, causing the
 *      system to display a dialog box that gives the user the choice of terminating the process
 *      and closing the console or allowing the process to continue execution. If the user
 *      chooses not to terminate the process, the system closes the console when the process
 *      finally terminates.
 *
 *      When a CTRL+BREAK, CTRL_LOGOFF_EVENT, or CTRL_SHUTDOWN_EVENT signal is received, the
 *      control handler returns FALSE. Doing this causes the signal to be passed to the next
 *      control handler function. If no other control handlers have been registered or none of
 *      the registered handlers returns TRUE, the default handler will be used, resulting in the
 *      process being terminated
 *
 *  Notes:
 *      Windows 7, Windows 8, Windows 8.1 and Windows 10:
 *
 *      If a console application loads the gdi32.dll or user32.dll library, the HandlerRoutine function
 *      that you specify when you call SetConsoleCtrlHandler does not get called for the CTRL_LOGOFF_EVENT
 *      and CTRL_SHUTDOWN_EVENT events. The operating system recognizes processes that load gdi32.dll
 *      or user32.dll as Windows applications rather than console applications. This behavior also occurs
 *      for console applications that do not call functions in gdi32.dll or user32.dll directly,
 *      but do call functions such as Shell functions that do in turn call functions in gdi32.dll or user32.dll.
 *
 *      To receive events when a user signs out or the device shuts down in these circumstances,
 *      create a hidden window in your console application, and then handle the WM_QUERYENDSESSION and
 *      WM_ENDSESSION window messages that the hidden window receives. You can create a hidden window
 *      by calling the CreateWindowEx method with the dwExStyle parameter set to 0.
 */
static BOOL __stdcall
CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType) {
    case CTRL_C_EVENT:
        sigintr_set (1);                        /* see: tty_got_interrupt() / tty.c */
        ++ctrlc_triggered;

        if (CancelSynchronousIoFn) {            /* attempt to cancel current i/o operation */
            const DWORD rc = CancelSynchronousIoFn(primary_thread);
                /* Marks pending synchronous I / O operations that are issued by the specified thread as canceled -- nonblocking */
            if (0 == rc && ERROR_NOT_FOUND == GetLastError()) {
                ++ctrlc_triggered;
                    /* nothing to cancel; ++ pure diagnostics only */
            }
        }

     /* XXX:
      * if (child_process) {
      *     GenerateConsoleCtrlEvent(child_process, CTRL_C_EVENT);
      * }
      */
        SetEvent(hConsoleIn);
        return TRUE;

    case CTRL_BREAK_EVENT:
#if defined(TryEnterDebugger)
        if (! TryEnterDebugger()) {
            ++ctrlbreak_triggered;
            return TRUE;
        }
#endif
        ++ctrlbreak_triggered;
        SetEvent(hConsoleIn);
        return TRUE;

    case CTRL_CLOSE_EVENT:
        Beep (600, 200);
        quiet_quit_cmd ();
        return FALSE;

    case CTRL_SHUTDOWN_EVENT:
    case CTRL_LOGOFF_EVENT:
        quiet_quit_cmd ();
        return FALSE;

    default:
        break;
    }
    return FALSE;
}


/*
 *  Translate the key press into a CRISP identifier.
 */
static int
key_mapwin32 (unsigned long dwCtrlKeyState, unsigned wVirtKeyCode, unsigned AsciiChar)
{
    int mod = 0, ch = -1;
    int i;

    /* Modifiers */
    if (dwCtrlKeyState &
            (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) {
        mod |= KEY_M_ALT;
    }

    if (dwCtrlKeyState &
            (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
        mod |= KEY_M_CTRL;
    }

    if (dwCtrlKeyState & (SHIFT_PRESSED)) {
        mod |= KEY_M_SHIFT;
    }

    /* Virtual keys */
    for (i = W32KEYS-1; i >= 0; i--) {
        if (w32Keys[i].vk == wVirtKeyCode &&
                ((w32Keys[i].mods == MOD_ALL || w32Keys[i].mods == MOD_FUNC) ||
                    (w32Keys[i].mods == MOD_ENHANCED && (dwCtrlKeyState & (ENHANCED_KEY))) ||
                    (w32Keys[i].mods >= 0 && w32Keys[i].mods == mod) )) {

            if ((ch = w32Keys[i].key) >= 0) {
                                                /* apply modifiers */
                if (w32Keys[i].mods == MOD_FUNC) {
                    if (ch >= KEY_F(1) && ch <= KEY_F(10) && (mod == KEY_M_SHIFT)) {
                        /* convert Shift+Fn to F(n+10) */
                        ch += 10;

                    } else {
                        /* otherwise, apply ignoring Shift */
                        ch |= (mod & ~KEY_M_SHIFT);
                    }

                } else if (w32Keys[i].mods == MOD_ALL) {
                    if (ch == (31 & 'd')) {
                        /* Ctrl-d is delete */
                        ch = KEY_DC;
                        mod &= ~KEY_M_CTRL;

                    } else if (ch == (31 & 'h')) {
                        /* Ctrl-h is backspace */
                        ch = KEY_BACKSPACE;
                        mod &= ~KEY_M_CTRL;

                    } else if (ch == KEY_BACKSPACE && (mod & KEY_M_SHIFT)) {
                        /* Shift+BackSpace is backspace */
                        mod &= ~KEY_M_SHIFT;
                    }

                    ch |= mod;
                }
            }
            break;
        }
    }

    /* Convert alt-digits to F-keys */
    if (-1 == ch && KEY_M_ALT == mod) {
        if (AsciiChar >= '0' && AsciiChar <= '9') {
            if (AsciiChar >= '1') {
                ch = KEY_F(AsciiChar - '1');    /* F1..F9 */
            } else {
                ch = KEY_F(10);                 /* F10 */
            }
            mod = 0;
        }
    }

    /* Ascii */
    if (-1 == ch && (AsciiChar & 0xff)) {
        ch = AsciiChar;                         /* ASCII value */
        ch |= (mod & ~KEY_M_SHIFT);             /* .. and modifiers (ignore shift) */
    }

    return ch;
}


int
get_key_code (int no_delay)
{
    int retry = 0;

    do {
        INPUT_RECORD k = {0};
        DWORD count = 0, rc;

        retry = 0;
        rc = WaitForSingleObject(hConsoleIn, no_delay ? 0 : INFINITE);
        if (rc == WAIT_OBJECT_0 &&
                ReadConsoleInput(hConsoleIn, &k, 1, &count) && 1 == count) {
            switch (k.EventType) {
            case KEY_EVENT:
                if (k.Event.KeyEvent.bKeyDown) {
                    const KEY_EVENT_RECORD *pKey = &k.Event.KeyEvent;
                    int c;

                    if ((c = key_mapwin32(pKey->dwControlKeyState,
                                pKey->wVirtualKeyCode, pKey->uChar.AsciiChar)) != -1) {
                        return c;
                    }
                } else {
                    retry = 1;                  /* consume */
                }
                check_winch(1);
                break;
            default:
                check_winch(1);
                break;
            }
        }
    } while (retry || no_delay == 0);

    return -1;
}


int
tty_getch (void)
{
    int key;

    while ((key = tty_get_event (NULL, 0, TRUE)) == EV_NONE) {
        ;
    }
    return key;
}


/*
 *  Check if we are idle, i.e. there are no pending keyboard or mouse events.
 *  Return 1 is idle, 0 is there are pending events.
 */
gboolean
is_idle (void)
{
    extern gboolean mc_args__nomouse;           /* args.c */
    DWORD count = 0;
    INPUT_RECORD k;

    while (hConsoleIn && WaitForSingleObject(hConsoleIn, 0 /*NONBLOCKING*/) == WAIT_OBJECT_0 &&
                PeekConsoleInput(hConsoleIn, &k, 1, &count) && count == 1) {
        if (KEY_EVENT == k.EventType) {
            if (!k.Event.KeyEvent.bKeyDown) {
                //
                //  key-up, consume
                //
                (void) ReadConsoleInput(hConsoleIn, &k, 1, &count);
                continue;
            }
        } else if (MOUSE_EVENT == k.EventType) {
            if (mc_args__nomouse ||
                    (0 == k.Event.MouseEvent.dwButtonState && MOUSE_MOVED == k.Event.MouseEvent.dwEventFlags)) {
                //
                //  nomouse or mouse-move only, consume
                //
                (void) ReadConsoleInput(hConsoleIn, &k, 1, &count);
                continue;
            }
        }
        return FALSE;
    }
    return TRUE;
}


/*
 *  set keypad to numeric or application mode. Only in application keypad mode
 *  it's possible to distinguish the '+' key and the '+' on the keypad
 *  ('*' and '-' ditto).
 */
void
numeric_keypad_mode (void)
{
}


void
application_keypad_mode (void)
{
}

/* ----------------------------------------------------------------------------------- */

gboolean bracketed_pasting_in_progress = FALSE;

void
enable_bracketed_paste (void)
{
}

void
disable_bracketed_paste (void)
{
}

/*end*/
