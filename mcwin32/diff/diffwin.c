/*-
 * diff --- Window support.
 */

#if !defined(MEAN_AND_LEAN)
#define MEAN_AND_LEAN
#endif
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#include <Windows.h>

#include <stdio.h>
#include <io.h>

#include "diffwin.h"

#if !defined(ENABLE_VIRTUAL_TERMINAL_PROCESSING)
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    // When writing with WriteFile or WriteConsole, characters are parsed for VT100 and similar control
    // character sequences that control cursor movement, color/font mode, and other operations that can
    // also be performed via the existing Console APIs.
#endif

#if defined(__MINGW32__)
typedef struct _OSVERSIONINFOW RTL_OSVERSIONINFOW, *PRTL_OSVERSIONINFOW;
typedef DWORD (WINAPI *fnRtlGetVersion_t)(PRTL_OSVERSIONINFOW);
#elif defined(__WATCOMC__)
typedef struct _OSVERSIONINFOW RTL_OSVERSIONINFOW, *PRTL_OSVERSIONINFOW;
#endif
typedef NTSTATUS (WINAPI *RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

static bool
hascolorconsole(void)
{
	const DWORD MINV_MAJOR = 10, MINV_MINOR = 0, MINV_BUILD = 10586;
	bool ret = false;

	HMODULE hMod = GetModuleHandle(TEXT("ntdll.dll"));
	if (hMod) {
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
		RtlGetVersionPtr fn = (RtlGetVersionPtr) GetProcAddress(hMod, "RtlGetVersion");
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif
		if (fn != NULL) {
			RTL_OSVERSIONINFOW rovi = { 0 };
			rovi.dwOSVersionInfoSize = sizeof(rovi);
			if (fn(&rovi) == 0) {
				if (rovi.dwMajorVersion > MINV_MAJOR ||
					(rovi.dwMajorVersion == MINV_MAJOR &&
						(rovi.dwMinorVersion > MINV_MINOR || (rovi.dwMinorVersion == MINV_MINOR	&& rovi.dwBuildNumber >= MINV_BUILD)))) {
					ret = true;
				}
			}
		}
	}
	return ret;
}

#if defined(__WATCOMC__)
static void
#else
static void __cdecl
#endif
disable_atexit(void)
{
	const int fd = _fileno(stdout);
	HANDLE h = (HANDLE) _get_osfhandle(fd);
	DWORD mode;

	if (h == INVALID_HANDLE_VALUE || GetFileType(h) != FILE_TYPE_CHAR)
		return;
	if (GetConsoleMode(h, &mode) && (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
		mode &= ~ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		(void) SetConsoleMode(h, mode);
	}
	return;
}

bool
iscolorconsole(void)
{
	const int fd = _fileno(stdout);
	HANDLE h = (HANDLE) _get_osfhandle(fd);
	CONSOLE_SCREEN_BUFFER_INFO sbi = {0};
	DWORD mode;

	if (h == INVALID_HANDLE_VALUE || GetFileType(h) != FILE_TYPE_CHAR) // stdout tty
		return false;
	if (!hascolorconsole()) // build 10.10586
		return false;
	if (!GetConsoleScreenBufferInfo(h, &sbi)) // output buffer
		return false;
	if (!GetConsoleMode(h, &mode)) // console
		return false;

	if (ENABLE_VIRTUAL_TERMINAL_PROCESSING & mode) // TTY enabled
		return true;
	if (!SetConsoleMode(h, mode|ENABLE_VIRTUAL_TERMINAL_PROCESSING)) // enable
		return false;
	atexit(disable_atexit); // restore
	return true;
}

//end
