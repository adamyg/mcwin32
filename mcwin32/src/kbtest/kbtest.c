#include <edidentifier.h>
__CIDENT_RCSID(btest_c,"$Id: kbtest.c,v 1.10 2025/01/29 13:33:04 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * console keyboard test application
 *
 * Copyright (c) 2015 - 2025, Adam Young.
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
#include <shlwapi.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h>
#include <getopt.h>
#include <assert.h>

#include "expat.h"

#include "kbmap.h"
#include "kbconsole.h"
#include "kbdefinition.h"
#include "kblayout.h"
#include "kbdump.h"

#include "kbbuildinfo.h"
#include "kbutil.h"

struct iobuf {
	unsigned length;
	unsigned char text[128];
};

struct record {
	key_event_t evt;
	const char *str;
	int key;
};

#if !defined(ENABLE_VIRTUAL_TERMINAL_PROCESSING)
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
	// When writing with WriteFile or WriteConsole, characters are parsed for VT100 and similar control
	// character sequences that control cursor movement, color/font mode, and other operations that can
	// also be performed via the existing Console APIs.
#endif
#if !defined(DISABLE_NEWLINE_AUTO_RETURN)
#define DISABLE_NEWLINE_AUTO_RETURN 0x0008
	// When writing with WriteFile or WriteConsole, this adds an additional state to end-of-line wrapping
	// that can delay the cursor move and buffer scroll operations.
#endif

#define VKS_DONE	0x01			// VK key status
#define VKS_SHIFT	0x02
#define VKS_CTRL	0x04
#define VKS_ALT		0x08
#define VKS_APPS	0x10
#define VKS_PRESS	0x40
#define VKS_ON		0x80

static void EnvironmentStatus(void);

static void KeyboardPush(HANDLE console, const struct KBRow **rows, BYTE *status, const key_event_t *evt);
static void KeyboardStatus(HANDLE console, const struct KBRow **rows, BYTE *status);
static int  KeyboardKey(HANDLE console, unsigned height, const struct KBRow *key, BYTE *status);

static void Usage(const char *msg, ...);
static void Version(void);

static BOOL WINAPI ConsoleHandler(DWORD dwCtrlType);
static void XKBTranslation(const key_event_t *evt);
static const char *latin1name(unsigned value);

static volatile int ctrl_break = 0;

#define SHORTOPTIONS "ak:l:V"

static const struct option long_options[] = {
	{ "help",       no_argument,        NULL, 1000 },
	{ "klid",       required_argument,  NULL, 'k'  },
	{ "layout",     required_argument,  NULL, 'l'  },
	{ "version",    no_argument,        NULL, 'V'  },
	{ NULL }
	};

static unsigned oklid = 0;
static const char *olayout = NULL;

int
main(int argc, char *argv[])
{
	char szKLID[KL_NAMELENGTH] = {0};
	DWORD consolemode = 0, oconsolemode = 0;
	BYTE vkstatus[256*3 /*vk,VK+,ascii*/] = {0};

	const struct KBDefinition *layout = NULL;
	const struct KBRow **rows = KBLayoutDefault();
	HANDLE console, oconsole;
	const char *cmd = "";
	INPUT_RECORD ir;
	int esc = 0, ch;

	// command line
	while ((ch = getopt_long(argc, argv, SHORTOPTIONS, long_options, NULL)) != -1)
		switch(ch) {
		case 'k':  // --klid=<id>
			oklid = (unsigned) strtoull(optarg, NULL, 0);
			break;
		case 'l':  // --layout=<definition>
			olayout = optarg;
			break;
		case 'V':  // --version
			Version();
			break;
		case 1000: // --help
		case '?':
			Usage(NULL);
			break;
		}
	argv += optind;
	if ((argc -= optind) < 1) {
		Usage("missing command");
	} else if (argc > 1) {
		Usage("unexpected argument(s) <%s ...>", argv[1]);
	}

	cmd = argv[0];
	if (0 != strcmp(cmd, "dump") && 0 != strcmp(cmd, "test")) {
		Usage("invalid command <%s>", argv[0]);
	}

	// setup
	console = GetStdHandle(STD_INPUT_HANDLE);
	oconsole = GetStdHandle(STD_OUTPUT_HANDLE);

	if (oklid) { // optional KLID
		char t_szKLID[KL_NAMELENGTH] = {0};

		snprintf(t_szKLID, sizeof(t_szKLID), "%08u", oklid);
		GetKeyboardLayoutNameA(szKLID);
		LoadKeyboardLayoutA(t_szKLID, KLF_ACTIVATE);

		// Layout must be already loaded, inform GUI
		// PostMessage(GetConsoleWindow(), WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)klid);
	}

	if (olayout) { // external layout
		if (NULL == (layout = KBDefinitionLoad(olayout)))
			exit(EXIT_FAILURE);
		rows = KBLayoutBuild(layout);
	} else {
		layout = KBDefinitionCurrent();
		rows = KBLayoutBuild(layout);
	}

	// execute
	if (0 == strcmp(cmd, "dump")) {
		EnvironmentStatus();
		KBDump(layout);
		return 0;
	}

	if (! GetConsoleMode(console, &consolemode) ||
		    ! GetConsoleMode(oconsole, &oconsolemode)) {
		Usage("console expected");
	}

	SetConsoleMode(console, ENABLE_WINDOW_INPUT);
	SetConsoleMode(oconsole, oconsolemode | \
		ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
	SetConsoleCtrlHandler(ConsoleHandler, TRUE);

	ConsoleClear(oconsole); // prime display
	KeyboardStatus(oconsole, rows, vkstatus);
	EnvironmentStatus();
	cprinta("Press <ESC> consecutively 3 times to exit.\n");

	while (! ctrl_break && esc < 3) { // foreach(key)
		const KEY_EVENT_RECORD *key = &ir.Event.KeyEvent;
		char modifiers[128];
		key_event_t evt = {0};
		unsigned scanCode;
		DWORD count;
		int cio = 1;

		// next event
		if (WAIT_OBJECT_0 != WaitForSingleObject(console, INFINITE) || ctrl_break ||
			    0 == ReadConsoleInputW(console, &ir, 1, &count) || ctrl_break) {
			break;			// eof or Ctrl-Break
		}

		if (WINDOW_BUFFER_SIZE_EVENT == ir.EventType) {
			ConsoleClear(oconsole);
			KeyboardStatus(oconsole, rows, vkstatus);
			EnvironmentStatus();
			continue;
		} else if (!count || ir.EventType != KEY_EVENT || !key->bKeyDown) {
			continue; // ignore non-key down events
		}

		// decode event
		esc = (0x1b == key->uChar.AsciiChar ? esc + 1 : 0);
		cio = KBMapEvent(key, &evt);

		ConsoleHome(oconsole);
		KeyboardPush(oconsole, rows, vkstatus, &evt);

		scanCode = key->wVirtualScanCode | (key->dwControlKeyState & ENHANCED_KEY ? 0xE000 : 0);
		KBPrintModifiers(key->dwControlKeyState, modifiers, sizeof(modifiers), TRUE);
		if (key->uChar.AsciiChar >= 0x20 && key->uChar.AsciiChar < 0x7f /*isprint*/) { // ASCII
			cprinta("%sVK:%u/0x%x SCAN:0x%04x/0x%04X Ascii(0x%x/%c) ",
			    modifiers, key->wVirtualKeyCode, key->wVirtualKeyCode, key->wVirtualScanCode, scanCode, key->uChar.AsciiChar, key->uChar.AsciiChar);
		} else { // UNICODE
			cprinta("%sVK:%u/0x%x SCAN:0x%04x/0x%04X Unicode(0x%x) ",
			    modifiers, key->wVirtualKeyCode, key->wVirtualKeyCode, key->wVirtualScanCode, scanCode, key->uChar.UnicodeChar);
		}
		XKBTranslation(&evt);

		// decode KEY_EVENT
		if (cio) {
			KBPrintModifiers(evt.vkmodifiers, modifiers, sizeof(modifiers), FALSE);
			cprinta("\n => %sVK(%u) ASCII(0x%x/%c)",
			    modifiers, evt.vkkey, evt.ascii, (evt.ascii >= 0x20 && evt.ascii <= 0x7f ? evt.ascii : ' '));
			if (evt.unicode != KEY_INVALID) {
				cprintw(L" UNICODE(U+%0*x/%c)", (evt.unicode & 0xff0000 ? 6 : 4), (unsigned)evt.unicode, (unsigned)evt.unicode);
			}
		} else {
			cprinta("\n => <dead>");
		}
		EnvironmentStatus();
	}

	if (szKLID[0]) LoadKeyboardLayoutA(szKLID, KLF_ACTIVATE);
	SetConsoleMode(oconsole, oconsolemode);
	SetConsoleMode(console, consolemode);
	return 0;
}


/*
 * KeyboardPush ---
 *	Push the event status into the keyboard-status buffer.
 **/
static void
KeyboardPush(HANDLE console, const struct KBRow **rows, BYTE *status, const key_event_t *evt)
{
	if (evt->vkext) { // extended key-code.
		const unsigned vkext = evt->vkext;
		if (evt->vkenhanced) {
			/*if (0 == evt->vkmodifier)*/ status[vkext|VK_ISENHANCED] |= VKS_DONE;
			status[vkext|VK_ISENHANCED] |= VKS_PRESS;
		} else {
			/*if (0 == evt->vkmodifier)*/ status[vkext] |= VKS_DONE;
			status[vkext] |= VKS_PRESS;
		}
	}

	if (evt->vkkey) { // primary key-code
		const unsigned modifiers = evt->kbmodifiers;
		const unsigned vkkey = evt->vkkey;

		if (evt->vkenhanced) {
			/*if (0 == evt->vkmodifier)*/ status[vkkey|VK_ISENHANCED] |= VKS_DONE;
			status[vkkey|VK_ISENHANCED] |= VKS_PRESS;
		} else {
			/*if (0 == evt->vkmodifier)*/ status[vkkey] |= VKS_DONE;
			status[vkkey] |= VKS_PRESS;
		}

		status[VK_SHIFT|VK_ISENHANCED] = 0;
		if (modifiers & MODIFIER_SHIFT) {
			status[VK_SHIFT|VK_ISENHANCED] = VKS_ON;
			status[vkkey] |= VKS_SHIFT;
		}

		status[VK_CONTROL|VK_ISENHANCED] = 0;
		if (modifiers & MODIFIER_CONTROL) {
			status[VK_CONTROL|VK_ISENHANCED] = VKS_ON;
			status[vkkey] |= VKS_CTRL;
		}

		status[VK_MENU|VK_ISENHANCED] = 0;
		if (modifiers & MODIFIER_ALT) {
			status[VK_MENU|VK_ISENHANCED] = VKS_ON;
			status[vkkey] |= VKS_ALT;
		}

		status[VK_APPS|VK_ISENHANCED] = 0;
		if (modifiers & MODIFIER_LOGO) {
			status[VK_APPS|VK_ISENHANCED] = VKS_ON;
			status[vkkey] |= VKS_APPS;
		}
	}

	KeyboardStatus(console, rows, status);
}


/*
 * EnvironmentStatus
 */
static void
EnvironmentStatus(void)
{
	wchar_t klid[KL_NAMELENGTH] = { 0 };
	wchar_t iso639[16] = { 0 }, iso3166[16] = { 0 }, displayname[256] = { 0 };
	const LCID slcid = GetSystemDefaultLCID();
	const LCID ulcid = GetUserDefaultLCID();
	const LCID tlcid = GetThreadLocale();

	cprintw(L"\n");

	GetKeyboardLayoutNameW(klid);
	cprintw(L"\nKLID: <%s>, type=0x%x/0x%x, fns=%u", klid, GetKeyboardType(0), GetKeyboardType(1), GetKeyboardType(2));

	cprintw(L"\nLCID:");
	cprintw(L" sys=%u/0x%x", slcid, slcid);
	if (GetLocaleInfoW(slcid, LOCALE_SISO639LANGNAME, iso639, _countof(iso639)) &&
		  GetLocaleInfoW(slcid, LOCALE_SISO3166CTRYNAME, iso3166, _countof(iso3166))) {
		  GetLocaleInfoW(slcid, LOCALE_SLOCALIZEDCOUNTRYNAME, displayname, _countof(displayname));
		cprintw(L" <%s_%s> (%s)", iso639, iso3166, displayname); // "9_9 (displayname)"
	}

	cprintw(L", user=%u/0x%x", ulcid, ulcid);
	if (GetLocaleInfoW(ulcid, LOCALE_SISO639LANGNAME, iso639, _countof(iso639)) &&
		  GetLocaleInfoW(ulcid, LOCALE_SISO3166CTRYNAME, iso3166, _countof(iso3166))) {
		  GetLocaleInfoW(ulcid, LOCALE_SLOCALIZEDCOUNTRYNAME, displayname, _countof(displayname));
		cprintw(L" <%s_%s> (%s)", iso639, iso3166, displayname); // "9_9 (displayname)"
	}

	cprintw(L", thr=%u/0x%x", tlcid, tlcid);
	if (GetLocaleInfoW(tlcid, LOCALE_SISO639LANGNAME, iso639, _countof(iso639)) &&
		  GetLocaleInfoW(tlcid, LOCALE_SISO3166CTRYNAME, iso3166, _countof(iso3166))) {
		  GetLocaleInfoW(tlcid, LOCALE_SLOCALIZEDCOUNTRYNAME, displayname, _countof(displayname));
		cprintw(L" <%s_%s> (%s)", iso639, iso3166, displayname); // "9_9 (displayname)"
	}

	cprintw(L"\nCP:   oem=%u/0x%x, acp=%u/0x%x, cin=%u/0x%x, cout=%u/0x%x",
	    GetOEMCP(), GetOEMCP(), GetACP(), GetACP(), GetConsoleCP(), GetConsoleCP(), GetConsoleOutputCP(), GetConsoleOutputCP());

	cprintw(L"\n\n");
}


/*
 * KeyboardStatus ---
 *	Echo the current keyboard status.
 **/
static void
KeyboardStatus(HANDLE console, const struct KBRow **rows, BYTE *status)
{
	COORD coord = {0, 1};
	const struct KBRow *key;
	unsigned r, c;

	for (r = 0; rows[r]; ++r) {
		for (c = 0, key = rows[r]; key->vk; ++key) {
			coord.X = (SHORT)(c + 1);
			SetConsoleCursorPosition(console, coord);
			c += KeyboardKey(console, 2, key, status);
		}
                ConsoleCEOL(console);
		coord.Y += 1;

		for (c = 0, key = rows[r]; key->vk; ++key) {
			coord.X = (SHORT)(c + 1);
			SetConsoleCursorPosition(console, coord);
			c += KeyboardKey(console, 1, key, status);
		}
                ConsoleCEOL(console);
		coord.Y += 2;
	}
	coord.X = 0;
	SetConsoleCursorPosition(console, coord);
}


/*
 * KeyboardKey ---
 *	Echo the specified key status.
 **/
static int
KeyboardKey(HANDLE console, unsigned line, const struct KBRow *key, BYTE *status)
{
	const int vk = key->vk;

	if (vk > 0) {
		const wchar_t t_name[2] = {(wchar_t) '?', 0};
		const wchar_t *label = (2 == line ? (key->label2 ? key->label2 : L"") :
					(key->label1 ? key->label1 : t_name));
		const WORD vkstatus = status[vk];
		WCHAR wbuffer[128] = {0};
		DWORD len1, len2;
		int length, width;

		length = wcslen(label);
		width = (key->width ? key->width : 4); // default=4
		if (length >= width) { // limit
			len1 = wnsprintfW(wbuffer, _countof(wbuffer)-1, L"%-*.*s", width, width, label);
		} else { // center
			const int extra = width - length, left = extra / 2;
			len1 = wnsprintfW(wbuffer, _countof(wbuffer)-1, L"%*s%s%*s", left, L"", label, width - (left + length), L"");
		}

		if (1 == line && key->label3) { // AltGr
			length = wcslen(key->label3);
			if (length) {
				wbuffer[len1 - 1] = key->label3[0];
			}
		}

		SetConsoleTextAttribute(console, FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY|
			((vkstatus & VKS_PRESS)  ? BACKGROUND_GREEN :
			  ((vkstatus & VKS_DONE) ? BACKGROUND_RED   :
			   ((vkstatus & VKS_ON)  ? BACKGROUND_BLUE  : BACKGROUND_INTENSITY))));
		WriteConsoleW(console, wbuffer, len1, NULL, NULL);

		len2 = wsprintfW(wbuffer, L"%c%c%c", // control bits, width=3
		    (vkstatus & VKS_SHIFT) ? 'S' : ' ', (vkstatus & VKS_CTRL) ? 'C' : ' ',  (vkstatus & VKS_ALT) ? 'A' : ' ');
		SetConsoleTextAttribute(console, FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
		WriteConsoleW(console, wbuffer, len2, NULL, NULL);

		if (1 == line && (vkstatus & VKS_PRESS))
			status[vk] &= ~VKS_PRESS;

		return len1 + len2 + 1;
	}

	if (key->width) return key->width;
	return 8; //4+3+1
}


/*
 * Usage ---
 *      Command line usage.
 **/
static void
Usage(const char *msg, ...)
{
	if (msg) {
		va_list ap;
		va_start(ap, msg);
		printf("kbtest: "); vprintf(msg, ap); printf("\n\n");
		va_end(ap);
	}

	printf("kbtest - keyboard encode test application\n" \
	    "\n" \
	    "usage: kbtest [options] <command}\n" \
	    "\n" \
	    "Options:\n" \
	    "   --klid=<id>         keyboard layout ID.\n" \
	    "   --layout=<path>     Layout definition.\n" \
	    "\n" \
	    "   --version           application version.\n" \
	    "   --help              command line.\n" \
	    "\n" \
	    "Commands:\n" \
	    "    dump               report keyboard definition.\n" \
	    "    test               interactive.\n" \
	    "\n");
	exit(3);
	/*NOTREACHED*/
}


/*
 * Version ---
 *      Application build information.
 **/
static void
Version(void)
{
	printf("kbtest - keyboard encode test application\n");
	exit(3);
	/*NOTREACHED*/
}


/*
 * HandlerRoutine ---
 *      Console event/signal handler.
 **/
static BOOL WINAPI
ConsoleHandler(DWORD dwCtrlType)
{
	switch(dwCtrlType) {
	case 3: // 3 is reserved!
	case 4: // 4 is reserved!
		assert(0);
		break;
	case CTRL_BREAK_EVENT: // Ctrl-Break
		++ctrl_break;			// exit main-loop
		SetEvent(GetStdHandle(STD_INPUT_HANDLE));
		return TRUE;
	}
	return FALSE;
}


static void
XKBTranslation(const key_event_t *evt)
{
	char text[128] = {0}, *cursor = text;
	const char *name;

	// modifiers
	if (evt->kbmodifiers) {
		const unsigned kbmodifiers = evt->kbmodifiers;
		unsigned modifiers = 0;

		if (kbmodifiers & MODIFIER_LOGO   ) cursor += sprintf(cursor, "%sLogo",  (modifiers++ ? "-" : ""));
		if (kbmodifiers & MODIFIER_ALT    ) cursor += sprintf(cursor, "%sAlt",   (modifiers++ ? "-" : ""));
		if (kbmodifiers & MODIFIER_CONTROL) cursor += sprintf(cursor, "%sCtrl",  (modifiers++ ? "-" : ""));
		if (kbmodifiers & MODIFIER_SHIFT  ) cursor += sprintf(cursor, "%sShift", (modifiers++ ? "-" : ""));
	}

	// event
	cursor += sprintf(cursor, "<Key>");
	if (evt->kblabel) { // resolved key-symbol.
		cursor += sprintf(cursor, "%s", evt->kblabel);

	} else if (evt->ascii && NULL != (name = latin1name(evt->ascii))) {
		cursor += sprintf(cursor, "%s", name); // latin1 key-symbol.

	} else if (evt->vkname) { // VK name.
		cursor += sprintf(cursor, "%s", evt->vkname);

	} else if (evt->ascii >= 0x20 && evt->ascii <= 0x7f /*isprint*/) {
		cursor += sprintf(cursor, "%c", evt->ascii); // value; unshifted see cio decoder.

	} else if (evt->unicode != KEY_INVALID) {
		cursor += sprintf(cursor, "0x%u", evt->unicode); // unicode.
	}

	assert(cursor < (text + sizeof(text)));
	cprinta("XKB: %s", text);
}


static const char *
latin1name(unsigned value)
{
	switch (value) {
	    // controls

	case 0x20: return "space";		// !   !
	case 0x21: return "exclam";		// ! ! !
	case 0x22: return "quotedbl";		// ! " !
	case 0x23: return "numbersign";		// ! # !
	case 0x24: return "dollar";		// ! $ !
	case 0x25: return "percent";		// ! % !
	case 0x26: return "ampersand";		// ! & !
	case 0x27: return "apostrophe";		// ! \ !
	case 0x28: return "parenleft";		// ! ( !
	case 0x29: return "parenright";		// ! ) !
	case 0x2a: return "asterisk";		// ! * !
	case 0x2b: return "plus";		// ! + !
	case 0x2c: return "comma";		// ! , !
	case 0x2d: return "minus";		// ! - !
	case 0x2e: return "period";		// ! . !
	case 0x2f: return "slash";		// ! / !
	    // 0 .. 9
	case 0x3a: return "colon";		// ! : !
	case 0x3b: return "semicolon";		// ! ; !
	case 0x3c: return "less ";		// ! < !
	case 0x3d: return "equal";		// ! = !
	case 0x3e: return "greater";		// ! > !
	case 0x3f: return "question";		// ! ? !
	case 0x40: return "at";			// ! @ !
	    // A .. Z
	case 0x5b: return "bracketleft";	// ! [ !
	case 0x5c: return "backslash";		// ! \ !
	case 0x5d: return "bracketright";	// ! ] !
	case 0x5e: return "asciicircum";	// ! ^ !
	case 0x5f: return "underscore";		// ! _ !
	    // a .. z
	case 0x60: return "grave";		// ! ` !
	case 0x7b: return "braceleft";		// ! { !
	case 0x7c: return "bar";		// ! | !
	case 0x7d: return "braceright";		// ! } !
	case 0x7e: return "asciitilde";		// ! ~ !
	    // controls

	    // Latin1
	case 0xa0: return "nobreakspace";	// !   !
	case 0xa1: return "exclamdown";		// ! ¡ !
	case 0xa2: return "cent";		// ! ¢ !
	case 0xa3: return "sterling";		// ! £ !
	case 0xa4: return "currency";		// ! ¤ !
	case 0xa5: return "yen";		// ! ¥ !
	case 0xa6: return "brokenbar";		// ! ¦ !
	case 0xa7: return "section";		// ! § !
	case 0xa8: return "diaeresis";		// ! ¨ !
	case 0xa9: return "copyright";		// ! © !
	case 0xaa: return "ordfeminine";	// ! ª !
	case 0xab: return "guillemotleft";	// ! « !
	case 0xac: return "notsign";		// ! ¬ !
	case 0xad: return "hyphen";		// ! ­ !
	case 0xae: return "registered";		// ! ® !
	case 0xaf: return "macron";		// ! ¯ !
	case 0xb0: return "degree";		// ! ° !
	case 0xb1: return "plusminus";		// ! ± !
	case 0xb2: return "twosuperior";	// ! ² !
	case 0xb3: return "threesuperior";	// ! ³ !
	case 0xb4: return "acute";		// ! ´ !
	case 0xb5: return "mu";			// ! µ !
	case 0xb6: return "paragraph";		// ! ¶ !
	case 0xb7: return "periodcentere";	// ! · !
	case 0xb8: return "cedilla";		// ! ç !
	case 0xb9: return "onesuperior";	// ! ¹ !
	case 0xba: return "masculine";		// ! º !
	case 0xbb: return "guillemotrigh";	// ! » !
	case 0xbc: return "onequarter"; 	// ! ¼ !
	case 0xbd: return "onehalf";		// ! ½ !
	case 0xbe: return "threequarters";	// ! ¾ !
	case 0xbf: return "questiondown";	// ! ¿ !
	case 0xc0: return "Agrave";		// ! À !
	case 0xc1: return "Aacute";		// ! Á !
	case 0xc2: return "Acircumflex";	// ! Â !
	case 0xc3: return "Atilde";		// ! Ã !
	case 0xc4: return "Adiaeresis";		// ! Ä !
	case 0xc5: return "Aring";		// ! Å !
	case 0xc6: return "AE";			// ! Æ !
	case 0xc7: return "Ccedilla";		// ! Ç !
	case 0xc8: return "Egrave";		// ! È !
	case 0xc9: return "Eacute";		// ! É !
	case 0xca: return "Ecircumflex";	// ! Ê !
	case 0xcb: return "Ediaeresis";		// ! Ë !
	case 0xcc: return "Igrave";		// ! Ì !
	case 0xcd: return "Iacute";		// ! Í !
	case 0xce: return "Icircumflex";	// ! Î !
	case 0xcf: return "Idiaeresis";		// ! Ï !
	case 0xd0: return "ETH";		// ! Ð !
	case 0xd1: return "Ntilde";		// ! Ñ !
	case 0xd2: return "Ograve";		// ! Ò !
	case 0xd3: return "Oacute";		// ! Ó !
	case 0xd4: return "Ocircumflex";	// ! Ô !
	case 0xd5: return "Otilde";		// ! Õ !
	case 0xd6: return "Odiaeresis";		// ! Ö !
	case 0xd7: return "multiply";		// ! × !
	case 0xd8: return "Ooblique";		// ! Ø !
	case 0xd9: return "Ugrave";		// ! Ù !
	case 0xda: return "Uacute";		// ! Ú !
	case 0xdb: return "Ucircumflex";	// ! Û !
	case 0xdc: return "Udiaeresis";		// ! Ü !
	case 0xdd: return "Yacute";		// ! Ý !
	case 0xde: return "THORN";		// ! Þ !
	case 0xdf: return "ssharp";		// ! ß !
	case 0xe0: return "agrave";		// ! à !
	case 0xe1: return "aacute";		// ! á !
	case 0xe2: return "acircumflex";	// ! â !
	case 0xe3: return "atilde";		// ! ã !
	case 0xe4: return "adiaeresis";		// ! ä !
	case 0xe5: return "aring";		// ! å !
	case 0xe6: return "ae"; 		// ! æ !
	case 0xe7: return "ccedilla";		// ! ç !
	case 0xe8: return "egrave";		// ! è !
	case 0xe9: return "eacute";		// ! é !
	case 0xea: return "ecircumflex";	// ! ê !
	case 0xeb: return "ediaeresis";		// ! ë !
	case 0xec: return "igrave";		// ! ì !
	case 0xed: return "iacute";		// ! í !
	case 0xee: return "icircumflex";	// ! î !
	case 0xef: return "idiaeresis";		// ! ï !
	case 0xf0: return "eth";		// ! ð !
	case 0xf1: return "ntilde";		// ! ñ !
	case 0xf2: return "ograve";		// ! ò !
	case 0xf3: return "oacute";		// ! ó !
	case 0xf4: return "ocircumflex";	// ! ô !
	case 0xf5: return "otilde";		// ! õ !
	case 0xf6: return "odiaeresis";		// ! ö !
	case 0xf7: return "division";		// ! ÷ !
	case 0xf8: return "oslash";		// ! ø !
	case 0xf9: return "ugrave";		// ! ù !
	case 0xfa: return "uacute";		// ! ú !
	case 0xfb: return "ucircumflex";	// ! û !
	case 0xfc: return "udiaeresis";		// ! ü !
	case 0xfd: return "yacute";		// ! ý !
	case 0xfe: return "thorn";		// ! þ !
	case 0xff: return "ydiaeresis";		// !   !
	}
	return NULL;
}

/*end*/
