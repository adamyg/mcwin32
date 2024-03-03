/* -*- mode: c; indent-width: 4; -*- */
/*
   WIN32 util implementation

        #include "../lib/utilunix.h"

            get_group
            get_user_permissions
            save_stop_handler
            my_systemv_flags
            my_system
            tilde_expand
            open_error_pipe
            close_error_pipe
            custom_canonicalize_pathname
            canonicalize_pathname
            mc_realpath
            my_build_filenamev
            my_build_filename

   Copyright (C) 2012
   The Free Software Foundation, Inc.

   Written by: Adam Young 2012 - 2024

   Portions sourced from lib/utilunix.c, see for additional information.

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

#if defined(_WIN32_WINNT) && (_WIN32_WINNT < 0x0600)
#undef  _WIN32_WINNT
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include "libw32.h"

#include <shlobj.h>                             /* SHxx */

#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>                             /* struct sigaction */
#include <limits.h>                             /* INT_MAX */
#include <malloc.h>

#include <sys/stat.h>
#include <stdarg.h>
#include <errno.h>                              /* errno */
#include <string.h>
#include <ctype.h>

#include <pwd.h>
#include <grp.h>

#include "lib/global.h"
#include "lib/vfs/vfs.h"                        /* VFS_ENCODING_PREFIX */
#include "lib/strutil.h"                        /* str_move() */
#include "lib/util.h"
#include "lib/widget.h"                         /* message() */
#include "lib/vfs/xdirentry.h"
#ifdef HAVE_CHARSET
#include "lib/charsets.h"
#endif
#include "lib/utilunix.h"

#include "src/setup.h"                          /* use_internal_busybox */

#include "win32_utl.h"
#include "win32_key.h"
#include "win32_trace.h"
#include "win32_internal.h"

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shfolder.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")

static void             set_shell (void);
static void             set_term (void);
static void             set_home (void);
static void             set_tmpdir (void);
static void             set_editor (void);
static void             set_busybox (void);

static void             my_setenv (const char *name, const char *value, int overwrite);
static void             my_setpathenv (const char *name, const char *value, int overwrite, int quote_ws);

static void             unixpath (char *path);
static void             dospath (char *path);

static int              system_impl (int flags, const char *shell, const char *cmd);
static int              system_bustargs (char *cmd, const char **argv, int cnt);
static int              system_SET (int argc, const char **argv);

static DWORD WINAPI     pipe_thread (void *data);

#include "busyboxcmds.h"                       /* busyboxcmds */

static const char *     busyboxexts[] = {      /* redirected commands (see vfs/sfs module) */
        "ar", "ash", "awk", "base32", "base64", "bash", "bunzip2", "bzcat", "bzip2", "cat", "cksum", "cpio", "dd", "diff",
        "dos2unix", "echo", "ed", "gunzip", "gzip", "ls",
        "lzcat", "lzma", "lzop", "lzopcat", "ps", "sed", "sh",
        "strings", "tar", "uncompress", "unexpand", "unix2dos", "unlzma", "unlzop",
        "unxz", "unzip", "uudecode", "uuencode", "xz", "xzcat", "zcat"
        };

static const char *     busybox_path = NULL;    /* resolve path to busybox */

static const char       bin_sh[] = "/bin/sh";
static const char       cmd_sh[] = "cmd.exe";

#define PE_BUFFER_SIZE          4096            /* pipe limit, plus terminator */

static CRITICAL_SECTION pe_guard;
static int              pe_open = -1;
static FILE *           pe_stream = NULL;
static char             pe_buffer[ PE_BUFFER_SIZE + 1024 + 1 ];


/**
 *  WIN32 initialisation ... see main.c
 */
void
WIN32_Setup(void)
{
    static int init = 0;

    w32_utf8filenames_enable();
    if (init) return;
    ++init;

#if defined(_MSC_VER) && (_MSC_VER >= 1900)
    _set_fmode(_O_BINARY);
#else
    _fmode = _O_BINARY;                         /* force binary mode */
#endif

    InitializeCriticalSection(&pe_guard);
#if defined(ENABLE_VFS)
        {   WSADATA wsaData = {0};
            WORD wVersionRequested = MAKEWORD(2,2); /* winsock2 */
            if (WSAStartup(wVersionRequested, &wsaData) != 0) {
                const DWORD err = GetLastError();
                char buffer[128];

                (void) _snprintf(buffer, sizeof(buffer), "WSAStartup failed, rc: %ld", (long)err);
                buffer[ sizeof(buffer) - 1] = 0;
                MessageBoxA(0, buffer, "Error", MB_OK);
            }
        }
#endif //ENABLE_VFS
    set_shell();
    set_term();
    set_home();
    set_editor();
    set_busybox();
    set_tmpdir();
}


static void
set_shell(void)
{
    my_setpathenv("SHELL", w32_getshell(), FALSE, FALSE);
}


static void
set_term(void)
{
    my_setenv("TERM", "dos-console", FALSE);
}


static void
set_home(void)
{
//  extern const char *g_get_user_config_dir(void);
//  const char *cp;
//
//  if (NULL != (cp = getenv("MC_HOME")) && *cp) {
//      my_setpathenv("MC_HOME", g_get_user_config_dir(), FALSE);
//  }
}


static void
set_editor(void)
{
    my_setenv("EDITOR", "notepad.exe", FALSE);
    my_setenv("PAGER", "notepad.exe", FALSE);
}


/**
 *  MC_BUSYBOX setup
 *
 *      <exepath>\busybox.exe
 *      <exepath>\..\share\
 */
static void
set_busybox(void)
{
    const char *busybox = NULL;
    char buffer[MAX_PATH] = {0};

    if (NULL != (busybox = getenv("MC_BUSYBOX")) && *busybox) {
        char *t_busybox = strdup(busybox);  // import external version.
        if ('"' == t_busybox[0]) { // remove quotes; optional.
            size_t len = strlen(t_busybox);
            if ('"' == t_busybox[len-1]) {
                memmove(t_busybox, t_busybox + 1, --len);
                t_busybox[len - 1] = 0;
            }
        }
        busybox_path = t_busybox;
        return;
    }

    busybox = "busybox";
    if (w32_getexedir(buffer, sizeof(buffer) - 16) > 0) {
        strcat(buffer, "/busybox.exe");
        buffer[sizeof(buffer) - 1] = 0;
        canonicalize_pathname (buffer);
        if (0 == _access(buffer, X_OK)) {
            dospath(buffer);
            busybox = buffer;
        }
    }

    /* publish, quote if path contains whitespace */
    my_setpathenv("MC_BUSYBOX", busybox, TRUE, TRUE);
    busybox_path = strdup(busybox);
}


const char *
mc_BUSYBOX(void)
{
    return busybox_path;
}


/**
 *  MC_TMPDIR
 *
 *      <TMP>, <TEMP>, <TMPDIR>, <sysdir>, <userprofile>
 */
static void
set_tmpdir(void)
{
    const char *tmpdir = NULL;

    if (NULL != (tmpdir = getenv("MC_TMPDIR")) && *tmpdir)  {
        return;
    }

    tmpdir = mc_TMPDIR();
    if (tmpdir && *tmpdir) {
        char buffer[MAX_PATH] = {0};
        struct passwd *pwd;
        struct stat st = {0};

        pwd = getpwuid (getuid ());             /* check permissions */
        snprintf (buffer, sizeof (buffer), "%s%cmc-%s", tmpdir, PATH_SEP, pwd->pw_name);
        buffer[sizeof(buffer) - 1] = 0;
        canonicalize_pathname (buffer);
        if (0 == w32_lstat(buffer, &st)) {
            if (! S_ISDIR(st.st_mode)) {
                tmpdir = NULL;
            }
        } else if (0 != w32_mkdir (buffer, S_IRWXU)) {
            tmpdir = NULL;
        }

        if (tmpdir) {
            my_setpathenv("MC_TMPDIR", tmpdir, TRUE, FALSE);
        }
    }
}


