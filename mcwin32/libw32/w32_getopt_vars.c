#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_getopt_vars_c,"$Id: w32_getopt_vars.c,v 1.2 2023/09/30 05:39:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 *  common getopt and getopt_long globals
 */

#include <sys/cdefs.h>

#include <stdlib.h>
#include "getopt.h"

/*LIBW32_VAR*/ int  opterr = 1;                 /* if error message should be printed */
/*LIBW32_VAR*/ int  optind = 1;                 /* index into parent argv vector */
/*LIBW32_VAR*/ int  optopt = '?';               /* character checked for validity */
/*LIBW32_VAR*/ int  optreset = 0;               /* reset getopt */
/*LIBW32_VAR*/ char *optarg = NULL;             /* argument associated with option */

void
__w32_getopt_globals(void)
{
}

/*end*/

