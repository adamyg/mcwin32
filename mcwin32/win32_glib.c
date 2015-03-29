/* -*- indent-width: 4; -*- */
/*
   win32 glib patches

        g_get_current_dir
        g_strdup_printf
        g_strdup_vprintf
        g_snprintf
        g_vsnprintf
        g_string_append_printf
        g_string_append_vprintf
        g_error_new_valist
        g_get_user_config_dir

        * Address PATH_SEP and va_list usage.

   Copyright (C) 2012
   The Free Software Foundation, Inc.

   Written by: Adam Young 2012-2015

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
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>                             /* struct sigaction */
#include <limits.h>                             /* INT_MAX */
#include <malloc.h>

#include <glib.h>

#include "lib/global.h"
#include "lib/vfs/vfs.h"                        /* VFS_ENCODING_PREFIX */
#include "lib/vfs/xdirentry.h"


static void
unixpathset(char *path)
{
    const char *in = path;

    while (*in) {
        if ('/' == *in || '\\' == *in) {
            ++in;
            while ('/' == *in || '\\' == *in) {
                ++in;
            }
            *path++ = PATH_SEP;
        } else {
            *path++ = *in++;
        }
    }
    *path = 0;
}


/*
 *  gettext hook replacement
 */
int
libintl_fprintf (FILE *file, const char *format, ...)
{
    va_list ap;
    int ret;

#undef vfprintf
    va_start(ap, format);
    ret = vfprintf(file, format, ap);
    va_end(ap);
    return ret;
}


int
libintl_vfprintf (FILE *file, const char *format, va_list ap)
{
#undef vfprintf
    return vfprintf(file, format, ap);
}


/*
 *  g_get_current_dir() replacement, returns path using PATH_SEP.
 */
char *
g_get_current_dir (void)
{
    char cwd[1024];
    w32_getcwd(cwd, sizeof(cwd));
    return g_strdup(cwd);
}


/*
 *  g_mktemps replacement
 */
int
g_mkstemp (char *path)
{
    return w32_mkstemp(path);
}


/*
 *  g_strdup_printf() replacement.
 */
char *
g_strdup_printf (const char *format, ...)
{
    char buffer[4 * 1024];
    va_list ap;

    va_start(ap, format);
    _vsnprintf(buffer, sizeof(buffer), format, ap);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(ap);
    return g_strdup(buffer);
}


/*
 *  g_strdup_vprintf() replacement.
 */
char *
g_strdup_vprintf (const char *format, va_list ap)
{
    char buffer[4 * 1024];

    _vsnprintf(buffer, sizeof(buffer), format, ap);
    buffer[sizeof(buffer) - 1] = 0;
    return g_strdup(buffer);
}


/*
 *  g_snprintf() replacement.
 *
 */
gint
g_snprintf (gchar *buffer, gulong length, const char *format, ...)
{
    va_list ap;
    int ret = 0;

    if (length > 0) {
        va_start(ap, format);
        ret = _vsnprintf(buffer, length, format, ap);
        buffer[length - 1] = 0;
        va_end(ap);
    }
    return ret;
}


/*
 *  g_vsnprintf() replacement.
 */
gint
g_vsnprintf (gchar *string, gulong n, const char *format, va_list ap)
{
    return _vsnprintf(string, n, format, ap);
}



/*
 *  g_string_append_printf() replacement
 */
void
g_string_append_printf (GString *string, const gchar *format, ...)
{
    char buffer[4 * 1024];
    va_list ap;

    va_start(ap, format);
    _vsnprintf(buffer, sizeof(buffer), format, ap);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(ap);
    g_string_append (string, buffer);
}


/*
 *  g_string_append_vprintf() replacement
 */
void
g_string_append_vprintf (GString *string, const gchar *format, va_list ap)
{
    char buffer[4 * 1024];

    _vsnprintf(buffer, sizeof(buffer), format, ap);
    buffer[sizeof(buffer) - 1] = 0;
    g_string_append (string, buffer);
}


/*
 *  g_error_new_valist() replacement
 */
GError *
g_error_new_valist (GQuark domain, gint code, const gchar * format, va_list ap)
{
    char *message;
    GError *ret_value;

    message = g_strdup_vprintf (format, ap);
    ret_value = g_error_new_literal (domain, code, message);
    g_free (message);

    return ret_value;
}


/*
 *  g_get_user_config_dir() replacement
 */
const char *
g_get_user_config_dir (void)
{
    return mc_USERCONFIGDIR(NULL);
}


/*
 *  g_build_filename() replacement
 */
extern void canonicalize_pathname (char *path); /*FIXME*/

char *
g_build_filename (const gchar *first_element, ...)
{
    const char *element;
    GString *path;
    char *ret;
    va_list ap;

    if (NULL == (element = (const char *)first_element)) {
        return NULL;
    }

    va_start (ap, first_element);
    path = g_string_new ("");

    do {
        if (! *element) {
            element = va_arg (ap, const char *);

        } else {
            char *tmp_element = g_strdup (element);
            size_t len;
            const char *start;

            element = va_arg (ap, const char *);
            canonicalize_pathname (tmp_element);
            len = strlen (tmp_element);
            start = (PATH_SEP == tmp_element[0]) ? tmp_element + 1 : tmp_element;
            g_string_append (path, start);
            if (tmp_element[len - 1] != PATH_SEP && NULL != element) {
                g_string_append_c (path, PATH_SEP);
            }
            g_free (tmp_element);
        }
    } while (NULL != element);

    if ('\\' == first_element[0] || '/' == first_element[0]) {
        if (0 == path->len || ':' != path->str[1]) {
            g_string_prepend_c (path, PATH_SEP);
        }
    }

    ret = g_string_free (path, FALSE);
    canonicalize_pathname (ret);
    return ret;
}
/*end*/


