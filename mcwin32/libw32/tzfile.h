#ifndef LIBW32_TZFILE_H_INCLUDED
#define LIBW32_TZFILE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_tzfile_h,"$Id: tzfile.h,v 1.1 2022/06/14 04:56:17 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Arthur David Olson of the National Cancer Institute.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 */

#define TZ_MAX_TIMES            2000

#define TZ_MAX_TYPES            256         /* This must be at least 17 for Europe/Samara and Europe/Vilnius.  */
                                            /* Limited by what (unsigned char)'s can hold */

#define TZ_MAX_CHARS            50          /* Maximum number of abbreviation characters */
                                            /* (limited by what unsigned chars can hold) */

#define TZ_MAX_LEAPS            50          /* Maximum number of leap second corrections */

#define SECSPERMIN              60
#define MINSPERHOUR             60
#define HOURSPERDAY             24
#define DAYSPERWEEK             7
#define DAYSPERNYEAR            365
#define DAYSPERLYEAR            366
#define SECSPERHOUR             (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY              ((int32_t) SECSPERHOUR * HOURSPERDAY)
#define MONSPERYEAR             12

#define TM_SUNDAY               0
#define TM_MONDAY               1
#define TM_TUESDAY              2
#define TM_WEDNESDAY            3
#define TM_THURSDAY             4
#define TM_FRIDAY               5
#define TM_SATURDAY             6

#define TM_JANUARY              0
#define TM_FEBRUARY             1
#define TM_MARCH                2
#define TM_APRIL                3
#define TM_MAY                  4
#define TM_JUNE                 5
#define TM_JULY                 6
#define TM_AUGUST               7
#define TM_SEPTEMBER            8
#define TM_OCTOBER              9
#define TM_NOVEMBER             10
#define TM_DECEMBER             11

#define TM_YEAR_BASE            1900

#define EPOCH_YEAR              1970
#define EPOCH_WDAY              TM_THURSDAY

#define isleap(y)               (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

#endif /*LIBW32_TZFILE_H_INCLUDED*/