const char *
mc_TMPDIR(void)
{
    static char x_buffer[MAX_PATH];

    if (0 == x_buffer[0]) {
        char sysdir[MAX_PATH] = {0};
        const char *tmpdir;

        if (NULL != (tmpdir = getenv("MC_TMPDIR"))) { // 4.8.27
            if (!*tmpdir || // verify either "/" or "X:/"
                    !(IS_PATH_SEP(tmpdir[0]) || (':' == tmpdir[1] && !IS_PATH_SEP(tmpdir[2])))) {
                tmpdir = NULL;
            }
        }
        if (!tmpdir) tmpdir = getenv("TMP");    /* determine the temp directory */
        if (!tmpdir) tmpdir = getenv("TEMP");
        if (!tmpdir) tmpdir = getenv("TMPDIR");
        if (!tmpdir) {
            if (w32_getsysdir (SYSDIR_TEMP, sysdir, sizeof(sysdir)) > 0) {
                tmpdir = sysdir;
            }
        }
        if (!tmpdir) tmpdir = getenv("USERPROFILE");
        if (!tmpdir) tmpdir = TMPDIR_DEFAULT;

        strncpy(x_buffer, tmpdir, sizeof(x_buffer));
        x_buffer[sizeof(x_buffer) - 1] = 0;
        unixpath(x_buffer);
    }

    return (x_buffer[0] ? x_buffer : NULL);
}


/**
 *  Retrieve aspell DLL directory, if available.
 *
 *      <EXEPATH>
 *
 *      <Software\Aspell\bin>
 */
const char *
mc_aspell_dllpath(void)
{
    static char x_buffer[MAX_PATH] = {0};
    HKEY hKey = 0;
    int len;

    if (x_buffer[0]) {
        return x_buffer;
    }

    // <EXEPATH>
    if ((len = w32_getexedir(x_buffer, sizeof(x_buffer))) > 0) {
        _snprintf(x_buffer + len, sizeof(x_buffer) - len, "\\%s.dll", ASPELL_DLLNAME);
        x_buffer[sizeof(x_buffer) - 1] = 0;
        if (0 == _access(x_buffer, 0)) {
            x_buffer[len] = 0; //exclude delimiter
            return x_buffer;
        }
    }

    // <Software\Aspell\bin>
    if (ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Aspell", 0, KEY_READ, &hKey)) {
        DWORD dwSize = sizeof(x_buffer) - (sizeof(ASPELL_DLLNAME) + 8);

        if (ERROR_SUCCESS == RegQueryValueExA(hKey, "Path", NULL, NULL, (LPBYTE)x_buffer, &dwSize) && dwSize) {
            if (0 == x_buffer[dwSize]) --dwSize;
            _snprintf(x_buffer + dwSize, sizeof(x_buffer) - dwSize, "\\%s.dll", ASPELL_DLLNAME);
            x_buffer[sizeof(x_buffer) - 1] = 0;
            if (0 == _access(x_buffer, 0)) {
                x_buffer[dwSize] = 0; //exclude delimiter
                return x_buffer;
            }
        }
        RegCloseKey(hKey);
    }

    return NULL;
}


/**
 *  Retrieve current locale.
 */
const char *
mc_get_locale(void)
{
    static char x_lang[64] = {0};

    if (0 == x_lang[0])
    {
        char iso639[16] = {0}, iso3166[16] = {0};
        LCID lcid = GetThreadLocale();

        if (GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME, iso639, sizeof(iso639)) &&
                GetLocaleInfoA(lcid, LOCALE_SISO3166CTRYNAME, iso3166, sizeof(iso3166))) {
            snprintf(x_lang, sizeof(x_lang), "%s_%s", iso639, iso3166); // "9_9"
            x_lang[sizeof(x_lang) - 1] = '\0';
        }
    }
    return x_lang[0] ? x_lang : NULL;
}


/**
 *  Retrieve file-name of magic database.
 *
 *      <SYSCONFDIR>\magic..
 */
const char *
mc_MAGICPATH(void)
{
    static char x_buffer[MAX_PATH];

    if (0 == x_buffer[0]) {
        _snprintf(x_buffer, sizeof(x_buffer), "%s/magic", mc_SYSCONFDIR());
        x_buffer[sizeof(x_buffer) - 1] = 0;
        unixpath(x_buffer);
    }
    return x_buffer;
}


/**
 *  Retrieve global system configuration path, equivalent to '/etc/mc'.
 *
 *      <EXEPATH>
 *          <exepath>\<subdir>\
 *
 *      <INSTALLPATH>
 *          X:\Program Files\<Midnight Commander>\<subdir>\
 *
 *              SHGetFolderPath(CSIDL_PROGRAM_FILES)
 *              or getenv(ProgramFiles)
 *
 *      <APPDATA>
 *          X:\Documents and Settings\All Users\Application Data\<Midnight Commander>\etc\
 *
 *              SHGetFolderPath(CSIDL_COMMON_APPDATA)
 *              or getenv(ALLUSERSPROFILE)
 */
static const char *
get_conf_dir(const char *subdir, char *buffer, size_t buflen)
{
    int len, done = FALSE;

    if (buffer[0]) {
        return buffer;
    }

    // <EXEPATH>, generally same as INSTALLDIR
    if ((len = w32_getexedir(buffer, buflen)) > 0) {
        _snprintf(buffer + len, buflen - len, "/%s/", subdir);
        buffer[buflen - 1] = 0;
        if (0 == _access(buffer, 0)) {
            done = TRUE;
        }
    }

    // <INSTALLPATH>
    if (! done) {
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES, NULL, 0, buffer))) {
            len = strlen(buffer);
            _snprintf(buffer + len, buflen - len, "/%s/%s/", MC_APPLICATION_DIR, subdir);
            buffer[buflen - 1] = 0;
            if (0 == _access(buffer, 0)) {
                done = TRUE;
            }
        }
    }

    // <APPDATA>
    if (! done)  {
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, buffer))) {
            len = strlen(buffer);
            _snprintf(buffer + len, buflen - len, "/%s/%s/", MC_APPLICATION_DIR, subdir);
            buffer[buflen - 1] = 0;
            if (0 == _access(buffer, 0)) {
                done = TRUE;
            }
        }
    }

    // default - INSTALLPATH
    if (! done) {
        const char *env;

        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES, NULL, 0, buffer))) {
            len = strlen(buffer);
            _snprintf(buffer + len, buflen - len, "/%s/%s/", MC_APPLICATION_DIR, subdir);

        } else if (NULL != (env = getenv("ProgramFiles"))) {
            _snprintf(buffer, buflen, "%s/%s/%s/", env, MC_APPLICATION_DIR, subdir);

        } else {
            _snprintf(buffer, buflen, "c:/Program Files/%s/%s/", MC_APPLICATION_DIR, subdir);
        }
        buffer[buflen - 1] = 0;
        mkdir(buffer, S_IRWXU);
    }

    unixpath(buffer);
    return buffer;
}


/**
 *  Retrieve global system configuration path, equivalent to '/etc/mc'.
 *
 *      <EXEPATH>
 *          <exepath>\etc\
 *
 *      <INSTALLPATH>
 *          X:\Program Files\<Midnight Commander>\etc
 *
 *              SHGetFolderPath(CSIDL_PROGRAM_FILES)
 *              or getenv(ProgramFiles)
 *
 *      <APPDATA>
 *          X:\Documents and Settings\All Users\Application Data\<Midnight Commander>\etc\
 *
 *              SHGetFolderPath(CSIDL_COMMON_APPDATA)
 *              or getenv(ALLUSERSPROFILE)
 */
const char *
mc_SYSCONFDIR(void)
{
    static char x_buffer[MAX_PATH];
    return get_conf_dir("etc", x_buffer, sizeof(x_buffer));
}


/**
 *  Retrieve global/share configuration path, equivalent to '/usr/share/mc'.
 *
 *      <EXEPATH>
 *          <exepath>\share\
 *
 *      <INSTALLPATH>
 *          X:\Program Files\<Midnight Commander>\share
 *
 *              SHGetFolderPath(CSIDL_PROGRAM_FILES)
 *              or getenv(ProgramFiles)
 *
 *      <APPDATA>
 *          X:\Documents and Settings\All Users\Application Data\<Midnight Commander>\share\
 *
 *              SHGetFolderPath(CSIDL_COMMON_APPDATA)
 *              or getenv(ALLUSERSPROFILE)
 */
const char *
mc_DATADIR(void)
{
    static char x_buffer[MAX_PATH];
    return get_conf_dir("share", x_buffer, sizeof(x_buffer));
}


/**
 *  Retrieve locale configuration path, equivalent to '/usr/share/locale'.
 *
 *      <EXEPATH>
 *          <exepath>\locale\
 *
 *      <INSTALLPATH>
 *          X:\Program Files\<Midnight Commander>\locale
 *
 *              SHGetFolderPath(CSIDL_PROGRAM_FILES)
 *              or getenv(ProgramFiles)
 *
 *      <APPDATA>
 *          X:\Documents and Settings\All Users\Application Data\<Midnight Commander>\locale\
 *
 *              SHGetFolderPath(CSIDL_COMMON_APPDATA)
 *              or getenv(ALLUSERSPROFILE)
 */
const char *
mc_LOCALEDIR(void)
{
    static char x_buffer[MAX_PATH];
    return get_conf_dir("locale", x_buffer, sizeof(x_buffer));
}


/**
 *  Retrieve global/share plugin configuration path, equivalent to '/lib/mc'.
 *
 *      <EXEPATH>
 *          <exepath>\etc\
 *
 *      <INSTALLPATH>
 *          X:\Program Files\<Midnight Commander>\etc
 *
 *              SHGetFolderPath(CSIDL_PROGRAM_FILES)
 *              or getenv(ProgramFiles)
 *
 *      <APPDATA>
 *          X:\Documents and Settings\All Users\Application Data\<Midnight Commander>\etc\
 *
 *              SHGetFolderPath(CSIDL_COMMON_APPDATA)
 *              or getenv(ALLUSERSPROFILE)
 */
const char *
mc_LIBEXECDIR(void)
{
    static char x_buffer[MAX_PATH];
    return get_conf_dir("plugin", x_buffer, sizeof(x_buffer));
}


/**
 *  Retrieve global/share plugin configuration path, equivalent to '/lib/mc'.
 *
 *      <EXEPATH>
 *          <exepath>\etc\
 *
 *      <INSTALLPATH>
 *          X:\Program Files\<Midnight Commander>\etc
 *
 *              SHGetFolderPath(CSIDL_PROGRAM_FILES)
 *              or getenv(ProgramFiles)
 *
 *      <APPDATA>
 *          X:\Documents and Settings\All Users\Application Data\<Midnight Commander>\etc\
 *
 *              SHGetFolderPath(CSIDL_COMMON_APPDATA)
 *              or getenv(ALLUSERSPROFILE)
 */
const char *
mc_EXTHELPERSDIR(void)
{
    return mc_LIBEXECDIR();                     // one and the same ....
}


/**
 *   Retrieve the user specific configuration path.
 *
 *      <XDG_CONFIG_HOME>
 *          If $XDG_CONFIG_HOME is either not set or empty, a default equal to $HOME/.config should be used.
 *          XDG Base Directory Specification, glib compatiblity ??
 *
 *      <SYSCONFDIR>
 *          x:\Documents and Settings\<user>\Application Data\<Midnight Commander>\
 *
 *              SHGetFolderPath(CSIDL_APPDATA)
 *              APPDATA
 *
 *      HOME
 *          x:\<home>\<Midnight Commander>\
 *
 *      CWD
 *          <cwd><Midnight Commander>\
 */
const char *
mc_USERCONFIGDIR(const char *subdir)
{
    static char x_buffer[MAX_PATH];

    if (0 == x_buffer[0]) {
        const char *env;
        int len, done = FALSE;

#if defined(MC_HOMEDIR_XDG)
        /* <XDG_CONFIG_HOME>/mc */
        if (NULL != (env = getenv("XDG_CONFIG_HOME")) && *env) {
            if (0 == _access(env, 0)) {
                _snprintf(x_buffer, sizeof(x_buffer), "%s/mc", env);
                mkdir(x_buffer, S_IRWXU);       /* auto create */
                if (0 == _access(x_buffer, 0)) {
                    done = TRUE;
                }
            }
        }

        /* <HOME>/.config/mc */
        if (!done) { //
            if (NULL != (env = getenv("HOME")) && *env) {
                _snprintf(x_buffer, sizeof(x_buffer), "%s/.config", env);
                if (0 == _access(x_buffer, 0)) {
                    _snprintf(x_buffer, sizeof(x_buffer), "%s/.config/mc", env);
                    mkdir(x_buffer, S_IRWXU);   /* auto create */
                    if (0 == _access(x_buffer, 0)) {
                        done = TRUE;
                    }
                }
            }
        }
#endif  //MC_HOMEDIR_XDG

        // <PERSONAL>
        if (!done) {
            if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, x_buffer)) &&
                                (len = strlen(x_buffer)) > 0) {
                                                /* personal settings */
                _snprintf(x_buffer + len, sizeof(x_buffer) - len, "/%s/", MC_USERCONF_DIR);
                x_buffer[sizeof(x_buffer) - 1] = 0;
                if (0 == _access(x_buffer, 0)) {
                    x_buffer[len+1] = 0;
                    done = TRUE;
                }
            }
        }

        // <APPDATA>
        if (! done) {
            if ((env = getenv("APPDATA")) != NULL && (len = strlen(env)) > 0) {
                                                /* personal settings */
                _snprintf(x_buffer, sizeof(x_buffer), "%s/%s/", env, MC_USERCONF_DIR);
                x_buffer[sizeof(x_buffer) - 1] = 0;
                if (0 == _access(x_buffer, 0)) {
                    x_buffer[len+1] = 0;
                    done = TRUE;
                }
            }
        }

        // <HOME> --- XXX, consider as first option.
        if (! done) {
            if ((env = getenv("HOME")) != NULL && (len = strlen(env)) > 0) {
                                                /* personal settings, new */
                _snprintf(x_buffer, sizeof(x_buffer), "%s/%s/", env, MC_USERCONF_DIR);
                x_buffer[sizeof(x_buffer) - 1] = 0;
                if (0 == _access(x_buffer, 0)) {
                    x_buffer[len+1] = 0;
                    done = TRUE;
                }
            }
        }

        // new user, create
        if (! done) {
                                                /* personal settings */
            if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, x_buffer)) &&
                                (len = strlen(x_buffer)) > 0) {
                _snprintf(x_buffer + len, sizeof(x_buffer) - len, "/%s/", MC_USERCONF_DIR);
                done = TRUE;
                                                /* old school configuration */
            } else if ((env = getenv("HOME")) != NULL && (len = strlen(env)) > 0) {
                _snprintf(x_buffer, sizeof(x_buffer), "%s/%s/", env, MC_USERCONF_DIR);
                done = TRUE;
                                                /* full back, current working directory */
            } else if (w32_getcwd(x_buffer, sizeof(x_buffer)) && (len = strlen(x_buffer)) > 0) {
                _snprintf(x_buffer + len, sizeof(x_buffer) - len, "/%s/", MC_USERCONF_DIR);
                done = TRUE;

            }

            if (! done) {
                strcpy(x_buffer, "./");         /* !! */
            } else {
                x_buffer[sizeof(x_buffer) - 1] = 0;
                mkdir(x_buffer, S_IRWXU);
                x_buffer[len+1] = 0;            /* remove trailing subdirectory, leave seperator */
            }
        }

        unixpath(x_buffer);
    }

    if (subdir && *subdir) {
        const int dirlen = strlen(x_buffer) + strlen(subdir) + 2;
        char *dir = g_malloc(dirlen);

        if (dir) {
            (void) _snprintf(dir, dirlen, "%s%s/", x_buffer, subdir);
            if (-1 == _access(dir, 0)) {
                w32_mkdir(dir, 0666);
            }
        }
        return dir;
    }

    return g_strdup(x_buffer);
}


static void
my_setenv(const char *name, const char *value, int overwrite)
{
    if ((1 == overwrite) || NULL == getenv(name)) {
#if defined(__WATCOMC__)
        setenv(name, value, TRUE);
#else
        char buf[1024];
        snprintf(buf, sizeof(buf), "%s=%s", name, value);
        buf[sizeof(buf)-1] = 0;
        putenv(strdup(buf));
#endif
    }
}


static void
my_setpathenv(const char *name, const char *value, int overwrite, int quote_ws)
{
    char path[1204]={0}, buf[1024]={0};

    if ((1 == overwrite) || NULL == getenv(name)) {
        strncpy(path, value, sizeof(path)-1);
        canonicalize_pathname (path);
        dospath(path);

#if defined(__WATCOMC__)
        if (quote_ws && strchr(path, ' ')) {
            snprintf(buf, sizeof(buf)-1, "\"%s\"", path);
            setenv(name, (const char *)buf, TRUE);
        } else {
            setenv(name, (const char *)path, TRUE);
        }
#else
        if (quote_ws && strchr(path, ' ')) {
            snprintf(buf, sizeof(buf)-1, "%s=\"%s\"", name, path);
        } else {
            snprintf(buf, sizeof(buf)-1, "%s=%s", name, path);
        }
        putenv(buf);
#endif
    }
}


