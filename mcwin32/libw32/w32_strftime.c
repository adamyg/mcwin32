/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "win32_internal.h"
#include <locale.h>
#include <string.h>
#include <time.h>

#define TM_YEAR_BASE        1900

typedef struct {
    const char *abday[7];
    const char *day[7];
    const char *abmon[12];
    const char *mon[12];
    const char *am_pm[2];
    const char *d_t_fmt;
    const char *d_fmt;
    const char *t_fmt;
    const char *t_fmt_ampm;
} _TimeLocale;

static const _TimeLocale _DefaultTimeLocale = {
    {                       // abday
	"Sun","Mon","Tue","Wed","Thu","Fri","Sat",
    },
    {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
	"Friday", "Saturday"
    },
    {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    },
    {
	"January", "February", "March", "April", "May", "June", "July",
	"August", "September", "October", "November", "December"
    },
    {
	"AM", "PM"
    },
    "%a %b %d %H:%M:%S %Y",
    "%m/%d/%y",
    "%H:%M:%S",
    "%I:%M:%S %p"
};

static const _TimeLocale *  _CurrentTimeLocale = &_DefaultTimeLocale;

static size_t               gsize;
static char *               pt;

static int                  _add (const char* str);
static int                  _conv (int n, int digits, int pad);
static int                  _secs (const struct tm *t);
static size_t               _fmt (const char *format, const struct tm *t);


/*
//  NAME
//      strftime -- format date and time
//
//  SYNOPSIS
//        #include <time.h>
//
//        size_t
//        srrftime(char * restrict buf, size_t maxsize,
//                  const char * restrict format, const struct tm * restrict timeptr);
//
//  DESCRIPTION*
//     The *strftime*() function formats the information from 'timeptr' into the
//     buffer 'buf' according to the string pointed to by 'format'.  The function
//     *strftime_l*() does the same as *strftime*() but takes an explicit locale
//     rather than using the current locale.
//
//     The 'format' string consists of zero or more conversion specifications and
//     ordinary characters.  All ordinary characters are copied directly into
//     the buffer.  A conversion specification consists of a percent sign
//     ```%''' and one other character.
//
//     No more than 'maxsize' characters will be placed into the array.  If the
//     total number of resulting characters, including the terminating NUL char-
//     acter, is not more than 'maxsize', *strftime*() returns the number of charac-
//     ters in the array, not counting the terminating NUL.  Otherwise, zero is
//     returned and the buffer contents are indeterminate.
//
//     The conversion specifications are copied to the buffer after expansion as
//     follows:-
//
//     *%A*     is replaced by national representation of the full weekday name.
//
//     *%a*     is replaced by national representation of the abbreviated weekday
//              name.
//
//     *%B*     is replaced by national representation of the full month name.
//
//     *%b*     is replaced by national representation of the abbreviated month
//              name.
//
//     *%C*     is replaced by (year / 100) as decimal number; single digits are
//              preceded by a zero.
//
//     *%c*     is replaced by national representation of time and date.
//
//     *%D*     is equivalent to ``%m/%d/%y''.
//
//     *%d*     is replaced by the day of the month as a decimal number (01-31).
//
//     *%E** *%O**
//              POSIX locale extensions.  The sequences %Ec %EC %Ex %EX %Ey %EY %Od
//              %Oe %OH %OI %Om %OM %OS %Ou %OU %OV %Ow %OW %Oy are supposed to
//              provide alternate representations.
//
//              Additionally %OB implemented to represent alternative months names
//              (used standalone, without day mentioned).
//
//     *%e*     is replaced by the day of the month as a decimal number (1-31);
//              single digits are preceded by a blank.
//
//     *%F*    is equivalent to ``%Y-%m-%d''.
//
//     *%G*     is replaced by a year as a decimal number with century.  This year
//              is the one that contains the greater part of the week (Monday as
//              the first day of the week).
//
//     *%g*     is replaced by the same year as in ``%G'', but as a decimal number
//              without century (00-99).
//
//     *%H*     is replaced by the hour (24-hour clock) as a decimal number
//              (00-23).
//
//     *%h*     the same as *%b*.
//
//     *%I*     is replaced by the hour (12-hour clock) as a decimal number
//              (01-12).
//
//     *%j*     is replaced by the day of the year as a decimal number (001-366).
//
//     *%k*     is replaced by the hour (24-hour clock) as a decimal number (0-23);
//              single digits are preceded by a blank.
//
//     *%l*     is replaced by the hour (12-hour clock) as a decimal number (1-12);
//              single digits are preceded by a blank.
//
//     *%M*     is replaced by the minute as a decimal number (00-59).
//
//     *%m*     is replaced by the month as a decimal number (01-12).
//
//     *%n*     is replaced by a newline.
//
//     *%O**    the same as *%E**.
//
//     *%p*     is replaced by national representation of either "ante meridiem"
//         (a.m.)  or "post meridiem" (p.m.)  as appropriate.
//
//     *%R*    is equivalent to ``%H:%M''.
//
//     *%r*    is equivalent to ``%I:%M:%S %p''.
//
//     *%S*    is replaced by the second as a decimal number (00-60).
//
//     *%s*    is replaced by the number of seconds since the Epoch.
//
//     *%T*    is equivalent to ``%H:%M:%S''.
//
//     *%t*    is replaced by a tab.
//
//     *%U*    is replaced by the week number of the year (Sunday as the first day
//              of the week) as a decimal number (00-53).
//
//     *%u*     is replaced by the weekday (Monday as the first day of the week) as
//              a decimal number (1-7).
//
//     *%V*     is replaced by the week number of the year (Monday as the first day
//              of the week) as a decimal number (01-53).  If the week containing
//              January 1 has four or more days in the new year, then it is week 1;
//              otherwise it is the last week of the previous year, and the next
//              weekis week 1.
//
//     *%v*     is equivalent to ``%e-%b-%Y''.
//
//     *%W*     is replaced by the week number of the year (Monday as the first day
//              of the week) as a decimal number (00-53).
//
//     *%w*     is replaced by the weekday (Sunday as the first day of the week) as
//              a decimal number (0-6).
//
//     *%X*     is replaced by national representation of the time.
//
//     *%x*     is replaced by national representation of the date.
//
//     *%Y*     is replaced by the year with century as a decimal number.
//
//     *%y*     is replaced by the year without century as a decimal number (00-99).
//
//     *%Z*    is replaced by the time zone name.
//
//     *%z*     is replaced by the time zone offset from UTC; a leading plus sign
//              stands for east of UTC, a minus sign for west of UTC, hours and
//              minutes follow with two digits each and no delimiter between them
//              (common form for RFC 822 date headers).
//
//     *%+*
//              is replaced by national representation of the date and time
//              (the format is similar to that produced by date(1)
//              </cgi/man.cgi?query=date&sektion=1&apropos=0&manpath=FreeBSD+9.1-RELEASE>).
//
//     *%-**    GNU libc extension. Do not do any padding when performing numerical outputs.
//
//     *%_**    GNU libc extension. Explicitly specify space for padding.
//
//     *%0**    GNU libc extension. Explicitly specify zero for padding.
//
//     *%%*     is replaced by `%'.
*/
size_t
w32_strftime(
    char *s, size_t maxsize, const char *format, const struct tm *t)
{
    WIN32_TZSET();
    pt = s;
    if ((gsize = maxsize) < 1)
        return(0);
    if (_fmt(format, t)) {
        *pt = '\0';
        return(maxsize - gsize);
    }
    return(0);
}


