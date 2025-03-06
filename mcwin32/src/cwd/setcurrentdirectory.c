//
//  SetCurrentDirectory - Test application.
//
//     "cl /Zi setcurrentdirectory.c"
//

#include  <stdio.h>

#define WINDOWS_MEAN_AND_LEAN
#include <Windows.h>

static const wchar_t *GetWorkingDirectoryW(void);
static char *Wctomb(const wchar_t *val);
static wchar_t *Mbtowc(const char *val);

int
main(int argc, char **argv)
{
    const wchar_t *ocwd, *ncwd;
    wchar_t *wpath;
    BOOL success;
    DWORD rc;

    if (argc != 2) {
        printf("usage: setcurrentdirectory <path>\n");
        return 1;
    }

    // Current
    ocwd = GetWorkingDirectoryW();
    wprintf(L"gwd: %s\n", ocwd);

    // Change current working directory
    wpath = Mbtowc(argv[1]);
    success = SetCurrentDirectoryW(wpath);
    rc = GetLastError();
    wprintf(L"swd: [%u] <%s>\n", success, wpath);
    if (! success) {
        wchar_t message[256];
        const DWORD ret =
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
                FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, rc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, _countof(message), NULL);
        wprintf(L"  ==> <%s>\n", message);
    }
    ncwd = GetWorkingDirectoryW();
    wprintf(L"gwd: %s\n", ncwd);

    // Restore
    SetCurrentDirectoryW(ocwd);
    return 0;
}


static const wchar_t *
GetWorkingDirectoryW(void)
{
    const DWORD cdlen = GetCurrentDirectoryW(0, NULL); // includes terminator.
    wchar_t *cd;

    if (NULL != (cd = (wchar_t *)malloc(cdlen * sizeof(wchar_t)))) {
        GetCurrentDirectoryW(cdlen, cd);
        return cd;
    }
    return NULL;

}


static wchar_t *
Mbtowc(const char *val)
{
    const DWORD valsz = (DWORD)strlen(val ? val : 0),
        wvalsz = MultiByteToWideChar(CP_UTF8, 0, val, (int)valsz, NULL, 0);
    wchar_t *wval;

    if (NULL != (wval = calloc(wvalsz + 1 /*NUL*/, sizeof(wchar_t)))) {
        (void) MultiByteToWideChar(CP_UTF8, 0, val, (int)valsz, wval, (int)wvalsz);
        wval[ wvalsz ] = 0;
        return wval;
    }
    return NULL;
}


static char *
Wctomb(const wchar_t *val)
{
    const DWORD valsz = (DWORD)wcslen(val ? val : 0),
        utf8sz = WideCharToMultiByte(CP_UTF8, 0, val, (int)valsz, NULL, 0, NULL, NULL);
    char *utf8;

    if (NULL != (utf8 = calloc(utf8sz + 1 /*NUL*/, sizeof(char)))) {
        (void) WideCharToMultiByte(CP_UTF8, 0, val, (int)valsz, utf8, utf8sz, NULL, NULL);
        utf8[ utf8sz ] = 0;
        return utf8;
    }
    return NULL;
}

//end