int
WIN32_checkheap(void)
{
    int rc = 0;

    switch (_heapchk()) {
    case _HEAPOK:
    case _HEAPEMPTY:
        break;
    case _HEAPBADBEGIN:
        printf("ERROR - heap is damaged\n");
        rc = -1;
        break;
    case _HEAPBADNODE:
        printf("ERROR - bad node in heap\n");
        rc = -1;
        break;
    }
    return (rc);
}


const char *
get_owner(uid_t uid)
{
    static char ubuf [10];
    struct passwd *pwd;

    pwd = getpwuid (uid);
    if (pwd) {
        return (char *)pwd->pw_name;
    }
    if (uid == 0) {
        return (char *)"root";
    }
    _snprintf (ubuf, sizeof (ubuf), "%d", uid);
    return ubuf;

}


const char *
get_group(gid_t gid)
{
    static char gbuf [10];
    struct group *grp;

    grp = getgrgid (gid);
    if (grp) {
        return (char *)grp->gr_name;
    }
    if (gid == 0) {
        return (char *)"root";
    }
    _snprintf (gbuf, sizeof (gbuf), "%d", gid);
    return gbuf;

}


int
get_user_permissions(struct stat *st)
{
    static gboolean initialized = FALSE;
    static gid_t *groups;
    static int ngroups;
    static uid_t uid;
    int i;

    if (!initialized) {
        uid = geteuid ();

        ngroups = getgroups (0, NULL);
        if (-1 == ngroups) {
            ngroups = 0;                        /* ignore errors */
        }


        /* allocate space for one element in addition to what
         * will be filled by getgroups(). */
        groups = g_new (gid_t, ngroups + 1);

        if (ngroups != 0) {
            ngroups = getgroups (ngroups, groups);
            if (-1 == ngroups) {
                ngroups = 0;                    /* ignore errors */
            }
        }

        /* getgroups() may or may not return the effective group ID,
         * so we always include it at the end of the list. */
        groups[ngroups++] = w32_getegid ();

        initialized = TRUE;
    }

    if (st->st_uid == uid || 0 == uid) {
        return 0;
    }

    for (i = 0; i < ngroups; ++i) {
        if (st->st_gid == groups[i]) {
            return 1;
        }
    }

    return 2;
}


void
save_stop_handler(void)
{
}


/**
 * Call external programs with flags and with array of strings as parameters.
 *
 * @parameter flags   addition conditions for running external programs.
 * @parameter command shell (if flags contain EXECUTE_AS_SHELL), command to run otherwise.
 *                    Shell (or command) will be found in paths described in PATH variable
 *                    (if shell parameter doesn't begin from path delimiter)
 * @parameter argv    Array of strings (NULL-terminated) with parameters for command
 * @return 0 if successfull, -1 otherwise
 */

int
my_systemv_flags (int flags, const char *command, char *const xargv[])
{
    const char **argv = NULL;
    char *cmd = NULL;
    unsigned idx;
    int status = -1;

    if (xargv && NULL != xargv[0]) {

        if ((flags & EXECUTE_AS_SHELL) && NULL == xargv[1]) {
            cmd = my_unquote(xargv[0], TRUE);

        } else {
            const char *str;
            size_t slen = 0;
            char *cursor;

            for (idx = 0; xargv[idx]; ++idx) continue;
            if (NULL == (argv = calloc(idx + 1, sizeof(void *)))) {
                return -1;
            }
            for (idx = 0; NULL != (str = xargv[idx]); ++idx) {
                if (NULL == (argv[idx] = my_unquote (str, FALSE))) {
                    goto error;
                }
            }

            for (idx = 0; NULL != (str = argv[idx]); ++idx) {
                if (*str) {
                    const int isquote = ('"' != *str && '\'' != *str && strchr(str, ' ') ? 1 : 0);

                    slen += strlen(str) + 1 /*nul or space*/;
                    if (isquote) slen += 2; /*quotes*/
                }
            }

            if (NULL == (cmd = calloc(slen, 1))) {
                goto error;
            }

            cursor = cmd;
            for (idx = 0; NULL != (str = argv[idx]); ++idx) {
                if (*str) {
                    const int isquote = ('"' != *str && '\'' != *str && strchr(str, ' ') ? 1 : 0);

                    slen = strlen(str);
                    if (cursor != cmd) *cursor++ = ' ';
                    if (isquote) *cursor++ = '"';
                    memcpy(cursor, str, slen);
                    cursor += slen;
                    if (isquote) *cursor++ = '"';
                }
            }
            *cursor = '\0';
        }
    }

    status = system_impl (flags, command, (cmd ? cmd : ""));
    free (cmd);

error:;
    if (argv) {
        for (idx = 0; argv[idx]; ++idx) {
            free((void *)argv[idx]);
        }
        free ((void *)argv);
    }
    return status;
}


/**
 * Call external programs.
 *
 * @parameter flags   addition conditions for running external programs.
 * @parameter shell   shell (if flags contain EXECUTE_AS_SHELL), command to run otherwise.
 *                    Shell (or command) will be found in paths described in PATH variable
 *                    (if shell parameter doesn't begin from path delimiter)
 * @parameter command Command for shell (or first parameter for command, if flags contain EXECUTE_AS_SHELL)
 * @return 0 if successfull, -1 otherwise
 */
int
my_system (int flags, const char *shell, const char *cmd)
{
    if (cmd && *cmd) {
        char *t_cmd;

        if (NULL != (t_cmd = my_unquote (cmd, TRUE))) {
            int ret = system_impl (flags, shell, t_cmd);
            free((void *)t_cmd);
            return ret;
        }
    }

    return system_impl (flags, shell, cmd);
}


static int
system_impl (int flags, const char *shell, const char *cmd)
{
    const char *busybox = mc_BUSYBOX(), *exec = NULL;
    int shelllen, ret = -1;

    if ((flags & EXECUTE_INTERNAL) && cmd) {
        printf("%s\n", cmd);                    /* echo command */
    }

    if (cmd) {
        while (' ' == *cmd) ++cmd;              /* consume leading whitespace (if any) */
            /*whitespace within "#! xxx" shall be visible; confusing matching logic below */

         if (busybox && *busybox) {
             if (shell && 0 == strncmp(shell, bin_sh, sizeof(bin_sh)-1)) {
                 /*
                  *  If <shell> = </bin/sh> <cmd ...>
                  *  execute as <busybox cmd ...>
                  */
                 const char *space;

                 if (NULL != (space = strchr(cmd, ' '))) {
                     const int cmdlen = space - cmd;
                     unsigned i;

                     for (i = 0; i < _countof(busyboxexts); ++i) {
                         if (0 == strncmp(busyboxexts[i], cmd, cmdlen)) {
                             char *t_cmd;

                             if (NULL != (t_cmd = g_strconcat("\"", busybox, "\" ", cmd, NULL))) {
                                 ret = w32_shell(NULL, t_cmd, NULL, NULL, NULL);
                                 g_free(t_cmd);
                             }
                             return ret;
                         }
                     }
                }

            } else if ((flags & EXECUTE_AS_SHELL) && NULL != (exec = mc_isscript(cmd))) {
                /*
                 *  If <#!> </bin/sh | /usr/bin/perl | /usr/bin/python | /usr/bin/env python>
                 *  note: currently limited to extfs usage.
                 */
                char *t_cmd;

                if (exec[0] == 'p') {           /* perl/python */
                    t_cmd = g_strconcat(exec, " ", cmd, NULL);
                } else {                        /* sh/ash/bash */
                    t_cmd = g_strconcat("\"", busybox, "\" ", exec, " ", cmd, NULL);
                }
                if (t_cmd) {
                    ret = w32_shell(shell, t_cmd, NULL, NULL, NULL);
                    g_free(t_cmd);
                }
                return ret;

            } else {
                 /*
                  *  If <cmd> </bin/sh ...>
                  *  execute as <busybox sh ...>
                  */
                 const char *space;

                 if (NULL != (space = strchr(cmd, ' ')) &&
                         space == (cmd + (sizeof(bin_sh) - 1)) && 0 == strncmp(cmd, bin_sh, sizeof(bin_sh)-1)) {
                    char *t_cmd;

                    if (NULL != (t_cmd = g_strconcat("\"", busybox, "\" sh", space, NULL))) {
                        ret = w32_shell(busybox, t_cmd, NULL, NULL, NULL);
                        g_free(t_cmd);
                    }
                    return ret;
                }
            }
        }
    }

    if ((flags & EXECUTE_AS_SHELL) && cmd) {    /* internal commands */
#define MAX_ARGV    10
#define MAX_CMDLINE (4 * 1024)
        const char *argv[MAX_ARGV];
        char cbuf[MAX_CMDLINE];
        int argc;

        (void) strncpy(cbuf, cmd, sizeof(cbuf));
        if (strlen(cmd) < sizeof(cbuf) &&
                (argc = system_bustargs(cbuf, argv, MAX_ARGV)) <= MAX_ARGV && argc > 0) {

            if (0 == strcmp(argv[0], "set")) {
                return system_SET(argc, argv);

            } else if (use_internal_busybox) {
                const size_t cmdlen = strlen(argv[0]);
                unsigned i;

                for (i = 0; i < _countof(busyboxcmds); ++i) {
                    if (0 == strcmp(busyboxcmds[i], argv[0])) {
                        ret = w32_shell(busybox, cmd, NULL, NULL, NULL);
                        return ret;
                    }
                }
            }
        }
    }

    /*
     *  If <cmd.exe> < ...>
     *  convert any / to \ in first word
     */
    shelllen = (shell ? strlen(shell) : 0);
    if ((shelllen -= (sizeof(cmd_sh)-1)) >= 0 &&
            0 == _strnicmp(shell + shelllen, cmd_sh, sizeof(cmd_sh)-1)) {
        char *t_cmd, *cursor;

        if (NULL != (t_cmd = strdup(cmd))) {
            for (cursor = t_cmd; *cursor && *cursor != ' '; ++cursor) {
                if ('/' == *cursor) *cursor = '\\';
            }
            ret = w32_shell(shell, t_cmd, NULL, NULL, NULL);
            free(t_cmd);
            return ret;
        }
    }

    ret = w32_shell(shell, cmd, NULL, NULL, NULL);
    return ret;
}


