#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_getlocale_c,"$Id: w32_getlocale.c,v 1.6 2025/02/16 12:04:05 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 getlocale() system calls
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained 
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0501              /* enable xp+ features */
#endif

#include "win32_internal.h"
#include "win32_misc.h"
#include <unistd.h>

/*  Function:           w32_getlanguage
 *      Retrieve the users language setting.
 *
 *  Parameters:
 *      buf -               Result buffer, should be 8 bytes or greater.
 *      len -               Buffer length.
 *
 *  Returns:
 *      Address of buffer if successful, otherwise NULL.
 *
 *  Notes:
 *      Source:  http://support.microsoft.com/kb/102978
 *
 *       The International subkey describes how to display dates, times, currency, and other items
 *       for a specific country, under the following Registry path:
 *
 *              HKEY_CURRENT_USER\Control Panel\International
 *
 *       o Locale
 *
 *              Specifies the current user's locale ID for the local language preferences, based on
 *              values defined in CurrentControlSet\Control\Nls\Language.
 *
 *       o sLanguage REG_SZ string
 *
 *              Specifies the language you want to work in. Windows applications that provide
 *              language specific tasks, such as sorting or spell checking, use this entry.
 *
 *
 *       o LocaleName
 *
 *              Specifies the current user's locale name (Vista+).
 */
static const struct sLanguageMap {
    const char *lang;
    const char *locale;
} x_sLanguages[] = {
    {"csy", "cs"},          // Czech
    {"dan", "da"},          // Danish
    {"deu", "de_DE"},       // German
    {"des", "de_CH"},       // German (Swiss)
    {"dea", "de_AT"},       // German (Austrian)
    {"ell", "el"},          // Greek
    {"ena", "en_AU"},       // English (Australia)
    {"enc", "en_CA"},       // English (canada)
    {"eng", "en_GB"},       // English (U.K.)
    {"eni", "en_IE"},       // English (Irish)
    {"enu", "en_US"},       // English (U.S.)
    {"enz", "en_NZ"},       // English (New Zeal.)
    {"esm", "es_MX"},       // Spanish (Mexican)
    {"esn", "es"},          // Modern Spanish
    {"esp", "es_ES"},       // Castilian Spanish
    {"fin", "fi"},          // Finnish
    {"fra", "fr_FR"},       // French
    {"frb", "fr_BE"},       // French (Belgian)
    {"frc", "fr_CA"},       // French (Canadian)
    {"frs", "fr_CH"},       // French (Swiss)
    {"hun", "hu"},          // Hungarian
    {"isl", "is"},          // Icelandic
    {"ita", "it_IT"},       // Italian
    {"its", "it_CH"},       // Italian (Swiss)
    {"nlb", "nl_BE"},       // Dutch (Belgian)
    {"nld", "nl_NL"},       // Dutch
    {"non", "nn"},          // Norwegian (Nynorsk)
    {"nor", "nb"},          // Norwegian (Bokmal)
    {"plk", "pl"},          // Polish
    {"ptb", "pt_BR"},       // Portuguese (Brazilian)
    {"ptg", "pt_PT"},       // Portuguese
    {"rus", "ru"},          // Russian
    {"sky", "sk"},          // Slovak
    {"svc", "sv"},          // Swedish
    {"trk", "tr"},          // Turkish
    };


