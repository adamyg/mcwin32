#include <edidentifier.h>
__CIDENT_RCSID(kbmap_c,"$Id: kbmap.c,v 1.9 2024/02/27 17:18:08 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * libtermemu console driver
 *
 * Copyright (c) 2015 - 2024, Adam Young.
 * All rights reserved.
 *
 * This file is part of the WinXSH project.
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
 * License for more details.
 * ==end==
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "kbvirtualextra.h"
#include "kbmap.h"
#include "kbutil.h"

struct xkb {
#define VK_VOID                         0xffff
#define __VKVALUE(__v)                  __v
#define VK(__v)                         { __VKVALUE(VK_##__v), VK_VOID, 0,   #__v }
#define VKN(__v,__n)                    { __VKVALUE(VK_##__v), VK_VOID, 0,   #__v, __n }
#define VKALT(__v,__a)                  { __VKVALUE(VK_##__v), VK_VOID, __a, #__v }
#define VKALTN(__v,__a,__n)             { __VKVALUE(VK_##__v), VK_VOID, __a, #__v, __n }
#define VKSCAN(__v,__s)                 { __VKVALUE(VK_##__v), __s,     0,   #__v }
#define VKSCANALT(__v,__s,__a)          { __VKVALUE(VK_##__v), __s,     __a, #__v }
#define VKSCANALTN(__v,__s,__a,__n)     { __VKVALUE(VK_##__v), __s,     __a, #__v, __n }

	WORD wVirtualKeyCode;                                       // virtual key code.
	WORD wVirtualScanCode;                                      // optional scan-code.
	WORD vkext;                                                 // alternative VK_ specials.
	const char *vkname;                                         // VK name.
	const char *kblabel;                                        // XKB optional label.
};

/*
 *  Standard key table.
 */
static const struct xkb standard[] = {
	VK(LBUTTON),                                                // 0x01 - Left mouse button
	VK(RBUTTON),                                                // 0x02 - Right mouse button
	VK(CANCEL),                                                 // 0x03 - Control-break processing
	VK(MBUTTON),                                                // 0x04 - Middle mouse button
	VK(XBUTTON1),                                               // 0x05 - X1 mouse button
	VK(XBUTTON2),                                               // 0x06 - X2 mouse button
	    // 0x07 -- Reserved
	VKN(BACK,                       "BackSpace"),               // 0x08 - BACKSPACE key
	VKN(TAB,                        "Tab"),                     // 0x09 - TAB key
	    // 0x0A-0B -- Reserved
	VKN(CLEAR,                      "Clear"),                   // 0x0C - CLEAR key
	VKN(RETURN,                     "Return"),                  // 0x0D - ENTER key
	    // 0x0E-0F -- Unassigned
	VKSCANALTN(SHIFT, 0x002a, VK_LSHIFT, "Shift_L"),            // 0x10 - SHIFT key
	VKSCANALTN(SHIFT, 0x0036, VK_RSHIFT, "Shift_R"),
	VK(SHIFT),
	VKALT(CONTROL, VK_LCONTROL),                                // 0x11 - CTRL key
	VKALT(MENU, VK_LMENU),                                      // 0x12 - ALT key
	VKN(PAUSE,                      "Pause"),                   // 0x13 - PAUSE key
	VKN(CAPITAL,                    "Caps_Lock"),               // 0x14 - CAPS LOCK key
	VK(KANA),                                                   // 0x15 - IME Kana mode
	VK(HANGUL),                                                 // 0x15 - IME Hangul mode (Korean)
#if defined(VK_IME_ON)
	VK(IME_ON),                                                 // 0x16 - IME On
#endif
	VK(JUNJA),                                                  // 0x17 - IME Junja mode
	VK(FINAL),                                                  // 0x18 - IME final mode
	VK(HANJA),                                                  // 0x19 - IME Hanja mode (Korean)
	VK(KANJI),                                                  // 0x19 - IME Kanji mode 
#if defined(IME_OFF)
	VK(IME_OFF),                                                // 0x1A - IME Off
#endif
	VKN(ESCAPE,                     "Escape"),                  // 0x1B - ESC key
	VK(CONVERT),                                                // 0x1C - IME convert
	VK(NONCONVERT),                                             // 0x1D - IME nonconvert
	VK(ACCEPT),                                                 // 0x1E - IME accept
	VK(MODECHANGE),                                             // 0x1F - IME mode change request
	VKN(SPACE,                      "Space"),                   // 0x20 - SPACEBAR
	VK(PRIOR),                                                  // 0x21 - PAGE UP key
	VK(NEXT),                                                   // 0x22 - PAGE DOWN key
	VK(END),                                                    // 0x23 - END key
	VK(HOME),                                                   // 0x24 - HOME key
	VK(LEFT),                                                   // 0x25 - LEFT ARROW key
	VK(UP),                                                     // 0x26 - UP ARROW key
	VK(RIGHT),                                                  // 0x27 - RIGHT ARROW key
	VK(DOWN),                                                   // 0x28 - DOWN ARROW key
	VK(SELECT),                                                 // 0x29 - SELECT key
	VK(PRINT),                                                  // 0x2A - PRINT key
	VK(EXECUTE),                                                // 0x2B - EXECUTE key
	VK(SNAPSHOT),                                               // 0x2C - PRINT SCREEN key
	VK(INSERT),                                                 // 0x2D - INS key
	VK(DELETE),                                                 // 0x2E - DEL key
	VK(HELP),                                                   // 0x2F - HELP key
//	VK(0),                                                      // 0x30 - 0 key
//	VK(1),                                                      // 0x31 - 1 key
//	VK(2),                                                      // 0x32 - 2 key
//	VK(3),                                                      // 0x33 - 3 key
//	VK(4),                                                      // 0x34 - 4 key
//	VK(5),                                                      // 0x35 - 5 key
//	VK(6),                                                      // 0x36 - 6 key
//	VK(7),                                                      // 0x37 - 7 key
//	VK(8),                                                      // 0x38 - 8 key
//	VK(9),                                                      // 0x39 - 9 key
	    // 0x3A-40 -- Undefined
//	VK(A),                                                      // 0x41 - A key
//	VK(B),                                                      // 0x42 - B key
//	VK(C),                                                      // 0x43 - C key
//	VK(D),                                                      // 0x44 - D key
//	VK(E),                                                      // 0x45 - E key
//	VK(F),                                                      // 0x46 - F key
//	VK(G),                                                      // 0x47 - G key
//	VK(H),                                                      // 0x48 - H key
//	VK(I),                                                      // 0x49 - I key
//	VK(J),                                                      // 0x4A - J key
//	VK(K),                                                      // 0x4B - K key
//	VK(L),                                                      // 0x4C - L key
//	VK(M),                                                      // 0x4D - M key
//	VK(N),                                                      // 0x4E - N key
//	VK(O),                                                      // 0x4F - O key
//	VK(P),                                                      // 0x50 - P key
//	VK(Q),                                                      // 0x51 - Q key
//	VK(R),                                                      // 0x52 - R key
//	VK(S),                                                      // 0x53 - S key
//	VK(T),                                                      // 0x54 - T key
//	VK(U),                                                      // 0x55 - U key
//	VK(V),                                                      // 0x56 - V key
//	VK(W),                                                      // 0x57 - W key
//	VK(X),                                                      // 0x58 - X key
//	VK(Y),                                                      // 0x59 - Y key
//	VK(Z),                                                      // 0x5A - Z key
//	VK(LWIN),                                                   // 0x5B - Left Windows key
//	VK(RWIN),                                                   // 0x5C - Right Windows key
//	VK(APPS),                                                   // 0x5D - Applications key
	VK(SLEEP),                                                  // 0x5F - Computer Sleep key
	VKN(NUMPAD0,                    "KP_0"),                    // 0x60 - Numeric keypad 0 key
	VKN(NUMPAD1,                    "KP_1"),                    // 0x61 - Numeric keypad 1 key
	VKN(NUMPAD2,                    "KP_2"),                    // 0x62 - Numeric keypad 2 key
	VKN(NUMPAD3,                    "KP_3"),                    // 0x63 - Numeric keypad 3 key
	VKN(NUMPAD4,                    "KP_4"),                    // 0x64 - Numeric keypad 4 key
	VKN(NUMPAD5,                    "KP_5"),                    // 0x65 - Numeric keypad 5 key
	VKN(NUMPAD6,                    "KP_6"),                    // 0x66 - Numeric keypad 6 key
	VKN(NUMPAD7,                    "KP_7"),                    // 0x67 - Numeric keypad 7 key
	VKN(NUMPAD8,                    "KP_8"),                    // 0x68 - Numeric keypad 8 key
	VKN(NUMPAD9,                    "KP_9"),                    // 0x69 - Numeric keypad 9 key
	VKN(MULTIPLY,                   "KP_Multiply"),             // 0x6A - Multiply key
	VKN(ADD,                        "KP_Add"),                  // 0x6B - Add key
	VKN(SEPARATOR,                  "KP_Separator"),            // 0x6C - Separator key
	VKN(SUBTRACT,                   "KP_Subtract"),             // 0x6D - Subtract key
	VKN(DECIMAL,                    "KP_Decimal"),              // 0x6E - Decimal key
	VKN(DIVIDE,                     "KP_Divide"),               // 0x6F - Divide key
	VKN(F1,                         "F1"),                      // 0x70 - F1 key
	VKN(F2,                         "F2"),                      // 0x71 - F2 key
	VKN(F3,                         "F3"),                      // 0x72 - F3 key
	VKN(F4,                         "F4"),                      // 0x73 - F4 key
	VKN(F5,                         "F5"),                      // 0x74 - F5 key
	VKN(F6,                         "F6"),                      // 0x75 - F6 key
	VKN(F7,                         "F7"),                      // 0x76 - F7 key
	VKN(F8,                         "F8"),                      // 0x77 - F8 key
	VKN(F9,                         "F9"),                      // 0x78 - F9 key
	VKN(F10,                        "F10"),                     // 0x79 - F10 key
	VKN(F11,                        "F11"),                     // 0x7A - F11 key
	VKN(F12,                        "F12"),                     // 0x7B - F12 key
	VKN(F13,                        "F13"),                     // 0x7C - F13 key
	VKN(F14,                        "F14"),                     // 0x7D - F14 key
	VKN(F15,                        "F15"),                     // 0x7E - F15 key
	VKN(F16,                        "F16"),                     // 0x7F - F16 key
	VKN(F17,                        "F17"),                     // 0x80 - F17 key
	VKN(F18,                        "F18"),                     // 0x81 - F18 key
	VKN(F19,                        "F19"),                     // 0x82 - F19 key
	VKN(F20,                        "F20"),                     // 0x83 - F20 key
	VKN(F21,                        "F21"),                     // 0x84 - F21 key
	VKN(F22,                        "F22"),                     // 0x85 - F22 key
	VKN(F23,                        "F23"),                     // 0x86 - F23 key
	VKN(F24,                        "F24"),                     // 0x87 - F24 key
	    // 0x88-8F -- Reserved
//	VK(NUMLOCK),                                                // 0x90 - NUM LOCK key
//	VK(SCROLL),                                                 // 0x91 - SCROLL LOCK key
	    // 0x92-96 -- OEM specific
	    // 0x97-9F -- Unassigned
//	VK(LSHIFT),                                                 // 0xA0 - Left SHIFT key
//	VK(RSHIFT),                                                 // 0xA1 - Right SHIFT key
//	VK(LCONTROL),                                               // 0xA2 - Left CONTROL key
//	VK(RCONTROL),                                               // 0xA3 - Right CONTROL key
//	VK(LMENU),                                                  // 0xA4 - Left ALT key
//	VK(RMENU),                                                  // 0xA5 - Right ALT key
	VKN(BROWSER_BACK,               "XF86Back"),                // 0xA6 - Browser Back key
	VKN(BROWSER_FORWARD,            "XF86Forward"),             // 0xA7 - Browser Forward key
	VKN(BROWSER_REFRESH,            "XF86Refresh"),             // 0xA8 - Browser Refresh key
	VKN(BROWSER_STOP,               "XF86Stop"),                // 0xA9 - Browser Stop key
	VKN(BROWSER_SEARCH,             "XF86Search"),              // 0xAA - Browser Search key
	VKN(BROWSER_FAVORITES,          "XF86Favorites"),           // 0xAB - Browser Favorites key
	VKN(BROWSER_HOME,               "XF86HomePage"),            // 0xAC - Browser Start and Home key
	VKN(VOLUME_MUTE,                "XF86AudioMute"),           // 0xAD - Volume Mute key
	VKN(VOLUME_DOWN,                "XF86AudioLowerVolume"),    // 0xAE - Volume Down key
	VKN(VOLUME_UP,                  "XF86AudioRaiseVolume"),    // 0xAF - Volume Up key
	VKN(MEDIA_NEXT_TRACK,           "XF86AudioNext"),           // 0xB0 - Next Track key
	VKN(MEDIA_PREV_TRACK,           "XF86AudioPrev"),           // 0xB1 - Previous Track key
	VKN(MEDIA_STOP,                 "XF86AudioStop"),           // 0xB2 - Stop Media key
	VKN(MEDIA_PLAY_PAUSE,           "XF86AudioPlay"),           // 0xB3 - Play/Pause Media key
	VKN(LAUNCH_MAIL,                "XF86Mail"),                // 0xB4 - Start Mail key
	VKN(LAUNCH_MEDIA_SELECT,        "XF86Select"),              // 0xB5 - Select Media key
	VKN(LAUNCH_APP1,                "XF86Launch0"),             // 0xB6 - Start Application 1 key
	VKN(LAUNCH_APP2,                "XF86Launch1"),             // 0xB7 - Start Application 2 key
	    // 0xB8-B9 -- Reserved
	VK(OEM_1),                                                  // 0xBA - Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ;: key
	VKN(OEM_PLUS,                   "Plus"),                    // 0xBB - For any country/region, the + key
	VKN(OEM_COMMA,                  "Comma"),                   // 0xBC - For any country/region, the , key
	VKN(OEM_MINUS,                  "Minus"),                   // 0xBD - For any country/region, the - key
	VKN(OEM_PERIOD,                 "Period"),                  // 0xBE - For any country/region, the . key
	VK(OEM_2),                                                  // 0xBF - Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the /? key
	VK(OEM_3),                                                  // 0xC0 - Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the `~ key
	    // 0xC1-DA -- Reserved
#if !defined(VK_ABNT_C1)
#define VK_ABNT_C1 0xC1                                             
#define VK_ABNT_C2 0xC2
#endif
	VK(ABNT_C1),                                                // 0xC1 - Brazilian special.
	VK(ABNT_C2),                                                // 0xC2 - Japanese keyboard layout.
	    //
	VK(OEM_4),                                                  // 0xDB - Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the [{ key
	VK(OEM_5),                                                  // 0xDC - Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the \\| key
	VK(OEM_6),                                                  // 0xDD - Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ]} key
	VK(OEM_7),                                                  // 0xDE - Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '" key
	VK(OEM_8),                                                  // 0xDF - Used for miscellaneous characters; it can vary by keyboard.
	    // 0xE0 -- Reserved
	    // 0xE1 -- OEM specific
	VK(OEM_102),                                                // 0xE2 - The <> keys on the US standard keyboard, or the \\| key on the non-US 102-key keyboard
	    // 0xE3-E4 -- OEM specifi
	VK(PROCESSKEY),                                             // 0xE5 - IME PROCESS key
	    // 0xE6 -- OEM specific
	VK(PACKET),                                                 // 0xE7 - Used to pass Unicode characters as if they were keystrokes.
					                            // The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods.
					                            // For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP
	    // 0xE8 -- Unassigned
	VK(OEM_RESET),                                              // 0xE9
	VK(OEM_JUMP),                                               // 0xEA
	VK(OEM_PA1),                                                // 0xEB
	VK(OEM_PA2),                                                // 0xEC
	VK(OEM_PA3),                                                // 0xED
	VK(OEM_WSCTRL),                                             // 0xEE
	VK(OEM_CUSEL),                                              // 0xEF
	VK(OEM_ATTN),                                               // 0xF0
	VK(OEM_FINISH),                                             // 0xF1
	VK(OEM_COPY),                                               // 0xF2
	VK(OEM_AUTO),                                               // 0xF3
	VK(OEM_ENLW),                                               // 0xF4
	VK(OEM_BACKTAB),                                            // 0xF5
	    // 0xE9-F5 -- OEM specific
	VK(ATTN),                                                   // 0xF6 - Attn key
	VK(CRSEL),                                                  // 0xF7 - CrSel key
	VK(EXSEL),                                                  // 0xF8 - ExSel key
	VK(EREOF),                                                  // 0xF9 - Erase EOF key
	VK(PLAY),                                                   // 0xFA - Play key
	VK(ZOOM),                                                   // 0xFB - Zoom key
	VK(NONAME),                                                 // 0xFC - Reserved
	VK(PA1),                                                    // 0xFD - PA1 key
	VK(OEM_CLEAR),                                              // 0xFE - Clear key
	    // Japanese Extra
	VK(DBE_ALPHANUMERIC),                                       // 0xf0
	VK(DBE_KATAKANA),                                           // 0xf1
	VK(DBE_HIRAGANA),                                           // 0xf2
	VK(DBE_SBCSCHAR),                                           // 0xf3
	VK(DBE_DBCSCHAR),                                           // 0xf4
	VK(DBE_ROMAN),                                              // 0xf5
	VK(DBE_NOROMAN),                                            // 0xf6
	VK(DBE_ENTERWORDREGISTERMODE),                              // 0xf7
	VK(DBE_ENTERIMECONFIGMODE),                                 // 0xf8
	VK(DBE_FLUSHSTRING),                                        // 0xf9
	VK(DBE_CODEINPUT),                                          // 0xfa
	VK(DBE_NOCODEINPUT),                                        // 0xfb
	VK(DBE_DETERMINESTRING),                                    // 0xfc
	VK(DBE_ENTERDLGCONVERSIONMODE)                              // 0xfd
	};


/*
 *  Enhanced key table.
 */
static const struct xkb enhanced[] = {
	//  Enhanced keys for the IBM-101, IBM-102 (plus MS-104 key keyboards are the
	//      INS, DEL, HOME, END, PAGE UP, PAGE DOWN, WIN, RALT, RWIN, APP, LCONTROL
	//
	//  and direction keys in the clusters to the left of the keypad;
	//  and the divide (/) and ENTER keys in the keypad.
	//
	//  Set-1 Scancodes:
	//
	//  +-------------------------------------------------------------------------------------------------------------------------------------------+
	//  |                                                                                                                                           |
	//  |  [ 01 ]      [ 3B ][ 3C ][ 3D ][ 3E ] [ 3F ][ 40 ][ 41 ][ 42 ] [ 43 ][ 44 ][ 57 ][ 58 ]    [ 54 ][ 46 ][E11D]       ----   ----   ----    |
	//  |                                                                                                                                           |
	//  |  [ 29 ][ 02 ][ 03 ][ 04 ][ 05 ][ 06 ][ 07 ][ 08 ][ 09 ][ 00 ][ 0B ][ 0C ][ 0D ][  OE  ]    [E052][E047][E049]   [ 45 ][E035][ 37 ][ 4A ]  |
	//  |  [ 0F  ][ 10 ][ 11 ][ 12 ][ 13 ][ 14 ][ 15 ][ 16 ][ 17 ][ 18 ][ 19 ][ 1A ][ 1B ][ __  ]    [E053][E04F][E051]   [ 47 ][ 48 ][ 49 ]| 4E |  |
	//  |  [ 3A__ ][ 1E ][ 1F ][ 20 ][ 21 ][ 22 ][ 23 ][ 24 ][ 25 ][ 26 ][ 27 ][ 28 ][ 2B ][ 1C ]                         [ 4B ][ 4C ][ 4D ]| __ |  |
	//  |  [ 2A  ][ 56 ][ 2C ][ 2D ][ 2E ][ 2F ][ 30 ][ 31 ][ 32 ][ 33 ][ 34 ][ 35 ][    36     ]          [E048]         [ 4F ][ 50 ][ 51 ]|E01C|  |
	//  |  [ 1D  ][E05B ][ 38  ][                 39                ][E038 ][E05C ][E05D ][E01D ]    [E04B][E050][E04D]   [     52   ][ 53 ]| __ |  |
	//  |                                                                                                                                           |
	//  +-------------------------------------------------------------------------------------------------------------------------------------------+
	//
	VKN(PRIOR,                      "Prior"),                   // 0xE049, PgUp
	VKN(NEXT,                       "Next"),                    // 0xE051, PgDn
	VKN(HOME,                       "Home"),                    // 0xE047
	VKN(END,                        "End"),                     // 0xE04F
	VKN(LEFT,                       "Left"),                    // 0xE04B
	VKN(RIGHT,                      "Right"),                   // 0xE04D
	VKN(UP,                         "Up"),                      // 0xE048
	VKN(DOWN,                       "Down"),                    // 0xE050
	VKN(INSERT,                     "Insert"),                  // 0xE052
	VKN(DELETE,                     "Delete"),                  // 0xE053
	VKN(DIVIDE,                     "KP_Divide"),               // 0xE035
	VKN(RETURN,                     "KP_Enter"),                // 0xE01C
	VKALTN(CONTROL, VK_RCONTROL,    "Control_R"),               // 0xE01D
	VKALTN(MENU, VK_RMENU,          "Alt_R")                    // 0xE038
	};

/*
 *  VK_xxxx names
 */
static const char *vknames[0x100] = {
	"NONE",                     // 0x00
	"VK_LBUTTON",               // 0x01
	"VK_RBUTTON",               // 0x02
	"VK_CANCEL",                // 0x03
	"VK_MBUTTON",               // 0x04
	"VK_XBUTTON1",              // 0x05
	"VK_XBUTTON2",              // 0x06
	/*
	 * 0x07 : unassigned
	 */
	"VK_RESERVED_07",           // 0x07
	    /**/
	"VK_BACK",                  // 0x08
	"VK_TAB",                   // 0x09
	/*
	 * 0x0A - 0x0B : reserved
	 */
	"VK_RESERVED_0A",           // 0x0A
	"VK_RESERVED_0B",           // 0x0B
	    /**/
	"VK_CLEAR",                 // 0x0C
	"VK_RETURN",                // 0x0D, Keys.Enter
	/*
	 * 0x0E - 0x0F : unassigned
	 */
	"VK_RESERVED_0E",           // 0x0E
	"VK_RESERVED_0F",           // 0x0F
	    /**/
	"VK_SHIFT",                 // 0x10
	"VK_CONTROL",               // 0x11
	"VK_MENU",                  // 0x12
	"VK_PAUSE",                 // 0x13
	"VK_CAPITAL",               // 0x14, Keys.CapsLock
	"VK_KANA",                  // 0x15
	"VK_HANGEUL",               // 0x15, Keys.HangulMode
	"VK_JUNJA",                 // 0x17
	"VK_FINAL",                 // 0x18
	"VK_HANJA",                 // 0x19
	"VK_KANJI",                 // 0x19
	"VK_ESCAPE",                // 0x1B
	"VK_CONVERT",               // 0x1C
	"VK_NONCONVERT",            // 0x1D
	"VK_ACCEPT",                // 0x1E, Keys.IMEAccept
	"VK_MODECHANGE",            // 0x1F
	"VK_SPACE",                 // 0x20
	"VK_PRIOR",                 // 0x21, Keys.PageUp
	"VK_NEXT",                  // 0x22, Keys.PageDown
	"VK_END",                   // 0x23
	"VK_HOME",                  // 0x24
	"VK_LEFT",                  // 0x25
	"VK_UP",                    // 0x26
	"VK_RIGHT",                 // 0x27
	"VK_DOWN",                  // 0x28
	"VK_SELECT",                // 0x29
	"VK_PRINT",                 // 0x2A
	"VK_EXECUTE",               // 0x2B
	"VK_SNAPSHOT",              // 0x2C, Keys.PrintScreen
	"VK_INSERT",                // 0x2D
	"VK_DELETE",                // 0x2E
	"VK_HELP",                  // 0x2F
	"VK_0",                     // 0x30
	"VK_1",                     // 0x31
	"VK_2",                     // 0x32
	"VK_3",                     // 0x33
	"VK_4",                     // 0x34
	"VK_5",                     // 0x35
	"VK_6",                     // 0x36
	"VK_7",                     // 0x37
	"VK_8",                     // 0x38
	"VK_9",                     // 0x39
	/*
	 *  0x3A-40: undefined
	 */
	"VK_RESERVED_3A",           // 0x3A
	"VK_RESERVED_3B",           // 0x3B
	"VK_RESERVED_3C",           // 0x3C
	"VK_RESERVED_3D",           // 0x3D
	"VK_RESERVED_3E",           // 0x3E
	"VK_RESERVED_3F",           // 0x3F
	"VK_RESERVED_40",           // 0x40
	    /**/
	"VK_A",                     // 0x41
	"VK_B",                     // 0x42
	"VK_C",                     // 0x43
	"VK_D",                     // 0x44
	"VK_E",                     // 0x45
	"VK_F",                     // 0x46
	"VK_G",                     // 0x47
	"VK_H",                     // 0x48
	"VK_I",                     // 0x49
	"VK_J",                     // 0x4A
	"VK_K",                     // 0x4B
	"VK_L",                     // 0x4C
	"VK_M",                     // 0x4D
	"VK_N",                     // 0x4E
	"VK_O",                     // 0x4F
	"VK_P",                     // 0x50
	"VK_Q",                     // 0x51
	"VK_R",                     // 0x52
	"VK_S",                     // 0x53
	"VK_T",                     // 0x54
	"VK_U",                     // 0x55
	"VK_V",                     // 0x56
	"VK_W",                     // 0x57
	"VK_X",                     // 0x58
	"VK_Y",                     // 0x59
	"VK_Z",                     // 0x5A
	"VK_LWIN",                  // 0x5B
	"VK_RWIN",                  // 0x5C
	"VK_APPS",                  // 0x5D
	/*
	 * 0x5E : reserved
	 */
	"VK_RESERVED_5E",           // 0x5E
	    /**/
	"VK_SLEEP",                 // 0x5F, Keys.Sleep
	"VK_NUMPAD0",               // 0x60
	"VK_NUMPAD1",               // 0x61
	"VK_NUMPAD2",               // 0x62
	"VK_NUMPAD3",               // 0x63
	"VK_NUMPAD4",               // 0x64
	"VK_NUMPAD5",               // 0x65
	"VK_NUMPAD6",               // 0x66
	"VK_NUMPAD7",               // 0x67
	"VK_NUMPAD8",               // 0x68
	"VK_NUMPAD9",               // 0x69
	"VK_MULTIPLY",              // 0x6A
	"VK_ADD",                   // 0x6B
	"VK_SEPARATOR",             // 0x6C
	"VK_SUBTRACT",              // 0x6D
	"VK_DECIMAL",               // 0x6E
	"VK_DIVIDE",                // 0x6F
	"VK_F1",                    // 0x70
	"VK_F2",                    // 0x71
	"VK_F3",                    // 0x72
	"VK_F4",                    // 0x73
	"VK_F5",                    // 0x74
	"VK_F6",                    // 0x75
	"VK_F7",                    // 0x76
	"VK_F8",                    // 0x77
	"VK_F9",                    // 0x78
	"VK_F10",                   // 0x79
	"VK_F11",                   // 0x7A
	"VK_F12",                   // 0x7B
	"VK_F13",                   // 0x7C
	"VK_F14",                   // 0x7D
	"VK_F15",                   // 0x7E
	"VK_F16",                   // 0x7F
	"VK_F17",                   // 0x80
	"VK_F18",                   // 0x81
	"VK_F19",                   // 0x82
	"VK_F20",                   // 0x83
	"VK_F21",                   // 0x84
	"VK_F22",                   // 0x85
	"VK_F23",                   // 0x86
	"VK_F24",                   // 0x87
	/*
	 * 0x88 - 0x8F : unassigned
	 */
	"VK_RESERVED_88",           // 0x88
	"VK_RESERVED_89",           // 0x89
	"VK_RESERVED_8A",           // 0x8A
	"VK_RESERVED_8B",           // 0x8B
	"VK_RESERVED_8C",           // 0x8C
	"VK_RESERVED_8D",           // 0x8D
	"VK_RESERVED_8E",           // 0x8E
	"VK_RESERVED_8F",           // 0x8F
	    /**/
	"VK_NUMLOCK",               // 0x90
	"VK_SCROLL",                // 0x91
	"VK_OEM_NEC_EQUAL",         // 0x92, NEC PC-9800 kbd definition
//dup	"VK_OEM_FJ_JISHO",          // 0x92, Fujitsu/OASYS kbd definition
	"VK_OEM_FJ_MASSHOU",        // 0x93, Fujitsu/OASYS kbd definition
	"VK_OEM_FJ_TOUROKU",        // 0x94, Fujitsu/OASYS kbd definition
	"VK_OEM_FJ_LOYA",           // 0x95, Fujitsu/OASYS kbd definition
	"VK_OEM_FJ_ROYA",           // 0x96, Fujitsu/OASYS kbd definition
	/*
	 * 0x97 - 0x9F : unassigned
	 */
	"VK_RESERVED_97",           // 0x97
	"VK_RESERVED_98",           // 0x98
	"VK_RESERVED_99",           // 0x99
	"VK_RESERVED_9A",           // 0x9A
	"VK_RESERVED_9B",           // 0x9B
	"VK_RESERVED_9C",           // 0x9C
	"VK_RESERVED_9D",           // 0x9D
	"VK_RESERVED_9E",           // 0x9E
	"VK_RESERVED_9F",           // 0x9F
	    /**/
	"VK_LSHIFT",                // 0xA0
	"VK_RSHIFT",                // 0xA1
	"VK_LCONTROL",              // 0xA2
	"VK_RCONTROL",              // 0xA3
	"VK_LMENU",                 // 0xA4
	"VK_RMENU",                 // 0xA5
	"VK_BROWSER_BACK",          // 0xA6
	"VK_BROWSER_FORWARD",       // 0xA7
	"VK_BROWSER_REFRESH",       // 0xA8
	"VK_BROWSER_STOP",          // 0xA9
	"VK_BROWSER_SEARCH",        // 0xAA
	"VK_BROWSER_FAVORITES",     // 0xAB
	"VK_BROWSER_HOME",          // 0xAC
	"VK_VOLUME_MUTE",           // 0xAD
	"VK_VOLUME_DOWN",           // 0xAE
	"VK_VOLUME_UP",             // 0xAF
	"VK_MEDIA_NEXT_TRACK",      // 0xB0
	"VK_MEDIA_PREV_TRACK",      // 0xB1
	"VK_MEDIA_STOP",            // 0xB2
	"VK_MEDIA_PLAY_PAUSE",      // 0xB3
	"VK_LAUNCH_MAIL",           // 0xB4
	"VK_LAUNCH_MEDIA_SELECT",   // 0xB5
	"VK_LAUNCH_APP1",           // 0xB6
	"VK_LAUNCH_APP2",           // 0xB7
	/*
	 * 0xB8 - 0xB9 : reserved
	 */
	"VK_RESERVED_B8",           // 0xB8
	"VK_RESERVED_B9",           // 0xB9
	    /**/
	"VK_OEM_1",                 // 0xBA, Keys.Oem1
	"VK_OEM_PLUS",              // 0xBB
	"VK_OEM_COMMA",             // 0xBC
	"VK_OEM_MINUS",             // 0xBD
	"VK_OEM_PERIOD",            // 0xBE
	"VK_OEM_2",                 // 0xBF, Keys.Oem2
	"VK_OEM_3",                 // 0xC0, Keys.Oem3
	/*
	 * 0xC1 - 0xD7 : reserved
	 */
//	"VK_RESERVED_C1",           // 0xC1
//	"VK_RESERVED_C2",           // 0xC2
	"VK_ABNT_C1",               // 0xC1 - Brazilian special.
	"VK_ABNT_C2",               // 0xC2 - Japanese keyboard layout.
	"VK_RESERVED_C3",           // 0xC3
	"VK_RESERVED_C4",           // 0xC4
	"VK_RESERVED_C5",           // 0xC5
	"VK_RESERVED_C6",           // 0xC6
	"VK_RESERVED_C7",           // 0xC7
	"VK_RESERVED_C8",           // 0xC8
	"VK_RESERVED_C9",           // 0xC9
	"VK_RESERVED_CA",           // 0xCA
	"VK_RESERVED_CB",           // 0xCB
	"VK_RESERVED_CC",           // 0xCC
	"VK_RESERVED_CD",           // 0xCD
	"VK_RESERVED_CE",           // 0xCE
	"VK_RESERVED_CF",           // 0xCF
	"VK_RESERVED_D0",           // 0xD0
	"VK_RESERVED_D1",           // 0xD1
	"VK_RESERVED_D2",           // 0xD2
	"VK_RESERVED_D3",           // 0xD3
	"VK_RESERVED_D4",           // 0xD4
	"VK_RESERVED_D5",           // 0xD5
	"VK_RESERVED_D6",           // 0xD6
	"VK_RESERVED_D7",           // 0xD7
	    /**/
	/*
	 * 0xD8 - 0xDA : unassigned
	 */
	"VK_RESERVED_D8",           // 0xD8
	"VK_RESERVED_D9",           // 0xD9
	"VK_RESERVED_DA",           // 0xDA
	    /**/
	"VK_OEM_4",                 // 0xDB, Keys.Oem4
	"VK_OEM_5",                 // 0xDC, Keys.Oem5
	"VK_OEM_6",                 // 0xDD, Keys.Oem6
	"VK_OEM_7",                 // 0xDE, Keys.Oem7
	"VK_OEM_8",                 // 0xDF
	/*
	 * 0xE0 : reserved
	 */
	"VK_RESERVED_E0",           // 0xE0
	    /**/
	"VK_OEM_AX",                // 0xE1, 'AX' key on Japanese AX kbd
	"VK_OEM_102",               // 0xE2, Keys.Oem102
	"VK_ICO_HELP",              // 0xE3, Help key on ICO
	"VK_ICO_00",                // 0xE4, 00 key on ICO
	"VK_PROCESSKEY",            // 0xE5
	"VK_ICO_CLEAR",             // 0xE6
	"VK_PACKET",                // 0xE7, Keys.Packet
	/*
	 * 0xE8 : unassigned
	 */
	"VK_RESERVED_E8",           // 0xE8
	    /**/
	"VK_OEM_RESET",             // 0xE9, Nokia/Ericsson definition
	"VK_OEM_JUMP",              // 0xEA, Nokia/Ericsson definition
	"VK_OEM_PA1",               // 0xEB, Nokia/Ericsson definition
	"VK_OEM_PA2",               // 0xEC, Nokia/Ericsson definition
	"VK_OEM_PA3",               // 0xED, Nokia/Ericsson definition
	"VK_OEM_WSCTRL",            // 0xEE, Nokia/Ericsson definition
	"VK_OEM_CUSEL",             // 0xEF, Nokia/Ericsson definition
	"VK_OEM_ATTN",              // 0xF0, Nokia/Ericsson definition
	"VK_OEM_FINISH",            // 0xF1, Nokia/Ericsson definition
	"VK_OEM_COPY",              // 0xF2, Nokia/Ericsson definition
	"VK_OEM_AUTO",              // 0xF3, Nokia/Ericsson definition
	"VK_OEM_ENLW",              // 0xF4, Nokia/Ericsson definition
	"VK_OEM_BACKTAB",           // 0xF5, Nokia/Ericsson definition
	"VK_ATTN",                  // 0xF6
	"VK_CRSEL",                 // 0xF7
	"VK_EXSEL",                 // 0xF8
	"VK_EREOF",                 // 0xF9
	"VK_PLAY",                  // 0xFA
	"VK_ZOOM",                  // 0xFB
	"VK_NONAME",                // 0xFC
	"VK_PA1",                   // 0xFD
	"VK_OEM_CLEAR",             // 0xFE
	/*
	 * 0xFF : reserved
	 */
	"VK_RESERVED_FF"            // 0xFF
	};

#undef __VKVALUE
#undef VK
#undef __XKBVALUE
#undef XKB

static inline int
ToUpper(WORD key)
{
	if (key >= 'a' && key <= 'z') {
		return (int)('A' + (key - 'a'));
	}
	return (int)key;
}


/*
 *  KBMapEvent ---
 *      Console key event parser.
 **/
int
KBMapEvent(const KEY_EVENT_RECORD *key, key_event_t *evt)
{
	const WORD wVirtualKeyCode = key->wVirtualKeyCode;
	const WORD wVirtualScanCode = key->wVirtualScanCode;
	const DWORD state = KBNormalizeAltGr(key);
	WCHAR uc = key->uChar.UnicodeChar;

	if (! key->bKeyDown) {
		return 0;                       // generally ignore key down events
	}

	// Base evt.
	evt->vkmodifiers = state;
	evt->vkkey       = wVirtualKeyCode;
	evt->vkext       = 0;
	evt->vkname      = NULL;
	evt->vkenhanced  = 0;
	evt->kblabel     = NULL;
	evt->kbmodifiers = KBMapModifiers(state, 1);
	evt->ascii       = 0;
	evt->unicode     = KEY_INVALID;

	// Virtual keys.
	if (ENHANCED_KEY & state) {
		const struct xkb *vkkey = enhanced,
			*vkend = vkkey + (sizeof(enhanced)/sizeof(enhanced[0]));
		for (;vkkey < vkend; ++vkkey) { // Specialised enhanced keys.
			if (vkkey->wVirtualKeyCode == wVirtualKeyCode &&
				    (VK_VOID == vkkey->wVirtualScanCode || vkkey->wVirtualScanCode == wVirtualScanCode)) {
				evt->vkname = vkkey->vkname;
				evt->kblabel = vkkey->kblabel;
				evt->vkext = vkkey->vkext;
				evt->vkenhanced = 1;
				return 1;
			}
		}
	}

	// Unicode keys (Printable/non-printable unicode characters)
	if (uc) {
		if (uc <= 0x1f && (evt->kbmodifiers & (MODIFIER_CONTROL|MODIFIER_ALT))) {
			evt->ascii = '@' + uc;  // Control, convert to source key; Ctrl-A ..
				// no equiv unicode character.
		} else {
			//assert((uc >= 0x20 && uc <= 0x7f) || (uc >= 0xa0 && uc < 0x10ffff));
			if (uc <= 0x7f) {       // Unshifted ASCII character
				evt->ascii = uc;
				if ((evt->kbmodifiers & (MODIFIER_SHIFT|MODIFIER_CAPSLOCK)) == MODIFIER_SHIFT)
					evt->ascii = ToUpper(uc); // Apply shift.
			}
			evt->unicode = uc;      // Shifted character value.
		}
	}

	// Standard key.
	{	const struct xkb *vkkey = standard,
			*vkend = vkkey + (sizeof(standard)/sizeof(standard[0]));
		for (;vkkey < vkend; ++vkkey) { // General or non-specialised enhanced keys.
			if (vkkey->wVirtualKeyCode == wVirtualKeyCode &&
				    (VK_VOID == vkkey->wVirtualScanCode || vkkey->wVirtualScanCode == wVirtualScanCode)) {
				evt->vkname = vkkey->vkname;
				evt->kblabel = vkkey->kblabel;
				evt->vkext = vkkey->vkext;
				return 2;
			}
		}
		if (uc) {
			evt->vkname = KBVirtualKeyName(wVirtualKeyCode);
			return 3;
		}
	}

	// Specials.
#define SPECIALMODS	(MODIFIER_ALT|MODIFIER_CONTROL)
	if (SPECIALMODS & evt->kbmodifiers) {
		//
		//  Not all keys are visible when control and/or alt are active
		//  Lookup the ASCII key against only the scancode possiblity shifted.
		//  Examples include ',' and '.'
		//
		//  These can be/are reportd via VK_OEM_xxx definitions, yet many are locale specific
		//  as such not portable beyond US keyboards; the follow method (so far) seems the cleanise.
		//
		//  References:
		//      https://msdn.microsoft.com/en-us/library/ms892480.aspx,
		//      scan-code to vk-code, by locale/keyboard.
		//
#if (0)
		BYTE keystate[256] = {0};       // VK key states.
		WORD keys[8] = {0};

		if (key->dwControlKeyState & SHIFT_PRESSED)
			keystate[VK_SHIFT] = 0x80; // down; high bit
		if (key->dwControlKeyState & CAPSLOCK_ON)
			keystate[VK_CAPITAL] = 0x01; // on; low bit

//See:KBNormalizeAltGr		evt->kbmodifiers &= ~SPECIALMODS;
		if (1 == ToUnicode(wVirtualKeyCode, key->wVirtualScanCode, keystate, keys, 8, 0)) {
			if (keys[0] >= 0x20 && keys[0] <= 0x7f) {
				evt->ascii = keys[0];
				if (evt->kbmodifiers & MODIFIER_SHIFT)
					evt=>ascii = tolower(evt->ascii)
				return 4;
			} else if (keys[0] >= 0x80) {
				evt->unicode = keys[0];
				return 4;
			}
		}
#else
		//See:KBNormalizeAltGr
		// evt->kbmodifiers &= ~SPECIALMODS;
		if ((uc = (WCHAR)MapVirtualKeyW(wVirtualKeyCode, MAPVK_VK_TO_CHAR)) != 0) {
			if (uc >= 0x20 && uc <= 0x7f) {
				evt->kbmodifiers &= ~MODIFIER_SHIFT;
				evt->ascii = ToUpper(uc);
				return 4;
			} else if (uc >= 0x80) {
				evt->unicode = uc;
				return 4;
			}
		}
#endif
	}

	return 0;
}


/*
 *  KBVirtualKeyName ---
 *      Retrieve the VirtualKey name, otherwise VK_UNKNOWN if out of range.
 **/
const char *
KBVirtualKeyName(unsigned vk)
{
	if (vk < 0x100)
		return vknames[vk];
	return "VK_UNKNOWN";
}


/*
 *  KBVirtualKeyValue ---
 *      Retrieve the VirtualKey value, otherwise 0 if unmapped.
 **/
unsigned
KBVirtualKeyValue(const char *name)
{
	static const struct vkextra {
		// Additional
		// See: http://www.kbdedit.com/manual/low_level_vk_list.html
		char name[30];
		unsigned value;
#define XVK(__v)        { #__v, __v }
	} vkextras[] = {
		/* 
		 *  duplicate
		 */
		XVK(VK_OEM_FJ_JISHO),
		/*
		 *  extra
		 */
		XVK(VK_DBE_ALPHANUMERIC),
		XVK(VK_DBE_KATAKANA),
		XVK(VK_DBE_HIRAGANA),
		XVK(VK_DBE_SBCSCHAR),
		XVK(VK_DBE_DBCSCHAR),
		XVK(VK_DBE_ROMAN),
		XVK(VK_DBE_NOROMAN),
		XVK(VK_DBE_ENTERWORDREGISTERMODE),
		XVK(VK_DBE_ENTERIMECONFIGMODE),
		XVK(VK_DBE_FLUSHSTRING),
		XVK(VK_DBE_CODEINPUT),
		XVK(VK_DBE_NOCODEINPUT),
		XVK(VK_DBE_DETERMINESTRING),
		XVK(VK_DBE_ENTERDLGCONVERSIONMODE)
		};

	for (unsigned vk = 1; vk < _countof(vknames); ++vk) {
		if (0 == strcmp(name, vknames[vk])) {
			return vk;
		}
	}

	for (const struct vkextra *it = vkextras, *end = it + _countof(vkextras); it != end; ++it) {
		if (0 == strcmp(name, it->name)) {
			return it->value;
		}
	}
	return 0;
}


/*
 *  KBNormalizeAltGr ---
 *      Filter AtrGr events from modifiers.
 **/
DWORD
KBNormalizeAltGr(const KEY_EVENT_RECORD *key)
{
	DWORD state = key->dwControlKeyState;

	// AltGr condition (LCtrl + RAlt)
	if (0 == (state & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)))
		return state;

	if (0 == (state & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)))
		return state;

	if (0 == key->uChar.UnicodeChar)
		return state;

	// Filter Left-Ctrl+Right+Alt, attempt to allow: 
	//
	//      Left-Alt + AltGr,
	//      Right-Ctrl + AltGr,
	// or   Left-Alt + Right-Ctrl + AltGr.
	//

	if (state & RIGHT_ALT_PRESSED) {
		// Remove Right-Alt.
		state &= ~RIGHT_ALT_PRESSED;

		// As a character was presented, Left-Ctrl is almost always set,
		// except if the user presses Right-Ctrl, then AltGr (in that specific order) for whatever reason.
		// At any rate, make sure the bit is not set.
		state &= ~LEFT_CTRL_PRESSED;

	} else if (state & LEFT_ALT_PRESSED) {
		// Remove Left-Alt.
		state &= ~LEFT_ALT_PRESSED;

		// Whichever Ctrl key is down, remove it from the state.
		// We only remove one key, to improve our chances of detecting the corner-case of Left-Ctrl + Left-Alt + Right-Ctrl.
		if ((state & LEFT_CTRL_PRESSED) != 0) {
			// Remove Left-Ctrl.
			state &= ~LEFT_CTRL_PRESSED;

		} else if ((state & RIGHT_CTRL_PRESSED) != 0) {
			// Remove Right-Ctrl.
			state &= ~RIGHT_CTRL_PRESSED;
		}
	}

	return state;
}


