/* $NetBSD: strptime.c,v 1.62.6.1 2024/08/24 16:15:40 martin Exp $ */

/*-
 * Copyright (c) 1997, 1998, 2005, 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code was contributed to The NetBSD Foundation by Klaus Klein.
 * Heavily optimised by David Laight
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if !defined(LIBCOMPAT_SOURCE)
#define LIBCOMPAT_SOURCE
#endif

#include "libcompat.h"

#if !defined(HAVE_STRPTIME) || defined(NEED_STRPTIME)

/*
//  NAME
//       strptime -- converts a character string to a time value
//
//  SYNOPSIS
//
//       #include <time.h>
//       char *
//       strptime(const char * restrict buf, const char * restrict format,
//           struct tm * restrict tm);
//
//  DESCRIPTION
//
//       The strptime() function converts the character string pointed to by buf
//       to values which are stored in the tm structure pointed to by tm, using
//       the format specified by format.
//
//       The format string consists of zero or more conversion specifications,
//       whitespace characters as defined by isspace(), and ordinary characters.
//       All ordinary characters in format are compared directly against the cor-
//       responding characters in buf; comparisons which fail will cause
//       strptime() to fail.  Whitespace characters in format match any number of
//       whitespace characters in buf, including none.
//
//       A conversion specification consists of a percent sign `%' followed by one
//       or two conversion characters which specify the replacement required.
//       There must be white-space or other non-alphanumeric characters between
//       any two conversion specifications.
//
//       Conversion of alphanumeric strings (such as month and weekday names) is
//       done without regard to case.  Conversion specifications which cannot be
//       matched will cause strptime() to fail.
//
//       The LC_TIME category defines the locale values for the conversion speci-
//       fications.  The following conversion specifications are supported:
//
//       %a    the day of week, using the locale's weekday names; either the
//             abbreviated or full name may be specified.
//
//       %A    the same as %a.
//
//       %b    the month, using the locale's month names; either the abbreviated
//             or full name may be specified.
//
//       %B    the same as %b.
//
//       %c    the date and time, using the locale's date and time format.
//
//       %C    the century number [0,99]; leading zeros are permitted but not
//             required.  This conversion should be used in conjunction with the
//             %y conversion.
//
//       %d    the day of month [1,31]; leading zeros are permitted but not
//             required.
//
//       %D    the date as %m/%d/%y.
//
//       %e    the same as %d.
//
//       %F    the date as %Y-%m-%d (the ISO 8601 date format).
//
//       %g    the year corresponding to the ISO week number, without the century.
//             (A NetBSD extension.)
//
//       %G    the year corresponding to the ISO week number, with the century.
//             (A NetBSD extension.)
//
//       %h    the same as %b.
//
//       %H    the hour (24-hour clock) [0,23]; leading zeros are permitted but
//             not required.
//
//       %I    the hour (12-hour clock) [1,12]; leading zeros are permitted but
//             not required.
//
//       %j    the day number of the year [1,366]; leading zeros are permitted but
//             not required.
//
//       %k    the same as %H.
//
//       %l    the same as %I.
//
//       %m    the month number [1,12]; leading zeros are permitted but not
//             required.
//
//       %M    the minute [0,59]; leading zeros are permitted but not required.
//
//       %n    any white-space, including none.
//
//       %p    the locale's equivalent of a.m. or p.m.
//
//       %r    the time (12-hour clock) with %p, using the locale's time format.
//
//       %R    the time as %H:%M.
//
//       %S    the seconds [0,60]; leading zeros are permitted but not required.
//
//       %s    the number of seconds since the Epoch, UTC (see mktime(3)).  (A
//             NetBSD extension.)
//
//       %t    any white-space, including none.
//
//       %T    the time as %H:%M:%S.
//
//       %u    the day of the week as a decimal number, where Monday = 1.  (A
//             NetBSD extension.)
//
//       %U    the week number of the year (Sunday as the first day of the week)
//             as a decimal number [0,53]; leading zeros are permitted but not
//             required.  All days in a year preceding the first Sunday are con-
//             sidered to be in week 0.
//
//       %V    the ISO 8601:1988 week number as a decimal number.  If the week
//             (starting on Monday) that contains January 1 has more than three
//             days in the new year, then it is considered the first week of the
//             year.  If it has fewer than four days in the new year, then it is
//             considered the last week of the previous year.  Weeks are numbered
//             from 1 to 53.  (A NetBSD extension.)
//
//       %w    the weekday as a decimal number [0,6], with 0 representing Sunday;
//             leading zeros are permitted but not required.
//
//       %W    the week number of the year (Monday as the first day of the week)
//             as a decimal number [0,53]; leading zeros are permitted but not
//             required.  All days in a year preceding the first Monday are con-
//             sidered to be in week 0.
//
//       %x    the date, using the locale's date format.
//
//       %X    the time, using the locale's time format.
//
//       %y    the year within the 20th century [69,99] or the 21st century
//             [0,68]; leading zeros are permitted but not required.  If specified
//             in conjunction with %C, specifies the year [0,99] within that cen-
//             tury.
//
//       %Y    the year, including the century (i.e., 1996).
//
//       %z    an ISO 8601, RFC-2822, or RFC-3339 time zone specification.  (A
//             NetBSD extension.)  This is one of the following:
//                   -   The offset from Coordinated Universal Time (`UTC') speci-
//                       fied as:
//                             -  [+-]hhmm
//                             -  [+-]hh:mm
//                             -  [+-]hh
//
//                   -   `UTC' specified as:
//                             -  UTC (`Coordinated Universal Time')
//                             -  GMT (`Greenwich Mean Time')
//                             -  UT (`Universal Time')
//                             -  Z (`Zulu Time')
//
//                   -   A three character US time zone specified as:
//                             -  EDT
//                             -  EST
//                             -  CDT
//                             -  CST
//                             -  MDT
//                             -  MST
//                             -  PDT
//                             -  PST
//
//                       with the first letter standing for `Eastern' (``E''),
//                       `Central' (``C''), `Mountain' (``M'') or `Pacific'
//                       (``P''), and the second letter standing for `Daylight'
//                       (``D'' or summer) time or `Standard' (``S'') time
//
//                   -   a single letter military or nautical time zone specified
//                       as:
//                             - ``A'' through ``I''
//                             - ``K'' through ``Y''
//                             - ``J'' (non-nautical local time zone)
//
//                   -   An arbitrary timezone name that can be loaded from the
//                       database.
//
//       %Z    time zone name or no characters when time zone information is
//             unavailable.  (A NetBSD extension.)
//
//       %%    matches a literal `%'.  No argument is converted.
//
//     Modified conversion specifications
//       For compatibility, certain conversion specifications can be modified by
//       the E and O modifier characters to indicate that an alternative format or
//       specification should be used rather than the one normally used by the
//       unmodified conversion specification.  As there are currently neither
//       alternative formats nor specifications supported by the system, the
//       behavior will be as if the unmodified conversion specification were used.
//
//       Case is ignored when matching string items in buf, such as month and
//       weekday names.
//
//
//  RETURN VALUES
//
//       If successful, the strptime() function returns a pointer to the character
//       following the last character parsed.  Otherwise, a NULL pointer is
//       returned.
//
*/

