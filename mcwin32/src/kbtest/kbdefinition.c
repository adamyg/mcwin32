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

#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h>
#include <getopt.h>
#include <assert.h>

#include <expat.h>

#include "kbdefinition.h"
#include "kbvirtualextra.h"
#include "kbmap.h"


#if !defined(_countof)
#define _countof(a) (sizeof(a)/sizeof(a[0]))
#endif

typedef enum Element {
	elmNone			= 0,
	elmKeyboardLayout	= 1,
	elmPhysicalKeys		= 2,
	elmPK			= 3,
	elmResult		= 4,
	elmDeadKeyTable		= 51,
	elmDeadKeyTableResult	= 52
} Element;

typedef struct {
	XML_Parser	parser;
	Element 	element;
	KBDefinition	*layout;
	FILE		*file;
	char		buffer[8 * 1024];
	char		error[256];
	PhysicalKey	*current;
} ParserContext;

static int __cdecl PhysicalKeysCompare(const void *a, const void *b);
static void XMLCALL OnStartElement(void *data, const char *name, const char **attrs);
static void XMLCALL OnEndElement(void *data, const char *name);
static void XMLCALL OnTextElement(void *data, const char *s, int len);


/*
 *  Keyboard definition ---
 *      Load an external keyboard definition.
 **/
KBDefinition *
KBDefinitionLoad(const char *source)
{
	XML_Parser parser = NULL;
	ParserContext *ctx = NULL;
	KBDefinition *ret = NULL;

	if (NULL == (parser = XML_ParserCreate(NULL))) {
		printf("Failed to create XML parser.");
		return NULL;
	}

	if (NULL == (ctx = (ParserContext *)calloc(sizeof(ParserContext), 1))) {
		printf("XML parser error: alloc context\n");

	} else if (NULL == (ctx->layout = (KBDefinition *)calloc(sizeof(KBDefinition), 1))) {
		printf("XML parser error: alloc context\n");

	} else {
		XML_SetElementHandler(parser, OnStartElement, OnEndElement);
		XML_SetCharacterDataHandler(parser, OnTextElement);
		XML_SetUserData(parser, ctx);

		if (NULL == (ctx->file = fopen(source, "r"))) {
			printf("XML parser error: opening source <%s> : %s\n", source, strerror(errno));

		} else {
			int done = XML_TRUE;

			ret = ctx->layout;
			ctx->parser = parser;
			do {
				const int length = (int)fread(ctx->buffer, 1, sizeof(ctx->buffer), ctx->file);

				done = ((length < (int)sizeof(ctx->buffer) && feof(ctx->file)) ? XML_TRUE : XML_FALSE);
				if (XML_Parse(parser, ctx->buffer, length, done) == XML_STATUS_ERROR) {
					if (ctx->error[0]) {
						printf("XML parser error: %s\n", ctx->error);
					} else {
						const enum XML_Error err = XML_GetErrorCode(parser);
						printf("XML parser error: %s (%d)\n", XML_ErrorString(err), (int)err);
					}
					ret = NULL;
					break;
				}
			} while (!done);
		}
	}

	XML_ParserFree(parser);
	if (NULL == ret) {
		free((void *)ctx->layout);
	} else {
		qsort(ret->PhysicalKeys, ret->PhysicalKeysCount, sizeof(ret->PhysicalKeys[0]), PhysicalKeysCompare);
	}
	free((void *)ctx);
	return ret;
}


static int __cdecl
PhysicalKeysCompare(const void *a, const void *b)
{
	const PhysicalKey *v1 = (const PhysicalKey *)(a), *v2 = (const PhysicalKey *)(b);
	if (v1->VK < v2->VK)
		return -1;
	if (v1->VK > v2->VK)
		return 1;
	return 0;
}


void
KBDefinitionFree(KBDefinition *layout)
{
	free(layout);
}


///////////////////////////////////////////////////////////////////////////////
//  Parser callbacks
//
//  Example: https://kbdlayout.info/kbdgr/download/xml
//
//  <KeyboardLayout RightAltIsAltGr="true" ShiftCancelsCapsLock="false" ChangesDirectionality="false">
//      <PhysicalKeys>
//          <PK VK="VK_ESCAPE" SC="01" Name="ESC">
//              <Result TextCodepoints="001B"/>
//              <Result TextCodepoints="001B" With="VK_SHIFT"/>
//              <Result TextCodepoints="001B" With="VK_CONTROL"/>
//          </PK>
//              :
//      </PhysicalKeys>
//  </KeyboardLayout>
//

static const char tagKeyboardLayout[]	= "KeyboardLayout";
static const char tagPhysicalKeys[]	= "PhysicalKeys";
static const char tagPK[]		= "PK";
static const char tagResult[]		= "Result";
static const char tagDeadKeyTable[]	= "DeadKeyTable";

