/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 argument support
 *
 * Copyright (c) 2024 - 2025 Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 * or (at your option) any later version.
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
 * license for more details.
 * ==end==
 */

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x501
#endif

#include <config.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <stdlib.h>
#include <assert.h>

#if !defined(_countof)
#define _countof(a) (sizeof(a)/sizeof(a[0]))
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


static int ArgumentSplit (char* cmd, char** argv, int cnt);
char ** GetUTF8Arguments (int* pargc);
char * GetUTF8Argument0 (void);


/**
 *  WIN32_Arguments ---
 *      Command line specialisation.
 * 
 *  Parameters:
 *      argv - Argument vector address, updated on exit.
 *
 *  Returns:
 *      Argument count.
 */
int
WIN32_Arguments(int argc, char ***argv)
{
    unsigned i;
    int nargc = 0;
    char **nargv;

    nargv = GetUTF8Arguments (&nargc);          // import utf8 command line
    assert(nargv && nargc == argc);

#define OPT_LOCALE  "--locale="

    nargc = 0;
    for (i = 0; nargv[i]; ++i) {
        const char *arg = nargv[i];

        if (arg[0] == '-' && arg[1] == '-') {   // options "--"

            if (0 == strncmp (arg, OPT_LOCALE, sizeof(OPT_LOCALE) - 1)) {
                const char *val = arg + (sizeof(OPT_LOCALE) - 1);
                LCID lcid = 0;
                unsigned l;

                if (isdigit (*val)) {
                    char *endptr = NULL;
                    unsigned long ul;

                    errno = 0;
                    ul = strtoul(val, &endptr, 0);
                    if (0 == errno && endptr && *endptr == 0) {
                        lcid = (LCID) ul;
                    }

                } else if (0 == strcmp(val, "help")) {
                    char delimiter [60];

                    memset (delimiter, '-', sizeof(delimiter));
                    tty_oprintf ("\t%-20s%s\n\t%.*s\n", "LCID", "Locale Name", sizeof(delimiter), delimiter);
                    for (l = 0; l < _countof(locales); ++l) {
                        tty_oprintf ("\t%-20s%s\n", locales[l].cilocale, locales[l].dispname);
                    }
                    tty_oprintf ("\t%.*s\n\n", sizeof(delimiter), delimiter);
                    nargv[nargc++] = "--help";  // suppress unknown option
                    continue;

                } else {
                    for (l = 0; l < _countof(locales); ++l) {
                        const char *cilocale = locales[l].cilocale, *part;

                        while (NULL != (part = strchr (cilocale, ','))) {
                            if (0 == _strnicmp (val, cilocale, part - cilocale)) {
                                break;          // example, "fr,fr-FR"
                            }
                            cilocale = part + 1;
                        }

                        if (part || 0 == _stricmp (val, cilocale)) {
                            lcid = locales[l].lcid;
                            break;
                        }
                    }
                }

                if (lcid) {
                    SetThreadLocale (lcid);
                    SetThreadUILanguage ((LANGID)lcid);
                    continue;                   // consume
                }
            }
        }

        nargv[nargc++] = (char *) arg; // export
    }

    nargv[nargc] = NULL; // terminate

    *argv = nargv;
    return nargc;
}


/**
 *  ArgumentSplit ---
 *      Split the command line, handling quoting and escapes.
 *
 *  Parameters:
 *      cmd  - Command line buffer; shall be modified.
 *      argv - Argument vector to be populated, otherwise NULL; only count is returned.
 *      cnt  - Argument limit, -1 unlimited.
 *
 *  Returns:  
 *      Argument count.
 */

static int
ArgumentSplit(char *cmd, char **argv, int cnt)
{
    char *start, *end;
    int argc;

    if (cmd == NULL) {
        return -1;
    }

    argc = 0;
    for (;;) {
        // Skip white-space
        while (*cmd == ' '|| *cmd == '\t' || *cmd == '\n') {
            ++cmd;
        }

        if (! *cmd)
            break;

        // element start
        if ('\"' == *cmd || '\'' == *cmd) { // quoted argument
            char quote = *cmd++;

            start = end = cmd;
            for (;;) {
                const char ch = *cmd;

                if (0 == ch)
                    break; // eos

                if ('\n' == ch || ch == quote) {
                    ++cmd; // delimiter
                    break;
                }

                if ('\\' == ch) { // quote
                    if (cmd[1] == '\"' || cmd[1] == '\'' || cmd[1] == '\\') {
                        ++cmd;
                    }
                }

                if (argv) *end++ = *cmd;
                ++cmd;
            }

        } else {
            start = end = cmd;
            for (;;) {
                const char ch = *cmd;

                if (0 == ch)
                    break; // eos

                if (ch == '\n' || ' ' == ch || '\t' == ch) {
                    ++cmd; // delimiter
                    break;
                }

                if ('\\' == ch) { // quote
                    if (cmd[1] == '\"' || cmd[1] == '\'' || cmd[1] == '\\') {
                        ++cmd;
                    }
                }

                if (argv) *end++ = *cmd;
                ++cmd;
            }
        }

        // element completion
        if (NULL == argv || cnt > 0) {
            if (argv) {
                argv[ argc ] = start;
                *end = '\0';
            }
            ++argc;
            --cnt;
        }
    }

    if (argv && cnt) {
        argv[argc] = NULL;
    }
    return argc;
}


