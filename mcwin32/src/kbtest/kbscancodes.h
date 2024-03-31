#include <edidentifier.h>
__CIDENT_RCSID(kbscancode_c, "$Id: kbscancodes.h,v 1.1 2024/02/28 16:00:49 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * console keyboard test application
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

enum USScancode {
	SC_Escape = 0x01,
	SC_1 = 0x02,
	SC_2 = 0x03,
	SC_3 = 0x04,
	SC_4 = 0x05,
	SC_5 = 0x06,
	SC_6 = 0x07,
	SC_7 = 0x08,
	SC_8 = 0x09,
	SC_9 = 0x0A,
	SC_0 = 0x0B,
	SC_Minus = 0x0C,
	SC_Equals = 0x0D,
	SC_Backspace = 0x0E,
	SC_Tab = 0x0F,
	SC_Q = 0x10,
	SC_W = 0x11,
	SC_E = 0x12,
	SC_R = 0x13,
	SC_T = 0x14,
	SC_Y = 0x15,
	SC_U = 0x16,
	SC_I = 0x17,
	SC_O = 0x18,
	SC_P = 0x19,
	SC_BracketLeft = 0x1A,
	SC_BracketRight = 0x1B,
	SC_Enter = 0x1C,
	SC_ControlLeft = 0x1D,
	SC_A = 0x1E,
	SC_S = 0x1F,
	SC_D = 0x20,
	SC_F = 0x21,
	SC_G = 0x22,
	SC_H = 0x23,
	SC_J = 0x24,
	SC_K = 0x25,
	SC_L = 0x26,
	SC_Semicolon = 0x27,
	SC_Apostrophe = 0x28,
	SC_Grave = 0x29,
	SC_ShiftLeft = 0x2A,
	SC_Backslash = 0x2B,
	SC_Z = 0x2C,
	SC_X = 0x2D,
	SC_C = 0x2E,
	SC_V = 0x2F,
	SC_B = 0x30,
	SC_N = 0x31,
	SC_M = 0x32,
	SC_Comma = 0x33,
	SC_Preiod = 0x34,
	SC_Slash = 0x35,
	SC_ShiftRight = 0x36,
	SC_Numpad_multiply = 0x37,
	SC_AltLeft = 0x38,
	SC_Space = 0x39,
	SC_CapsLock = 0x3A,
	SC_F1 = 0x3B,
	SC_F2 = 0x3C,
	SC_F3 = 0x3D,
	SC_F4 = 0x3E,
	SC_F5 = 0x3F,
	SC_F6 = 0x40,
	SC_F7 = 0x41,
	SC_F8 = 0x42,
	SC_F9 = 0x43,
	SC_F10 = 0x44,
	SC_NumLock = 0x45,
	SC_ScrollLock = 0x46,
	SC_Numpad_7 = 0x47,
	SC_Numpad_8 = 0x48,
	SC_Numpad_9 = 0x49,
	SC_Numpad_minus = 0x4A,
	SC_Numpad_4 = 0x4B,
	SC_Numpad_5 = 0x4C,
	SC_Numpad_6 = 0x4D,
	SC_Numpad_plus = 0x4E,
	SC_Numpad_1 = 0x4F,
	SC_Numpad_2 = 0x50,
	SC_Numpad_3 = 0x51,
	SC_Numpad_0 = 0x52,
	SC_Numpad_period = 0x53,
	SC_Alt_printScreen = 0x54, /* Alt + print screen. MapVirtualKeyEx( VK_SNAPSHOT, MAPVK_VK_TO_VSC_EX, 0 ) returns scancode 0x54. */
	SC_BracketAngle = 0x56, /* Key between the left shift and Z. */
	SC_F11 = 0x57,
	SC_F12 = 0x58,
	SC_OEM_1 = 0x5a, /* VK_OEM_WSCTRL */
	SC_OEM_2 = 0x5b, /* VK_OEM_FINISH */
	SC_OEM_3 = 0x5c, /* VK_OEM_JUMP */
	SC_EraseEOF = 0x5d,
	SC_OEM_4 = 0x5e, /* VK_OEM_BACKTAB */
	SC_OEM_5 = 0x5f, /* VK_OEM_AUTO */
	SC_Zoom = 0x62,
	SC_Help = 0x63,
	SC_F13 = 0x64,
	SC_F14 = 0x65,
	SC_F15 = 0x66,
	SC_F16 = 0x67,
	SC_F17 = 0x68,
	SC_F18 = 0x69,
	SC_F19 = 0x6a,
	SC_F20 = 0x6b,
	SC_F21 = 0x6c,
	SC_F22 = 0x6d,
	SC_F23 = 0x6e,
	SC_OEM_6 = 0x6f, /* VK_OEM_PA3 */
	SC_Katakana = 0x70,
	SC_OEM_7 = 0x71, /* VK_OEM_RESET */
	SC_F24 = 0x76,
	SC_Sbcschar = 0x77,
	SC_Convert = 0x79,
	SC_Nonconvert = 0x7B, /* VK_OEM_PA1 */

	SC_Media_previous = 0xE010,
	SC_Media_next = 0xE019,
	SC_Numpad_enter = 0xE01C,
	SC_ControlRight = 0xE01D,
	SC_Volume_mute = 0xE020,
	SC_Launch_app2 = 0xE021,
	SC_Media_play = 0xE022,
	SC_Media_stop = 0xE024,
	SC_Volume_down = 0xE02E,
	SC_Volume_up = 0xE030,
	SC_Browser_home = 0xE032,
	SC_Numpad_divide = 0xE035,
	SC_PrintScreen = 0xE037,
	/*
	SC_PrintScreen:
	- mAke: 0xE02A 0xE037
	- bReak: 0xE0B7 0xE0AA
	- MApVirtualKeyEx( VK_SNAPSHOT, MAPVK_VK_TO_VSC_EX, 0 ) returns scancode 0x54;
	- THere is no VK_KEYDOWN with VK_SNAPSHOT.
	*/
	SC_AltRight = 0xE038,
	SC_Cancel = 0xE046, /* CTRL + Pause */
	SC_Home = 0xE047,
	SC_ArrowUp = 0xE048,
	SC_PageUp = 0xE049,
	SC_ArrowLeft = 0xE04B,
	SC_ArrowRight = 0xE04D,
	SC_End = 0xE04F,
	SC_ArrowDown = 0xE050,
	SC_PageDown = 0xE051,
	SC_Insert = 0xE052,
	SC_Delete = 0xE053,
	SC_MetaLeft = 0xE05B,
	SC_MetaRight = 0xE05C,
	SC_Application = 0xE05D,
	SC_Power = 0xE05E,
	SC_Sleep = 0xE05F,
	SC_Wake = 0xE063,
	SC_Browser_search = 0xE065,
	SC_Browser_favorites = 0xE066,
	SC_Browser_refresh = 0xE067,
	SC_Browser_stop = 0xE068,
	SC_Browser_forward = 0xE069,
	SC_Browser_back = 0xE06A,
	SC_Launch_app1 = 0xE06B,
	SC_Launch_email = 0xE06C,
	SC_Launch_media = 0xE06D,

	SC_Pause = 0xE11D45,
	/*
	SC_Pause:
	- make: 0xE11D 45 0xE19D C5
	- make in raw input: 0xE11D 0x45
	- break: none
	- No repeat when you hold the key down
	*/
};

//end
