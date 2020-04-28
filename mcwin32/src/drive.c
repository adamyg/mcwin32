/* -*- mode: c; indent-width: 4; -*- */
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
   Adam Young 2012 - 2020

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
#include "libw32.h"

#include <sys/types.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <glib.h>

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

#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Advapi32.lib")
#include <lm.h>

static void             drive_sel (WPanel *panel);
static cb_ret_t         drive_dlg_callback (Widget * h, Widget * sender, widget_msg_t msg, int parm, void *data);


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


static void
drive_sel(WPanel *panel)
{
    char t_drives[27 * 8] = {0}, *cursor,       /* temporary drives and cursor */
            drives[27 * 8] = {0};               /* resulting */
    int x_pos, y_pos, y_height, x_width;
    char root[4] = "?:\\";
    WDialog *drive_dlg;
    int totaldrives, d;
    const char *path;
    WGroup *g;
    GQueue *buttons;

    /* Get active drives name and count */
    GetLogicalDriveStrings(sizeof(t_drives) - 1, t_drives);
    for (totaldrives = 0, path = t_drives, cursor = drives; *path;) {
        const int length = strlen(path);
        const char drive = toupper(*path);
        int type;

        assert(length < 8);
        assert(drive >= 'A' && drive <= 'Z');

        root[0] = drive;
        if ((type = GetDriveType(root)) != DRIVE_UNKNOWN && type != DRIVE_NO_ROOT_DIR)
            if (drive < 'C' ||                  /* assume floppies; FIXME, shell32?? */
                    GetDiskFreeSpaceExA(root, NULL, NULL, NULL)) {
                (void) memcpy(cursor, path, length);
                cursor[0] = drive;
                cursor += length + 1;
                ++totaldrives;
            }
        path += length + 1;
    }

    /* Create Dialog */
#define D_PERLINE       12                      /* dynamic, based on screen width?? */
#define D_BUTSTART      3
#define D_BUTWIDTH      4

    if (totaldrives > D_PERLINE) {
        x_width  = 5 + (D_BUTWIDTH * D_PERLINE);
        y_height = 7 + ((totaldrives / D_PERLINE) * 2);
    } else {
        x_width  = 5 + (D_BUTWIDTH * totaldrives);
        if (x_width < 20) x_width = 20;         /* min width, inclusive of title/border */
        y_height = 7;
    }

    y_pos = ((LINES - 6) / 2) - 3;
    x_pos = panel->widget.x +                   /* center relative to panel */
                ((panel->widget.cols -
                    ((totaldrives > D_PERLINE ? D_PERLINE : totaldrives) * D_BUTWIDTH)) / 2) + 2;

    do_refresh ();
    drive_dlg =
        dlg_create(TRUE, y_pos, x_pos, y_height, x_width, WPOS_CENTER | WPOS_TRYUP, FALSE,
                dialog_colors, drive_dlg_callback, NULL /*TODO-MOUSE*/, "[Chdrive]", _("Change Drive"));
    g = GROUP (drive_dlg);

    /*
     *  Drive buttons
     *      build and then push in reverse so button navigation is correct
     */
    buttons = g_queue_new();

    if (totaldrives > D_PERLINE) {
        int y = 1, x = D_BUTSTART;
        char buffer[7];
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
        char buffer[7];
                                                /* single line */
        for (path = drives, d = 0; d < totaldrives; ++d) {
            sprintf(buffer, "&%c", *path);
            g_queue_push_tail(buttons, button_new(3, x, B_USER + d, NARROW_BUTTON, buffer, 0));
            path += strlen(path) + 1;
            x += D_BUTWIDTH;
        }
    }

    while (! g_queue_is_empty (buttons)) {
        group_add_widget (g, g_queue_pop_tail(buttons));
    }

    g_queue_free (buttons);

    /* do action */
    dlg_run (drive_dlg);

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
                        if (get_panel_type (is_right) != view_listing) {
                            create_panel (is_right, view_listing);
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

    dlg_destroy (drive_dlg);
    repaint_screen ();
}


static cb_ret_t
drive_dlg_callback (Widget * h, Widget * sender, widget_msg_t msg, int parm, void *data)
{
    WDialog *d = DIALOG (h);

    switch (msg) {
    case MSG_DRAW:
        group_default_callback (h, NULL, MSG_DRAW, 0, NULL); /* frame + buttons */
        return MSG_HANDLED;

    case MSG_KEY:
        switch (parm) {
        case KEY_LEFT:
        case KEY_UP:
            group_select_next_widget (GROUP (h));
            return MSG_HANDLED;

        case KEY_RIGHT:
        case KEY_DOWN:
            group_select_prev_widget (GROUP (h));
            return MSG_HANDLED;
        }
        return MSG_NOT_HANDLED;

    default:
        return widget_default_callback (h, sender, msg, parm, data);
    }
    /*NOTREACHED*/
    return 0;
}

/*end*/
