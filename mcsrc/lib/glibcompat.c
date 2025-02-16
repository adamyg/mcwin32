/*
   GLIB - Library of useful routines for C programming

   Copyright (C) 2009-2025
   Free Software Foundation, Inc.

   Written by:
   Slava Zanko <slavazanko@gmail.com>, 2009, 2013.

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

/** \file glibcompat.c
 *  \brief Source: compatibility with older versions of glib
 *
 *  Following code was copied from glib to GNU Midnight Commander to
 *  provide compatibility with older versions of glib.
 */

#include <config.h>
#include <string.h>

#include "global.h"
#include "glibcompat.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

#if ! GLIB_CHECK_VERSION (2, 54, 0)
/**
 * g_ptr_array_find_with_equal_func: (skip)
 * @haystack: pointer array to be searched
 * @needle: pointer to look for
 * @equal_func: (nullable): the function to call for each element, which should
 *    return %TRUE when the desired element is found; or %NULL to use pointer
 *    equality
 * @index_: (optional) (out): return location for the index of
 *    the element, if found
 *
 * Checks whether @needle exists in @haystack, using the given @equal_func.
 * If the element is found, %TRUE is returned and the element^A^A^As index is
 * returned in @index_ (if non-%NULL). Otherwise, %FALSE is returned and @index_
 * is undefined. If @needle exists multiple times in @haystack, the index of
 * the first instance is returned.
 *
 * @equal_func is called with the element from the array as its first parameter,
 * and @needle as its second parameter. If @equal_func is %NULL, pointer
 * equality is used.
 *
 * Returns: %TRUE if @needle is one of the elements of @haystack
 * Since: 2.54
 */
gboolean
g_ptr_array_find_with_equal_func (GPtrArray *haystack, gconstpointer needle, GEqualFunc equal_func,
                                  guint *index_)
{
    guint i;

    g_return_val_if_fail (haystack != NULL, FALSE);

    if (equal_func == NULL)
        equal_func = g_direct_equal;

    for (i = 0; i < haystack->len; i++)
        if (equal_func (g_ptr_array_index (haystack, i), needle))
        {
            if (index_ != NULL)
                *index_ = i;
            return TRUE;
        }

    return FALSE;
}
#endif /* ! GLIB_CHECK_VERSION (2, 54, 0) */

/* --------------------------------------------------------------------------------------------- */

#if ! GLIB_CHECK_VERSION (2, 63, 3)
/**
 * g_clear_slist: (skip)
 * @slist_ptr: (not nullable): a #GSList return location
 * @destroy: (nullable): the function to pass to g_slist_free_full() or NULL to not free elements
 *
 * Clears a pointer to a #GSList, freeing it and, optionally, freeing its elements using @destroy.
 *
 * @slist_ptr must be a valid pointer. If @slist_ptr points to a null #GSList, this does nothing.
 *
 * Since: 2.64
 */
void
g_clear_slist (GSList **slist_ptr, GDestroyNotify destroy)
{
    GSList *slist;

    slist = *slist_ptr;

    if (slist != NULL)
    {
        *slist_ptr = NULL;

        if (destroy != NULL)
            g_slist_free_full (slist, destroy);
        else
            g_slist_free (slist);
    }
}

/* --------------------------------------------------------------------------------------------- */

/**
 * g_clear_list:
 * @list_ptr: (not nullable): a #GList return location
 * @destroy: (nullable): the function to pass to g_list_free_full() or NULL to not free elements
 *
 * Clears a pointer to a #GList, freeing it and, optionally, freeing its elements using @destroy.
 *
 * @list_ptr must be a valid pointer. If @list_ptr points to a null #GList, this does nothing.
 *
 * Since: 2.64
 */
void
g_clear_list (GList **list_ptr, GDestroyNotify destroy)
{
    GList *list;

    list = *list_ptr;

    if (list != NULL)
    {
        *list_ptr = NULL;

        if (destroy != NULL)
            g_list_free_full (list, destroy);
        else
            g_list_free (list);
    }
}

#endif /* ! GLIB_CHECK_VERSION (2, 63, 3) */

/* --------------------------------------------------------------------------------------------- */

#if ! GLIB_CHECK_VERSION (2, 60, 0)
/**
 * g_queue_clear_full:
 * @queue: a pointer to a #GQueue
 * @free_func: (nullable): the function to be called to free memory allocated
 *
 * Convenience method, which frees all the memory used by a #GQueue,
 * and calls the provided @free_func on each item in the #GQueue.
 *
 * Since: 2.60
 */
void
g_queue_clear_full (GQueue *queue, GDestroyNotify free_func)
{
    g_return_if_fail (queue != NULL);

    if (free_func != NULL)
        g_queue_foreach (queue, (GFunc) free_func, NULL);

    g_queue_clear (queue);
}
#endif /* ! GLIB_CHECK_VERSION (2, 60, 0) */

/* --------------------------------------------------------------------------------------------- */

#if ! GLIB_CHECK_VERSION (2, 77, 0)
/**
 * g_string_new_take:
 * @init: (nullable): initial text used as the string.
 *     Ownership of the string is transferred to the #GString.
 *     Passing NULL creates an empty string.
 *
 * Creates a new #GString, initialized with the given string.
 *
 * After this call, @init belongs to the #GString and may no longer be
 * modified by the caller. The memory of @data has to be dynamically
 * allocated and will eventually be freed with g_free().
 *
 * Returns: the new #GString
 */
GString *
g_string_new_take (char *init)
{
    GString *string;

    if (init == NULL)
        return g_string_new (NULL);

    string = g_slice_new (GString);

    string->str = init;
    string->len = strlen (string->str);
    string->allocated_len = string->len + 1;

    return string;
}
#endif /* ! GLIB_CHECK_VERSION (2, 77, 0) */

/* --------------------------------------------------------------------------------------------- */

/**
 * mc_g_string_copy:
 * @dest: (not nullable): the destination #GString. Its current contents are destroyed
 * @src: (not nullable): the source #GString
 * @return: @dest
 *
 * Copies the bytes from a #GString into a #GString, destroying any previous contents.
 * It is rather like the standard strcpy() function, except that you do not have to worry about
 * having enough space to copy the string.
 *
 * There is no such API in GLib2.
 */
GString *
mc_g_string_copy (GString *dest, const GString *src)
{
    g_return_val_if_fail (src != NULL, NULL);
    g_return_val_if_fail (dest != NULL, NULL);

    g_string_set_size (dest, 0);
    g_string_append_len (dest, src->str, src->len);

    return dest;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * mc_g_string_dup:
 * @s: (nullable): the source #GString
 * @return: @copy of @s
 *
 * Copies the bytes from one #GString to another.
 *
 * There is no such API in GLib2.
 */
GString *
mc_g_string_dup (const GString *s)
{
    GString *ret = NULL;

    if (s != NULL)
        ret = g_string_new_len (s->str, s->len);

    return ret;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * mc_g_string_append_c_len:
 * @s: (not nullable): the destination #GString.
 * @c: the byte to append onto the end of @s
 * @len: the number of bytes @c to append onto the end of @s
 * @return: @s
 *
 * Adds @len bytes @c onto the end of @s.
 *
 * There is no such API in GLib2.
 */
GString *
mc_g_string_append_c_len (GString *s, gchar c, guint len)
{
    g_return_val_if_fail (s != NULL, NULL);

    if (len != 0)
    {
        guint s_len = s->len;

        g_string_set_size (s, s->len + len);
        memset (s->str + s_len, (unsigned char) c, len);
    }

    return s;
}

/* --------------------------------------------------------------------------------------------- */
