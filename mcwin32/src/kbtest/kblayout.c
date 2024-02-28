#include <edidentifier.h>
__CIDENT_RCSID(kblayout_c, "$Id: kblayout.c,v 1.7 2024/02/28 16:00:49 cvsuser Exp $")

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

static const wchar_t CUp[2]    = {0x25B2};
static const wchar_t CLeft[2]  = {0x25C4};
static const wchar_t CRight[2] = {0x25BA};
static const wchar_t CDown[2]  = {0x25BC};
static const wchar_t CApp[2]   = {0x2261};

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
		{-1},{EK(UP),CUp},{-1},
		{VK(NUMPAD1),L"1"},{VK(NUMPAD2),L"2"},{VK(NUMPAD3),L"3"},{EK(RETURN),L"CR"},
		{0}};
	static const struct KBRow row6[] = {
		{VK(LCONTROL),L"Ctrl"},{EK(LWIN),L"Win"},{VK(LMENU),L"Alt"},{VK(SPACE),L"<SPACE>",44},{EK(RMENU),L"AGr"},{EK(RWIN),L"App"},{VK(APPS),CApp},{EK(RCONTROL),L"Ctrl"},
		{EK(LEFT),CLeft},{EK(DOWN),CDown},{EK(RIGHT),CRight},
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
		SP(2),{-1},SCT(0xE048,CUp),{-1},
		SP(2),SC(0x4F),SC(0x50),SC(0x51),{-1},
		{0}};
	static struct KBRow row6[] = {
		SCX(0x1D,5),SCX(0xE05B,5),SCX(0x38,5),SCTX(0x39,L"<SPACE>",49),SCX(0xE038,5),SCX(0xE05C,5),SCX(0xE05D,5),SCX(0xE01D,5),
		SP(2),SCT(0xE04B,CLeft),SCT(0xE050,CDown),SCT(0xE04D,CRight),
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