static int AttributeBOOL(const char *value);
static unsigned AttributeWITH(const char *value);

static void
ParserError(ParserContext* ctx, const char *fmt, ...)
{
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = snprintf(ctx->error, sizeof(ctx->error)-1, "%u: ", (unsigned)XML_GetCurrentLineNumber(ctx->parser));
	vsnprintf(ctx->error + len, (sizeof(ctx->error) - 1) - len, fmt, ap);
	XML_StopParser(ctx->parser, XML_FALSE);
	va_end(ap);
}


static void XMLCALL
OnStartElement(void *data, const char *name, const char **attrs)
{
	ParserContext* ctx = (ParserContext*)(data);
	KBDefinition *layout = ctx->layout;

	if (elmDeadKeyTable == ctx->element) {
		if (0 == strcmp(name, tagResult)) {
			ctx->element = elmDeadKeyTableResult;
			return;
		}
		ParserError(ctx, "<%s/%s> expected", tagDeadKeyTable, tagResult);

	} else if (elmResult == ctx->element) {
		if (0 == strcmp(name, tagDeadKeyTable)) {
			ctx->element = elmDeadKeyTable;
			return;
		}
		ParserError(ctx, "<%s> expected", tagResult);

	} else if (elmPK == ctx->element) {
		if (0 == strcmp(name, tagResult)) {
			PhysicalResults *results;

			ctx->element = elmResult;
			if (layout->ResultsCount > _countof(layout->Results)) {
				ParserError(ctx, "<%s> key element limit of %s exceeded", tagPK, _countof(layout->Results));
				return;
			}

			assert((0 == ctx->current->Count && NULL == ctx->current->Results) || (0 != ctx->current->Count && NULL != ctx->current->Results));
			if (0 == ctx->current->Count) {
				ctx->current->Results = layout->Results + layout->ResultsCount;
			}
			results = ctx->current->Results + ctx->current->Count++;
			++layout->ResultsCount;

			for (unsigned i = 0; attrs[i];) {
				const char *tag = attrs[i++], *value = attrs[i++];

				if (0 == strcmp(tag, "Text")) {
					MultiByteToWideChar(CP_UTF8, 0, value, -1, results->Text, _countof(results->Text)-1);

				} else if (0 == strcmp(tag, "TextCodepoints")) {
					results->TextCodepoints = (wchar_t)strtoul(value, 0, 16);

				} else if (0 == strcmp(tag, "VK")) {
					if (results->VK) {
						ParserError(ctx, "<%s> duplicate 'VK' value <%s>", tagResult, value);
						return;
					}
					if (0 == (results->VK = KBVirtualKeyValue(value))) {
						ParserError(ctx, "<%s> unknown 'VK' value <%s>", tagResult, value);
						return;
					}

				} else if (0 == strcmp(tag, "With")) {
					if (results->With) {
						ParserError(ctx, "<%s> duplicate 'With' value <%s>", tagResult, value);
						return;
					}
					if (0 == (results->With = AttributeWITH(value))) {
						ParserError(ctx, "<%s> unknown 'With' value <%s>", tagResult, value);
						return;
					}

				} else {
					ParserError(ctx, "<%s> unexpected attribute <%s>", tagResult, tag);
					return;
				}
			}
			return;
		}
		ParserError(ctx, "<%s> expected", tagResult);

	} else if (elmPhysicalKeys == ctx->element) {
		if (0 == strcmp(name, tagPK)) {
			PhysicalKey *pk;

			ctx->element = elmPK;

			if (layout->PhysicalKeysCount > _countof(layout->PhysicalKeys)) {
				ParserError(ctx, "<%s> key element limit of %s exceeded", tagPK, _countof(layout->PhysicalKeys));
				return;
			}
			pk = ctx->current = layout->PhysicalKeys + layout->PhysicalKeysCount++;

			for (unsigned i = 0; attrs[i];) {
				const char *tag = attrs[i++], *value = attrs[i++];

				if (0 == strcmp(tag, "SC")) {
					if (pk->SC) {
						ParserError(ctx, "<%s> duplicate 'SC' value <%s>", tagPK, value);
						return;
					}
					pk->SC = (unsigned)strtoul(value, 0, 16);

				} else if (0 == strcmp(tag, "VK")) {
					if (pk->VK) {
						ParserError(ctx, "<%s> duplicate 'VK' value <%s>", tagPK, value);
						return;
					}
					if (0 == (pk->VK = KBVirtualKeyValue(value))) {
						ParserError(ctx, "<%s> unknown 'VK' value <%s>", tagPK, value);
						return;
					}

				} else if (0 == strcmp(tag, "Name")) {
					MultiByteToWideChar(CP_UTF8, 0, value, -1, pk->Name, _countof(pk->Name)-1);

				} else {
					ParserError(ctx, "<%s> unexpected attribute <%s>", tagPK, tag);
					return;
				}
			}
			return;
		}
		ParserError(ctx, "<%s> expected", tagPK);

	} else if (elmKeyboardLayout == ctx->element) {
		if (0 == strcmp(name, tagPhysicalKeys)) {
			ctx->element = elmPhysicalKeys;
			return;
		}
		ParserError(ctx, "<%s> expected", tagPhysicalKeys);

	} else {
		if (0 == strcmp(name, tagKeyboardLayout)) {
			ctx->element = elmKeyboardLayout;
			for (unsigned i = 0; attrs[i];) {
				const char *tag = attrs[i++], *value = attrs[i++];

				if (0 == strcmp(tag, "RightAltIsAltGr")) {
					layout->RightAltIsAltGr = AttributeBOOL(value);
				} else if (0 == strcmp(tag, "ShiftCancelsCapsLock")) {
					layout->ShiftCancelsCapsLock = AttributeBOOL(value);
				} else if (0 == strcmp(tag, "ChangesDirectionality")) {
					layout->ChangesDirectionality = AttributeBOOL(value);
				} else {
					ParserError(ctx, "<%s> unexpected attribute <%s>", tagKeyboardLayout, tag);
					return;
				}
			}
			return;
		}
		ParserError(ctx, "<%s> expected", tagKeyboardLayout);
	}
}


