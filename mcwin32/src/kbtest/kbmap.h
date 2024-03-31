#ifndef KBMAP_H_INCLUDED
#define KBMAP_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(kbmap_h,"$Id: kbmap.h,v 1.6 2024/02/27 17:18:08 cvsuser Exp $")
__CPRAGMA_ONCE

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

#include <sys/cdefs.h>

enum key_modifier {
	MODIFIER_SHIFT          = (1 << 0),
	MODIFIER_CONTROL        = (1 << 1),
	MODIFIER_ALT            = (1 << 2),
	MODIFIER_NUMLOCK        = (1 << 3),
	MODIFIER_SCROLLLOCK     = (1 << 4),
	MODIFIER_CAPSLOCK       = (1 << 5),
	MODIFIER_LOGO           = (1 << 6)
};

#define KEY_INVALID -1

typedef struct event {
	unsigned vkmodifiers;   // Cooked VK modifiers.
	unsigned vkkey;         // VK_xxxx key code; otherwise 0.
	unsigned vkext;         // Extended VK_xxxx key code; otherwise 0.
	const char *vkname;     // VK symbol name; otherwise NULL.
	const char *kblabel;    // XKB label; otherwise NULL.
	int vkenhanced;         // Whether an enhanced VK, 1 otherwise 0.
	unsigned kbmodifiers;   // MODIFIER_XXXK(s) <enum key_modifier.
	int ascii;              // ASCII character value; otherwise -1
	int unicode;            // Unicode character value; otherwise -1
} key_event_t;

__BEGIN_DECLS

int KBMapEvent(const KEY_EVENT_RECORD *key, key_event_t *evt);
const char *KBVirtualKeyName(unsigned vk);
unsigned KBVirtualKeyValue(const char *name);
DWORD KBNormalizeAltGr(const KEY_EVENT_RECORD *key);
unsigned KBMapModifiers(const DWORD dwControlKeyState, int apps);
int KBPrintModifiers(const DWORD dwControlKeyState, char *buffer, size_t buflen, unsigned mask);

__END_DECLS

#endif // KBMAP_H_INCLUDED

//end