/**
 *  GetUTF8Arguments ---
 *      Generate a UTF8-8 encoding argument vector from the wide-char command-line.
 *
 *  Parameters:
 *      pargv = Buffer populated with the argument count.
 *
 *  Returns:
 *      Argument vector, otherwise NULL.
 */

char **
GetUTF8Arguments(int *pargc)
{
    const wchar_t *wcmdline;

    if (NULL == (wcmdline = GetCommandLineW()) || wcmdline[0] == 0) {
        // import application name; fully qualified path; on empty command-line
        unsigned pathsz = GetModuleFileNameW(NULL, NULL, 0); // length, excluding terminating null character.

        if (pathsz) {
            if (NULL != (wcmdline = calloc(pathsz + 1 /*NUL*/, sizeof(wchar_t)))) {
                GetModuleFileNameW(NULL, (wchar_t *)wcmdline, pathsz + 1 /*NUL*/);
            }
        }
    }

    if (NULL != wcmdline) {
        // split into arguments
        const unsigned wcmdsz = wcslen(wcmdline),
            cmdsz = WideCharToMultiByte(CP_UTF8, 0, wcmdline, (int)wcmdsz, NULL, 0, NULL, NULL);
        char *cmd;

        if (NULL != (cmd = calloc(cmdsz + 1 /*NUL*/, sizeof(char)))) {
            char **argv = NULL;
            char *t_cmd;
            int argc;

            WideCharToMultiByte(CP_UTF8, 0, wcmdline, (int)wcmdsz, cmd, cmdsz + 1, NULL, NULL);

            if (NULL != (t_cmd = _strdup(cmd))) { // temporary working copy
                if ((argc = ArgumentSplit(t_cmd, NULL, -1)) > 0) { // argument count
                    if (NULL != (argv = calloc(argc + 1, sizeof(char*)))) {
                        ArgumentSplit(cmd, argv, argc + 1); // populate arguments
                        if (pargc) *pargc = argc;
                        free(t_cmd);
                        return argv;
                    }
                }
                free(t_cmd);
            }
        }

        free(cmd);
    }

    if (pargc) *pargc = 0;
    return NULL;
}


char *
GetUTF8Argument0(void)
{
    const wchar_t *wcmd, *wmodule = NULL;
    char *arg0 = NULL;

    if (NULL == (wcmd = GetCommandLineW()) || wcmd[0] == 0) {
        // import application name; fully qualified path; on empty command-line
        unsigned pathsz = GetModuleFileNameW(NULL, NULL, 0); // length, excluding terminating null character.

        if (pathsz) {
            if (NULL != (wmodule = calloc(pathsz + 1 /*NUL*/, sizeof(wchar_t)))) {
                GetModuleFileNameW(NULL, (wchar_t *)wmodule, pathsz + 1 /*NUL*/);
                wcmd = wmodule;
            }
        }
    }

    if (NULL != wcmd) {
        // derive argv[0]
        const unsigned wcmdsz = wcslen(wcmd),
            cmdsz = WideCharToMultiByte(CP_UTF8, 0, wcmd, (int)wcmdsz, NULL, 0, NULL, NULL);
        char *cursor;

        if (NULL != (cursor = malloc(cmdsz))) {

            arg0 = cursor;
            WideCharToMultiByte(CP_UTF8, 0, wcmd, (int)wcmdsz, arg0, cmdsz, NULL, NULL);

            if (arg0[0] == '"') {
                if (NULL != (cursor = strchr(arg0 + 1, '"'))) { // quoted
                    *cursor = 0;
                    ++arg0;
                }
            } else {
                if (NULL != (cursor = strchr(arg0, ' '))) { // unquoted
                    *cursor = 0;
                }
            }
        }
    }

    free((void *)wmodule); // working storage
    return arg0;
}

//end