#if defined(XXX_NOTUSED)
const static struct sLocaleMap {
    int locale;
    const char *locale;
} x_sLocales[] = {
    //
    //  Sourced from a number of web sources, including:
    //      http://msdn.microsoft.com/en-us/goglobal/bb895996.aspx
    //
    {0x0436, "af"},         // <AFK> <Afrikaans> <Afrikaans>
    {0x041c, "sq"},         // <SQI> <Albanian> <Albanian>

    {0x0401, "ar_SA"},      // <ARA> <Arabic> <Arabic (Saudi Arabia)>
    {0x0801, "ar_IQ"},      // <ARI> <Arabic> <Arabic (Iraq)>
    {0x0C01, "ar_EG"},      // <ARE> <Arabic> <Arabic (Egypt)>
    {0x1001, "ar_LY"},      // <ARL> <Arabic> <Arabic (Libya)>
    {0x1401, "ar_DZ"},      // <ARG> <Arabic> <Arabic (Algeria)>
    {0x1801, "ar_MA"},      // <ARM> <Arabic> <Arabic (Morocco)>
    {0x1C01, "ar_TN"},      // <ART> <Arabic> <Arabic (Tunisia)>
    {0x2001, "ar_OM"},      // <ARO> <Arabic> <Arabic (Oman)>
    {0x2401, "ar_YE"},      // <ARY> <Arabic> <Arabic (Yemen)>
    {0x2801, "ar_SY"},      // <ARS> <Arabic> <Arabic (Syria)>
    {0x2C01, "ar_JO"},      // <ARJ> <Arabic> <Arabic (Jordan)>
    {0x3001, "ar_LB"},      // <ARB> <Arabic> <Arabic (Lebanon)>
    {0x3401, "ar_KW"},      // <ARK> <Arabic> <Arabic (Kuwait)>
    {0x3801, "ar_AE"},      // <ARU> <Arabic> <Arabic (U.A.E.)>
    {0x3C01, "ar_BH"},      // <ARH> <Arabic> <Arabic (Bahrain)>
    {0x4001, "ar_QA"},      // <ARQ> <Arabic> <Arabic (Qatar)>

    {0x042b, "hy"},         // <HYE> <Armenian> <Armenian>
    {0x044d, "as"},         // <ASM> <Assamese> <Assamese>
    {0x042c, "az_LATN"},    // <AZE> <Azeri> <Azeri (Latin)>
    {0x082c, "az_CYRL"},    // <AZC> <Azeri> <Azeri (Cyrillic)>
    {0x042D, "eu"},         // <EUQ> <Basque> <Basque>
    {0x0423, "be"},         // <BEL> <Belarussian> <Belarussian>
    {0x0445, "bn"},         // <BEN> <Bengali> <Bengali>
    {0x0402, "bg"},         // <BGR> <Bulgarian> <Bulgarian>
    {0x0403, "ca"},         // <CAT> <Catalan> <Catalan>

    {0x0404, "zh_TW"},      // <CHT> <Chinese> <Chinese (Taiwan)>
    {0x0804, "zh_CN"},      // <CHS> <Chinese> <Chinese (PRC)>
    {0x0C04, "zh_HK"},      // <ZHH> <Chinese> <Chinese (Hong Kong)>
    {0x1004, "zh_SG"},      // <ZHI> <Chinese> <Chinese (Singapore)>
    {0x1404, "zh_MO"},      // <ZHM> <Chinese> <Chinese (Macau SAR)>

    {0x041a, "hr"},         // <HRV> <Croatian> <Croatian>
    {0x0405, "cs"},         // <CSY> <Czech> <Czech>
    {0x0406, "da"},         // <DAN> <Danish> <Danish>
    {0x0413, "nl_NL"},      // <NLD> <Dutch> <Dutch (Netherlands)>
    {0x0813, "nl_BE"},      // <NLB> <Dutch> <Dutch (Belgium)>

    {0x0409, "en_US"},      // <ENU> <English> <English (United States)>
    {0x0809, "en_GB"},      // <ENG> <English> <English (United Kingdom)>
    {0x0c09, "en_AU"},      // <ENA> <English> <English (Australia)>
    {0x1009, "en_CA"},      // <ENC> <English> <English (Canada)>
    {0x1409, "en_NZ"},      // <ENZ> <English> <English (New Zealand)>
    {0x1809, "en_IE"},      // <ENI> <English> <English (Ireland)>
    {0x1c09, "en_ZA"},      // <ENS> <English> <English (South Africa)>
    {0x2009, "en_JM"},      // <ENJ> <English> <English (Jamaica)>
    {0x2409, "en_JM"},      // <ENB> <English> <English (Caribbean)>
    {0x2809, "en_BZ"},      // <ENL> <English> <English (Belize)>
    {0x2c09, "en_TT"},      // <ENT> <English> <English (Trinidad)>
    {0x3009, "en_ZW"},      // <ENW> <English> <English (Zimbabwe)>
    {0x3409, "en_PH"},      // <ENP> <English> <English (Philippines)>

    {0x0425, "et"},         // <ETI> <Estonian> <Estonian>
    {0x0438, "fo"},         // <FOS> <Faeroese> <Faeroese>
    {0x0429, "pa"},         // <FAR> <Farsi> <Farsi> =Persian
    {0x040b, "fi"},         // <FIN> <Finnish> <Finnish>

    {0x040c, "fr_FR"},      // <FRA> <French> <French (France)>
    {0x080c, "fr_BE"},      // <FRB> <French> <French (Belgium)>
    {0x0c0c, "fr_CA"},      // <FRC> <French> <French (Canada)>
    {0x100c, "fr_CH"},      // <FRS> <French> <French (Switzerland)>
    {0x140c, "fr_LU"},      // <FRL> <French> <French (Luxembourg)>
    {0x180c, "fr_MC"},      // <FRM> <French> <French (Monaco)>

    {0x0437, "ka"},         // <KAT> <Georgian> <Georgian>

    {0x0407, "de_DE"},      // <DEU> <German> <German (Germany)>
    {0x0807, "de_CH"},      // <DES> <German> <German (Switzerland)>
    {0x0c07, "de_AT"},      // <DEA> <German> <German (Austria)>
    {0x1007, "de_LU"},      // <DEL> <German> <German (Luxembourg)>
    {0x1407, "de_LI"},      // <DEC> <German> <German (Liechtenstein)>

    {0x0408, "el"},         // <ELL> <Greek> <Greek>
    {0x0447, "gu"},         // <GUJ> <Gujarati> <Gujarati>
    {0x040D, "he"},         // <HEB> <Hebrew> <Hebrew>
    {0x0439, "hi"},         // <HIN> <Hindi> <Hindi>
    {0x040e, "hu"},         // <HUN> <Hungarian> <Hungarian>
    {0x040F, "is"},         // <ISL> <Icelandic> <Icelandic>
    {0x0421, "id"},         // <IND> <Indonesian> <Indonesian>
    {0x0410, "it_IT"},      // <ITA> <Italian> <Italian (Italy)>
    {0x0810, "it_CH"},      // <ITS> <Italian> <Italian (Switzerland)>
    {0x0411, "ja"},         // <JPN> <Japanese> <Japanese>
    {0x044b, "kn"},         // <KAN> <Kannada> <Kannada>
    {0x0860, "ks"},         // <KAI> <Kashmiri> <Kashmiri (India)>
    {0x043f, "kk"},         // <KAZ> <Kazakh> <Kazakh>
    {0x0457, "kok"},        // <KOK> <Konkani> <Konkani>
    {0x0412, "ko"},         // <KOR> <Korean> <Korean>
    {0x0812, "ko"},         // <KOJ> <Korean> <Korean (Johab)>  ?
    {0x0426, "lv"},         // <LVI> <Latvian> <Latvian> = lettish
    {0x0427, "lt"},         // <LTH> <Lithuanian> <Lithuanian>
    {0x0827, "lt"},         // <LTH> <Lithuanian> <Lithuanian (Classic)>  ?
    {0x042f, "mk"},         // <MKD> <FYOR Macedonian> <FYOR Macedonian>
    {0x043e, "ms"},         // <MSL> <Malay> <Malaysian>
    {0x083e, "ms_BN"},      // <MSB> <Malay> <Malay Brunei Darussalam>
    {0x044c, "ml"},         // <MAL> <Malayalam> <Malayalam>
    {0x044e, "mr"},         // <MAR> <Marathi> <Marathi>
    {0x0461, "ne_NP"},      // <NEP> <Nepali> <Nepali (Nepal)>
    {0x0861, "ne_IN"},      // <NEI> <Nepali> <Nepali (India)>

    {0x0414, "nb"},         // <NOR> <Norwegian> <Norwegian (Bokmal)>
    {0x0814, "nn"},         // <NON> <Norwegian> <Norwegian (Nynorsk)>
                                    // ??? "no" ("Norwegian")

    {0x0448, "or"},         // <ORI> <Oriya> <Oriya>
    {0x0415, "pl"},         // <PLK> <Polish> <Polish>
    {0x0416, "pt_BR"},      // <PTB> <Portuguese> <Portuguese (Brazil)>
    {0x0816, "pt_PT"},      // <PTG> <Portuguese> <Portuguese (Portugal)>
    {0x0446, "pa"},         // <PAN> <Punjabi> <Punjabi>
    {0x0417, "rm"},         // <RMS> <Rhaeto-Romanic> <Rhaeto-Romanic>
    {0x0418, "ro"},         // <ROM> <Romanian> <Romanian>
    {0x0818, "ro_MD"},      // <ROV> <Romanian> <Romanian (Moldova)>
    {0x0419, "ru"},         // <RUS> <Russian> <Russian>
    {0x0819, "ru_MD"},      // <RUM> <Russian> <Russian (Moldova)>
    {0x043b, "se"},         // <SZI> <Sami> <Sami (Lappish)> == Northern Sami
    {0x044f, "sa"},         // <SAN> <Sanskrit> <Sanskrit>
    {0x0c1a, "sr_CYRL"},    // <SRB> <Serbian> <Serbian (Cyrillic)>
    {0x081a, "sr_LATN"},    // <SRL> <Serbian> <Serbian (Latin)>
    {0x0459, "sd"},         // <SND> <Sindhi> <Sindhi>
    {0x041b, "sk"},         // <SKY> <Slovak> <Slovak>
    {0x0424, "sl"},         // <SLV> <Slovenian> <Slovenian>
    {0x042e, "wen"},        // <SBN> <Sorbian> <Sorbian>

    {0x040a, "es_ES"},      // <ESP> <Spanish> <Spanish (Spain - Traditional Sort)>
    {0x080a, "es_MX"},      // <ESM> <Spanish> <Spanish (Mexico)>
    {0x0c0a, "es_ES"},      // <ESN> <Spanish> <Spanish (Spain _ Modern Sort)>
    {0x100a, "es_GT"},      // <ESG> <Spanish> <Spanish (Guatemala)>
    {0x140a, "es_CR"},      // <ESC> <Spanish> <Spanish (Costa Rica)>
    {0x180a, "es_PA"},      // <ESA> <Spanish> <Spanish (Panama)>
    {0x1c0a, "es_DO"},      // <ESD> <Spanish> <Spanish (Dominican Republic)>
    {0x200a, "es_VE"},      // <ESV> <Spanish> <Spanish (Venezuela)>
    {0x240a, "es_CO"},      // <ESO> <Spanish> <Spanish (Colombia)>
    {0x280a, "es_PE"},      // <ESR> <Spanish> <Spanish (Peru)>
    {0x2c0a, "es_AR"},      // <ESS> <Spanish> <Spanish (Argentina)>
    {0x300a, "es_EC"},      // <ESF> <Spanish> <Spanish (Ecuador)>
    {0x340a, "es_CL"},      // <ESL> <Spanish> <Spanish (Chile)>
    {0x380a, "es_UY"},      // <ESY> <Spanish> <Spanish (Uruguay)>
    {0x3c0a, "es_PY"},      // <ESZ> <Spanish> <Spanish (Paraguay)>
    {0x400a, "es_BO"},      // <ESB> <Spanish> <Spanish (Bolivia)>
    {0x440a, "es_SV"},      // <ESE> <Spanish> <Spanish (El Salvador)>
    {0x480a, "es_HN"},      // <ESH> <Spanish> <Spanish (Honduras)>
    {0x4c0a, "es_NI"},      // <ESI> <Spanish> <Spanish (Nicaragua)>
    {0x500a, "es_PR"},      // <ESU> <Spanish> <Spanish (Puerto Rico)>

    {0x0430, "st"},         // <SXT> <Sutu> <Sutu>  == soto, sesotho
    {0x0441, "sw_KE"},      // <SWK> <Swahili> <Swahili (Kenya)>
    {0x041D, "sv"},         // <SVE> <Swedish> <Swedish>
    {0x081d, "sv_FI"},      // <SVF> <Swedish> <Swedish (Finland)>
    {0x0449, "ta"},         // <TAM> <Tamil> <Tamil>
    {0x0444, "tt"},         // <TAT> <Tatar> <Tatar (Tatarstan)>
    {0x044a, "te"},         // <TEL> <Telugu> <Telugu>
    {0x041E, "th"},         // <THA> <Thai> <Thai>
    {0x0431, "ts"},         // <TSG> <Tsonga> <Tsonga>
    {0x0432, "tn"},         // <TNA> <Tswana> <Tswana>  == Setswana
    {0x041f, "tr"},         // <TRK> <Turkish> <Turkish>
    {0x0422, "uk"},         // <UKR> <Ukrainian> <Ukrainian>
    {0x0420, "ur_PK"},      // <URD> <Urdu> <Urdu (Pakistan)>
    {0x0820, "ur_IN"},      // <URI> <Urdu> <Urdu (India)>
    {0x0443, "uz_LATN"},    // <UZB> <Uzbek> <Uzbek (Latin)>
    {0x0843, "uz_CYRL"},    // <UZC> <Uzbek> <Uzbek (Cyrillic)>
    {0x0433, "ven"},        // <VEN> <Venda> <Venda>
    {0x042a, "vi"},         // <VIT> <Vietnamese> <Vietnamese>
    {0x0434, "xh"},         // <XHS> <Xhosa> <Xhosa>
    {0x043d, "yi"},         // <JII> <Yiddish> <Yiddish>
    {0x0435, "zu"},         // <ZUL> <Zulu> <Zulu>
    );
