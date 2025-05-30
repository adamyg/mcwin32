/*
   Command line viewer

   Copyright (C) 2025, Adam Young (https://github.com/adamyg/)

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

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include "lib/global.h"
#include "lib/tty/tty.h"
#include "lib/tty/key.h"
#include "lib/util.h"
#include "lib/widget.h"

#include "src/filemanager/cmd.h"
#include "src/filemanager/command.h"        /* cmdline */
#include "src/filemanager/filemanager.h"    /* the_prompt */
#include "src/filemanager/filenot.h"        /* my_mkdir */

#include "src/consaver/cons.saver.h"        /* show_console_contents */

#include "src/keymap.h"
#include "src/history.h"
#include "src/setup.h"

#include "cmdview.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */

/**
 * Borrow the widget from its current owner, insert into the additional group, returning the 
 * previous owner. It is intended for the ownership to be returned when the local context completes.
 * Current widget remains unchanged.
 *
 */

static WGroup *
group_borrow_widget (WGroup *g, Widget *w)
{
    WGroup *previous_owner;

    previous_owner = w->owner;

    w->owner = g;
    g->widgets = g_list_append (g->widgets, w);

    return previous_owner;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Return the widget to its previous owner, removing it from the current group.
 *
 */

static void
group_return_widget (WGroup *g, Widget *w, WGroup *previous_owner)
{
    GList *d;

    d = g_list_find (g->widgets, w);
    if (d == g->current)
        group_set_current_widget_next (g);

    g->widgets = g_list_delete_link (g->widgets, d);
    if (g->widgets == NULL)
        g->current = NULL;

    w->owner = previous_owner;
}

/* --------------------------------------------------------------------------------------------- */

static void
cmdview_compute_areas (WCmdView *cview)
{
    Widget *w = WIDGET (cview);
    WRect rb;

    setup_cmdline (CONST_WIDGET (w));

    rb = w->rect;
    rb.y = w->rect.lines - 1;
    rb.lines = 1;

    widget_set_size_rect (WIDGET(cview->bar), &rb);
    widget_set_visibility (WIDGET(cview->bar), mc_global.keybar_visible);
}


/* --------------------------------------------------------------------------------------------- */

static void
cmdview_labels (WCmdView *cview)
{
    Widget *d = WIDGET (cview);
    WButtonBar *b;

    b = buttonbar_find (DIALOG (d->owner));

    buttonbar_set_label (b, 1, Q_ ("ButtonBar|Help"), d->keymap, d);
    buttonbar_set_label (b, 3, Q_ ("ButtonBar|View"), d->keymap, d);
    buttonbar_set_label (b, 4, Q_ ("ButtonBar|Edit"), d->keymap, d);
#ifdef ENABLE_CMDVIEW_MKDIR
    buttonbar_set_label (b, 7, Q_ ("ButtonBar|Mkdir"), d->keymap, d);
#endif
    buttonbar_set_label (b, 10, Q_ ("ButtonBar|Quit"), d->keymap, d);
}

/* --------------------------------------------------------------------------------------------- */

static void
cmdview_edit_view_file (int command)
{
    vfs_path_t *vpath = NULL;
    char *fname = NULL;

    switch (command)
    {
    case CK_Edit:
        fname = input_expand_dialog (_("Edit file"), _("Enter file name:"),
                                     MC_HISTORY_EDIT_LOAD, "", INPUT_COMPLETE_FILENAMES);
        break;
    case CK_View:
        fname = input_expand_dialog (_("View file"), _("Enter file name:"),
                                     MC_HISTORY_FM_VIEW_FILE, "", INPUT_COMPLETE_FILENAMES);
        break;
#ifdef ENABLE_CMDVIEW_MKDIR
    case CK_MakeDir:
        fname = input_expand_dialog (_("Create a new Directory"), _("Enter directory name:"), 
                                     MC_HISTORY_FM_MKDIR, "", INPUT_COMPLETE_FILENAMES);
        break;
#endif
    default:
        break;
    }

    if (fname == NULL)
        return;

    if (*fname != '\0')
        vpath = vfs_path_from_str (fname);

    if (vpath)
    {
        switch (command)
        {
        case CK_Edit:
            edit_file_at_line (vpath, use_internal_edit, 0);
            break;
        case CK_View:
            view_file (vpath, use_internal_view, FALSE);
            break;
#ifdef ENABLE_CMDVIEW_MKDIR
        case CK_MakeDir:
            if (my_mkdir (vpath, 0777) != 0)
                message (D_ERROR, MSG_ERROR, "%s", unix_error_string(errno));
            break;
#endif
        default:
            break;
        }
        vfs_path_free (vpath, TRUE);
    }

    g_free (fname);
}


/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
cmdview_execute_cmd (WCmdView *cview, long command)
{
    cb_ret_t res = MSG_HANDLED;

    switch (command)
    {
    case CK_Quit:
    case CK_Shell:
        cview->view_quit = TRUE;
        break;
    case CK_Edit:
    case CK_View:
#ifdef ENABLE_CMDVIEW_MKDIR
    case CK_MakeDir:
#endif
        cmdview_edit_view_file (command);
        break;
    case CK_History:
    case CK_HistoryNext:
    case CK_HistoryPrev:
    case CK_Complete:
        send_message (cmdline, NULL, MSG_ACTION, command, NULL);
        break;
    case CK_Cancel:
        break;
    default:
        res = MSG_NOT_HANDLED;
        break;
    }
    return res;
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
cmdview_handle_key (WCmdView *cview, int key)
{
    long command;

    command = widget_lookup_key (WIDGET (cview), key);
    if (command != CK_IgnoreKey)
        return cmdview_execute_cmd (cview, command);
    return send_message (cmdline, NULL, MSG_KEY, key, NULL);
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
cmdview_set_state (Widget *w, widget_state_t state, gboolean enable)
{
    if (enable)
    {
        WCmdView *cview = CMDVIEW (w);
        WDialog *h = DIALOG (w->owner);
        WGroup *g = GROUP (h);

        switch (state) 
        {
        case WST_ACTIVE:
            if (cview->ogroups[0] == NULL)
            {
                /* Dialog initialisation, take temporary ownership of prompt and command-line resources */
                assert (NULL == cview->ogroups[1]);
                assert (mc_global.cmdview_visible);

                cview->original_output_lines = output_lines;
                output_lines = w->rect.lines - 1; // see do_executev()

                cview->ogroups[1] = group_borrow_widget (g, WIDGET(the_prompt));
                cview->ogroups[0] = group_borrow_widget (g, WIDGET(cmdline));
                cview->original_command_prompt = command_prompt;
                command_prompt = TRUE;
            }
            break;

        case WST_FOCUSED:
            break;

        case WST_SUSPENDED:
        case WST_CLOSED:
            if (cview->ogroups[0] != NULL)
            {
                /* Restore ownership of prompt and command-line resources */
                assert (cview->ogroups[1]);
                assert (mc_global.cmdview_visible);
                if (state == WST_CLOSED)
                    mc_global.cmdview_visible = FALSE;

                output_lines = cview->original_output_lines;

                command_prompt = cview->original_command_prompt;
                if (! command_prompt)
                {
                    widget_hide (WIDGET(cmdline));
                    widget_hide (WIDGET(the_prompt));
                }

                group_return_widget (g, WIDGET(the_prompt), cview->ogroups[1]);
                group_return_widget (g, WIDGET(cmdline), cview->ogroups[0]);

                cview->ogroups[1] = NULL;
                cview->ogroups[0] = NULL;
            }
            break;

        default:
            break;
        }
    }

    return widget_default_set_state (w, state, enable);
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
cmdview_callback (Widget *w, Widget *sender, widget_msg_t msg, int parm, void *data)
{
    WCmdView *cview = CMDVIEW (w);
    WDialog *h = DIALOG (w->owner);
    cb_ret_t v = MSG_NOT_HANDLED;

    switch (msg)
    {
    case MSG_INIT:
        cmdview_labels (cview);
        cmdview_compute_areas (cview);
        assert (w->set_state == widget_default_set_state);
        assert (w->set_state != cmdview_set_state);
        w->set_state = cmdview_set_state;
        return MSG_HANDLED;

    case MSG_DRAW:
        show_console_contents (0, 0, w->rect.lines - (mc_global.keybar_visible ? 2 : 1));
        return MSG_HANDLED;

    case MSG_KEY:
        v = cmdview_handle_key (cview, parm);
        if (cview->view_quit)
            dlg_close (h);
        return v;

    case MSG_UNHANDLED_KEY:
        v = cmdview_handle_key (cview, parm);
        if (cview->view_quit)
            dlg_close (h);
        return v;

    case MSG_ACTION:
        v = cmdview_execute_cmd (cview, parm);
        if (cview->view_quit)
            dlg_close (h);
        return v;
 
    case MSG_RESIZE:
        widget_default_callback (w, NULL, MSG_RESIZE, 0, data);
        cmdview_compute_areas (cview);
        return MSG_HANDLED;

    case MSG_DESTROY:
        assert (cview->ogroups[0] == NULL);
        return MSG_HANDLED;

    default:
        return widget_default_callback (w, sender, msg, parm, data);
    }
}

/* --------------------------------------------------------------------------------------------- */

static void
cmdview_mouse_callback (Widget *w, mouse_msg_t msg, mouse_event_t *event)
{
    WCmdView *cview = (WCmdView *) w;

    (void) event;

    switch (msg)
    {
    case MSG_MOUSE_SCROLL_UP:
    case MSG_MOUSE_SCROLL_DOWN:
        break;

    default:
        break;
    }
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
cmdview_dialog_callback (Widget *w, Widget *sender, widget_msg_t msg, int parm, void *data)
{
    switch (msg)
    {
    case MSG_ACTION:
        switch (parm) 
        {
        case CK_ScreenList:
        case CK_ScreenNext:
        case CK_ScreenPrev:
            // disable screen-list support
            return MSG_HANDLED;
        }
        break;

    case MSG_VALIDATE:
        return MSG_HANDLED;

    default:
        break;
    }

    return dlg_default_callback (w, sender, msg, parm, data);
}

/* --------------------------------------------------------------------------------------------- */

static char *
cmdview_get_title (const WDialog *h, size_t len)
{
    const char *cwd;
    const char *label;
    char *ret_str;
   
    cwd = vfs_path_as_str (vfs_get_raw_current_dir());
    label = str_term_trim (cwd, len - str_term_width1(_("Command: ")));

    ret_str = g_strconcat (_("Command: "), label, (char*)NULL);
    return ret_str;
}

/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

gboolean
cmdview_cmd (void)
{
    WCmdView *cview;
    Widget *w;
    WDialog *cview_dlg;
    Widget *dw;
    WRect r;
    WGroup *g;

    /* Console restore available and non-recursive invocation */
    if (mc_global.tty.console_flag == '\0' || mc_global.cmdview_visible)
        return FALSE;

    /* Create dialog and associated widgets */
    cview_dlg =
        dlg_create (FALSE, 0, 0, 1, 1, WPOS_FULLSCREEN, FALSE, NULL, cmdview_dialog_callback, NULL,
                    "[Input Line Keys]", NULL);
    dw = WIDGET (cview_dlg);
    widget_want_tab (dw, TRUE);
    r = dw->rect;

    cview = g_new0 (WCmdView, 1);
    g = GROUP (cview_dlg);
    w = WIDGET (cview);
    widget_init (w, &r, cmdview_callback, cmdview_mouse_callback);
    w->options |= WOP_SELECTABLE;
    w->keymap = cmdview_map;
    group_add_widget_autopos (g, w, WPOS_KEEP_ALL, NULL);

    cview->bar = buttonbar_new ();
    w = WIDGET (cview->bar);
    group_add_widget_autopos (g, w, w->pos_flags, NULL);

    cview_dlg->get_title = cmdview_get_title;

    /* Run dialog, temporary take ownership of prompt and command-line resources */
    mc_global.cmdview_visible = TRUE;
    dlg_run (cview_dlg);

    /* Cleanup */
    if (widget_get_state (dw, WST_CLOSED))
        widget_destroy (dw);

    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */
