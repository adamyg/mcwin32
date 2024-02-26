#include <edidentifier.h>
__CIDENT_RCSID(kblayout_c, "$Id: kblayout.c,v 1.5 2024/02/17 08:23:13 cvsuser Exp $")

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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <assert.h>

#include "kbvirtualextra.h"
#include "kbdefinition.h"
#include "kblayout.h"

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


/*
 *  KBLayoutDefault ---
 *      Default, US keyboard layout.
 **/
const struct KBRow **
KBLayoutDefault(void)
{
#define __V(__v)	__v				// Value helper
#define __X(__a,__b)	__a##__b			// Concat helper
#define AK(__k)		__V(VK_##__k),__X(L,#__k)	// Alphanum VK_[0-9][A-Z]
#define VK(__k)		__V(VK_##__k)			// VK_xxx
#define EK(__k)		(__V(VK_##__k)|VK_ISENHANCED)	// Enhanced VK_xxx
#define NK()		(0xfff)				// NUL key
#define AS(__c)		(__c)				// ASCII

	//  US/AU/HK
	//
	//  ---------------------------------------------------------------------------------------------------------------------------------------
	//  [ESC ]   [ F1 ][ F2 ][ F3 ][ F4 ]   [ F5 ][ F6 ][ F7 ][ F8 ]   [ F9 ][ F10][ F11][ F12]    [Prt] [Slk] [Pse]
	//
	//  [ `  ] [ 1  ][ 2  ][ 3  ][ 4  ][ 5  ][ 6  ][ 7  ][ 8  ][ 9  ][ 0  ][ -  ][ =  ][  _<_ ]    [INS] [HOM] [PUp]   [NUM] [ / ] [ * ] [ - ]
	//  [ |- ] [ Q  ][ W  ][ E  ][ R  ][ T  ][ Y  ][ U  ][ I  ][ O  ][ P  ][ {  ][ }  ] [.....]    [DEL] [END] [PDn]   [ 7 ] [ 8 ] [ 9 ] |   |
	//  [CAPS.]  [ A  ][ S  ][ D  ][ F  ][ G  ][ H  ][ J  ][ K  ][ L  ][ ;  ][ '  ][  | ] [ENT]                        [ 4 ] [ 5 ] [ 6 ] | _ |
	//  [SHIFT...] [ \ ][ Z  ][ X  ][ C  ][ V  ][ B  ][ N  ][ M  ][ ,  ][ .  ][ /  ] [SHIFT...]          [ ^ ]         [ 1 ] [ 2 ] [ 3 ] |   |
	//  [CTRL_][_WIN_][_ALT_][                SPACE                ][_ALT_][_APP_][MENU][_CTRL]    [ < ] [ V ] [ > ]   [   IN    ] [ . ] | _ |
	//
	//
	static const struct KBRow row1[] = {
		{VK(ESCAPE),L"ESC"},{VK(F1),L"F1"},{VK(F2),L"F2"},{VK(F3),L"F3"},{VK(F4),L"F4"},{VK(F5),L"F5"},{VK(F6),L"F6"},{VK(F7),L"F7"},
		{VK(F8),L"F8"},{VK(F9),L"F9"},{VK(F10),L"F10"},{VK(F11),L"F11"},{VK(F11),L"F12"},
		{VK(PRINT),L"PRT"},{VK(SCROLL),L"SLk"},{EK(PAUSE),L"Brk"},
		{0}};
	static const struct KBRow row2[] = {
		{VK(OEM_3),L"~"},{AK(1)},{AK(2)},{AK(3)},{AK(4)},{AK(5)},{AK(6)},{AK(7)},{AK(8)},{AK(9)},{VK(OEM_MINUS),L"-"},{VK(OEM_PLUS),L"="},{VK(BACK),L"Bck"},
		{EK(INSERT),L"Ins"},{EK(HOME),L"Hom"},{EK(PRIOR),L"PUp"},
		{VK(NUMLOCK),L"Num"},{EK(DIVIDE),L"/"},{VK(MULTIPLY),L"*"},{VK(SUBTRACT),L"-"},
		{0}};
	static const struct KBRow row3[] = {
		{VK(TAB),L"Tab"},{AK(Q)},{AK(W)},{AK(E)},{AK(T)},{AK(Y)},{AK(U)},{AK(I)},{AK(O)},{AK(P)},{VK(OEM_4),L"["},{VK(OEM_6),L"]"},{-1},
		{EK(DELETE),L"Del"},{EK(END),L"End"},{EK(NEXT),L"PUp"},
		{VK(NUMPAD7),L"7"},{VK(NUMPAD8),L"8"},{VK(NUMPAD9),L"9"},{VK(ADD),L"+"},
		{0}};
	static const struct KBRow row4[] = {
		{VK(CAPITAL),L"Cap"},{AK(A)},{AK(S)},{AK(D)},{AK(F)},{AK(G)},{AK(H)},{AK(J)},{AK(K)},{VK(OEM_1),L";"},{VK(OEM_7),L"\'"},{VK(OEM_5),L"\\"},{VK(RETURN),L"CR"},
		{-1},{-1},{-1},
		{VK(NUMPAD4),L"4"},{VK(NUMPAD5),L"5"},{VK(NUMPAD6),L"6"},{-1},
		{0}};
	static const struct KBRow row5[] = {
		{VK(LSHIFT),L"LSH"},{VK(OEM_102),L"\\"},{AK(Z)},{AK(X)},{AK(C)},{AK(V)},{AK(B)},{AK(N)},{AK(M)},{VK(OEM_COMMA),L","},{VK(OEM_PERIOD),L"."},{VK(OEM_2),L"/"},{VK(RSHIFT),L"RSH"},
		{-1},{EK(UP),L"\u25B2"},{-1},
		{VK(NUMPAD1),L"1"},{VK(NUMPAD2),L"2"},{VK(NUMPAD3),L"3"},{EK(RETURN),L"CR"},
		{0}};
	static const struct KBRow row6[] = {
		{VK(LCONTROL),L"Ctrl"},{EK(LWIN),L"Win"},{VK(LMENU),L"Alt"},{VK(SPACE),L"<SPACE>",44},{EK(RMENU),L"AGr"},{EK(RWIN),L"App"},{VK(APPS),L"\u2261"},{EK(RCONTROL),L"Ctrl"},
		{EK(LEFT),L"\u25C4"},{EK(DOWN),L"\u25BC"},{EK(RIGHT),L"\u25BA"},
		{VK(NUMPAD0),L" 0  <INS> ",12},{VK(DECIMAL),L"."},
		{0}};
	static const struct KBRow *rows[] =
		{ row1, row2, row3, row4, row5, row6, NULL };

#undef __V
#undef __X
#undef AK
#undef VK
#undef EK
#undef NK
#undef AS
	return rows;
}


/*
 *  KBLayoutBuild ---
 *      Build a layout from the supplied definition.
 **/
const struct KBRow **
KBLayoutBuild(const KBDefinition *layout)
{
	// Set-1 Scancodes (see above):
	//
	//  ---------------------------------------------------------------------------------------------------------------------------------------------
	//  [ 01 ]      [ 3B ][ 3C ][ 3D ][ 3E ] [ 3F ][ 40 ][ 41 ][ 42 ] [ 43 ][ 44 ][ 57 ][ 58 ]    [ 54 ][ 46 ][E11D]       ----   ----   ----
	//
	//  [ 29 ][ 02 ][ 03 ][ 04 ][ 05 ][ 06 ][ 07 ][ 08 ][ 09 ][ 00 ][ 0B ][ 0C ][ 0D ][  OE  ]    [E052][E047][E049]   [ 45 ][E035][ 37 ][ 4A ]
	//  [ 0F  ][ 10 ][ 11 ][ 12 ][ 13 ][ 14 ][ 15 ][ 16 ][ 17 ][ 18 ][ 19 ][ 1A ][ 1B ][ __  ]    [E053][E04F][E051]   [ 47 ][ 48 ][ 49 ]| 4E |
	//  [ 3A__ ][ 1E ][ 1F ][ 20 ][ 21 ][ 22 ][ 23 ][ 24 ][ 25 ][ 26 ][ 27 ][ 28 ][ 2B ][ 1C ]                         [ 4B ][ 4C ][ 4D ]| __ |
	//  [ 2A  ][ 56 ][ 2C ][ 2D ][ 2E ][ 2F ][ 30 ][ 31 ][ 32 ][ 33 ][ 34 ][ 35 ][    36     ]          [E048]         [ 4F ][ 50 ][ 51 ]|E01C|
	//  [ 1D  ][E05B ][ 38  ][                 39                ][E038 ][E05C ][E05D ][E01D ]    [E04B][E050][E04D]   [     52   ][ 53 ]| __ |
	//
	//  ---------------------------------------------------------------------------------------------------------------------------------------------
	//

#define SC(_sc)             {0,L"",0,_sc}
#define SCX(_sc,_wc)        {0,L"",_wc,_sc}
#define SCT(_sc,_tx)        {0,_tx,0,_sc}
#define SCTX(_sc,_tx,_wc)   {0,_tx,_wc,_sc}
#define SP(_wc)             {-1,NULL,_wc,0}

	static struct KBRow row1[] = {
		SC(0x01),{-1},SC(0x3B),SC(0x3C),SC(0x3D),SC(0x3E),SP(2),SC(0x3F),SC(0x40),SC(0x41),SC(0x42),SP(2),SC(0x43),SC(0x44),SC(0x57),SC(0x58),
		SP(2),SC(0x54),SC(0x46),SC(0xE11D),
		{0}};
	static struct KBRow row2[] = {
		SC(0x29),SC(0x02),SC(0x03),SC(0x04),SC(0x05),SC(0x06),SC(0x07),SC(0x08),SC(0x09),SC(0x0A),SC(0x0B),SC(0x0C),SC(0x0D),SCX(0x0E,8),
		SP(2),SC(0xE052),SC(0xE047),SC(0xE049),
		SP(2),SC(0x45),SC(0xE035),SC(0x37),SC(0x4A),
		{0}};
	static struct KBRow row3[] = {
		SCX(0x0F,6),SC(0x10),SC(0x11),SC(0x12),SC(0x13),SC(0x14),SC(0x15),SC(0x16),SC(0x17),SC(0x18),SC(0x19),SC(0x1A),SC(0x1B),SP(10),
		SP(2),SC(0xE053),SC(0xE04F),SC(0xE051),
		SP(2),SC(0x47),SC(0x48),SC(0x49),{-1},
		{0}};
	static struct KBRow row4[] = {
		SCX(0x3A,7),SC(0x1E),SC(0x1F),SC(0x20),SC(0x21),SC(0x22),SC(0x23),SC(0x24),SC(0x25),SC(0x26),SC(0x27),SC(0x28),SC(0x2B),SCX(0x1C,5),
		SP(2),{-1},{-1},{-1},
		SP(2),SC(0x4B),SC(0x4C),SC(0x4D),SC(0x4E),
		{0}};
	static struct KBRow row5[] = {
		SCX(0x2A,5),SC(0x56),SC(0x2C),SC(0x2D),SC(0x2E),SC(0x2F),SC(0x30),SC(0x31),SC(0x32),SC(0x33),SC(0x34),SC(0x35),SCX(0x36,15),
		SP(2),{-1},SCT(0xE048,L"\u25B2"),{-1},
		SP(2),SC(0x4F),SC(0x50),SC(0x51),{-1},
		{0}};
	static struct KBRow row6[] = {
		SCX(0x1D,5),SCX(0xE05B,5),SCX(0x38,5),SCTX(0x39,L"<SPACE>",49),SCX(0xE038,5),SCX(0xE05C,5),SCX(0xE05D,5),SCX(0xE01D,5),
		SP(2),SCT(0xE04B,L"\u25C4"),SCT(0xE050,L"\u25BC"),SCT(0xE04D,L"\u25BA"),
		SP(2),SCX(0x52,12),SC(0x53),SC(0xE01C),
		{0}};
	static struct KBRow *rows[] =
		{ row1, row2, row3, row4, row5, row6, NULL };

	for (const PhysicalKey *pk = layout->PhysicalKeys, *end = pk + layout->PhysicalKeysCount; pk != end; ++pk) {
		for (unsigned r = 0; rows[r]; ++r) {
			struct KBRow *row = rows[r];
			if (0 == row->sc) continue; // void
			for (;row->sc||row->vk; ++row) {
				if (row->vk) continue;
				if (row->sc == (int)pk->SC) { // ScanCode match

					row->vk = pk->VK;
					if ((row->sc & 0xFF00) == 0xE000)
						row->vk |= VK_ISENHANCED; // Enhanced

					if (pk->Results && pk->Results->Text[0]) {
						row->label1 = pk->Results->Text;
					} else {
						if (0 == row->label1[0] && pk->Name[0]) {
							row->label1 = pk->Name;
						}
					}

					for (const PhysicalResults *rs = pk->Results, *rsend = rs + pk->Count; rs != rsend; ++rs) { // Results
						if (rs->With == WITH_SHIFT) { // Shift
							row->label2 = rs->Text;
						} else if (rs->With == (WITH_CONTROL|WITH_MENU)) { // AltGr
							row->label3 = rs->Text;
						}
					}

					r = 5; // done
					break;
				}
			}
		}
	}
	return (const struct KBRow **)(rows);
}

//end
