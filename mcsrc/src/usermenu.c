/*
   User Menu implementation

   Copyright (C) 1994-2025
   Free Software Foundation, Inc.

   Written by:
   Slava Zanko <slavazanko@gmail.com>, 2013
   Andrew Borodin <aborodin@vmail.ru>, 2013

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

/** \file usermenu.c
 *  \brief Source: user menu implementation
 */

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lib/global.h"
#include "lib/fileloc.h"
#include "lib/tty/tty.h"
#include "lib/skin.h"
#include "lib/search.h"
#include "lib/vfs/vfs.h"
#include "lib/strutil.h"
#include "lib/util.h"

#ifdef USE_INTERNAL_EDIT
#include "src/editor/edit.h"    /* WEdit */
#endif
#include "src/viewer/mcviewer.h"        /* for default_* externs */

#include "src/args.h"           /* mc_run_param0 */
#include "src/execute.h"
#include "src/setup.h"
#include "src/history.h"

#include "src/filemanager/dir.h"
#include "src/filemanager/filemanager.h"
#include "src/filemanager/layout.h"

#include "usermenu.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

#define MAX_ENTRY_LEN 60

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

static gboolean debug_flag = FALSE;
static gboolean debug_error = FALSE;
static char *menu = NULL;

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/** strip file's extension */
static char *
strip_ext (char *ss)
{
    char *s;
    char *e = NULL;

    if (ss == NULL)
        return NULL;

    for (s = ss; *s != '\0'; s++)
    {
        if (*s == '.')
            e = s;
        if (IS_PATH_SEP (*s) && e != NULL)
            e = NULL;           /* '.' in *directory* name */
    }

    if (e != NULL)
        *e = '\0';

    return (*ss == '\0' ? NULL : ss);
}

/* --------------------------------------------------------------------------------------------- */
/**
 * Check for the "shell_patterns" directive.  If it's found and valid,
 * interpret it and move the pointer past the directive.  Return the
 * current pointer.
 */

static char *
check_patterns (char *p)
{
    static const char def_name[] = "shell_patterns=";
    char *p0 = p;

    if (strncmp (p, def_name, sizeof (def_name) - 1) != 0)
        return p0;

    p += sizeof (def_name) - 1;
    if (*p == '1')
        easy_patterns = TRUE;
    else if (*p == '0')
        easy_patterns = FALSE;
    else
        return p0;

    /* Skip spaces */
    p++;
    while (whiteness (*p))
        p++;
    return p;
}

/* --------------------------------------------------------------------------------------------- */
/** Copies a whitespace separated argument from p to arg. Returns the
   point after argument. */

static char *
extract_arg (char *p, char *arg, int size)
{
    while (*p != '\0' && whiteness (*p))
        p++;

    /* support quote space .mnu */
    while (*p != '\0' && (*p != ' ' || *(p - 1) == '\\') && *p != '\t' && *p != '\n')
    {
        char *np;

        np = str_get_next_char (p);
        if (np - p >= size)
            break;
        memcpy (arg, p, np - p);
        arg += np - p;
        size -= np - p;
        p = np;
    }
    *arg = '\0';
    if (*p == '\0' || *p == '\n')
        str_prev_char (&p);
    return p;
}

/* --------------------------------------------------------------------------------------------- */
/* Tests whether the selected file in the panel is of any of the types
   specified in argument. */

static gboolean
test_type (WPanel *panel, char *arg)
{
    const file_entry_t *fe;
    int result = 0;             /* False by default */
    mode_t st_mode;

    fe = panel_current_entry (panel);
    if (fe == NULL)
        return FALSE;

    st_mode = fe->st.st_mode;

    for (; *arg != '\0'; arg++)
    {
        switch (*arg)
        {
        case 'n':              /* Not a directory */
            result |= !S_ISDIR (st_mode);
            break;
        case 'r':              /* Regular file */
            result |= S_ISREG (st_mode);
            break;
        case 'd':              /* Directory */
            result |= S_ISDIR (st_mode);
            break;
        case 'l':              /* Link */
            result |= S_ISLNK (st_mode);
            break;
        case 'c':              /* Character special */
            result |= S_ISCHR (st_mode);
            break;
        case 'b':              /* Block special */
            result |= S_ISBLK (st_mode);
            break;
        case 'f':              /* Fifo (named pipe) */
            result |= S_ISFIFO (st_mode);
            break;
        case 's':              /* Socket */
            result |= S_ISSOCK (st_mode);
            break;
        case 'x':              /* Executable */
            result |= (st_mode & 0111) != 0 ? 1 : 0;
            break;
        case 't':
            result |= panel->marked != 0 ? 1 : 0;
            break;
        default:
            debug_error = TRUE;
            break;
        }
    }

    return (result != 0);
}

