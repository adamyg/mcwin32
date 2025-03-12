/*
   Virtual File System: interface functions

   Copyright (C) 2011-2025
   Free Software Foundation, Inc.

   Written by:
   Slava Zanko <slavazanko@gmail.com>, 2011, 2013
   Andrew Borodin <aborodin@vmail.ru>, 2011-2022

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

/**
 * \file
 * \brief Source: Virtual File System: path handlers
 * \author Slava Zanko
 * \date 2011
 */


#include <config.h>

#include <stdio.h>
#include <stdlib.h>             /* For atol() */
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>              /* is_digit() */
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#include "lib/global.h"

#include "lib/widget.h"         /* message() */
#include "lib/strutil.h"        /* str_crt_conv_from() */
#include "lib/util.h"

#include "vfs.h"
#include "utilvfs.h"
#include "path.h"
#include "gc.h"
#include "xdirentry.h"

/* TODO: move it to separate private .h */
extern GString *vfs_str_buffer;
extern vfs_class *current_vfs;
extern struct vfs_dirent *mc_readdir_result;

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

#if defined(WIN32) //WIN32, drive
#undef  mkdir
#undef  rmdir
#undef  chdir

#undef  read
#undef  write
#undef  close

#undef  link
#undef  unlink
#undef  rename

#undef  utime
#undef  chmod

#undef  readlink
#undef  symlink
#undef  utime
#undef  write
#undef  stat
#undef  lstat
#undef  fstat

#define OPEN            w32_open
#define CLOSE           w32_close
#define MKDIR           w32_mkdir
#define LINK            w32_link
#define UNLINK          w32_unlink
#define STAT            w32_stat
#define LSTAT           w32_lstat
#define WRITE           w32_write
#define READ            w32_read

#else
#define MKDIR           mkdir
#define LINK            link
#define UNLINK          unlink
#define stat            stat
#define LSTAT           lstat
#define WRITE           write
#define READ            read
#endif

static vfs_path_t *
mc_def_getlocalcopy (const vfs_path_t * filename_vpath)
{
    vfs_path_t *tmp_vpath = NULL;
    int fdin = -1, fdout = -1;
    ssize_t i;
    char buffer[BUF_1K * 8];
    struct stat mystat;

    fdin = mc_open (filename_vpath, O_RDONLY | O_LINEAR);
    if (fdin == -1)
        goto fail;

    fdout = vfs_mkstemps (&tmp_vpath, "vfs", vfs_path_get_last_path_str (filename_vpath));
    if (fdout == -1)
        goto fail;

    while ((i = mc_read (fdin, buffer, sizeof (buffer))) > 0)
    {
        if (WRITE (fdout, buffer, i) != i)
            goto fail;
    }
    if (i == -1)
        goto fail;
    i = mc_close (fdin);
    fdin = -1;
    if (i == -1)
        goto fail;

    i = close (fdout);
    fdout = -1;
    if (i == -1)
        goto fail;

    if (mc_stat (filename_vpath, &mystat) != -1)
        mc_chmod (tmp_vpath, mystat.st_mode);

    return tmp_vpath;

  fail:
    vfs_path_free (tmp_vpath, TRUE);
    if (fdout != -1)
        close (fdout);
    if (fdin != -1)
        mc_close (fdin);
    return NULL;
}

/* --------------------------------------------------------------------------------------------- */

