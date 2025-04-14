/* -*- mode: c; indent-width: 4; -*- */
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

   Adam Young 2015 - 2025

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

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>                             /* struct sigaction */
#include <limits.h>                             /* INT_MAX */
#include <malloc.h>

#undef LIBW32_DYNAMIC
#include <glib.h>

#include "lib/global.h"
#include "lib/vfs/vfs.h"                        /* VFS_ENCODING_PREFIX */
#include "lib/vfs/xdirentry.h"

/*
 *  g_strerror replacement, extended error code support.
 */
const char *
g_strerror(int errnum)
{
    return w32_strerror(errnum);
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
//  int
//  g_mkstemp (char *path)
//  {
//	return w32_mkstemp(path);
//  }


/*
 *  g_get_user_config_dir() replacement
 */
const char *
g_get_user_config_dir (void)
{
    /* confirm symbol resolution/link options */
#if !defined(__GNUC__)
#if !defined(NDEBUG)
    static const char glib_var[] = G_STRINGIFY(GLIB_VAR);
    assert(0 == strcmp(glib_var, "extern __declspec(dllimport)"));
#endif
#endif
    assert(NULL != g_utf8_skip);

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