/* --------------------------------------------------------------------------------------------- */
/** Calculates the truth value of the next condition starting from
   p. Returns the point after condition. */

static char *
test_condition (const Widget *edit_widget, char *p, gboolean *condition)
{
    char arg[256];
    const mc_search_type_t search_type = easy_patterns ? MC_SEARCH_T_GLOB : MC_SEARCH_T_REGEX;
#ifdef USE_INTERNAL_EDIT
    const WEdit *e = CONST_EDIT (edit_widget);
#endif

    /* Handle one condition */
    for (; *p != '\n' && *p != '&' && *p != '|'; p++)
    {
        WPanel *panel = NULL;

        /* support quote space .mnu */
        if ((*p == ' ' && *(p - 1) != '\\') || *p == '\t')
            continue;
        if (*p >= 'a')
            panel = current_panel;
        else if (get_other_type () == view_listing)
            panel = other_panel;

        *p |= 0x20;

        switch (*p++)
        {
        case '!':
            p = test_condition (edit_widget, p, condition);
            *condition = !*condition;
            str_prev_char (&p);
            break;
        case 'f':              /* file name pattern */
            p = extract_arg (p, arg, sizeof (arg));
#ifdef USE_INTERNAL_EDIT
            if (e != NULL)
            {
                const char *edit_filename;

                edit_filename = edit_get_file_name (e);
                *condition = mc_search (arg, MC_DEFAULT_CHARSET, edit_filename, search_type);
            }
            else
#endif
            {
                if (panel == NULL)
                    *condition = FALSE;
                else
                {
                    const file_entry_t *fe;

                    fe = panel_current_entry (panel);
                    *condition = fe != NULL
                        && mc_search (arg, MC_DEFAULT_CHARSET, fe->fname->str, search_type);
                }
            }
            break;
        case 'y':              /* syntax pattern */
#ifdef USE_INTERNAL_EDIT
            if (e != NULL)
            {
                const char *syntax_type;

                syntax_type = edit_get_syntax_type (e);
                if (syntax_type != NULL)
                {
                    p = extract_arg (p, arg, sizeof (arg));
                    *condition = mc_search (arg, MC_DEFAULT_CHARSET, syntax_type, MC_SEARCH_T_NORMAL);
                }
            }
#endif
            break;
        case 'd':
            p = extract_arg (p, arg, sizeof (arg));
            *condition = panel != NULL
                && mc_search (arg, MC_DEFAULT_CHARSET, vfs_path_as_str (panel->cwd_vpath),
                              search_type);
            break;
        case 't':
            p = extract_arg (p, arg, sizeof (arg));
            *condition = panel != NULL && test_type (panel, arg);
            break;
        case 'x':              /* executable */
            {
                mc_stat_t status;

                p = extract_arg (p, arg, sizeof (arg));
                *condition = stat (arg, &status) == 0 && is_exe (status.st_mode);
                break;
            }
        default:
            debug_error = TRUE;
            break;
        }                       /* switch */
    }                           /* while */
    return p;
}

/* --------------------------------------------------------------------------------------------- */
/** General purpose condition debug output handler */

static void
debug_out (char *start, char *end, gboolean condition)
{
    static char *msg = NULL;

    if (start == NULL && end == NULL)
    {
        /* Show output */
        if (debug_flag && msg != NULL)
        {
            size_t len;

            len = strlen (msg);
            if (len != 0)
                msg[len - 1] = '\0';
            message (D_NORMAL, _("Debug"), "%s", msg);

        }
        debug_flag = FALSE;
        MC_PTR_FREE (msg);
    }
    else
    {
        const char *type;
        char *p;

        /* Save debug info for later output */
        if (!debug_flag)
            return;
        /* Save the result of the condition */
        if (debug_error)
        {
            type = _("ERROR:");
            debug_error = FALSE;
        }
        else if (condition)
            type = _("True:");
        else
            type = _("False:");
        /* This is for debugging, don't need to be super efficient.  */
        if (end == NULL)
            p = g_strdup_printf ("%s %s %c \n", msg ? msg : "", type, *start);
        else
            p = g_strdup_printf ("%s %s %.*s \n", msg ? msg : "", type, (int) (end - start), start);
        g_free (msg);
        msg = p;
    }
}