#include <sys/cdefs.h>

#include <sys/types.h>
#include <ctype.h>
#include <locale.h>
#include <string.h>
#include <time.h>
#include <tzfile.h>

#ifdef __weak_alias
__weak_alias(strptime,_strptime)
__weak_alias(strptime_l, _strptime_l)
#endif

#if !defined(u_char)
#define u_char unsigned char
#define uint unsigned int
#endif

static const u_char *conv_num(const unsigned char *, int *, uint, uint);
static const u_char *find_string(const u_char *, int *, const char * const *,
	const char * const *, int);

#if defined(_WIN32)
#include "tzalloc.c"
#else
#define _TIME_LOCALE(loc) \
    ((_TimeLocale *)((loc)->part_impl[(size_t)LC_TIME]))
#endif

/*
 * We do not implement alternate representations. However, we always
 * check whether a given modifier is allowed for a certain conversion.
 */
#define ALT_E			0x01
#define ALT_O			0x02
#define LEGAL_ALT(x)		{ if (alt_format & ~(x)) return NULL; }

#define S_YEAR			(1 << 0)
#define S_MON			(1 << 1)
#define S_YDAY			(1 << 2)
#define S_MDAY			(1 << 3)
#define S_WDAY			(1 << 4)
#define S_HOUR			(1 << 5)

#define HAVE_MDAY(s)		(s & S_MDAY)
#define HAVE_MON(s)		(s & S_MON)
#define HAVE_WDAY(s)		(s & S_WDAY)
#define HAVE_YDAY(s)		(s & S_YDAY)
#define HAVE_YEAR(s)		(s & S_YEAR)
#define HAVE_HOUR(s)		(s & S_HOUR)

static char utc[] = { "UTC" };
/* RFC-822/RFC-2822 */
static const char * const nast[5] = {
       "EST",    "CST",    "MST",    "PST",    "\0\0\0"
};
static const char * const nadt[5] = {
       "EDT",    "CDT",    "MDT",    "PDT",    "\0\0\0"
};

/*
 * Table to determine the ordinal date for the start of a month.
 * Ref: http://en.wikipedia.org/wiki/ISO_week_date
 */
static const int start_of_month[2][13] = {
	/* non-leap year */
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
	/* leap year */
	{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

/*
 * Calculate the week day of the first day of a year. Valid for
 * the Gregorian calendar, which began Sept 14, 1752 in the UK
 * and its colonies. Ref:
 * http://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
 */

static int
first_wday_of(int yr)
{
	return ((2 * (3 - (yr / 100) % 4)) + (yr % 100) + ((yr % 100) /  4) +
	    (isleap(yr) ? 6 : 0) + 1) % 7;
}

#define delim(p)	((p) == '\0' || isspace((unsigned char)(p)))

#if defined(_WIN32) && !defined(HAVE_LOCALTIME_R)
static struct tm *
localtime_r(time_t *then, struct tm *result)
{
        struct tm *p = localtime(then);
        if (p) {
                *(result) = *p;
        }
        return p;
}

//      static struct tm *
//      gmtime_r(const time_t *then struct tm *tm)
//      {
//              if (gmtime_s(tm, then)) {
//                      return NULL;
//              }
//              return _tm;
//      }
#endif  //_WIN32 && !HAVE_LOCALTIME_R


static int
fromzone(const unsigned char **bp, struct tm *tm, int mandatory)
{
	timezone_t tz;
	char buf[512], *p;
	const unsigned char *rp;

	for (p = buf, rp = *bp; !delim(*rp) && p < &buf[sizeof(buf) - 1]; rp++)
		*p++ = *rp;
	*p = '\0';

	if (mandatory)
		*bp = rp;
	if (!isalnum((unsigned char)*buf))
		return 0;
	tz = tzalloc(buf);
	if (tz == NULL)
		return 0;

	*bp = rp;
	tm->tm_isdst = 0;	/* XXX */
#ifdef TM_GMTOFF
	tm->TM_GMTOFF = tzgetgmtoff(tz, tm->tm_isdst);
#endif
#ifdef TM_ZONE
	// Can't use tzgetname() here because we are going to free()
	tm->TM_ZONE = NULL;     /* XXX */
#endif
	tzfree(tz);
	return 1;
}

char *
strptime(const char *buf, const char *fmt, struct tm *tm)
{
	return strptime_l(buf, fmt, tm, _current_locale());
}

char *
strptime_l(const char *buf, const char *fmt, struct tm *tm, locale_t loc)
{
	unsigned char c;
	const unsigned char *bp, *ep, *zname;
	int alt_format, i, split_year = 0, neg = 0, state = 0,
	    day_offset = -1, week_offset = 0, offs, mandatory;
	const char *new_fmt;

	bp = (const u_char *)buf;

	while (bp != NULL && (c = *fmt++) != '\0') {
		/* Clear `alternate' modifier prior to new conversion. */
		alt_format = 0;
		i = 0;

		/* Eat up white-space. */
		if (isspace(c)) {
			while (isspace(*bp))
				bp++;
			continue;
		}

		if (c != '%')
			goto literal;


again:		switch (c = *fmt++) {
		case '%':	/* "%%" is converted to "%". */
literal:
			if (c != *bp++)
				return NULL;
			LEGAL_ALT(0);
			continue;

		/*
		 * "Alternative" modifiers. Just set the appropriate flag
		 * and start over again.
		 */
		case 'E':	/* "%E?" alternative conversion modifier. */
			LEGAL_ALT(0);
			alt_format |= ALT_E;
			goto again;

		case 'O':	/* "%O?" alternative conversion modifier. */
			LEGAL_ALT(0);
			alt_format |= ALT_O;
			goto again;

		/*
		 * "Complex" conversion rules, implemented through recursion.
		 */
		case 'c':	/* Date and time, using the locale's format. */
			new_fmt = _TIME_LOCALE(loc)->d_t_fmt;
			state |= S_WDAY | S_MON | S_MDAY | S_YEAR;
			goto recurse;

		case 'D':	/* The date as "%m/%d/%y". */
			new_fmt = "%m/%d/%y";
			LEGAL_ALT(0);
			state |= S_MON | S_MDAY | S_YEAR;
			goto recurse;

		case 'F':	/* The date as "%Y-%m-%d". */
			new_fmt = "%Y-%m-%d";
			LEGAL_ALT(0);
			state |= S_MON | S_MDAY | S_YEAR;
			goto recurse;

		case 'R':	/* The time as "%H:%M". */
			new_fmt = "%H:%M";
			LEGAL_ALT(0);
			goto recurse;

		case 'r':	/* The time in 12-hour clock representation. */
			new_fmt = _TIME_LOCALE(loc)->t_fmt_ampm;
			LEGAL_ALT(0);
			goto recurse;

		case 'T':	/* The time as "%H:%M:%S". */
			new_fmt = "%H:%M:%S";
			LEGAL_ALT(0);
			goto recurse;

		case 'X':	/* The time, using the locale's format. */
			new_fmt = _TIME_LOCALE(loc)->t_fmt;
			goto recurse;

		case 'x':	/* The date, using the locale's format. */
			new_fmt = _TIME_LOCALE(loc)->d_fmt;
			state |= S_MON | S_MDAY | S_YEAR;
		    recurse:
			bp = (const u_char *)strptime((const char *)bp,
							    new_fmt, tm);
			LEGAL_ALT(ALT_E);
			continue;

		/*
		 * "Elementary" conversion rules.
		 */
		case 'A':	/* The day of week, using the locale's form. */
		case 'a':
			bp = find_string(bp, &tm->tm_wday,
			    _TIME_LOCALE(loc)->day, _TIME_LOCALE(loc)->abday, 7);
			LEGAL_ALT(0);
			state |= S_WDAY;
			continue;

		case 'B':	/* The month, using the locale's form. */
		case 'b':
		case 'h':
			bp = find_string(bp, &tm->tm_mon,
			    _TIME_LOCALE(loc)->mon, _TIME_LOCALE(loc)->abmon,
			    12);
			LEGAL_ALT(0);
			state |= S_MON;
			continue;

		case 'C':	/* The century number. */
			i = 20;
			bp = conv_num(bp, &i, 0, 99);

			i = i * 100 - TM_YEAR_BASE;
			if (split_year)
				i += tm->tm_year % 100;
			split_year = 1;
			tm->tm_year = i;
			LEGAL_ALT(ALT_E);
			state |= S_YEAR;
			continue;

		case 'd':	/* The day of month. */
		case 'e':
			bp = conv_num(bp, &tm->tm_mday, 1, 31);
			LEGAL_ALT(ALT_O);
			state |= S_MDAY;
			continue;

		case 'k':	/* The hour (24-hour clock representation). */
			LEGAL_ALT(0);
			/* FALLTHROUGH */
		case 'H':
			bp = conv_num(bp, &tm->tm_hour, 0, 23);
			LEGAL_ALT(ALT_O);
			state |= S_HOUR;
			continue;

		case 'l':	/* The hour (12-hour clock representation). */
			LEGAL_ALT(0);
			/* FALLTHROUGH */
		case 'I':
			bp = conv_num(bp, &tm->tm_hour, 1, 12);
			if (tm->tm_hour == 12)
				tm->tm_hour = 0;
			LEGAL_ALT(ALT_O);
			state |= S_HOUR;
			continue;

		case 'j':	/* The day of year. */
			i = 1;
			bp = conv_num(bp, &i, 1, 366);
			tm->tm_yday = i - 1;
			LEGAL_ALT(0);
			state |= S_YDAY;
			continue;

		case 'M':	/* The minute. */
			bp = conv_num(bp, &tm->tm_min, 0, 59);
			LEGAL_ALT(ALT_O);
			continue;

		case 'm':	/* The month. */
			i = 1;
			bp = conv_num(bp, &i, 1, 12);
			tm->tm_mon = i - 1;
			LEGAL_ALT(ALT_O);
			state |= S_MON;
			continue;

		case 'p':	/* The locale's equivalent of AM/PM. */
			bp = find_string(bp, &i, _TIME_LOCALE(loc)->am_pm,
			    NULL, 2);
			if (HAVE_HOUR(state) && tm->tm_hour > 11)
				return NULL;
			tm->tm_hour += i * 12;
			LEGAL_ALT(0);
			continue;

		case 'S':	/* The seconds. */
			bp = conv_num(bp, &tm->tm_sec, 0, 61);
			LEGAL_ALT(ALT_O);
			continue;

		case 's': {	/* seconds since the epoch */
//XXX			const time_t TIME_MAX = __type_max(time_t);
			const time_t TIME_MAX = (((time_t) 1 << (sizeof(time_t) * CHAR_BIT - 2)) - 1) * 2 + 1;
			time_t sse, d;

			if (*bp < '0' || *bp > '9') {
				bp = NULL;
				continue;
			}

			sse = *bp++ - '0';
			while (*bp >= '0' && *bp <= '9') {
				d = *bp++ - '0';
				if (sse > TIME_MAX/10) {
					bp = NULL;
					break;
				}
				sse *= 10;
				if (sse > TIME_MAX - d) {
					bp = NULL;
					break;
				}
				sse += d;
			}
			if (bp == NULL)
				continue;

			if (localtime_r(&sse, tm) == NULL)
				bp = NULL;
			else
				state |= S_YDAY | S_WDAY |
				    S_MON | S_MDAY | S_YEAR;
			continue;
		}

		case 'U':	/* The week of year, beginning on sunday. */
		case 'W':	/* The week of year, beginning on monday. */
			/*
			 * This is bogus, as we can not assume any valid
			 * information present in the tm structure at this
			 * point to calculate a real value, so save the
			 * week for now in case it can be used later.
			 */
			bp = conv_num(bp, &i, 0, 53);
			LEGAL_ALT(ALT_O);
			if (c == 'U')
				day_offset = TM_SUNDAY;
			else
				day_offset = TM_MONDAY;
			week_offset = i;
			continue;

		case 'w':	/* The day of week, beginning on sunday. */
			bp = conv_num(bp, &tm->tm_wday, 0, 6);
			LEGAL_ALT(ALT_O);
			state |= S_WDAY;
			continue;

		case 'u':	/* The day of week, monday = 1. */
			bp = conv_num(bp, &i, 1, 7);
			tm->tm_wday = i % 7;
			LEGAL_ALT(ALT_O);
			state |= S_WDAY;
			continue;

		case 'g':	/* The year corresponding to the ISO week
				 * number but without the century.
				 */
			bp = conv_num(bp, &i, 0, 99);
			continue;

		case 'G':	/* The year corresponding to the ISO week
				 * number with century.
				 */
			do
				bp++;
			while (isdigit(*bp));
			continue;

		case 'V':	/* The ISO 8601:1988 week number as decimal */
			bp = conv_num(bp, &i, 0, 53);
			continue;

		case 'Y':	/* The year. */
			i = TM_YEAR_BASE;	/* just for data sanity... */
			bp = conv_num(bp, &i, 0, 9999);
			tm->tm_year = i - TM_YEAR_BASE;
			LEGAL_ALT(ALT_E);
			state |= S_YEAR;
			continue;

		case 'y':	/* The year within 100 years of the epoch. */
			/* LEGAL_ALT(ALT_E | ALT_O); */
			bp = conv_num(bp, &i, 0, 99);

			if (split_year)
				/* preserve century */
				i += (tm->tm_year / 100) * 100;
			else {
				split_year = 1;
				if (i <= 68)
					i = i + 2000 - TM_YEAR_BASE;
				else
					i = i + 1900 - TM_YEAR_BASE;
			}
			tm->tm_year = i;
			state |= S_YEAR;
			continue;

		case 'Z':
		case 'z':
			tzset();
			mandatory = c == 'z';
			/*
			 * We recognize all ISO 8601 formats:
			 * Z	= Zulu time/UTC
			 * [+-]hhmm
			 * [+-]hh:mm
			 * [+-]hh
			 * We recognize all RFC-822/RFC-2822 formats:
			 * UT|GMT
			 *          North American : UTC offsets
			 * E[DS]T = Eastern : -4 | -5
			 * C[DS]T = Central : -5 | -6
			 * M[DS]T = Mountain: -6 | -7
			 * P[DS]T = Pacific : -7 | -8
			 *          Nautical/Military
			 * [A-IL-M] = -1 ... -9 (J not used)
			 * [N-Y]  = +1 ... +12
			 * Note: J maybe used to denote non-nautical
			 *       local time
			 */
			if (mandatory)
				while (isspace(*bp))
					bp++;

			zname = bp;
			switch (*bp++) {
			case 'G':
				if (*bp++ != 'M')
					goto namedzone;
				/*FALLTHROUGH*/
			case 'U':
				if (*bp++ != 'T')
					goto namedzone;
				else if (!delim(*bp) && *bp++ != 'C')
					goto namedzone;
				/*FALLTHROUGH*/
			case 'Z':
				if (!delim(*bp))
					goto namedzone;
				tm->tm_isdst = 0;
#ifdef TM_GMTOFF
				tm->TM_GMTOFF = 0;
#endif
#ifdef TM_ZONE
				tm->TM_ZONE = utc;
#endif
				continue;
			case '+':
				neg = 0;
				break;
			case '-':
				neg = 1;
				break;
			default:
namedzone:
				bp = zname;

				/* Nautical / Military style */
				if (delim(bp[1]) &&
				    ((*bp >= 'A' && *bp <= 'I') ||
				     (*bp >= 'L' && *bp <= 'Y'))) {
#ifdef TM_GMTOFF
					/* Argh! No 'J'! */
					if (*bp >= 'A' && *bp <= 'I')
						tm->TM_GMTOFF =
						    (int)*bp - ('A' - 1);
					else if (*bp >= 'L' && *bp <= 'M')
						tm->TM_GMTOFF = (int)*bp - 'A';
					else if (*bp >= 'N' && *bp <= 'Y')
						tm->TM_GMTOFF = 'M' - (int)*bp;
					tm->TM_GMTOFF *= SECSPERHOUR;
#endif
#ifdef TM_ZONE
					tm->TM_ZONE = NULL; /* XXX */
#endif
					bp++;
					continue;
				}
				/* 'J' is local time */
				if (delim(bp[1]) && *bp == 'J') {
#ifdef TM_GMTOFF
					tm->TM_GMTOFF = -timezone;
#endif
#ifdef TM_ZONE
					tm->TM_ZONE = NULL; /* XXX */
#endif
					bp++;
					continue;
				}

				/*
				 * From our 3 letter hard-coded table
				 * XXX: Can be removed, handled by tzload()
				 */
				if (delim(bp[0]) || delim(bp[1]) ||
				    delim(bp[2]) || !delim(bp[3]))
					goto loadzone;
				ep = find_string(bp, &i, nast, NULL, 4);
				if (ep != NULL) {
#ifdef TM_GMTOFF
					tm->TM_GMTOFF = (-5 - i) * SECSPERHOUR;
#endif
#ifdef TM_ZONE
					tm->TM_ZONE = __UNCONST(nast[i]);
#endif
					bp = ep;
					continue;
				}
				ep = find_string(bp, &i, nadt, NULL, 4);
				if (ep != NULL) {
					tm->tm_isdst = 1;
#ifdef TM_GMTOFF
					tm->TM_GMTOFF = (-4 - i) * SECSPERHOUR;
#endif
#ifdef TM_ZONE
					tm->TM_ZONE = __UNCONST(nadt[i]);
#endif
					bp = ep;
					continue;
				}
				/*
				 * Our current timezone
				 */
#if defined(_MSC_VER)
				ep = find_string(bp, &i, (const char * const *)_tzname, NULL, 2);
#else
				ep = find_string(bp, &i, (const char * const *)tzname, NULL, 2);
#endif
				if (ep != NULL) {
					tm->tm_isdst = i;
#ifdef TM_GMTOFF
					tm->TM_GMTOFF = -timezone;
#endif
#ifdef TM_ZONE
					tm->TM_ZONE = tzname[i];
#endif
					bp = ep;
					continue;
				}
loadzone:
				/*
				 * The hard way, load the zone!
				 */
				if (fromzone(&bp, tm, mandatory))
					continue;
				goto out;
			}
			offs = 0;
			for (i = 0; i < 4; ) {
				if (isdigit(*bp)) {
					offs = offs * 10 + (*bp++ - '0');
					i++;
					continue;
				}
				if (i == 2 && *bp == ':') {
					bp++;
					continue;
				}
				break;
			}
			if (isdigit(*bp))
				goto out;
			switch (i) {
			case 2:
				offs *= SECSPERHOUR;
				break;
			case 4:
				i = offs % 100;
				offs /= 100;
				if (i >= SECSPERMIN)
					goto out;
				/* Convert minutes into decimal */
				offs = offs * SECSPERHOUR + i * SECSPERMIN;
				break;
			default:
			out:
				if (mandatory)
					return NULL;
				bp = zname;
				continue;
			}
			/* ISO 8601 & RFC 3339 limit to 23:59 max */
			if (offs >= (HOURSPERDAY * SECSPERHOUR))
				goto out;
			if (neg)
				offs = -offs;
			tm->tm_isdst = 0;	/* XXX */
#ifdef TM_GMTOFF
			tm->TM_GMTOFF = offs;
#endif
#ifdef TM_ZONE
			tm->TM_ZONE = NULL;	/* XXX */
#endif
			continue;

		/*
		 * Miscellaneous conversions.
		 */
		case 'n':	/* Any kind of white-space. */
		case 't':
			while (isspace(*bp))
				bp++;
			LEGAL_ALT(0);
			continue;


		default:	/* Unknown/unsupported conversion. */
			return NULL;
		}
	}

	if (!HAVE_YDAY(state) && HAVE_YEAR(state)) {
		if (HAVE_MON(state) && HAVE_MDAY(state)) {
			/* calculate day of year (ordinal date) */
			tm->tm_yday =  start_of_month[isleap_sum(tm->tm_year,
			    TM_YEAR_BASE)][tm->tm_mon] + (tm->tm_mday - 1);
			state |= S_YDAY;
		} else if (day_offset != -1) {
			/*
			 * Set the date to the first Sunday (or Monday)
			 * of the specified week of the year.
			 */
			if (!HAVE_WDAY(state)) {
				tm->tm_wday = day_offset;
				state |= S_WDAY;
			}
			tm->tm_yday = (7 -
			    first_wday_of(tm->tm_year + TM_YEAR_BASE) +
			    day_offset) % 7 + (week_offset - 1) * 7 +
			    tm->tm_wday  - day_offset;
			state |= S_YDAY;
		}
	}

	if (HAVE_YDAY(state) && HAVE_YEAR(state)) {
		int isleap;

		if (!HAVE_MON(state)) {
			/* calculate month of day of year */
			i = 0;
			isleap = isleap_sum(tm->tm_year, TM_YEAR_BASE);
			while (tm->tm_yday >= start_of_month[isleap][i])
				i++;
			if (i > 12) {
				i = 1;
				tm->tm_yday -= start_of_month[isleap][12];
				tm->tm_year++;
			}
			tm->tm_mon = i - 1;
			state |= S_MON;
		}

		if (!HAVE_MDAY(state)) {
			/* calculate day of month */
			isleap = isleap_sum(tm->tm_year, TM_YEAR_BASE);
			tm->tm_mday = tm->tm_yday -
			    start_of_month[isleap][tm->tm_mon] + 1;
			state |= S_MDAY;
		}

		if (!HAVE_WDAY(state)) {
			/* calculate day of week */
			i = 0;
			week_offset = first_wday_of(tm->tm_year);
			while (i++ <= tm->tm_yday) {
				if (week_offset++ >= 6)
					week_offset = 0;
			}
			tm->tm_wday = week_offset;
			state |= S_WDAY;
		}
	}

	return __UNCONST(bp);
}


static const u_char *
conv_num(const unsigned char *buf, int *dest, uint llim, uint ulim)
{
	uint result = 0;
	unsigned char ch;

	/* The limit also determines the number of valid digits. */
	uint rulim = ulim;

	ch = *buf;
	if (ch < '0' || ch > '9')
		return NULL;

	do {
		result *= 10;
		result += ch - '0';
		rulim /= 10;
		ch = *++buf;
	} while ((result * 10 <= ulim) && rulim && ch >= '0' && ch <= '9');

	if (result < llim || result > ulim)
		return NULL;

	*dest = result;
	return buf;
}

static const u_char *
find_string(const u_char *bp, int *tgt, const char * const *n1,
		const char * const *n2, int c)
{
	int i;
	size_t len;

	/* check full name - then abbreviated ones */
	for (; n1 != NULL; n1 = n2, n2 = NULL) {
		for (i = 0; i < c; i++, n1++) {
			len = strlen(*n1);
			if (strncasecmp(*n1, (const char *)bp, len) == 0) {
				*tgt = i;
				return bp + len;
			}
		}
	}

	/* Nothing matched */
	return NULL;
}

#endif  //!HAVE_STRPTIME || NEED_STRPTIME
