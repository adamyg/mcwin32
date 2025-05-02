/* -*- mode: c; indent-width: 4; -*- */
/*
    win32 subshell research framework 

        init_subshell
        invoke_subshell
        flush_subshell
        read_subshell_prompt
        do_update_prompt
        exit_subshell
        subshell_chdir
        subshell_get_console_attributes
        sigchld_handler

        add_select_channel
        subshell_prompt
        update_subshell_prompt
        should_read_new_subshell_prompt

   Adam Young 2025

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

#include "lib/global.h"
#include "lib/fileloc.h"
    //#include "lib/unixcompat.h"
    //#include "lib/tty/tty.h"              /* LINES */
    //#include "lib/tty/key.h"              /* XCTRL */
#include "lib/vfs/vfs.h"
#include "lib/strutil.h"
#include "lib/mcconfig.h"
#include "lib/util.h"
#include "lib/widget.h"

#include "src/filemanager/layout.h"         /* setup_cmdline() */
#include "src/filemanager/command.h"        /* cmdline */

#include "src/subshell/subshell.h"
    //#include "internal.h"

enum subshell_state_enum subshell_state = INACTIVE;

GString *subshell_prompt = NULL;
gboolean update_subshell_prompt = FALSE;
gboolean should_read_new_subshell_prompt = FALSE;


void
init_subshell (void)
{
}

int 
invoke_subshell (const char *command, int how, vfs_path_t ** new_dir)
{
    return -1;
}

gboolean
flush_subshell (int max_wait_length, int how)
{
    return FALSE;
}

gboolean
read_subshell_prompt (void)
{
    return FALSE;
}

void
do_update_prompt (void)
{
}

gboolean
exit_subshell (void)
{
    return TRUE;
}

void
subshell_chdir (const vfs_path_t * vpath)
{
}

void
subshell_get_console_attributes (void)
{
}

void
sigchld_handler (int signo)
{
}

void
add_select_channel (int fd, void (*load_prompt)(int fd, void *data))
{
}

//end