#endif  /*XXX_NOTUSED*/


LIBW32_API const char *
w32_getlanguage(char *buffer, int len)
{
    const char *subkey = "Control Panel\\International";
    char sLanguage[64], LocaleName[64];
    int ret;

    if ((ret = w32_regstrget(subkey, "LocaleName", LocaleName, sizeof(LocaleName))) > 0) {
        /*
         *  Locale xxx[-xxx]
         */
        if (ret < len) {
            const char *cursor = LocaleName;
            char *result = buffer;

            for (cursor = LocaleName; *cursor && ret--; ++cursor, --ret) {
                *result++ = ('-' == *cursor ? '_' : *cursor);
            }
            *result = 0;
            return buffer;
        }

    } else if (len > 8 && w32_regstrget(subkey, "sLanguage", sLanguage, sizeof(sLanguage)) > 0) {
        /*
         *  Language emumeration xxx
         */
        const struct sLanguageMap *cursor = x_sLanguages;
        unsigned i;

        for (i = 0; i < (sizeof(x_sLanguages)/sizeof(x_sLanguages[0])); ++i, ++cursor) {
            if (0 == _stricmp(sLanguage, cursor->lang)) {
                strcpy(buffer, cursor->locale);
                return buffer;
            }
        }
    }
    return NULL;
}


LIBW32_API int
w32_regstrget(const char *subkey, const char *valuename, char *buf, int len)
{
    int ret;

    if (-1 == (ret = w32_regstrgetx(HKEY_CURRENT_USER, subkey, valuename, buf, len))) {
        ret = w32_regstrgetx(HKEY_LOCAL_MACHINE, subkey, valuename, buf, len);
    }
    return ret;
}


LIBW32_API int
w32_regstrgetx(HKEY hkey, const char *subkey, const char *valuename, char *buf, int len)
{
    HKEY handle;
    int ret = -1;

    if (RegOpenKeyEx(hkey, subkey, 0, KEY_READ, &handle) == ERROR_SUCCESS) {
        DWORD type = 0, datalen = len;

        if (RegQueryValueEx(handle, valuename, 0, &type, (unsigned char *)buf, &datalen) == ERROR_SUCCESS &&
                (type == REG_SZ || type == REG_EXPAND_SZ)) {
            ret = (int)datalen;
        }
        RegCloseKey(handle);
    }
    return ret;
}
