#ifndef KBDEFINITION_H_INCLUDED
#define KBDEFINITION_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(kbdefinition_h,"$Id: kbdefinition.h,v 1.6 2025/01/29 13:33:04 cvsuser Exp $")
__CPRAGMA_ONCE

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

typedef struct Result {
	unsigned        VK;
	wchar_t         Text[31];
	wchar_t         TextCodepoints;
#define WITH_SHIFT          (1<<0)
#define WITH_CONTROL        (1<<1)
#define WITH_CAPITAL        (1<<2)
#define WITH_NUMLOCK        (1<<3)
#define WITH_MENU           (1<<4)
	unsigned        With;
} PhysicalResults;

typedef struct {
	unsigned        VK;
	unsigned        SC;
	wchar_t         Name[31];
	unsigned        Count;
	PhysicalResults *Results;
} PhysicalKey;

typedef struct KBDefinition {
	int             RightAltIsAltGr;
	int             ShiftCancelsCapsLock;
	int             ChangesDirectionality;
	unsigned        PhysicalKeysCount;
	unsigned        ResultsCount;
	PhysicalKey     PhysicalKeys[256];
	PhysicalResults Results[245*4];
} KBDefinition;

KBDefinition *KBDefinitionCurrent(void);
KBDefinition *KBDefinitionLoad(const char *source);
void KBDefinitionFree(KBDefinition *);

#endif //KBDEFINITION_H_INCLUDED

//end