/* --------------------------------------------------------------------------------------------- */
/** Calculates the truth value of one lineful of conditions. Returns
   the point just before the end of line. */

static char *
test_line (const Widget *edit_widget, char *p, gboolean *result)
{
    char operator;

    /* Repeat till end of line */
    while (*p != '\0' && *p != '\n')
    {
        char *debug_start, *debug_end;
        gboolean condition = TRUE;

        /* support quote space .mnu */
        while ((*p == ' ' && *(p - 1) != '\\') || *p == '\t')
            p++;
        if (*p == '\0' || *p == '\n')
            break;
        operator = *p++;
        if (*p == '?')
        {
            debug_flag = TRUE;
            p++;
        }
        /* support quote space .mnu */
        while ((*p == ' ' && *(p - 1) != '\\') || *p == '\t')
            p++;
        if (*p == '\0' || *p == '\n')
            break;

        debug_start = p;
        p = test_condition (edit_widget, p, &condition);
        debug_end = p;
        /* Add one debug statement */
        debug_out (debug_start, debug_end, condition);

        switch (operator)
        {
        case '+':
        case '=':
            /* Assignment */
            *result = condition;
            break;
        case '&':              /* Logical and */
            *result = *result && condition;
            break;
        case '|':              /* Logical or */
            *result = *result || condition;
            break;
        default:
            debug_error = TRUE;
            break;
        }                       /* switch */
        /* Add one debug statement */
        debug_out (&operator, NULL, *result);

    }                           /* while (*p != '\n') */
    /* Report debug message */
    debug_out (NULL, NULL, TRUE);

    if (*p == '\0' || *p == '\n')
        str_prev_char (&p);
    return p;
}

/* --------------------------------------------------------------------------------------------- */
/** FIXME: recode this routine on version 3.0, it could be cleaner */