static void XMLCALL
OnEndElement(void *data, const char *name)
{
	ParserContext* ctx = (ParserContext*)(data);

	if (elmDeadKeyTableResult == ctx->element) {
		if (0 == strcmp(name, tagResult)) {
			ctx->element = elmDeadKeyTable;
			return;
		}
		ParserError(ctx, "<%s> expected", tagResult);

	} else if (elmDeadKeyTable == ctx->element) {
		if (0 == strcmp(name, tagDeadKeyTable)) {
			ctx->element = elmResult;
			return;
		}
		ParserError(ctx, "<%s> expected", tagDeadKeyTable);

	} else if (elmResult == ctx->element) {
		if (0 == strcmp(name, tagResult)) {
			ctx->element = elmPK;
			return;
		}
		ParserError(ctx, "<%s> expected", tagResult);

	} else if (elmPK == ctx->element) {
		if (0 == strcmp(name, tagPK)) {
			ctx->element = elmPhysicalKeys;
			return;
		}
		ParserError(ctx, "<%s> expected", tagPK);

	} else if (elmPhysicalKeys == ctx->element) {
		if (0 == strcmp(name, tagPhysicalKeys)) {
			ctx->element = elmKeyboardLayout;
			return;
		}
		ParserError(ctx, "<%s> expected", tagPhysicalKeys);

	} else if (elmKeyboardLayout == ctx->element) {
		if (0 == strcmp(name, tagKeyboardLayout)) {
			ctx->element = elmNone;
			return;
		}
		ParserError(ctx, "<%s> expected", tagKeyboardLayout);

	} else {
		ParserError(ctx, "expected token");
	}
}


static void XMLCALL
OnTextElement(void *data, const char *s, int len)
{
	ParserContext* ctx = (ParserContext*)(data);
	if (NULL == ctx || 0 == len || s == NULL) return;
}


static int
AttributeBOOL(const char *value)
{
	if (0 == strcmp(value, "true")) return 1;
	if (0 == strcmp(value, "false")) return 0;
	return -1;
}


static unsigned
AttributeWITH(const char *value)
{
	static const struct mvk {
		char name[16];
		unsigned len;
		unsigned value;
#define MVK(__v,__c)	{ #__v, sizeof(#__v)-1, __c }
	} mvks[] = {
		MVK(VK_SHIFT, WITH_SHIFT),
		MVK(VK_CONTROL, WITH_CONTROL),
		MVK(VK_CAPITAL, WITH_CAPITAL),
		MVK(VK_NUMLOCK, WITH_NUMLOCK),
		MVK(VK_MENU, WITH_MENU)
		};

	unsigned ret = 0;
	if (value && *value) {
		do {
			const char *end;
			unsigned len = 0;
			while (' ' == *value) ++value; // leading whitespace
			for (end = value; *end;) {
				if (*end == ' ') break;
				++end, ++len;
			}
			if (len) {
				unsigned t_ret = 0;
				for (const struct mvk *it = mvks, *itend = it + _countof(mvks); it != itend; ++it) {
					if (len == it->len && 0 == strncmp(value, it->name, len)) {
						t_ret = it->value;
						break;
					}
				}
				if (0 == t_ret) return 0; // unmatched
				ret |= t_ret;
			}
			value = end;
		} while (*value);
	}
	return ret;
}

//end