const char **
mc_busybox_exts(unsigned *count)
{
    if (count) *count = _countof(busyboxexts);
    return busyboxexts;
}


/**
 *  'set' command replacement
 */
static int
system_SET(int argc, const char **argv)
{
    if (argc == 1) {
        extern char **environ;                  /* MSVC/WATCOM */
        char **env = environ;

        if (env) {
            while (*env) {
                printf( "%s\n", *env);
                ++env;
            }
        }
    } else {
        char *p;

        if ((p = strchr(argv[1], '=')) != NULL) {
            *p++ = '\0';
            my_setenv(argv[1], p, 1);
        } else {
            if ((p = getenv(argv[1])) == NULL) {
                printf("Environment variable %s not defined\n", argv[1]);
            } else {
                printf("%s=%s\n", argv[1], p );
            }
        }
    }
    return (0);
}


/**
 *  Determine if a #! script and return underlying exec handler.
 *  TODO: Return resolved path to perl, python etc (utilise file/extension association)
 */
static const char *
ScriptMagic(int fd)
{
    char magic[128] = { 0 };
    const char *script = NULL;

    if (_read(fd, magic, sizeof(magic) - 1) > 2) {
        if (magic[0] == '#' && magic[1] == '!') {
            const char *exec = magic + 2;       // sha-ban
            int len = -1;

            while (*exec && ' ' == *exec) ++exec;
            if (*exec == '/') {
                if (0 == strncmp(exec, "/bin/sh", len = (sizeof("/bin/sh")-1))) {
                    script = "sh";
                } else if (0 == strncmp(exec, "/bin/ash", len = (sizeof("/bin/ash")-1))) {
                    script = "ash";
                } else if (0 == strncmp(exec, "/bin/bash", len = (sizeof("/bin/bash")-1))) {
                    script = "bash";
                } else if (0 == strncmp(exec, "/bin/sed", len = (sizeof("/bin/sed")-1))) {
                    script = "sed";
                } else if (0 == strncmp(exec, "/bin/awk", len = (sizeof("/bin/awk")-1))) {
                    script = "awk";
                } else if (0 == strncmp(exec, "/usr/bin/perl", len = (sizeof("/usr/bin/perl")-1))) {
                    script = "perl";
                } else if (0 == strncmp(exec, "/usr/bin/python", len = (sizeof("/usr/bin/python")-1))) {
                    script = "python";
                } else if (0 == strncmp(exec, "/usr/bin/env", len = (sizeof("/usr/bin/env")-1))) {
                    //
                    //  Example:
                    //  #! /usr/bin/env python
                    const char *exec2 = exec + len;
                    int len2;

                    while (*exec2 && ' ' == *exec2) { ++exec2, ++len; }
                    if (0 == strncmp(exec2, "python", len2 = (sizeof("python")-1))) {
                        script = "python";
                        len += len2;
                    } else if (0 == strncmp(exec2, "python3", len2 = (sizeof("python3")-1))) {
                        script = "python3";
                        len += len2;
                    }
                }
                //else, ignore others

                if (script && exec[len] != ' ' && exec[len] != '\n' && exec[len] != '\r') {
                    script = NULL;              // bad termination, ignore
                }
            }
        }
    }
    return script;
}


const char *
mc_isscript(const char *cmd)
{
    char t_cmd[1024] = { 0 };
    const char *script = NULL;
    const char *argv[3] = { 0 };
    int fd;

    strncpy(t_cmd, cmd, sizeof(t_cmd)-1);
    if (system_bustargs(t_cmd, argv, 2) >= 1 && argv[0]) {
        if (w32_utf8filenames_state()) {
            wchar_t *warg0 = NULL;

            if (NULL != (warg0 = w32_utf2wca(argv[0], NULL))) {
                if ((fd = _wopen(warg0, O_RDONLY | O_BINARY)) >= 0) {
                    script = ScriptMagic(fd);
                    _close(fd);
                }
                free(warg0);
            }
            return script;
        }

        if ((fd = _open(argv[0], O_RDONLY | O_BINARY)) >= 0) {
            script = ScriptMagic(fd);
            _close(fd);
        }
    }

    win32Trace(("mc_isscript: <%s>=%s\n", cmd, (script ? script : "NA")))
    return script;
}


/**
 *  unquote an excaped command line; yet retain any existing quoted elements.
 */
char *
my_unquote(const char *cmd, int quotews)
{
    char *ret, *cursor, *start = NULL;
    int blen, instring = 0, quoting = 0;

    win32Trace(("mc_unqquote: in:<%s>", cmd))

    blen = (cmd ? strlen(cmd) * 2 : 0);
    if (0 == blen || NULL == (ret = (char *)calloc(blen, 1))) {
        return NULL;
    }

    cursor = ret;
    while (1) {
        if ('\\' == *cmd) {
            switch(*++cmd) {
            case '\'': // escapeable characters
            case '\\':
            case '"':
            case ';':
            case '?':
            case '|':
            case '[': case ']':
            case '{': case '}':
            case '<': case '>':
            case '`':
            case '!':
            case '$':
            case '&':
            case '*':
            case '(': case ')':
            case '~':
            case '#':
            case '\r': case '\n': case '\t':
                *cursor++ = *cmd++;
                break;
            case ' ': // space
                if (quotews && !instring && !quoting) {
                    if (start) { // encase white-space element.
                        char *end = ++cursor;
                        while (end > start) {
                            end[0] = end[-1];
                            --end;
                        }
                        start[0] = '"';
                        quoting = 1;
                    }
                }
                *cursor++ = *cmd++;
                break;
            case 0:  // eos
                *cursor++ = '\\';
                goto null;
            default: // other, retain escape
                *cursor++ = '\\';
                *cursor++ = *cmd++;
                break;
            }

        } else {
            switch (*cmd) {
            case '"': case '\'': // quotes
                if (!quoting) {
                    if (instring) { // end-of-string "xxx" or 'xxx'
                        if (instring == *cmd) {
                            instring = 0;
                        }
                    } else if (NULL == start) { // first non-white?
                        instring = *cmd; // start-of-string
                    }
                } else {
                    if ('"' == *cmd) {  // double quote
                        *cursor++ = '"';
                    }
                }
                /*FALLTHRU*/
            default: // non-whitespace
                if (NULL == start) {
                    start = cursor;  // first non-whitespace
                }
                break;
            case ' ': case '\t': // whitespace
                if (!instring) {
                    if (quoting) { // end-of-string
                        *cursor++ = '"'; // close quotes
                        quoting = 0;
                    }
                    start = NULL;
                }
                break;
            case 0: //null
                goto null;
            }
            *cursor++ = *cmd++;
        }
    }

null:;
    if (quoting) *cursor++ = '"';
    assert(cursor < (ret + blen));
    *cursor = 0;

    win32Trace(("mc_unqquote: rt:<%s>", ret))
    return ret;
}