static void
execute_menu_command (const Widget *edit_widget, const char *commands, gboolean show_prompt)
{
    FILE *cmd_file;
    int cmd_file_fd;
    gboolean expand_prefix_found = FALSE;
    char *parameter = NULL;
    gboolean do_quote = FALSE;
    char lc_prompt[80];
    int col;
    vfs_path_t *file_name_vpath;
    gboolean run_view = FALSE;
    char *cmd;

    /* Skip menu entry title line */
    commands = strchr (commands, '\n');
    if (commands == NULL)
        return;

    cmd_file_fd = mc_mkstemps (&file_name_vpath, "mcusr", SCRIPT_SUFFIX);

    if (cmd_file_fd == -1)
    {
        message (D_ERROR, MSG_ERROR, _("Cannot create temporary command file\n%s"),
                 unix_error_string (errno));
        return;
    }

    cmd_file = fdopen (cmd_file_fd, "w");
    fputs ("#! /bin/sh\n", cmd_file);
    commands++;

    for (col = 0; *commands != '\0'; commands++)
    {
        if (col == 0)
        {
            if (!whitespace (*commands))
                break;
            while (whitespace (*commands))
                commands++;
            if (*commands == '\0')
                break;
        }
        col++;
        if (*commands == '\n')
            col = 0;
        if (parameter != NULL)
        {
            if (*commands == '}')
            {
                *parameter = '\0';
                parameter =
                    input_dialog (_("Parameter"), lc_prompt, MC_HISTORY_FM_MENU_EXEC_PARAM, "",
                                  INPUT_COMPLETE_FILENAMES | INPUT_COMPLETE_CD |
                                  INPUT_COMPLETE_HOSTNAMES | INPUT_COMPLETE_VARIABLES |
                                  INPUT_COMPLETE_USERNAMES);
                if (parameter == NULL || *parameter == '\0')
                {
                    /* User canceled */
                    g_free (parameter);
                    fclose (cmd_file);
                    mc_unlink (file_name_vpath);
                    vfs_path_free (file_name_vpath, TRUE);
                    return;
                }
                if (do_quote)
                {
                    char *tmp;

                    tmp = name_quote (parameter, FALSE);
                    if (tmp != NULL)
                    {
                        fputs (tmp, cmd_file);
                        g_free (tmp);
                    }
                }
                else
                    fputs (parameter, cmd_file);

                MC_PTR_FREE (parameter);
            }
            else if (parameter < lc_prompt + sizeof (lc_prompt) - 1)
                *parameter++ = *commands;
        }
        else if (expand_prefix_found)
        {
            expand_prefix_found = FALSE;
            if (g_ascii_isdigit ((gchar) * commands))
            {
                do_quote = (atoi (commands) != 0);
                while (g_ascii_isdigit ((gchar) * commands))
                    commands++;
            }
            if (*commands == '{')
                parameter = lc_prompt;
            else
            {
                char *text;

                text = expand_format (edit_widget, *commands, do_quote);
                if (text != NULL)
                {
                    fputs (text, cmd_file);
                    g_free (text);
                }
            }
        }
        else if (*commands == '%')
        {
            int i;

            i = check_format_view (commands + 1);
            if (i != 0)
            {
                commands += i;
                run_view = TRUE;
            }
            else
            {
                do_quote = TRUE;        /* Default: Quote expanded macro */
                expand_prefix_found = TRUE;
            }
        }
        else
            fputc (*commands, cmd_file);
    }

    fclose (cmd_file);
    mc_chmod (file_name_vpath, S_IRWXU);

    /* Execute the command indirectly to allow execution even on no-exec filesystems. */
    cmd = g_strconcat ("/bin/sh ", vfs_path_as_str (file_name_vpath), (char *) NULL);

    if (run_view)
    {
        mcview_viewer (cmd, NULL, 0, 0, 0);
        dialog_switch_process_pending ();
    }
    else if (show_prompt)
        shell_execute (cmd, EXECUTE_HIDE);
    else
    {
        gboolean ok;

        /* Prepare the terminal by setting its flag to the initial ones. This will cause \r
         * to work as expected, instead of being ignored. */
        tty_reset_shell_mode ();

        ok = (system (cmd) != -1);

        /* Restore terminal configuration. */
        tty_raw_mode ();

        /* Redraw the original screen's contents. */
        tty_clear_screen ();
        repaint_screen ();

        if (!ok)
            message (D_ERROR, MSG_ERROR, "%s", _("Error calling program"));
    }

    g_free (cmd);

    mc_unlink (file_name_vpath);
    vfs_path_free (file_name_vpath, TRUE);
}

/* --------------------------------------------------------------------------------------------- */
/**
 **     Check owner of the menu file. Using menu file is allowed, if
 **     owner of the menu is root or the actual user. In either case
 **     file should not be group and word-writable.
 **
 **     Q. Should we apply this routine to system and home menu (and .ext files)?
 */

