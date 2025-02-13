#include <edidentifier.h>
__CIDENT_RCSID(kblayout_c, "$Id: kbdump.c,v 1.7 2025/01/29 13:33:04 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * console keyboard test application
 *
 * Copyright (c) 2024 - 2025, Adam Young.
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

#include "kbdump.h"
#include "kbdefinition.h"
#include "kbmap.h"
#include "kbconsole.h"
#include "kbutil.h"


/*
 *  KBDump ---
 *      Dump the keyboard scan-code, virtual-key definition.
 **/
void
KBDump(const struct KBDefinition *def)
{
	static const unsigned with[] = {
		0,
		WITH_SHIFT,
		WITH_CAPITAL,
		WITH_SHIFT|WITH_CAPITAL,
		WITH_CONTROL,
		WITH_SHIFT|WITH_CONTROL,
		WITH_CONTROL|WITH_MENU,
		WITH_SHIFT|WITH_CONTROL|WITH_MENU
		};

	cprinta("\n");
	cprinta(" SC      VK                                  TEXT                           _       s       C       sC      c       sc      ca      sca\n");
	cprinta(" ==========================================================================================================================================\n");
	    //   1234567 12 - 123456789012345678901234567890 123456789012345678901234567890 1234567 1234567 1234567 1234567 1234567 1234567 1234567 1234567

	for (const PhysicalKey *pk = def->PhysicalKeys, *end = pk + def->PhysicalKeysCount; pk != end; ++pk) {
		const unsigned sc = pk->SC;
		const unsigned vk = pk->VK;
		wchar_t values[_countof(with)] = {0};

		for (const PhysicalResults *rs = pk->Results, *rsend = rs + pk->Count; rs != rsend; ++rs) {
			for (unsigned w = 0; w < _countof(with); ++w) {
				if (rs->With == with[w]) {
					if (rs->TextCodepoints) {
						values[w] = rs->TextCodepoints;
					} else {
						values[w] = rs->Text[0];
					}
					break;
				}
			}
		}

		if (0xE000 & sc) {
			cprinta(" %04x    ", sc);
		} else {
			cprinta(" %02x      ", sc);
		}

		cprinta("%02x - %-30.30s ", vk, KBVirtualKeyName(vk));
		cprintw(L"%-30.30s ", pk->Name);

		for (unsigned v = 0; v < _countof(values); ++v) {
			const wchar_t value = values[v];
			if (value) {
				if (value & 0xff00) {
					cprinta("%04x    ", value);
				} else {
					cprinta("%02x      ", value);
				}
			} else {
				cprinta("-1      ");
			}
		}

		cprinta("\n");
	}
	cprinta("\n");
}

//end
