/*
 * Console information.
 *
 * Copyright (c) 2024 - 2025, Adam Young.
 * All rights reserved.
 */

#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x601)
#undef  _WIN32_WINNT
#undef  _WIN32_VER
#define _WIN32_WINNT 0x601
#define _WIN32_VER 0x601
#endif

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#define WINDOWS_MEAN_AND_LEAN
#include <Windows.h>

#ifndef _countof
#define _countof(__type) (sizeof(__type)/sizeof(__type[0]))
#endif

static const struct {
        const char *dispname;
        const char *cilocale;
        DWORD lcid;

} locales[] = {
        { "Neutral",                        "neutral",          MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL) }, // (use built-in word breaking)
        { "Userdefault",                    "user-default",     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT) },
        { "FakeBibi",                       "fake-bidi",        MAKELANGID(LANG_HEBREW, SUBLANG_NEUTRAL) },

        { "Afrikaans",                      "af",               MAKELANGID(LANG_AFRIKAANS, SUBLANG_DEFAULT) },
        { "Amharic",                        "am",               MAKELANGID(LANG_AMHARIC, SUBLANG_DEFAULT) },
        { "Arabic",                         "ar",               MAKELANGID(LANG_ARABIC, SUBLANG_DEFAULT) },
        { "Basque (Basque)",                "eu",               MAKELANGID(LANG_BASQUE, SUBLANG_DEFAULT) },
        { "Belarusian",                     "be",               MAKELANGID(LANG_BELARUSIAN, SUBLANG_DEFAULT) },
        { "Bengali",                        "bn",               MAKELANGID(LANG_BENGALI, SUBLANG_DEFAULT) },
        { "Bulgarian",                      "bg",               MAKELANGID(LANG_BULGARIAN, SUBLANG_DEFAULT) },
        { "Catalan",                        "ca",               MAKELANGID(LANG_CATALAN, SUBLANG_DEFAULT) },
        { "Chinese (China)",                "zh,zh-CN",         MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED) },
        { "Chinese (Hong Kong)",            "zh-HK",            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG) },
        { "Chinese (Macau)",                "zh-MO",            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_MACAU) },
        { "Chinese (Singapore)",            "zh-SG",            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE) },
        { "Chinese (Taiwan)",               "zh-TW",            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL) },
        { "Croatian",                       "hr",               MAKELANGID(LANG_CROATIAN, SUBLANG_DEFAULT) },
        { "Czech",                          "cs",               MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT) },
        { "Danish",                         "da",               MAKELANGID(LANG_DANISH, SUBLANG_DEFAULT) },
        { "Dutch",                          "nl",               MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH) },
        { "English",                        "en",               MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT) },
        { "English (Australian)",           "en-AU",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_AUS) },
        { "English (Belize)",               "en-BZ",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_BELIZE) },
        { "English (Canadian)",             "en-CA",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_CAN) },
        { "English (Caribbean)",            "en-CB",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_CARIBBEAN) },
        { "English (Eire)",                 "en-IE",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_EIRE) },
        { "English (Great Britain)",        "en-GB",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_UK) },
        { "English (India)",                "en-IN",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_INDIA) },
        { "English (Jamaica)",              "en-JM",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_JAMAICA) },
        { "English (Malaysia)",             "en-MY",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_MALAYSIA) },
        { "English (New Zealand)",          "en-NZ",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_NZ) },
        { "English (Philippines)",          "en-PH",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_PHILIPPINES) },
        { "English (Singapore)",            "en-SG",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_SINGAPORE) },
        { "English (South Africa)",         "en-ZA",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_SOUTH_AFRICA) },
        { "English (Trinidad & Tobago)",    "en-TT",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_TRINIDAD) },
        { "English (United States)",        "en-US",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US) },
        { "English (Zimbabwe)",             "en-ZW",            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_ZIMBABWE) },
        { "Estonian",                       "et",               MAKELANGID(LANG_ESTONIAN, SUBLANG_DEFAULT) },
        { "Faeroese",                       "fo",               MAKELANGID(LANG_FAEROESE, SUBLANG_DEFAULT) },
        { "Finnish",                        "fi",               MAKELANGID(LANG_FINNISH, SUBLANG_DEFAULT) },
        { "French",                         "fr,fr-FR",         MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH) },
        { "French (Belgium)",               "fr-BE",            MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_BELGIAN) },
        { "French (Canadian)",              "fr-CA",            MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_CANADIAN) },
        { "French (Luxembourg)",            "fr-LU",            MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_LUXEMBOURG) },
        { "French (Monco)",                 "fr-MC",            MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_MONACO) },
        { "French (Switzerland)",           "fr-CH",            MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_SWISS) },
        { "Galician",                       "gl",               MAKELANGID(LANG_GALICIAN, SUBLANG_DEFAULT) },
        { "German",                         "de",               MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN) },
        { "Greek",                          "el",               MAKELANGID(LANG_GREEK, SUBLANG_DEFAULT) },
        { "Gujarati",                       "gu",               MAKELANGID(LANG_GUJARATI, SUBLANG_DEFAULT) },
        { "Hebrew",                         "he,iw",            MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT) },
        { "Hindi",                          "hi",               MAKELANGID(LANG_HINDI, SUBLANG_DEFAULT) },
        { "Hungarian",                      "hu",               MAKELANGID(LANG_HUNGARIAN, SUBLANG_DEFAULT) },
        { "Icelandic",                      "is",               MAKELANGID(LANG_ICELANDIC, SUBLANG_DEFAULT) },
        { "Indonesian",                     "id,in",            MAKELANGID(LANG_INDONESIAN, SUBLANG_DEFAULT) },
        { "Italian",                        "it",               MAKELANGID(LANG_ITALIAN, SUBLANG_ITALIAN) },
        { "Japanese",                       "ja",               MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT) },
        { "Kannada",                        "kn",               MAKELANGID(LANG_KANNADA, SUBLANG_DEFAULT) },
        { "Korean",                         "ko",               MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT) },
        { "Latvian",                        "lv",               MAKELANGID(LANG_LATVIAN, SUBLANG_DEFAULT) },
        { "Lithuanian",                     "lt",               MAKELANGID(LANG_LITHUANIAN, SUBLANG_DEFAULT) },
        { "Lithuanian",                     "lt",               MAKELANGID(LANG_LITHUANIAN, SUBLANG_LITHUANIAN) },
        { "Malay",                          "ms",               MAKELANGID(LANG_MALAY, SUBLANG_DEFAULT) },
        { "Malayalam",                      "ml",               MAKELANGID(LANG_MALAYALAM, SUBLANG_DEFAULT) },
        { "Marathi",                        "mr",               MAKELANGID(LANG_MARATHI, SUBLANG_DEFAULT) },
        { "Nepali",                         "ne",               MAKELANGID(LANG_NEPALI, SUBLANG_NEPALI_NEPAL) },
        { "Norwegian (Nynorsk)",            "nn",               MAKELANGID(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_NYNORSK) },
        { "Norwegian",                      "nb",               MAKELANGID(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_BOKMAL) },
        { "Norwegian",                      "no",               MAKELANGID(LANG_NORWEGIAN, SUBLANG_DEFAULT) },
        { "Oriya",                          "or",               MAKELANGID(LANG_ORIYA, SUBLANG_DEFAULT) },
        { "Persian",                        "fa",               MAKELANGID(LANG_PERSIAN, SUBLANG_DEFAULT) },
        { "Polish",                         "pl",               MAKELANGID(LANG_POLISH, SUBLANG_DEFAULT) },
        { "Portuguese (Brazil)",            "pt-br",            MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN) },
        { "Portuguese",                     "pt",               MAKELANGID(LANG_PORTUGUESE, SUBLANG_DEFAULT) },
        { "Punjabi",                        "pa",               MAKELANGID(LANG_PUNJABI, SUBLANG_PUNJABI_INDIA) },
        { "Romanian",                       "ro",               MAKELANGID(LANG_ROMANIAN, SUBLANG_DEFAULT) },
        { "Russian",                        "ru",               MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT) },
        { "Sanskrit",                       "sa",               MAKELANGID(LANG_SANSKRIT, SUBLANG_SANSKRIT_INDIA) },
        { "Sami (Northern)",                "se",               MAKELANGID(LANG_SAMI, SUBLANG_DEFAULT) },
        { "Sami (Inari) (Finland)",         "se-FI",            MAKELANGID(LANG_SAMI, SUBLANG_SAMI_INARI_FINLAND) },
        { "Sami (Lule) (Norway)",           "se-NO",            MAKELANGID(LANG_SAMI, SUBLANG_SAMI_LULE_NORWAY) },
        { "Sami (Lule) (Sweden)",           "se-SE",            MAKELANGID(LANG_SAMI, SUBLANG_SAMI_LULE_SWEDEN) },
        { "Sami (Northern) (Finland)",      "se-FI",            MAKELANGID(LANG_SAMI, SUBLANG_SAMI_NORTHERN_FINLAND) },
        { "Sami (Northern) (Norway)",       "se-NO",            MAKELANGID(LANG_SAMI, SUBLANG_SAMI_NORTHERN_NORWAY) },
        { "Sami (Northern) (Sweden)",       "se-SE",            MAKELANGID(LANG_SAMI, SUBLANG_SAMI_NORTHERN_SWEDEN) },
        { "Sami (Skolt) (Finland)",         "se-FI",            MAKELANGID(LANG_SAMI, SUBLANG_SAMI_SKOLT_FINLAND) },
        { "Sami (Southern) (Norway)",       "se-NO",            MAKELANGID(LANG_SAMI, SUBLANG_SAMI_LULE_SWEDEN) },
        { "Sami (Southern) (Sweden)",       "se-SE",            MAKELANGID(LANG_SAMI, SUBLANG_SAMI_SOUTHERN_SWEDEN) },
        { "Serbian",                        "sr",               MAKELANGID(LANG_SERBIAN, SUBLANG_DEFAULT) },
        { "Sinhalese",                      "si",               MAKELANGID(LANG_SINHALESE, SUBLANG_SINHALESE_SRI_LANKA) },
        { "Slovak",                         "sk",               MAKELANGID(LANG_SLOVAK, SUBLANG_DEFAULT) },
        { "Slovenian",                      "sl",               MAKELANGID(LANG_SLOVENIAN, SUBLANG_DEFAULT) },
        { "Spanish",                        "es,es-es",         MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH) },
        { "Swahili",                        "sw",               MAKELANGID(LANG_SWAHILI, SUBLANG_DEFAULT) },
        { "Swedish",                        "sv",               MAKELANGID(LANG_SWEDISH, SUBLANG_DEFAULT) },
        { "Tamil",                          "ta",               MAKELANGID(LANG_TAMIL, SUBLANG_DEFAULT) },
        { "Telugn",                         "te",               MAKELANGID(LANG_TELUGU, SUBLANG_DEFAULT) },
        { "Thai",                           "th",               MAKELANGID(LANG_THAI, SUBLANG_DEFAULT) },
    //  { "Tigrigna",                       "ti",               MAKELANGID(LANG_TIGRIGNA, SUBLANG_TIGRIGNA_ERITREA) },
        { "Turkish",                        "tr",               MAKELANGID(LANG_TURKISH, SUBLANG_DEFAULT) },
        { "Ukrainian",                      "uk",               MAKELANGID(LANG_UKRAINIAN, SUBLANG_DEFAULT) },
        { "Urdu",                           "ur",               MAKELANGID(LANG_URDU, SUBLANG_DEFAULT) },
        { "Vietnamese",                     "vi",               MAKELANGID(LANG_VIETNAMESE, SUBLANG_DEFAULT) },
        { "Zulu",                           "zu",               MAKELANGID(LANG_ZULU, SUBLANG_DEFAULT) },
};