static gboolean
menu_file_own (char *path)
{
    mc_stat_t st;

    if (stat (path, &st) == 0 && (st.st_uid == 0 || (st.st_uid == geteuid ()) != 0)
        && ((st.st_mode & (S_IWGRP | S_IWOTH)) == 0))
        return TRUE;

    if (verbose)
        message (D_NORMAL, _("Warning -- ignoring file"),
                 _("File %s is not owned by root or you or is world writable.\n"
                   "Using it may compromise your security"), path);

    return FALSE;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* Formats defined:
   %%  The % character
   %f  The current file in the active panel (if non-local vfs, file will be copied locally
   and %f will be full path to it) or the opened file in the internal editor.
   %p  Likewise.
   %d  The current working directory
   %s  "Selected files"; the tagged files if any, otherwise the current file
   %t  Tagged files
   %u  Tagged files (and they are untagged on return from expand_format)
   %view Runs the commands and pipes standard output to the view command.
   If %view is immediately followed by '{', recognize keywords
   ascii, hex, nroff and unform

   If the format letter is in uppercase, it refers to the other panel.

   With a number followed the % character you can turn quoting on (default)
   and off. For example:
   %f    quote expanded macro
   %1f   ditto
   %0f   don't quote expanded macro

   expand_format returns a memory block that must be free()d.
 */

/* Returns how many characters we should advance if %view was found */
int
check_format_view (const char *p)
{
    const char *q = p;

    if (strncmp (p, "view", 4) == 0)
    {
        q += 4;
        if (*q == '{')
        {
            for (q++; *q != '\0' && *q != '}'; q++)
            {
                if (strncmp (q, MC_DEFAULT_CHARSET, 5) == 0)
                {
                    mcview_global_flags.hex = FALSE;
                    q += 4;
                }
                else if (strncmp (q, "hex", 3) == 0)
                {
                    mcview_global_flags.hex = TRUE;
                    q += 2;
                }
                else if (strncmp (q, "nroff", 5) == 0)
                {
                    mcview_global_flags.nroff = TRUE;
                    q += 4;
                }
                else if (strncmp (q, "unform", 6) == 0)
                {
                    mcview_global_flags.nroff = FALSE;
                    q += 5;
                }
            }
            if (*q == '}')
                q++;
        }
        return q - p;
    }
    return 0;
}

/* --------------------------------------------------------------------------------------------- */

int
check_format_cd (const char *p)
{
    return (strncmp (p, "cd", 2)) != 0 ? 0 : 3;
}

/* --------------------------------------------------------------------------------------------- */
/* Check if p has a "^var\{var-name\}" */
/* Returns the number of skipped characters (zero on not found) */
/* V will be set to the expanded variable name */

int
check_format_var (const char *p, char **v)
{
    *v = NULL;

    if (strncmp (p, "var{", 4) == 0)
    {
        const char *q = p;
        const char *dots = NULL;
        const char *value;
        char *var_name;

        for (q += 4; *q != '\0' && *q != '}'; q++)
        {
            if (*q == ':')
                dots = q + 1;
        }
        if (*q == '\0')
            return 0;

        if (dots == NULL || dots == q + 5)
        {
            message (D_ERROR,
                     _("Format error on file Extensions File"),
                     dots == NULL ? _("The %%var macro has no default")
                     : _("The %%var macro has no variable"));
            return 0;
        }

        /* Copy the variable name */
        var_name = g_strndup (p + 4, dots - 2 - (p + 3));
        value = getenv (var_name);
        g_free (var_name);

        if (value != NULL)
            *v = g_strdup (value);
        else
            *v = g_strndup (dots, q - dots);

        return q - p;
    }
    return 0;
}

/* --------------------------------------------------------------------------------------------- */

char *
expand_format (const Widget *edit_widget, char c, gboolean do_quote)
{
    WPanel *panel = NULL;
    char *(*quote_func) (const char *, gboolean);
    const char *fname = NULL;
    char *result;
    char c_lc;

#ifdef USE_INTERNAL_EDIT
    const WEdit *e = CONST_EDIT (edit_widget);
#else
    (void) edit_widget;
#endif

    if (c == '%')
        return g_strdup ("%");

    switch (mc_global.mc_run_mode)
    {
    case MC_RUN_FULL:
#ifdef USE_INTERNAL_EDIT
        if (e != NULL)
            fname = edit_get_file_name (e);
        else
#endif
        {
            const file_entry_t *fe;

            if (g_ascii_islower ((gchar) c))
                panel = current_panel;
            else
            {
                if (get_other_type () != view_listing)
                    return NULL;
                panel = other_panel;
            }

            fe = panel_current_entry (panel);
            fname = fe == NULL ? NULL : fe->fname->str;
        }
        break;

#ifdef USE_INTERNAL_EDIT
    case MC_RUN_EDITOR:
        fname = edit_get_file_name (e);
        break;
#endif

    case MC_RUN_VIEWER:
        /* mc_run_param0 is not NULL here because mcviewer isn't run without input file */
        fname = (const char *) mc_run_param0;
        break;

    default:
        /* other modes don't use formats */
        return NULL;
    }

    if (do_quote)
        quote_func = name_quote;
    else
        quote_func = fake_name_quote;

    c_lc = g_ascii_tolower ((gchar) c);

    switch (c_lc)
    {
    case 'f':
    case 'p':
        result = quote_func (fname, FALSE);
        goto ret;
    case 'x':
        result = quote_func (extension (fname), FALSE);
        goto ret;
    case 'd':
        {
            const char *cwd;

            if (panel != NULL)
                cwd = vfs_path_as_str (panel->cwd_vpath);
            else
                cwd = vfs_get_current_dir ();

            result = quote_func (cwd, FALSE);
            goto ret;
        }
    case 'c':
#ifdef USE_INTERNAL_EDIT
        if (e != NULL)
        {
            result = g_strdup_printf ("%u", (unsigned int) edit_get_cursor_offset (e));
            goto ret;
        }
#endif
        break;
    case 'i':                  /* indent equal number cursor position in line */
#ifdef USE_INTERNAL_EDIT
        if (e != NULL)
        {
            result = g_strnfill (edit_get_curs_col (e), ' ');
            goto ret;
        }
#endif
        break;
    case 'y':                  /* syntax type */
#ifdef USE_INTERNAL_EDIT
        if (e != NULL)
        {
            const char *syntax_type;

            syntax_type = edit_get_syntax_type (e);
            if (syntax_type != NULL)
            {
                result = g_strdup (syntax_type);
                goto ret;
            }
        }
#endif
        break;
    case 'k':                  /* block file name */
    case 'b':                  /* block file name / strip extension */
#ifdef USE_INTERNAL_EDIT
        if (e != NULL)
        {
            char *file;

            file = mc_config_get_full_path (EDIT_HOME_BLOCK_FILE);
            result = quote_func (file, FALSE);
            g_free (file);
            goto ret;
        }
#endif
        if (c_lc == 'b')
        {
            result = strip_ext (quote_func (fname, FALSE));
            goto ret;
        }
        break;
    case 'n':                  /* strip extension in editor */
#ifdef USE_INTERNAL_EDIT
        if (e != NULL)
        {
            result = strip_ext (quote_func (fname, FALSE));
            goto ret;
        }
#endif
        break;
    case 'm':                  /* menu file name */
        if (menu != NULL)
        {
            result = quote_func (menu, FALSE);
            goto ret;
        }
        break;
    case 's':
        if (panel == NULL || panel->marked == 0)
        {
            result = quote_func (fname, FALSE);
            goto ret;
        }

        MC_FALLTHROUGH;

    case 't':
    case 'u':
        {
            GString *block = NULL;
            int i;

            if (panel == NULL)
            {
                result = NULL;
                goto ret;
            }

            for (i = 0; i < panel->dir.len; i++)
                if (panel->dir.list[i].f.marked != 0)
                {
                    char *tmp;

                    tmp = quote_func (panel->dir.list[i].fname->str, FALSE);
                    if (tmp != NULL)
                    {
                        if (block == NULL)
                            block = g_string_new_take (tmp);
                        else
                        {
                            g_string_append (block, tmp);
                            g_free (tmp);
                        }
                        g_string_append_c (block, ' ');
                    }

                    if (c_lc == 'u')
                        do_file_mark (panel, i, 0);
                }
            result = block == NULL ? NULL : g_string_free (block, block->len == 0);
            goto ret;
        }                       /* sub case block */
    default:
        break;
    }                           /* switch */

    result = g_strdup ("% ");
    result[1] = c;
  ret:
    return result;
}

/* --------------------------------------------------------------------------------------------- */
/**
 * If edit_widget is NULL then we are called from the mc menu,
 * otherwise we are called from the mcedit menu.
 */

gboolean
user_menu_cmd (const Widget *edit_widget, const char *menu_file, int selected_entry)
{
    char *data, *p;
    GPtrArray *entries = NULL;
    int max_cols = 0;
    int col = 0;
    gboolean accept_entry = TRUE;
    int selected = 0;
    gboolean old_patterns;
    gboolean res = FALSE;
    gboolean interactive = TRUE;

    if (!vfs_current_is_local ())
    {
        message (D_ERROR, MSG_ERROR, "%s", _("Cannot execute commands on non-local filesystems"));
        return FALSE;
    }

    menu = g_strdup (menu_file != NULL ? menu_file : edit_widget != NULL ?
                     EDIT_LOCAL_MENU : MC_LOCAL_MENU);

    if (!exist_file (menu) || !menu_file_own (menu))
    {
        if (menu_file != NULL)
        {
            message (D_ERROR, MSG_ERROR, _("Cannot open file %s\n%s"), menu,
                     unix_error_string (errno));
            MC_PTR_FREE (menu);
            return FALSE;
        }

        g_free (menu);
        menu = mc_config_get_full_path (edit_widget != NULL ? EDIT_HOME_MENU : MC_USERMENU_FILE);
        if (!exist_file (menu))
        {
            const char *global_menu;

            global_menu = edit_widget != NULL ? EDIT_GLOBAL_MENU : MC_GLOBAL_MENU;

            g_free (menu);
            menu = mc_build_filename (mc_config_get_home_dir (), global_menu, (char *) NULL);
            if (!exist_file (menu))
            {
                g_free (menu);
                menu = mc_build_filename (mc_global.sysconfig_dir, global_menu, (char *) NULL);
                if (!exist_file (menu))
                {
                    g_free (menu);
                    menu = mc_build_filename (mc_global.share_data_dir, global_menu, (char *) NULL);
                }
            }
        }
    }

    if (!g_file_get_contents (menu, &data, NULL, NULL))
    {
        message (D_ERROR, MSG_ERROR, _("Cannot open file %s\n%s"), menu, unix_error_string (errno));
        MC_PTR_FREE (menu);
        return FALSE;
    }

    old_patterns = easy_patterns;

    /* Parse the menu file */
    for (p = check_patterns (data); *p != '\0'; str_next_char (&p))
    {
        unsigned int menu_lines = entries == NULL ? 0 : entries->len;

        if (col == 0 && (entries == NULL || menu_lines == entries->len))
            switch (*p)
            {
            case '#':
                /* do not show prompt if first line of external script is #silent */
                if (selected_entry >= 0 && strncmp (p, "#silent", 7) == 0)
                    interactive = FALSE;
                /* A commented menu entry */
                accept_entry = TRUE;
                break;

            case '+':
                if (*(p + 1) == '=')
                {
                    /* Combined adding and default */
                    p = test_line (edit_widget, p + 1, &accept_entry);
                    if (selected == 0 && accept_entry)
                        selected = menu_lines;
                }
                else
                {
                    /* A condition for adding the entry */
                    p = test_line (edit_widget, p, &accept_entry);
                }
                break;

            case '=':
                if (*(p + 1) == '+')
                {
                    /* Combined adding and default */
                    p = test_line (edit_widget, p + 1, &accept_entry);
                    if (selected == 0 && accept_entry)
                        selected = menu_lines;
                }
                else
                {
                    /* A condition for making the entry default */
                    gboolean ok = TRUE;

                    p = test_line (edit_widget, p, &ok);
                    if (selected == 0 && ok)
                        selected = menu_lines;
                }
                break;

            default:
                if (!whitespace (*p) && str_isprint (p))
                {
                    /* A menu entry title line */
                    if (accept_entry)
                    {
                        if (entries == NULL)
                            entries = g_ptr_array_new ();
                        g_ptr_array_add (entries, p);
                    }
                    else
                        accept_entry = TRUE;
                }
                break;
            }

        if (*p == '\n')
        {
            if (entries != NULL && entries->len > menu_lines)
                accept_entry = TRUE;
            max_cols = MAX (max_cols, col);
            col = 0;
        }
        else
        {
            if (*p == '\t')
                *p = ' ';
            col++;
        }
    }

    if (entries == NULL)
        message (D_ERROR, MSG_ERROR, _("No suitable entries found in %s"), menu);
    else
    {
        if (selected_entry >= 0)
            selected = selected_entry;
        else
        {
            Listbox *listbox;
            unsigned int i;

            max_cols = MIN (MAX (max_cols, col), MAX_ENTRY_LEN);

            /* Create listbox */
            listbox = listbox_window_new (entries->len, max_cols + 2, _("User menu"),
                                          "[Edit Menu File]");
            /* insert all the items found */
            for (i = 0; i < entries->len; i++)
            {
                p = g_ptr_array_index (entries, i);
                LISTBOX_APPEND_TEXT (listbox, (unsigned char) p[0],
                                     extract_line (p, p + MAX_ENTRY_LEN, NULL), p, FALSE);
            }
            /* Select the default entry */
            listbox_set_current (listbox->list, selected);

            selected = listbox_run (listbox);
        }

        if (selected >= 0)
        {
            execute_menu_command (edit_widget, g_ptr_array_index (entries, selected), interactive);
            res = TRUE;
        }

        g_ptr_array_free (entries, TRUE);

        do_refresh ();
    }

    easy_patterns = old_patterns;
    MC_PTR_FREE (menu);
    g_free (data);
    return res;
}

/* --------------------------------------------------------------------------------------------- */
