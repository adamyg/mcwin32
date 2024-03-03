/* -*- mode: c; indent-width: 8; -*- */

/*
 * Copyright (c) 1997 Kungliga Tekniska Hgskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>

#include "libcompat.h"

#include <time.h>
#include <assert.h>

#if defined(_MSC_VER) || defined(__WATCOMC__) || \
        (defined(__MINGW32__) && !defined(HAVE_TIMEGM))

/*
 * Simplifed version of timegm() that wont allow out of bound values.
 *
 * The timegm() function interprets the input structure as representing Universal Coordinated Time (UTC).
 */

static int
calc_leap_years(int y)
{
        y -= 1;
        return y / 4 - y / 100 + y / 400;
}


static int
is_leap(unsigned y)
{
        y += 1900;
        return (y % 4) == 0 && ((y % 100) != 0 || (y % 400) == 0);
}


time_t
timegm(struct tm *tm)
{
        static const unsigned ndays[2][12] = {
                {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
                {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

        time_t res = 0;
        int i, leap;

        if (! tm)
                return -1;

        if (tm->tm_year < 0)
                return -1;

        leap = is_leap(tm->tm_year);

        if (tm->tm_mon  < 0  || tm->tm_mon  > 11 ||
            tm->tm_mday < 1  || tm->tm_mday > (int)ndays[leap][tm->tm_mon] ||
            tm->tm_min  < 0  || tm->tm_min  > 59 ||
            tm->tm_sec  < 0  || tm->tm_sec  > 60 ||
            tm->tm_hour < 0  || tm->tm_hour > 23) {
                return -1;
        }

//      for (i = 70; i < tm->tm_year; ++i)
//              res += is_leap(i) ? 366 : 365;

        res  = (tm->tm_year - 70) * 365 +
                (calc_leap_years(tm->tm_year + 1900) - calc_leap_years(1970));

        leap = is_leap(tm->tm_year);
        for (i = 0; i < tm->tm_mon; ++i)
                res += ndays[leap][i];

        res += tm->tm_mday - 1;
        res *= 24;
        res += tm->tm_hour;
        res *= 60;
        res += tm->tm_min;
        res *= 60;
        res += tm->tm_sec;

#if defined(_MSC_VER)
        assert(res == _mkgmtime(tm));
#endif
        return res;
}


#else
extern void __stdlibrary_has_timegm(void);

void
__stdlibrary_has_timegm(void)
{
}

#endif

//end
