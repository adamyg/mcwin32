/* -*- mode: c; indent-width: 4; -*- */
/*
 * console keyboard test application
 *
 * Copyright (c) 2024, Adam Young.
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

#include "kbdefinition.h"
#include "kbutil.h"

static unsigned VirtualKeys(const HKL hkl, unsigned *vk2sc);

/*
 *  Keyboard definition ---
 *      Derive the current keyboard definition.
 **/
KBDefinition *
KBDefinitionCurrent(void)
{
	static const struct State {
		unsigned shift : 1;
		unsigned capital : 1;
		unsigned control : 1;
		unsigned alt : 1;

	} states[] = {  // TODO: L/R Shift/Control/Alt
		{ 0, 0, 0, 0 }, // Normal
		{ 1, 0, 0, 0 }, // Shift
		{ 0, 1, 0, 0 }, // Capital
		{ 1, 1, 0, 0 }, // Shift-Capital
		{ 0, 0, 1, 0 }, // Control
		{ 1, 0, 1, 0 }, // Shift-Control
		{ 0, 0, 1, 1 }, // Control-Alt
		{ 1, 0, 1, 1 }  // Shift-Control-Alt
		};

	const HKL hkl = GetKeyboardLayout(0);
	KBDefinition *def;
	unsigned vk2sc[512] = { 0 };
	BYTE keyState[256] = { 0 };

	if (NULL == (def = (KBDefinition *)calloc(sizeof(KBDefinition), 1))) {
		return NULL;
	}

	VirtualKeys(hkl, vk2sc);

	// ForEach(VK)
	for (unsigned idx = 0x01; idx < _countof(vk2sc); ++idx) {
		const unsigned sc = vk2sc[ idx ];
		const unsigned vk = idx & 0xff;
		PhysicalKey *pk;

		if (0 == sc) // void
			continue;

		assert(def->PhysicalKeysCount < _countof(def->PhysicalKeys));
		if (def->PhysicalKeysCount == _countof(def->PhysicalKeys)) {
			break; // overflow
		}
		pk = def->PhysicalKeys + def->PhysicalKeysCount++;

		pk->SC = sc;
		pk->VK = vk;

		GetKeyNameTextW(sc << 16, pk->Name, _countof(pk->Name)-1);
		if (pk->Name[0] < ' ') // unprintable
			pk->Name[0] = 0;

		for (unsigned s = 0; s < _countof(states); ++s) {
			const struct State *state = states + s;
			wchar_t keys[4];

			keyState[VK_SHIFT]    = (state->shift   ? 0x80 : 0);
			keyState[VK_CONTROL]  = (state->control ? 0x80 : 0);
			keyState[VK_MENU]     = (state->alt     ? 0x80 : 0);
			keyState[VK_CAPITAL]  = (state->capital ? 0x01 : 0);

			if (1 == ToUnicodeEx(vk, sc, keyState, keys, _countof(keys), 0, hkl)) {
				PhysicalResults *results = def->Results + def->ResultsCount++;

				if (pk->Count++ == 0)
					pk->Results = results;
				assert(pk->Results + (pk->Count - 1) == results);

				if (keys[0] >= ' ') { // printable
					results->Text[0] = keys[0];
				} else { 
					results->TextCodepoints = keys[0];
				}

				if (state->shift)
					results->With |= WITH_SHIFT;
				if (state->capital)
					results->With |= WITH_CAPITAL;
				if (state->control)
					results->With |= WITH_CONTROL;
				if (state->alt)
					results->With |= WITH_MENU;
			}
		}
	}

	return def;
}


/*
 *  VirtualKeys ---
 *      Iterate Scan Code (SC) values and get the valid Virtual Key (VK) values for them.
 **/
static unsigned
VirtualKeys(const HKL hkl, unsigned *vk2sc)
{
	unsigned count = 0;

//#define VKDUMP
#if defined(VKDUMP)
	printf("\n");
	printf(" SC      VK\n");
	printf(" ===========================================\n");
#endif

	// Iterate Scan Code (SC) values and get the valid Virtual Key (VK) values in it.
	for (unsigned sc = 0x01; sc <= 0x79; ++sc) { // standard, #01..#79
		unsigned vk = MapVirtualKeyExW(sc, MAPVK_VSC_TO_VK_EX, hkl);
		if (vk) {
			assert(vk <= 0xff);
			if (0 == vk2sc[vk]) { // unique
				vk2sc[vk] = sc;
			}
#if defined(VKDUMP)
			printf("%c%02x      %02x - %s\n", (vk2sc[vk] == sc ? ' ' : '*'),
			    sc, vk, KBVirtualKeyName(vk));
#endif
			++count;
		}
	}

	for (unsigned sc = 0xe001; sc <= 0xe079; ++sc) { // enhanced 0xE001..0xE079
		unsigned vk = MapVirtualKeyExW(sc, MAPVK_VSC_TO_VK_EX, hkl);
		if (vk) {
			assert(vk <= 0xff);
			if (0 == vk2sc[vk|0x100]) { // unique
				vk2sc[vk|0x100] = sc;
			}
#if defined(VKDUMP)
			printf("%c%04x    %02x - %s\n", (vk2sc[vk|0x100] == sc ? ' ' : '*'),
			    sc, vk, KBVirtualKeyName(vk));
#endif
			++count;
		}
	}

#if defined(VKDUMP)
	printf(" ===========================================\n");
	printf(" Total:  %u\n", count);
#endif

	return count;
}

//end

