#ifndef KBLAYOUT_H_INCLUDED
#define KBLAYOUT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(kblayout_h,"$Id: kblayout.h,v 1.3 2024/02/27 17:18:08 cvsuser Exp $")
__CPRAGMA_ONCE

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

struct KBRow {
#define VK_ISENHANCED   0x100   // Enhanced VirtualKey

	int vk;                 // Virtual key {-1} = None.
	const wchar_t *label1;  // First key label.
	int width;              // Optional, text is centred width.
	int sc;                 // Optional, Scancode.
	const wchar_t *label2;  // OPtional, second key label.
	const wchar_t *label3;  // Optional, third key label (AltGr).
};

const struct KBRow **KBLayoutDefault(void);
const struct KBRow **KBLayoutBuild(const struct KBDefinition *def);

#endif //KBLAYOUT_H_INCLUDED

//end