/*
 *  KBMapModifiers ---
 *     Console event modifier parser.
 **/
unsigned
KBMapModifiers(const DWORD state, int apps)
{
	unsigned modifiers = 0;
	if (state & (SHIFT_PRESSED)) {
		modifiers |= MODIFIER_SHIFT;
	}
	if (state & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
		modifiers |= MODIFIER_CONTROL;
	}
	if (state & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) {
		modifiers |= MODIFIER_ALT;
	}
	if (state & (NUMLOCK_ON)) {
		modifiers |= MODIFIER_NUMLOCK;
	}
	if (state & (SCROLLLOCK_ON)) {
		modifiers |= MODIFIER_SCROLLLOCK;
	}
	if (state & (CAPSLOCK_ON)) {
		modifiers |= MODIFIER_CAPSLOCK;
	}
	if (apps && GetKeyState(VK_APPS)) {
		modifiers |= MODIFIER_LOGO; // Hamburger Button, not windows key
			// beware: Key toggles like CapsLock, ie: press on, when press off
	}
	return modifiers;
}


/*
 *  KBMapModifiers ---
 *     Console event modifier parser.
 **/
int
KBPrintModifiers(const DWORD state, char *buffer, size_t buflen, unsigned detailed)
{
	static const struct State {
		unsigned detailed;
		const char *label;
		DWORD bits;
	} states[] = {
		// Detailed
		{ 0x01, "Enhanced", ENHANCED_KEY },
		{ 0x01, "ScrLk",    SCROLLLOCK_ON  },
		{ 0x01, "Caps",     CAPSLOCK_ON },
		{ 0x01, "NumLk",    NUMLOCK_ON  },
		{ 0x01, "AltL",     LEFT_ALT_PRESSED },
		{ 0x01, "AltR",     RIGHT_ALT_PRESSED },
		{ 0x01, "CtrlL",    LEFT_CTRL_PRESSED  },
		{ 0x01, "CtrlR",    RIGHT_CTRL_PRESSED },
		{ 0x01, "Shift",    SHIFT_PRESSED },
		// Summary
		{ 0x00, "Alt",      LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED },
		{ 0x00, "Ctrl",     LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED },
		{ 0x00, "Shift",    SHIFT_PRESSED },
		};

	char *cursor = buffer;
	unsigned modifiers = 0;

	if (NULL == buffer || 0 == buflen)
		return -1;

	--buflen; // nul

	if (GetKeyState(VK_APPS)) { // hamburger key
		const int len = _snprintf(cursor, buflen, "%sApps", (modifiers++ ? "-" : ""));
		if ((size_t)len <= buflen) {
			cursor += len, buflen -= len;
		}
	}

	for (const struct State *it = states, *end = states + (sizeof(states)/sizeof(states[0])); buflen && it != end; ++it) {
		if (it->detailed == detailed && (state & it->bits)) {
			const int len = _snprintf(cursor, buflen, "%s%s", (modifiers++ ? "-" : ""), it->label);
			if ((size_t)len <= buflen) {
				cursor += len, buflen -= len;
				continue;
			}
			break; // overflow
		}
	}

	if (buflen) {
		const int len = snprintf(cursor, buflen, "%s", (modifiers ? "-" : ""));
		if ((size_t)len <= buflen) {
			cursor += len, buflen -= len;
		}
	}

	*cursor = 0; // nul
	return cursor - buffer;
}

/*end*/
