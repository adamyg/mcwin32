/* -*- indent-width: 4; -*- */
/*
   win32 tty/win implementation

        void do_enter_ca_mode (void)
        void do_exit_ca_mode (void)
        void show_rxvt_contents (int starty, unsigned char y1, unsigned char y2)
        gboolean look_for_rxvt_extensions (void)

   Copyright (C) 2012
   The Free Software Foundation, Inc.

   Written by:
   Adam Young 2012-2015

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <config.h>
#include "win32.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib/global.h"
#include "lib/tty/win.h"

char *smcup = NULL;                             /* usage tty.c */
char *rmcup = NULL;


void
do_enter_ca_mode (void)
{
}


void 
do_exit_ca_mode (void)
{
}


void
show_rxvt_contents (int starty, unsigned char y1, unsigned char y2)
{
}


gboolean 
look_for_rxvt_extensions (void)
{
    return FALSE;
}

/*end*/