#if defined(_DEBUG)
void
my_unquote_test(void)
{
    char *result;

    result = my_unquote("C:/Program\\ Files\\ \\(x86\\)/Midnight\\ Commander/plugin/extfs.d/uzip list", TRUE);
    assert(0 == strcmp(result, "\"C:/Program Files (x86)/Midnight Commander/plugin/extfs.d/uzip\" list"));
    free(result);

    result = my_unquote("\"C:/Program Files (x86)\\Midnight Commander/plugin/extfs.d/uzip\" list", TRUE);
    assert(0 == strcmp(result, "\"C:/Program Files (x86)\\Midnight Commander/plugin/extfs.d/uzip\" list"));
    free(result);
}
#endif


/**
 *  popen() implementation
 */
FILE *
win32_popen(const char *cmd, const char *mode)
{
    const char *busybox = mc_BUSYBOX();
    const char *space, *exec;
    FILE *file = NULL;

    if (busybox && *busybox &&
            NULL != (space = strchr(cmd, ' ')) &&
                space == (cmd + (sizeof(bin_sh) - 1)) && 0 == strncmp(cmd, bin_sh, sizeof(bin_sh) - 1)) {
        /*
         *  If <cmd> </bin/sh ...>
         *  execute as <shell> <busybox sh ...>
         */
        char *t_cmd;

        if (NULL != (t_cmd = g_strconcat("\"", busybox, "\" sh", space, NULL))) {
            file = w32_popen(t_cmd, mode);
            g_free(t_cmd);
        }

    } else if (busybox && *busybox && NULL != (exec = mc_isscript(cmd))) {
        /*
         *  If <#!> </bin/sh | /usr/bin/perl | /usr/bin/python | /usr/bin/env python>
         *      note: currently limited to extfs usage.
         */
        char *t_cmd = NULL;

        if (exec[0] == 'p') {                   // perl/python
            t_cmd = g_strconcat(exec, " ", cmd, NULL);
        } else {                                // sh/ash/bash
            t_cmd = g_strconcat("\"", busybox, "\" ", exec, " ", cmd, NULL);
        }
        if (t_cmd) {
            file = w32_popen(t_cmd, mode);
            g_free(t_cmd);
        }

    } else {
        char *t_cmd;
        if (NULL != (t_cmd = my_unquote(cmd, TRUE))) {
            file = w32_popen(t_cmd, mode);
            free((void *)t_cmd);
        } else {
            file = w32_popen(cmd, mode);
        }
    }

    if (pe_open >= 0) {
        if (NULL == file) {
            pe_open += _snprintf(pe_buffer, PE_BUFFER_SIZE, "popen : %s", strerror(errno));

        } else {
            HANDLE hThread;

            pe_stream = file;
            if (0 != (hThread = CreateThread(NULL, 0, pipe_thread, NULL, 0, NULL))) {
                SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
                CloseHandle(hThread);
                sleep(3);                       // yield
            }
        }
    }
    return file;
}


/**
 *  pclose() implementation
 */
int
win32_pclose(FILE *file)
{
    EnterCriticalSection(&pe_guard);
    pe_stream = NULL;
    LeaveCriticalSection(&pe_guard);
    return w32_pclose(file);
}


/**
 *  Creates a pipe to hold standard error for a later analysis.  The pipe
 *  can hold 4096 bytes. Make sure no more is written or a deadlock
 *  might occur.
 *
 *  Returns true if an error was displayed
 *  error: -1 - ignore errors, 0 - display warning, 1 - display error
 *  text is prepended to the error message from the pipe
 */
void
error_pipe_open(void)
{
    pe_open = 0;                                // open stream
}


int
error_pipe_close(int error, const char *text)
{
    const char *title;
    int len;

    EnterCriticalSection(&pe_guard);
    len = pe_open;
    pe_open = -1;
    pe_stream = NULL;
    LeaveCriticalSection(&pe_guard);
    if (len < 0) {
        return 0;
    }

    if (error < 0 || (error > 0 && (error & D_ERROR) != 0)) {
        title = MSG_ERROR;
    } else {
        title = _("Warning");
    }

    if (error < 0) {
        return 0;                               /* just ignore error message */
    }

    /* Show message from pipe */
    if (text == NULL) {
        if (len <= 0) {
            return 0;                           /* Nothing to show */
        }
        pe_buffer[len] = 0;
        text = pe_buffer;

    } else {                                    /* Show given text and possible message from pipe */
        const size_t textlen = strlen(text);

        if (textlen + len < sizeof(pe_buffer)) {
            memmove(pe_buffer + textlen + 1, (const char *)pe_buffer, len);
            memmove(pe_buffer, text, textlen);
            len  += textlen + 1;
            pe_buffer[textlen] = '\n';
            pe_buffer[len] = 0;
            text = pe_buffer;
        }
    }

    query_dialog(title, text, D_NORMAL, 1, _("&Ok"));
    return 1;
}


/**
 *  popen stderr consumer
 */
static DWORD WINAPI
pipe_thread(void *data)
{
    FILE *file;
    char buffer[ PE_BUFFER_SIZE ];

    (void) data;
    EnterCriticalSection(&pe_guard);
    if (pe_open >= 0 && NULL != (file = pe_stream)) {
        while (1) {
            int len;

            LeaveCriticalSection(&pe_guard);    /* consume stderr */
            len = w32_pread_err(pe_stream, buffer, sizeof(buffer));
            EnterCriticalSection(&pe_guard);

            if (len >= 0 && pe_open >= 0 && file == pe_stream) {
                if (0 == len) {
                    if (GetLastError() == ERROR_BROKEN_PIPE) {
                        break;                  /* pipe done */
                    }
                } else {
                    int newlen = 0;
                    const char *peend, *cursor = buffer;
                    char *pe = pe_buffer;

                    peend = pe + PE_BUFFER_SIZE, pe += pe_open;
                    while (len-- > 0 && pe < peend) {
                        if ('\r' != *cursor) {
                            *pe++ = *cursor;
                            ++newlen;
                        }
                        ++cursor;
                    }
                    pe_open += newlen;
                }
                continue;
            }
            break;                              /* stream invalid */
        }
    }
    LeaveCriticalSection(&pe_guard);
    return 0;
}


static void
unixpath(char *path)
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


static void
dospath(char *path)
{
    const char *in = path;

    while (*in) {
        if ('/' == *in || '\\' == *in) {
            ++in;
            while ('/' == *in || '\\' == *in) {
                ++in;
            }
            *path++ = '\\';
        } else {
            *path++ = *in++;
        }
    }
    *path = 0;
}


/**
 *  Splits 'cmd' into list of argument pointers.
 */
static int
system_bustargs(char *cmd, const char **argv, int cnt)
{
    char *start, *end;
    int argc;

    --cnt;                                      /* nul terminator storage */
    for (argc = 0;;) {
        /* Skip over blanks */
        while (*cmd == ' '|| *cmd == '\t' || *cmd == '\n') {
            ++cmd;                              /* eat white space */
        }
        if (*cmd == '\0') {                     /* termination */
             break;
        }

        /* Retrieve argument */
        if (*cmd == '\"') {                     /* quoted argument */
            ++cmd;
            start = end = cmd;
            for (;;) {
                if (*cmd == '\n' || *cmd == '\0')
                    break;
                if (*cmd == '\"')
                    break;
                if (*cmd == '\\') {
                    if (cmd[1] == '\"' || cmd[1] == '\\') {
                        ++cmd;
                    }
                }
                *end++ = *cmd++;
             }
        } else {
            start = end = cmd;
            for (;;) {
                if (*cmd == '\n' || *cmd == '\0')
                    break;
                if (*cmd == ' '  || *cmd == '\t')
                    break;
                if (*cmd == '\\' && cmd[1] == '\"')
                    ++cmd;
                *end++ = *cmd++;
            }
        }

        /* reallocate argument list index */
        if (cnt > 0) {
            argv[ argc++ ] = start;
            if (*cmd == '\0')
                break;
            *end = '\0';
            ++cmd;
            --cnt;
        }
    }
    argv[ argc ] = NULL;
    return argc;
}