static void Usage(void);
static void OutputA(const char *, ...);
static void OutputW(const wchar_t *, ...);


static const char *
isoption(const char *argv, const char *option)
{
        const size_t olen = strlen(option);
        if (strncmp(argv, option, olen) == 0)
                return argv + olen;
        return NULL;
}


int
main(int argc, char *argv[])
{
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFOEX sbi = {0};
        CONSOLE_FONT_INFOEX cfi = {sizeof(CONSOLE_FONT_INFOEX)};
        wchar_t iso639[16] = {0}, iso3166[16] = {0},
            countryname[256], abbrevcountryname[10] = {0},
            displayname[256] = {0};
        LCID lcid, setlcid = 0;
        COORD fsz, lbws;

        if (argc == 2) {
                const char *option = argv[1], *val;
                unsigned i;

                if ((val = isoption(option, "--dispname=")) != NULL) {
                        for (i = 0; i < _countof(locales); ++i) {
                                if (stricmp(val, locales[i].dispname) == 0) {
                                        setlcid = locales[i].lcid;
                                        break;
                                }
                        }
                } else if ((val = isoption(option, "--cilocale=")) != NULL) {
                        for (i = 0; i < _countof(locales); ++i) {
                                if (stricmp(val, locales[i].cilocale) == 0) {
                                        setlcid = locales[i].lcid;
                                        break;
                                }
                        }
                } else if ((val = isoption(option, "--lcid=")) != NULL) {
                        char *endptr = NULL;
                        errno = 0;
                        setlcid = (LCID) strtoul(val, &endptr, 0);
                        if (errno || endptr == NULL || *endptr != 0) {
                                setlcid = 0;
                        }
                } else if ((val = isoption(option, "--help")) != NULL) {
                        Usage();
                        return EXIT_FAILURE;
                } else {
                        fprintf(stderr, "coninfo: invalid option <%s>\n", option);
                        return EXIT_FAILURE;
                }

                if (0 == setlcid) {
                        fprintf(stderr, "coninfo: unknown lcid value option <%s>\n", option);
                        return EXIT_FAILURE;
                }

        } else if (argc > 2) {
               fprintf(stderr, "coninfo: unexcepted option <%s>\n", argv[2]);
               return EXIT_FAILURE;
        }

        sbi.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
        GetConsoleScreenBufferInfoEx(console, &sbi);

        GetCurrentConsoleFontEx(console, FALSE, &cfi);
        fsz = GetConsoleFontSize(console, cfi.nFont);
                // Note: Retrieval of the font size is not possible within the Windows Terminal by design; always 0x16
                // https://github.com/microsoft/terminal/issues/6395

        lbws = GetLargestConsoleWindowSize(console);

        OutputA("SIZE:     %u/%u\n", (unsigned)sbi.dwSize.X, (unsigned)sbi.dwSize.Y);
        OutputA("MAX:      %u/%u\n", (unsigned)sbi.dwMaximumWindowSize.X, (unsigned)sbi.dwMaximumWindowSize.Y);
        OutputA("WIN:      %u/%u to %u/%u\n",
            (unsigned)sbi.srWindow.Left, (unsigned)sbi.srWindow.Top, (unsigned)sbi.srWindow.Right, (unsigned)sbi.srWindow.Bottom);
        OutputA("FULL:     %s\n", sbi.wPopupAttributes ? "yes" : "no");
        OutputA("\n");
        OutputA("FONT:     %u, %u/%u\n", (unsigned)cfi.nFont, (unsigned)fsz.X, (unsigned)fsz.Y);
        OutputA("LARGEST:  %u/%u\n", (unsigned)lbws.X, (unsigned)lbws.Y);
        OutputA("\n");

        ////////////////////////////

        lcid = GetSystemDefaultLCID();
        OutputA("LCID: system   %u/0x%x\n", lcid, lcid);
        lcid = GetUserDefaultLCID();
        OutputA("LCID: user     %u/0x%x\n", lcid, lcid);

        lcid = GetThreadLocale();
        OutputA("LCID: thread   %u/0x%x\n", lcid, lcid);

        if (GetLocaleInfoW(lcid, LOCALE_SISO639LANGNAME, iso639, _countof(iso639)) &&
                    GetLocaleInfoW(lcid, LOCALE_SISO3166CTRYNAME, iso3166, _countof(iso3166))) {
                GetLocaleInfoW(lcid, LOCALE_SLOCALIZEDCOUNTRYNAME, countryname, _countof(countryname));
                GetLocaleInfoW(lcid, LOCALE_SABBREVCTRYNAME, abbrevcountryname, _countof(abbrevcountryname));
                GetLocaleInfoW(lcid, LOCALE_SLOCALIZEDDISPLAYNAME, displayname, _countof(displayname));
                OutputW(L"LCID: name %s_%s (%s - %s) <%s>\n",
                    iso639, iso3166, countryname, abbrevcountryname, displayname); // "9_9 (countryname - abbrevcountryname) <displayname>"
        }
        OutputA("\n");

        OutputA("OEMCP:    %u/0x%x\n", GetOEMCP(), GetOEMCP());
        OutputA("ACP:      %u/0x%x\n", GetACP(), GetACP());
        OutputA("ICP:      %u/0x%x\n", GetConsoleCP(), GetConsoleCP());
        OutputA("OCP:      %u/0x%x\n", GetConsoleOutputCP(), GetConsoleOutputCP());
        OutputA("\n");

        ////////////////////////////

        if (0 == setlcid) {
                return 0;
        }

        OutputA("LCID: set    %u/0x%x\n", setlcid, setlcid);
        SetThreadLocale(setlcid);
        SetThreadUILanguage((LANGID)setlcid);

        lcid = GetUserDefaultLCID();
        OutputA("LCID: user   %u/0x%x\n", lcid, lcid);

        lcid = GetThreadLocale();
        OutputA("LCID: thread %u/0x%x\n", lcid, lcid);

        if (GetLocaleInfoW(lcid, LOCALE_SISO639LANGNAME, iso639, _countof(iso639)) &&
                GetLocaleInfoW(lcid, LOCALE_SISO3166CTRYNAME, iso3166, _countof(iso3166))) {
                GetLocaleInfoW(lcid, LOCALE_SLOCALIZEDCOUNTRYNAME, countryname, _countof(countryname));
                GetLocaleInfoW(lcid, LOCALE_SABBREVCTRYNAME, abbrevcountryname, _countof(abbrevcountryname));
                GetLocaleInfoW(lcid, LOCALE_SLOCALIZEDDISPLAYNAME, displayname, _countof(displayname));
                OutputW(L"LCID: name   %s_%s (%s - %s) <%s>\n",
                    iso639, iso3166, countryname, abbrevcountryname, displayname); // "9_9 (countryname - abbrevcountryname) <displayname>"
        }

        OutputA("OEMCP:       %u/0x%x\n", GetOEMCP(), GetOEMCP());
        OutputA("ACP:         %u/0x%x\n", GetACP(), GetACP());

        SetConsoleCP(GetACP());
        SetConsoleOutputCP(GetACP());
        OutputA("IP:          %u/0x%x\n", GetConsoleCP(), GetConsoleCP());
        OutputA("OCP:         %u/0x%x\n", GetConsoleOutputCP(), GetConsoleOutputCP());
        return 0;
}


static void
Usage(void)
{
        fprintf(stderr,
            "\n" \
            "coninfo [option]\n" \
            "\n" \
            "options:\n" \
            "   --dispname=<displayname>\n" \
            "   --cilocale=<cilocale>\n" \
            "   --lcid=<lcid>\n" \
            "   --help\n" \
            "\n");
}



static void
OutputA(const char *fmt, ...)
{
        HANDLE cout = GetStdHandle(STD_OUTPUT_HANDLE);
        char out[512];
        va_list ap;
        int len;

        va_start(ap, fmt);
        if ((len = vsnprintf(out, _countof(out), fmt, ap)) > (int)_countof(out)) {
            len = _countof(out);
        }
        WriteConsoleA(cout, out, len, NULL, NULL);
        va_end(ap);
}


static void
OutputW(const wchar_t *fmt, ...)
{
        HANDLE cout = GetStdHandle(STD_OUTPUT_HANDLE);
        wchar_t out[512];
        va_list ap;
        int len;

        va_start(ap, fmt);
        if ((len = vswprintf(out, _countof(out), fmt, ap)) > (int)_countof(out)) {
            len = _countof(out);
        }
        WriteConsoleW(cout, out, len, NULL, NULL);
        va_end(ap);
}

//end