static int
mc_def_ungetlocalcopy (const vfs_path_t * filename_vpath,
                       const vfs_path_t * local_vpath, gboolean has_changed)
{
    int fdin = -1, fdout = -1;
    const char *local;

    local = vfs_path_get_last_path_str (local_vpath);

    if (has_changed)
    {
        char buffer[BUF_1K * 8];
        ssize_t i;

        if (vfs_path_get_last_path_vfs (filename_vpath)->write == NULL)
            goto failed;

        fdin = OPEN (local, O_RDONLY);
        if (fdin == -1)
            goto failed;
        fdout = mc_open (filename_vpath, O_WRONLY | O_TRUNC);
        if (fdout == -1)
            goto failed;
        while ((i = READ (fdin, buffer, sizeof (buffer))) > 0)
            if (mc_write (fdout, buffer, (size_t) i) != i)
                goto failed;
        if (i == -1)
            goto failed;

        if (close (fdin) == -1)
        {
            fdin = -1;
            goto failed;
        }
        fdin = -1;
        if (mc_close (fdout) == -1)
        {
            fdout = -1;
            goto failed;
        }
    }
    UNLINK (local);
    return 0;

  failed:
    message (D_ERROR, _("Changes to file lost"), "%s", vfs_path_get_last_path_str (filename_vpath));
    if (fdout != -1)
        mc_close (fdout);
    if (fdin != -1)
        CLOSE (fdin);
    UNLINK (local);
    return (-1);
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

int
mc_open (const vfs_path_t * vpath, int flags, ...)
{
    int result = -1;
    mode_t mode = 0;
    struct vfs_class *me;

    if (vpath == NULL)
        return (-1);

    /* Get the mode flag */
    if ((flags & O_CREAT) != 0)
    {
        va_list ap;

        va_start (ap, flags);
        /* We have to use PROMOTED_MODE_T instead of mode_t. Doing 'va_arg (ap, mode_t)'
         * fails on systems where 'mode_t' is smaller than 'int' because of C's "default
         * argument promotions". */
        mode = va_arg (ap, PROMOTED_MODE_T);
        va_end (ap);
    }

    me = VFS_CLASS (vfs_path_get_last_path_vfs (vpath));
    if (me != NULL && me->open != NULL)
    {
        void *info;

        /* open must be supported */
        info = me->open (vpath, flags, mode);
        if (info == NULL)
            errno = vfs_ferrno (me);
        else
            result = vfs_new_handle (me, info);
    }
    else
        errno = ENOTSUP;

    return result;
}

/* --------------------------------------------------------------------------------------------- */

/* *INDENT-OFF* */

#define MC_NAMEOP(name, inarg, callarg) \
int mc_##name inarg \
{ \
    int result; \
    struct vfs_class *me; \
\
    if (vpath == NULL) \
        return (-1); \
\
    me = VFS_CLASS (vfs_path_get_last_path_vfs (vpath)); \
    if (me == NULL) \
        return (-1); \
\
    result = me->name != NULL ? me->name callarg : -1; \
    if (result == -1) \
        errno = me->name != NULL ? vfs_ferrno (me) : ENOTSUP; \
    return result; \
}

MC_NAMEOP (chmod, (const vfs_path_t *vpath, mode_t mode), (vpath, mode))
MC_NAMEOP (chown, (const vfs_path_t *vpath, uid_t owner, gid_t group), (vpath, owner, group))
MC_NAMEOP (fgetflags, (const vfs_path_t *vpath, unsigned long *flags), (vpath, flags))
MC_NAMEOP (fsetflags, (const vfs_path_t *vpath, unsigned long flags), (vpath, flags))
MC_NAMEOP (utime, (const vfs_path_t *vpath, mc_timesbuf_t * times), (vpath, times))
MC_NAMEOP (readlink, (const vfs_path_t *vpath, char *buf, size_t bufsiz), (vpath, buf, bufsiz))
MC_NAMEOP (unlink, (const vfs_path_t *vpath), (vpath))
MC_NAMEOP (mkdir, (const vfs_path_t *vpath, mode_t mode), (vpath, mode))
MC_NAMEOP (rmdir, (const vfs_path_t *vpath), (vpath))
MC_NAMEOP (mknod, (const vfs_path_t *vpath, mode_t mode, dev_t dev), (vpath, mode, dev))

/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */

int
mc_symlink (const vfs_path_t * vpath1, const vfs_path_t * vpath2)
{
    int result = -1;

    if (vpath1 != NULL && vpath2 != NULL)
    {
        struct vfs_class *me;

        me = VFS_CLASS (vfs_path_get_last_path_vfs (vpath2));
        if (me != NULL)
        {
            result = me->symlink != NULL ? me->symlink (vpath1, vpath2) : -1;
            if (result == -1)
                errno = me->symlink != NULL ? vfs_ferrno (me) : ENOTSUP;
        }
    }
    return result;
}

/* --------------------------------------------------------------------------------------------- */

/* *INDENT-OFF* */

#define MC_HANDLEOP(rettype, name, inarg, callarg) \
rettype mc_##name inarg \
{ \
    struct vfs_class *vfs; \
    void *fsinfo = NULL; \
    rettype result; \
\
    if (handle == -1) \
        return (-1); \
\
    vfs = vfs_class_find_by_handle (handle, &fsinfo); \
    if (vfs == NULL) \
        return (-1); \
\
    result = vfs->name != NULL ? vfs->name callarg : -1; \
    if (result == -1) \
        errno = vfs->name != NULL ? vfs_ferrno (vfs) : ENOTSUP; \
    return result; \
}

MC_HANDLEOP (ssize_t, read, (int handle, void *buf, size_t count), (fsinfo, buf, count))
MC_HANDLEOP (ssize_t, write, (int handle, const void *buf, size_t count), (fsinfo, buf, count))
MC_HANDLEOP (int, fstat, (int handle, struct stat *buf), (fsinfo, buf))

/* --------------------------------------------------------------------------------------------- */

#define MC_RENAMEOP(name) \
int mc_##name (const vfs_path_t *vpath1, const vfs_path_t *vpath2) \
{ \
    int result; \
    struct vfs_class *me1, *me2; \
\
    if (vpath1 == NULL || vpath2 == NULL) \
        return (-1); \
\
    me1 = VFS_CLASS (vfs_path_get_last_path_vfs (vpath1)); \
    me2 = VFS_CLASS (vfs_path_get_last_path_vfs (vpath2)); \
\
    if (me1 == NULL || me2 == NULL || me1 != me2) \
    { \
        errno = EXDEV; \
        return (-1); \
    } \
\
    result = me1->name != NULL ? me1->name (vpath1, vpath2) : -1; \
    if (result == -1) \
        errno = me1->name != NULL ? vfs_ferrno (me1) : ENOTSUP; \
    return result; \
}

MC_RENAMEOP (link)
MC_RENAMEOP (rename)

/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */

int
mc_ctl (int handle, int ctlop, void *arg)
{
    struct vfs_class *vfs;
    void *fsinfo = NULL;

    vfs = vfs_class_find_by_handle (handle, &fsinfo);

    return (vfs == NULL || vfs->ctl == NULL) ? 0 : vfs->ctl (fsinfo, ctlop, arg);
}

/* --------------------------------------------------------------------------------------------- */

int
mc_setctl (const vfs_path_t * vpath, int ctlop, void *arg)
{
    int result = -1;
    struct vfs_class *me;

    if (vpath == NULL)
        vfs_die ("You don't want to pass NULL to mc_setctl.");

    me = VFS_CLASS (vfs_path_get_last_path_vfs (vpath));
    if (me != NULL)
        result = me->setctl != NULL ? me->setctl (vpath, ctlop, arg) : 0;

    return result;
}

/* --------------------------------------------------------------------------------------------- */

int
mc_close (int handle)
{
    struct vfs_class *vfs;
    void *fsinfo = NULL;
    int result;

    if (handle == -1)
        return (-1);

    vfs = vfs_class_find_by_handle (handle, &fsinfo);
    if (vfs == NULL || fsinfo == NULL)
        return (-1);

    if (handle < 3)
        return close (handle);

    if (vfs->close == NULL)
        vfs_die ("VFS must support close.\n");
    result = vfs->close (fsinfo);
    vfs_free_handle (handle);
    if (result == -1)
        errno = vfs_ferrno (vfs);

    return result;
}

/* --------------------------------------------------------------------------------------------- */

DIR *
mc_opendir (const vfs_path_t * vpath)
{
    int handle, *handlep;
    void *info;
    vfs_path_element_t *path_element;

    if (vpath == NULL)
        return NULL;

    path_element = (vfs_path_element_t *) vfs_path_get_by_index (vpath, -1);
    if (!vfs_path_element_valid (path_element))
    {
        errno = ENOTSUP;
        return NULL;
    }

    info = path_element->class->opendir ? path_element->class->opendir (vpath) : NULL;
    if (info == NULL)
    {
        errno = path_element->class->opendir ? vfs_ferrno (path_element->class) : ENOTSUP;
        return NULL;
    }

    path_element->dir.info = info;

#ifdef HAVE_CHARSET
    path_element->dir.converter = (path_element->encoding != NULL) ?
        str_crt_conv_from (path_element->encoding) : str_cnv_from_term;
    if (path_element->dir.converter == INVALID_CONV)
        path_element->dir.converter = str_cnv_from_term;
#endif

    handle = vfs_new_handle (path_element->class, vfs_path_element_clone (path_element));

    handlep = g_new (int, 1);
    *handlep = handle;
    return (DIR *) handlep;
}

/* --------------------------------------------------------------------------------------------- */

struct vfs_dirent *
mc_readdir (DIR * dirp)
{
    int handle;
    struct vfs_class *vfs;
    void *fsinfo = NULL;
    struct vfs_dirent *entry = NULL;
    vfs_path_element_t *vfs_path_element;

    if (dirp == NULL)
    {
        errno = EFAULT;
        return NULL;
    }

    handle = *(int *) dirp;

    vfs = vfs_class_find_by_handle (handle, &fsinfo);
    if (vfs == NULL || fsinfo == NULL)
        return NULL;

    vfs_path_element = (vfs_path_element_t *) fsinfo;
    if (vfs->readdir != NULL)
    {
        entry = vfs->readdir (vfs_path_element->dir.info);
        if (entry == NULL)
            return NULL;

        g_string_set_size (vfs_str_buffer, 0);
#ifdef HAVE_CHARSET
        str_vfs_convert_from (vfs_path_element->dir.converter, entry->d_name, vfs_str_buffer);
#else
        g_string_assign (vfs_str_buffer, entry->d_name);
#endif
        vfs_dirent_assign (mc_readdir_result, vfs_str_buffer->str, entry->d_ino);
        vfs_dirent_free (entry);
    }
    if (entry == NULL)
        errno = vfs->readdir ? vfs_ferrno (vfs) : ENOTSUP;
    return (entry != NULL) ? mc_readdir_result : NULL;
}

/* --------------------------------------------------------------------------------------------- */

int
mc_closedir (DIR * dirp)
{
    int handle;
    struct vfs_class *vfs;
    void *fsinfo = NULL;
    int result = -1;

    if (dirp == NULL)
        return result;

    handle = *(int *) dirp;

    vfs = vfs_class_find_by_handle (handle, &fsinfo);
    if (vfs != NULL && fsinfo != NULL)
    {
        vfs_path_element_t *vfs_path_element = (vfs_path_element_t *) fsinfo;

#ifdef HAVE_CHARSET
        if (vfs_path_element->dir.converter != str_cnv_from_term)
        {
            str_close_conv (vfs_path_element->dir.converter);
            vfs_path_element->dir.converter = INVALID_CONV;
        }
#endif

        result = vfs->closedir ? (*vfs->closedir) (vfs_path_element->dir.info) : -1;
        vfs_free_handle (handle);
        vfs_path_element_free (vfs_path_element);
    }
    g_free (dirp);
    return result;
}

/* --------------------------------------------------------------------------------------------- */

/* *INDENT-OFF* */

#define MC_STATOP(name) \
int mc_##name (const vfs_path_t *vpath, struct stat *buf) \
{ \
    int result = -1; \
    struct vfs_class *me; \
\
    if (vpath == NULL) \
        return (-1); \
\
    me = VFS_CLASS (vfs_path_get_last_path_vfs (vpath)); \
    if (me != NULL) \
    { \
        result = me->name ? me->name (vpath, buf) : -1; \
        if (result == -1) \
            errno = me->name ? vfs_ferrno (me) : ENOTSUP; \
    } \
\
    return result; \
}

MC_STATOP (stat)
MC_STATOP (lstat)

/* --------------------------------------------------------------------------------------------- */

vfs_path_t *
mc_getlocalcopy (const vfs_path_t * pathname_vpath)
{
    vfs_path_t *result = NULL;
    struct vfs_class *me;

    if (pathname_vpath == NULL)
        return NULL;

    me = VFS_CLASS (vfs_path_get_last_path_vfs (pathname_vpath));
    if (me != NULL)
    {
        result = me->getlocalcopy != NULL ?
            me->getlocalcopy (pathname_vpath) : mc_def_getlocalcopy (pathname_vpath);
        if (result == NULL)
            errno = vfs_ferrno (me);
    }
    return result;
}

/* --------------------------------------------------------------------------------------------- */

int
mc_ungetlocalcopy (const vfs_path_t * pathname_vpath, const vfs_path_t * local_vpath,
                   gboolean has_changed)
{
    int result = -1;
    const struct vfs_class *me;

    if (pathname_vpath == NULL)
        return (-1);

    me = vfs_path_get_last_path_vfs (pathname_vpath);
    if (me != NULL)
        result = me->ungetlocalcopy != NULL ?
            me->ungetlocalcopy (pathname_vpath, local_vpath, has_changed) :
            mc_def_ungetlocalcopy (pathname_vpath, local_vpath, has_changed);

    return result;
}

/* --------------------------------------------------------------------------------------------- */
/**
 * VFS chdir.
 *
 * @param vpath VFS path.
 *              May be NULL. In this case NULL is returned and errno set to 0.
 *
 * @return 0 on success, -1 on failure.
 */

int
mc_chdir (const vfs_path_t * vpath)
{
    struct vfs_class *old_vfs;
    vfsid old_vfsid;
    int result;
    struct vfs_class *me;
    const vfs_path_element_t *path_element;
    vfs_path_t *cd_vpath;

    if (vpath == NULL)
    {
        errno = 0;
        return (-1);
    }

    if (vpath->relative)
        cd_vpath = vfs_path_to_absolute (vpath);
    else
        cd_vpath = vfs_path_clone (vpath);

    me = VFS_CLASS (vfs_path_get_last_path_vfs (cd_vpath));
    if (me == NULL)
    {
        errno = EINVAL;
        goto error_end;
    }

    if (me->chdir == NULL)
    {
        errno = ENOTSUP;
        goto error_end;
    }

    result = me->chdir (cd_vpath);
    if (result == -1)
    {
        errno = vfs_ferrno (me);
        goto error_end;
    }

    old_vfsid = vfs_getid (vfs_get_raw_current_dir ());
    old_vfs = current_vfs;

    /* Actually change directory */
    vfs_set_raw_current_dir (cd_vpath);
    current_vfs = me;

    /* This function uses the new current_dir implicitly */
    vfs_stamp_create (old_vfs, old_vfsid);

    /* Sometimes we assume no trailing slash on cwd */
    path_element = vfs_path_get_by_index (vfs_get_raw_current_dir (), -1);
    if (vfs_path_element_valid (path_element))
    {
        if (*path_element->path != '\0')
        {
            char *p;

            p = strchr (path_element->path, 0) - 1;
            if (IS_PATH_SEP (*p) && p > path_element->path)
                *p = '\0';
        }

#ifdef ENABLE_VFS_NET
        {
            struct vfs_s_super *super;

            super = vfs_get_super_by_vpath (vpath);
            if (super != NULL && super->path_element != NULL)
            {
                g_free (super->path_element->path);
                super->path_element->path = g_strdup (path_element->path);
            }
        }
#endif /* ENABLE_VFS_NET */
    }

    return 0;

  error_end:
    vfs_path_free (cd_vpath, TRUE);
    return (-1);
}

/* --------------------------------------------------------------------------------------------- */

off_t
mc_lseek (int fd, off_t offset, int whence)
{
    struct vfs_class *vfs;
    void *fsinfo = NULL;
    off_t result;

    if (fd == -1)
        return (-1);

    vfs = vfs_class_find_by_handle (fd, &fsinfo);
    if (vfs == NULL)
        return (-1);

    result = vfs->lseek ? vfs->lseek (fsinfo, offset, whence) : -1;
    if (result == -1)
        errno = vfs->lseek ? vfs_ferrno (vfs) : ENOTSUP;
    return result;
}

/* --------------------------------------------------------------------------------------------- */
/* Following code heavily borrows from libiberty, mkstemps.c */
/*
 * Arguments:
 * pname (output) - pointer to the name of the temp file (needs g_free).
 *                  NULL if the function fails.
 * prefix - part of the filename before the random part.
 *          Prepend $TMPDIR or /tmp if there are no path separators.
 * suffix - if not NULL, part of the filename after the random part.
 *
 * Result:
 * handle of the open file or -1 if couldn't open any.
 */

int
mc_mkstemps (vfs_path_t ** pname_vpath, const char *prefix, const char *suffix)
{
    char *p1, *p2;
    int fd;

    if (strchr (prefix, PATH_SEP) != NULL)
        p1 = g_strdup (prefix);
    else
    {
        /* Add prefix first to find the position of XXXXXX */
        p1 = g_build_filename (mc_tmpdir (), prefix, (char *) NULL);
    }

    p2 = g_strconcat (p1, "XXXXXX", suffix, (char *) NULL);
    g_free (p1);

    fd = g_mkstemp (p2);
    if (fd >= 0)
        *pname_vpath = vfs_path_from_str (p2);
    else
    {
        *pname_vpath = NULL;
        fd = -1;
    }

    g_free (p2);

    return fd;
}

/* --------------------------------------------------------------------------------------------- */
/**
 * Return the directory where mc should keep its temporary files.
 * This directory is (in Bourne shell terms) "${TMPDIR=/tmp}/mc-$USER"
 * When called the first time, the directory is created if needed.
 * The first call should be done early, since we are using fprintf()
 * and not message() to report possible problems.
 */

const char *
mc_tmpdir (void)
{
    static char buffer[PATH_MAX];
    static const char *tmpdir = NULL;
    const char *sys_tmp;
    struct passwd *pwd;
    struct stat st;
    const char *error = NULL;

    /* Check if already correctly initialized */
    if (tmpdir != NULL && LSTAT (tmpdir, &st) == 0 && S_ISDIR (st.st_mode) &&
        st.st_uid == getuid () && (st.st_mode & 0777) == 0700)
        return tmpdir;

#if defined(WIN32) //WIN32, config
    if (NULL == (sys_tmp = mc_TMPDIR())) {
        sys_tmp = TMPDIR_DEFAULT;
    }
#else
    sys_tmp = getenv ("MC_TMPDIR");
    if (sys_tmp == NULL || !IS_PATH_SEP (sys_tmp[0]))
    {
        sys_tmp = getenv ("TMPDIR");
        if (sys_tmp == NULL || !IS_PATH_SEP (sys_tmp[0]))
            sys_tmp = TMPDIR_DEFAULT;
    }
#endif

    pwd = getpwuid (getuid ());
    if (pwd != NULL)
        g_snprintf (buffer, sizeof (buffer), "%s/mc-%s", sys_tmp, pwd->pw_name);
    else
        g_snprintf (buffer, sizeof (buffer), "%s/mc-%lu", sys_tmp, (unsigned long) getuid ());

    canonicalize_pathname (buffer);

    /* Try to create directory */
#if defined(WIN32)
    if (MKDIR (buffer, S_IRWXU) != 0)
    {
        if (errno != EEXIST || STAT (buffer, &st) != 0)
        {
            fprintf (stderr,
                     _("Cannot create temporary directory %s: %s\n"),
                     buffer, unix_error_string (errno));
            error = "";
        }
    }
#else
    if (mkdir (buffer, S_IRWXU) != 0)
    {
        if (errno == EEXIST && lstat (buffer, &st) == 0)
        {
            /* Sanity check for existing directory */
            if (!S_ISDIR (st.st_mode))
                error = _("%s is not a directory\n");
            else if (st.st_uid != getuid ())
                error = _("Directory %s is not owned by you\n");
            else if (((st.st_mode & 0777) != 0700) && (chmod (buffer, 0700) != 0))
                error = _("Cannot set correct permissions for directory %s\n");
        }
        else
        {
            fprintf (stderr,
                     _("Cannot create temporary directory %s: %s\n"),
                     buffer, unix_error_string (errno));
            error = "";
        }
    }
#endif

    if (error != NULL)
    {
        int test_fd;
        char *fallback_prefix;
        gboolean fallback_ok = FALSE;
        vfs_path_t *test_vpath;

        if (*error != '\0')
            fprintf (stderr, error, buffer);

        /* Test if sys_tmp is suitable for temporary files */
        fallback_prefix = g_strdup_printf ("%s/mctest", sys_tmp);
        test_fd = mc_mkstemps (&test_vpath, fallback_prefix, NULL);
        g_free (fallback_prefix);
        if (test_fd != -1)
        {
            close (test_fd);
            test_fd = OPEN (vfs_path_as_str (test_vpath), O_RDONLY);
            if (test_fd != -1)
            {
                close (test_fd);
                UNLINK (vfs_path_as_str (test_vpath));
                fallback_ok = TRUE;
            }
        }

        if (fallback_ok)
        {
            fprintf (stderr, _("Temporary files will be created in %s\n"), sys_tmp);
            g_snprintf (buffer, sizeof (buffer), "%s", sys_tmp);
            error = NULL;
        }
        else
        {
            fprintf (stderr, _("Temporary files will not be created\n"));
            g_snprintf (buffer, sizeof (buffer), "%s", "/dev/null/");
        }

        vfs_path_free (test_vpath, TRUE);
        fprintf (stderr, "%s\n", _("Press any key to continue..."));
        getc (stdin);
    }

    tmpdir = buffer;

    if (error == NULL)
        g_setenv ("MC_TMPDIR", tmpdir, TRUE);

    return tmpdir;
}

/* --------------------------------------------------------------------------------------------- */