#define SUN_WEEK(t) (((t)->tm_yday + 7 - ((t)->tm_wday)) / 7)
#define MON_WEEK(t) (((t)->tm_yday + 7 - ((t)->tm_wday ? (t)->tm_wday - 1 : 6)) / 7)

static size_t
_fmt(const char *format, const struct tm *t)
{
    for (; *format; ++format) {
        if (*format == '%') {
            ++format;
            if (*format == 'E') {
                                /* Alternate Era */
                ++format;
            } else if (*format == 'O') {
                                /* Alternate numeric symbols */
                ++format;
            }
            switch(*format) {
                case '\0':
                    --format;
                    break;
                case 'A':
                    if (t->tm_wday < 0 || t->tm_wday > 6)
                        return(0);
                    if (!_add(_CurrentTimeLocale->day[t->tm_wday]))
                        return(0);
                    continue;
                case 'a':
                    if (t->tm_wday < 0 || t->tm_wday > 6)
                        return(0);
                    if (!_add(_CurrentTimeLocale->abday[t->tm_wday]))
                        return(0);
                    continue;
                case 'B':
                    if (t->tm_mon < 0 || t->tm_mon > 11)
                        return(0);
                    if (!_add(_CurrentTimeLocale->mon[t->tm_mon]))
                        return(0);
                    continue;
                case 'b':
                case 'h':
                    if (t->tm_mon < 0 || t->tm_mon > 11)
                        return(0);
                    if (!_add(_CurrentTimeLocale->abmon[t->tm_mon]))
                        return(0);
                    continue;
                case 'C':
                    if (!_conv((t->tm_year + TM_YEAR_BASE) / 100,
                            2, '0'))
                        return(0);
                    continue;
                case 'c':
                    if (!_fmt(_CurrentTimeLocale->d_t_fmt, t))
                        return(0);
                    continue;
                case 'D':
                    if (!_fmt("%m/%d/%y", t))
                        return(0);
                    continue;
                case 'd':
                    if (!_conv(t->tm_mday, 2, '0'))
                        return(0);
                    continue;
                case 'e':
                    if (!_conv(t->tm_mday, 2, ' '))
                        return(0);
                    continue;
                case 'H':
                    if (!_conv(t->tm_hour, 2, '0'))
                        return(0);
                    continue;
                case 'I':
                    if (!_conv(t->tm_hour % 12 ?
                            t->tm_hour % 12 : 12, 2, '0'))
                        return(0);
                    continue;
                case 'j':
                    if (!_conv(t->tm_yday + 1, 3, '0'))
                        return(0);
                    continue;
                case 'k':
                    if (!_conv(t->tm_hour, 2, ' '))
                        return(0);
                    continue;
                case 'l':
                    if (!_conv(t->tm_hour % 12 ?
                            t->tm_hour % 12: 12, 2, ' '))
                        return(0);
                    continue;
                case 'M':
                    if (!_conv(t->tm_min, 2, '0'))
                        return(0);
                    continue;
                case 'm':
                    if (!_conv(t->tm_mon + 1, 2, '0'))
                        return(0);
                    continue;
                case 'n':
                    if (!_add("\n"))
                        return(0);
                    continue;
                case 'p':
                    if (!_add(_CurrentTimeLocale->am_pm[t->tm_hour >= 12]))
                        return(0);
                    continue;
                case 'R':
                    if (!_fmt("%H:%M", t))
                        return(0);
                    continue;
                case 'r':
                    if (!_fmt(_CurrentTimeLocale->t_fmt_ampm, t))
                        return(0);
                    continue;
                case 'S':
                    if (!_conv(t->tm_sec, 2, '0'))
                        return(0);
                    continue;
                case 's':
                    if (!_secs(t))
                        return(0);
                    continue;
                case 'T':
                    if (!_fmt("%H:%M:%S", t))
                        return(0);
                    continue;
                case 't':
                    if (!_add("\t"))
                        return(0);
                    continue;
                case 'U':
                    if (!_conv(SUN_WEEK(t), 2, '0'))
                        return(0);
                    continue;
                case 'u':
                    if (!_conv(t->tm_wday ? t->tm_wday : 7, 1, '0'))
                        return(0);
                    continue;
                case 'V':  {
                    /* ISO 8601 Week Of Year:
                       If the week (Monday - Sunday) containing
                       January 1 has four or more days in the new
                       year, then it is week 1; otherwise it is
                       week 53 of the previous year and the next
                       week is week one. */

                        int week = MON_WEEK(t);

                        int days = (((t)->tm_yday + 7 - \
                                    ((t)->tm_wday ? (t)->tm_wday - 1 : 6)) % 7);

                        if (days >= 4) {
                            week++;
                        } else if (week == 0) {
                            week = 53;
                        }
                        if (!_conv(week, 2, '0'))
                            return(0);
                    }
                    continue;
                case 'W':
                    if (!_conv(MON_WEEK(t), 2, '0'))
                        return(0);
                    continue;
                case 'w':
                    if (!_conv(t->tm_wday, 1, '0'))
                        return(0);
                    continue;
                case 'x':
                    if (!_fmt(_CurrentTimeLocale->d_fmt, t))
                        return(0);
                    continue;
                case 'X':
                    if (!_fmt(_CurrentTimeLocale->t_fmt, t))
                        return(0);
                    continue;
                case 'y':
                    if (!_conv((t->tm_year + TM_YEAR_BASE) % 100,
                            2, '0'))
                        return(0);
                    continue;
                case 'Y':
                    if (!_conv((t->tm_year + TM_YEAR_BASE), 4, '0'))
                        return(0);
                    continue;
                case 'Z': {
                    if (!_add("TZ")) {
                        return 0;
                    }
                    continue;
                }
                case '%':
                    /*
                     * X311J/88-090 (4.12.3.5): if conversion char is
                     * undefined, behavior is undefined.  Print out the
                     * character itself as printf(3) does.
                     */
                default:
                    break;
            }
        }
        if (!gsize--)
            return(0);
        *pt++ = *format;
    }
    return(gsize);
}


static int
_secs (const struct tm *t)
{
    static char buf[15];
    register time_t s;
    register char *p;
    struct tm tmp;

    /* Make a copy, mktime(3) modifies the tm struct. */
    tmp = *t;
    s = mktime(&tmp);
    for (p = buf + sizeof(buf) - 2; s > 0 && p > buf; s /= 10)
        *p-- = (char)(s % 10 + '0');
    return(_add(++p));
}


static int
_conv (int n, int digits, int pad)
{
    static char buf[10];
    register char *p;

    for (p = buf + sizeof(buf) - 2; n > 0 && p > buf; n /= 10, --digits)
        *p-- = (char)(n % 10 + '0');
    while (p > buf && digits-- > 0)
        *p-- = (char) pad;
    return(_add(++p));
}


static int
_add (const char *str)
{
    for (;; ++pt, --gsize) {
        if (!gsize)
            return(0);
        if (!(*pt = *str++))
            return(1);
    }
}

/*end*/