/**
 *  Directory references expansion ..
 */
char *
tilde_expand(const char *directory)
{
    if (0 == directory[0]) {
        return g_strdup ("");                   /* empty */
    }

    if (PATH_SEP == directory[0] &&             /* fix '/X:', vfs work around */
            directory[1] && ':' == directory[2] && isalpha((unsigned char)directory[1])) {
        ++directory;
    }

    if (PATH_SEP == *directory) {               /* / ==> x:/ */

        if (PATH_SEP != directory[1] ||         /* preserve URL's (//<server) */
                0 == directory[2] || PATH_SEP == directory[2]) {
            const char *cwd = vfs_get_current_dir ();
            char path[WIN32_PATH_MAX];

            if ('/' == cwd[0] && 0 == cwd[1]) { /* vfs, possible ftp/sftp */
                if (w32_getcwd (path, sizeof(path))) {
                    cwd = path;  /* apply underlying cwd */
                }
            }

            if (cwd[0] && ':' == cwd[1]) {
                char drive[3] = "X:";
                drive[0] = toupper (cwd[0]);
                return g_strconcat (drive, directory, NULL);
            }
        }

    } else if ('.' == *directory && 0 == directory[1]) {

        char *cwd = vfs_get_current_dir_n ();
        if (cwd) {                              /* . ==> <cwd> */
            return cwd;
        }

    } else if (':' == directory[1] && isalpha((unsigned char)directory[0]) &&
                    (0 == directory[2] || ('.'== directory[2] && 0 == directory[3]))) {
        char path[WIN32_PATH_MAX];

        if (w32_getcwdd(directory[0], path, sizeof(path))) {
            return g_strdup (path);             /* X: and X:. ==> <drive><cwd> */
        }

    } else if ('~' == *directory) {

        struct passwd *passwd = NULL;
        const char *home = NULL, *p, *q;

        p = directory + 1;
        q = strchr (p, PATH_SEP);

        if (!(*p) || (*p == PATH_SEP)) {        /* d = "~" or d = "~/" */
            passwd = getpwuid (geteuid ());
            if (passwd) home = passwd->pw_dir;
            if (NULL == home || !*home) home = getenv("HOME");
            q = (*p == PATH_SEP) ? p + 1 : "";

        } else {
            if (!q) {
                passwd = getpwnam (p);
            } else {
                char *name;

                if (NULL != (name = g_malloc (q - p + 1))) {
                    strncpy (name, p, q - p);
                    name[q - p] = 0;
                    passwd = getpwnam (name);
                    g_free (name);
                }
            }
            if (passwd) home = passwd->pw_dir;
        }

        if (home && *home) {
            return g_strconcat (home, PATH_SEP_STR, q, NULL);
        }
    }

    return g_strdup (directory);
}


/**
 *  Canonicalize path, and return a new path.
 *  Everything done in-place, hence the result cannot extend the buffer.
 *
 *  The new path differs from path in:
 *      Multiple `/'s are collapsed to a single `/'.
 *      Leading  `./'s and trailing `/.'s are removed.
 *      Trailing `/'s are removed.
 *      Non-leading `../'s and trailing `..'s are handled by removing
 *      portions of the path.
 *
 *  Well formed UNC paths are modified only in the local part.
 *
 *  Notes: Sourced from lib/utilunix.c
 */

static int
current_drive(char *path)
{
    int driveno = w32_getdrive();

    if (driveno <= 0) driveno = w32_getlastdrive();
    if (driveno <= 0) driveno = w32_getsystemdrive();
    if (driveno > 0) {
        path[0] = driveno + ('A' - 1);
        path[1] = ':';
        path[2] = PATH_SEP;
        path[3] = 0;
        return 1;
    }
    return 0;
}


void
#if !defined(CANON_PATH_FLAGS)
canonicalize_pathname_custom(char *orgpath, canon_path_flags_t flags) // 4.8.29
#else
custom_canonicalize_pathname(char *orgpath, CANON_PATH_FLAGS flags)
#endif
{
    const size_t url_delim_len = strlen (VFS_PATH_URL_DELIMITER);
    char *lpath = orgpath;                      /* path without leading UNC part */
    int unc = FALSE;
    char *p, *s;

    /* Standardise to the system seperator */
    if (0 == lpath[0])
        return;                                 /* empty */

    for (s = lpath; *s; ++s)
        if ('\\' == *s || '/' == *s)
            *s = PATH_SEP;

    /* Detect and preserve UNC paths: "//server/" */
    if ((flags & CANON_PATH_GUARDUNC) != 0 && IS_PATH_SEP (lpath[0]) && IS_PATH_SEP (lpath[1]) && lpath[2])
    {
        for (p = lpath + 2; p[0] != '\0' && !IS_PATH_SEP (p[0]); p++)
            ;

        if (p[0] == PATH_SEP && p > (orgpath + 2))
        {
            if (0 == strcmp(p + 1, ".."))
            {                                   /* "//servername/.." --> "X:/" */
                if (current_drive (lpath))
                    return;
            }
            lpath = p;
            unc = TRUE;
        }
    }

    if (0 == lpath[0] || 0 == lpath[1])
        return;

    /* DOS'ish
     *  o standardize seperator
     *  o preserve leading drive
     */
    if (!unc)
    {
        if (PATH_SEP == lpath[0] &&
                ':' == lpath[2] && isalpha((unsigned char)lpath[1])) {
            str_move (lpath, lpath + 1);        /* /X:, remove leading '/' vfs name mangling */
            lpath[0] = toupper(lpath[0]);
            lpath += 2;

        } else if (':' == lpath[1] && isalpha((unsigned char)lpath[0])) {
            lpath[0] = toupper(lpath[0]);
            lpath += 2;                         /* skip drive */
        }
    }

    /* Execute based on specified flags */
    if (flags & CANON_PATH_JOINSLASHES)
    {
        /* Collapse multiple slashes */
        for (p = lpath; *p != '\0'; p++)
            if (IS_PATH_SEP (p[0]) && IS_PATH_SEP (p[1]) && (p == lpath || *(p - 1) != ':'))
            {
                s = p + 1;
                while (*s && IS_PATH_SEP(*s))
                    ++s;
                str_move (p + 1, s);
            }

        /* Collapse "/./" -> "/" */
        for (p = lpath; *p != '\0';)
            if (IS_PATH_SEP (p[0]) && p[1] == '.' && IS_PATH_SEP (p[2]))
                str_move (p, p + 2);
            else
                p++;
    }

    if (flags & CANON_PATH_REMSLASHDOTS)
    {
        size_t len;

        /* Remove trailing slashes */
        for (p = lpath + strlen (lpath) - 1; p > lpath && IS_PATH_SEP (*p); p--)
        {
            if (p >= lpath + url_delim_len - 1
                && strncmp (p - url_delim_len + 1, VFS_PATH_URL_DELIMITER, url_delim_len) == 0)
                break;
            *p = '\0';
        }

        /* Remove leading "./" */
        if (lpath[0] == '.' && IS_PATH_SEP (lpath[1]))
        {
            if (lpath[2] == '\0')
            {
                lpath[1] = '\0';
                return;
            }

            str_move (lpath, lpath + 2);
        }

        /* Remove trailing "/" or "/." */
        len = strlen (lpath);
        if (len < 2)
            return;

        if (IS_PATH_SEP (lpath[len - 1])
            && (len < url_delim_len
                || strncmp (lpath + len - url_delim_len, VFS_PATH_URL_DELIMITER,
                            url_delim_len) != 0))
            lpath[len - 1] = '\0';
        else if (lpath[len - 1] == '.' && IS_PATH_SEP (lpath[len - 2]))
        {
            if (len == 2)
            {
                lpath[1] = '\0';
                return;
            }

            lpath[len - 2] = '\0';
        }
    }

    /* Collapse "/.." with the previous part of path */
    if (flags & CANON_PATH_REMDOUBLEDOTS)
    {
#ifdef HAVE_CHARSET
        const size_t enc_prefix_len = strlen (VFS_ENCODING_PREFIX);
#endif /* HAVE_CHARSET */

        for (p = lpath; p[0] != '\0' && p[1] != '\0' && p[2] != '\0';)
        {
            if (!IS_PATH_SEP (p[0]) || p[1] != '.' || p[2] != '.'
                || (!IS_PATH_SEP (p[3]) && p[3] != '\0'))
            {
                p++;
                continue;
            }

            /* search for the previous token */
            s = p - 1;
            if (s >= lpath + url_delim_len - 2
                && strncmp (s - url_delim_len + 2, VFS_PATH_URL_DELIMITER, url_delim_len) == 0)
            {
                s -= (url_delim_len - 2);
                while (s >= lpath && !IS_PATH_SEP (*s--))
                    ;
            }

            while (s >= lpath)
            {
                if (s - url_delim_len > lpath
                    && strncmp (s - url_delim_len, VFS_PATH_URL_DELIMITER, url_delim_len) == 0)
                {
                    char *vfs_prefix = s - url_delim_len;
                    vfs_class *vclass;

                    while (vfs_prefix > lpath && !IS_PATH_SEP (*--vfs_prefix))
                        ;
                    if (IS_PATH_SEP (*vfs_prefix))
                        vfs_prefix++;
                    *(s - url_delim_len) = '\0';

                    vclass = vfs_prefix_to_class (vfs_prefix);
                    *(s - url_delim_len) = *VFS_PATH_URL_DELIMITER;

                    if (vclass != NULL && (vclass->flags & VFSF_REMOTE) != 0)
                    {
                        s = vfs_prefix;
                        continue;
                    }
                }

                if (IS_PATH_SEP (*s))
                    break;

                s--;
            }

            s++;

            /* If the previous token is "..", we cannot collapse it */
            if (s[0] == '.' && s[1] == '.' && s + 2 == p)
            {
                p += 3;
                continue;
            }

            if (p[3] != 0)
            {
                if (s == lpath && *s == PATH_SEP)
                {
                    /* "/../foo" -> "/foo" */
                    str_move (s + 1, p + 4);
                }
                else
                {
                    /* "token/../foo" -> "foo" */
#ifdef HAVE_CHARSET
                    if ((strncmp (s, VFS_ENCODING_PREFIX, enc_prefix_len) == 0)
                            && (is_supported_encoding (s + enc_prefix_len)))
                        /* special case: remove encoding */
                        str_move (s, p + 1);
                    else
#endif /* HAVE_CHARSET */
                        str_move (s, p + 4);
                }
                p = (s > lpath) ? s - 1 : s;
                continue;
            }

            /* trailing ".." */
            if (s == lpath)
            {
                /* "token/.." -> "." */
                if (lpath[0] != PATH_SEP)
                    lpath[0] = '.';
                lpath[1] = 0;
            }
            else
            {
                /* "foo/token/.." -> "foo" */
                if (s == lpath + 1)
                    s[0] = 0;
#ifdef HAVE_CHARSET
                else if ((strncmp (s, VFS_ENCODING_PREFIX, enc_prefix_len) == 0)
                            && (is_supported_encoding (s + enc_prefix_len)))
                {
                    /* special case: remove encoding */
                    s[0] = '.';
                    s[1] = '.';
                    s[2] = '\0';

                    /* search for the previous token */
                    /* IS_PATH_SEP (s[-1]) */
                    for (p = s - 1; p >= lpath && !IS_PATH_SEP (*p); p--)
                        ;

                    if (p >= lpath)
                        continue;
                }
#endif /* HAVE_CHARSET */
                else
                {
                    if (s >= lpath + url_delim_len
                            && strncmp (s - url_delim_len, VFS_PATH_URL_DELIMITER, url_delim_len) == 0)
                        *s = '\0';
                    else
                        s[-1] = '\0';
                }
                break;
            }

            break;
        }
    }
}


