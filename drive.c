/*
   win32 Change-drive command

        +------- Change Drive -------+
        |
        |   [C] [D] [E] [F] ....     |
        |
        +----------------------------+

   Copyright (C) 2012
   The Free Software Foundation, Inc.

   Written by:
   Adam Young 2012

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
#include <win32.h>

#include <sys/types.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <glib/gqueue.h>

#include "lib/tty/tty.h"
#include "lib/skin.h"
#include "lib/vfs/vfs.h"
#include "lib/strutil.h"
#include "lib/util.h"
#include "lib/widget.h"
#include "lib/keybind.h"                        /* CK_Cancel etc */
#include "src/filemanager/panel.h"
#include "src/filemanager/cmd.h"                /* reread_cmd() */
#include "src/filemanager/midnight.h"           /* left/right panel */

#include "drive.h"

static void             drive_sel (WPanel *panel);
static int              drive_dlg_callback (Dlg_head *h, Widget * sender, dlg_msg_t msg, int parm, void *data);
static void             drive_dlg_refresh (Dlg_head *h);


void
drive_cmd(void)
{
    WPanel *panel;

    if (NULL != (panel = current_panel)) {
        drive_sel(panel);
    }
}


void
drive_cmd_a(void)
{
    WPanel *panel;

    if (NULL != (panel = left_panel)) {
        drive_sel(panel);
    }
}


void
drive_cmd_b(void)
{
    WPanel *panel;

    if (NULL != (panel = right_panel)) {
        drive_sel(panel);
    }
}


#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Advapi32.lib")
#include <lm.h>

static void
drive_sel(WPanel *panel)
{   
    char buffer[7], drives[27 * 8] = {0};       /* temporary and drive buffer */
    int x_pos, y_pos, y_height, x_width;
    struct Dlg_head *drive_dlg;
    int totaldrives, d;
    const char *path;
    GQueue *buttons;

    /*
     *  Get drives name and count
     */

    GetLogicalDriveStrings(sizeof(drives) - 1, drives);
    for (totaldrives = 0, path = drives; *path; ++totaldrives) {
        *((char *)path) = toupper(*path);
        path += strlen(path) + 1;
    }

    /* Create Dialog */
#define D_PERLINE       12                      /* dynamic, based on screen width?? */
#define D_BUTSTART      3
#define D_BUTWIDTH      4

    if (totaldrives > D_PERLINE) {
        x_width  = 5 + (D_BUTWIDTH * D_PERLINE);
        y_height = 6 + ((totaldrives / D_PERLINE) * 2);
    } else {
        x_width  = 5 + (D_BUTWIDTH * totaldrives);
        if (x_width < 18) x_width = 18;         /* min width, inclusive of title/border */
        y_height = 6;
    }

    y_pos = ((LINES - 6) / 2) - 3;
    x_pos = panel->widget.x +                   /* center relative to panel */
                ((panel->widget.cols -
                    ((totaldrives > D_PERLINE ? D_PERLINE : totaldrives) * D_BUTWIDTH)) / 2) + 2;

    do_refresh ();
    drive_dlg =
        create_dlg(TRUE, y_pos, x_pos, y_height, x_width, dialog_colors,
                drive_dlg_callback, NULL, "[Chdrive]", _("Chdrive command"), 0);

    /*
     *  Drive buttons
     *      build and then push in reverse so button navigation is correct
     */
    buttons = g_queue_new();

    if (totaldrives > D_PERLINE) {
        int y = 1, x = D_BUTSTART;
                                                /* multiple lines */
        for (path = drives, d = 0; d < totaldrives; ++d) {
            sprintf(buffer, "&%c", *path);
            if (0 == (d % D_PERLINE)) {
                y += 2, x = D_BUTSTART;         /* new line */
            }
            g_queue_push_tail(buttons, button_new(y, x, B_USER + d, NARROW_BUTTON, buffer, 0));
            path += strlen(path) + 1;
            x += D_BUTWIDTH;
        }

    } else {
        int x = ((x_width - (totaldrives * D_BUTWIDTH)) / 2) + 1;

                                                /* single line */
        for (path = drives, d = 0; d < totaldrives; ++d) {
            sprintf(buffer, "&%c", *path);
            g_queue_push_tail(buttons, button_new(3, x, B_USER + d, NARROW_BUTTON, buffer, 0));
            path += strlen(path) + 1;
            x += D_BUTWIDTH;
        }
    }

    while (! g_queue_is_empty (buttons)) {
        add_widget (drive_dlg, g_queue_pop_tail(buttons));
    }

    g_queue_free (buttons);

    /* do action */
    run_dlg (drive_dlg);

    if (drive_dlg->ret_value != B_CANCEL) {
        const int is_right = (panel == right_panel ? 1 : 0);
        const int drive = (drive_dlg->ret_value - B_USER);

        for (path = drives, d = 0; d < totaldrives; ++d) {
            if (d == drive) {
                char t_cwd[MAX_PATH];

                w32_getcwd(t_cwd, sizeof(t_cwd));
                if (toupper(*t_cwd) != *path) {
                    char t_path[MAX_PATH];
                    
                    if (! w32_getcwdd(*path, t_path, sizeof(t_path))) {
                        t_path[0] = *path;
                        t_path[1] = ':';
                        t_path[2] = 0;
                    }

                    if (0 == w32_chdir (t_path)) {
                        vfs_path_t *cwd_vdir;

                        w32_getcwd (t_cwd, sizeof(t_cwd));
                        cwd_vdir = vfs_path_from_str(t_cwd);
                        if (get_display_type (is_right) != view_listing) {
                            set_display_type (is_right, view_listing);
                        }
                        do_cd (cwd_vdir, cd_exact);
                        vfs_path_free (cwd_vdir);

                    } else {
                        message (D_ERROR, MSG_ERROR, _("Cannot change drive to \"%s\"\n%s"), t_path,
                                    unix_error_string (errno));
                    }
                }
                break;
            }
            path += strlen(path) + 1;
        }
    }

    destroy_dlg (drive_dlg);
    repaint_screen ();
}


static int
drive_dlg_callback (Dlg_head *h, Widget * sender, dlg_msg_t msg, int parm, void *data)
{
    switch (msg) {
    case DLG_DRAW:
        drive_dlg_refresh (h);
        return MSG_HANDLED;

    case DLG_KEY:
        switch (parm) {
        case KEY_LEFT:
        case KEY_UP:
            dlg_one_down (h);                   /* prev drive button */
            return MSG_HANDLED;

        case KEY_RIGHT:
        case KEY_DOWN:
            dlg_one_up (h);                     /* next drive button */
            return MSG_HANDLED;
        }
        return MSG_NOT_HANDLED;

    default:
        return default_dlg_callback (h, sender, msg, parm, data);
    }
    /*NOTREACHED*/
    return 0;
}


static void
drive_dlg_refresh (Dlg_head *h)
{
    common_dialog_repaint (h);

    tty_setcolor (dialog_colors[0]);
    dlg_erase (h);
    draw_box (h, 1, 1, h->lines - 2, h->cols - 2, FALSE);
    tty_setcolor (dialog_colors[2]);
    dlg_move (h, 1, h->cols/2 - 7);             /* center title */
    tty_print_string (" Change Drive ");
}