void
canonicalize_pathname(char *path)
{
#if !defined(CANON_PATH_FLAGS)
    canonicalize_pathname_custom (path, CANON_PATH_ALL);
#else
    custom_canonicalize_pathname (path, CANON_PATH_ALL);
#endif
}


/**
 *  realpath() implementation.
 */
char *
mc_realpath(const char *path, char *resolved_path)
{
    if (NULL == w32_realpath(path, resolved_path /*MAX_PATH*/)) {
        strcpy(resolved_path, path);
    }

    unixpath(resolved_path);
    return resolved_path;
}


/**
 *  Build filename from arguments.
 *  Like to g_build_filename(), but respect VFS_PATH_URL_DELIMITER
 */
char *
mc_build_filenamev(const char *first_element, va_list args)
{
    gboolean absolute;
    const char *element = first_element;
    GString *path;
    char *ret;

    if (element == NULL)
        return NULL;

    path = g_string_new ("");
    absolute = IS_PATH_SEP (*first_element);

    do {
        if (*element == '\0') {
            element = va_arg (args, char *);
        } else {
            char *tmp_element;
            size_t len;
            const char *start;

            tmp_element = g_strdup (element);

            element = va_arg (args, char *);

            canonicalize_pathname (tmp_element);
            len = strlen (tmp_element);
            start = IS_PATH_SEP (tmp_element[0]) ? tmp_element + 1 : tmp_element;

            g_string_append (path, start);
            if (!IS_PATH_SEP (tmp_element[len - 1]) && element != NULL)
                g_string_append_c (path, PATH_SEP);
            g_free (tmp_element);
        }
    }
    while (element != NULL);

    if (absolute)
    {
        if (! path->len || ':' != path->str[1] /*not-drive*/)
        {
            g_string_prepend_c (path, PATH_SEP);    // reapply leading

            //WIN32, drive
            if (NULL == strchr (path->str, ':') &&  // Neither special (ftp://)
                    PATH_SEP != path->str[1]) {     // nor url (//server ..)
                int driveno = w32_getdrive();
                if (driveno <= 0) driveno = w32_getlastdrive();

                // see: vfs_canon() generally when we are returning from a ftp/sftp or UNC reference.
                if (driveno > 0)
                {
                    char drive[3] = "X:";
                    drive[0] = driveno + ('A' - 1);
                    g_string_prepend (path, drive); // "/" --> "X:/"
                }
            }
        }
    }

    ret = g_string_free (path, FALSE);
    canonicalize_pathname (ret);

    return ret;
}


/**
 *  Build filename from arguments.
 *  Like to g_build_filename(), but respect VFS_PATH_URL_DELIMITER
 */
char *
mc_build_filename(const char *first_element, ...)
{
    va_list args;
    char *ret;

    if (first_element == NULL)
        return NULL;
    va_start (args, first_element);
    ret = mc_build_filenamev (first_element, args);
    va_end (args);
    return ret;
}


/**
 *  inet_ntop - convert IPv4 and IPv6 addresses from binary to text.
 */
const char *
mc_inet_ntop(int af, const void *src, char *dst, size_t /*socklen_t*/ size)
{
#if (0)
    switch (af) {
    case AF_INET: {
            struct sockaddr_in in;
            memset(&in, 0, sizeof(in));
            in.sin_family = AF_INET;
            memcpy(&in.sin_addr, src, sizeof(struct in_addr));
            getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in), dst, size, NULL, 0, NI_NUMERICHOST);
        }
        break;
    case AF_INET6: {
            struct sockaddr_in6 in;
            memset(&in, 0, sizeof(in));
            in.sin6_family = AF_INET6;
            memcpy(&in.sin6_addr, src, sizeof(struct in_addr6));
            getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in6), dst, size, NULL, 0, NI_NUMERICHOST);
        }
        break;
    defaullt:
        return NULL;
    }
    return dst;

#else
    struct sockaddr_storage ss = {0};
    unsigned long s = (unsigned long)size;

    ss.ss_family = af;
    switch (af) {
    case AF_INET:
        ((struct sockaddr_in *)&ss)->sin_addr = *(struct in_addr *)src;
        break;
    case AF_INET6:
        ((struct sockaddr_in6 *)&ss)->sin6_addr = *(struct in6_addr *)src;
        break;
    default:
        return NULL;
    }
    return (WSAAddressToStringA((struct sockaddr *)&ss, sizeof(ss), NULL, dst, &s) == 0) ? dst : NULL;
#endif
}

/*end*/
