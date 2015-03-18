#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/*
 *  WIN32 config.h
 */

#ifndef RC_INVOKED
#include <w32config.h>
#include <stdio.h>
#endif

/*
 *  build information
 */
#include <buildinfo.h>

#define PACKAGE             "mc-win32-native"
#define VERSION             "4.8.12"            /* 5 Aug 14 */

#ifdef  RC_INVOKED                              /* see: mc.rc */
#define RC_PRODUCTVERSION   4,8,12,0
#define RC_FILEVERSION      4,8,12,1
#endif

#define MC_CONFIGURE_ARGS   "win32-native"
#define MC_APPLICATION_DIR  "Midnight Commander"


/*
 *  application runtime configuration
 */
#ifndef RC_INVOKED

#define MC_USERCONF_DIR     MC_APPLICATION_DIR  /* see: fileloc.h, default "mc" */
#undef  MC_HOMEDIR_XDG                          /* enforce Freedesktop recommended dirs, not required */

const char *                mc_TMPDIR(void);
const char *                mc_SYSCONFDIR(void);
const char *                mc_DATADIR(void);
const char *                mc_LOCALEDIR(void);
const char *                mc_MAGICPATH(void);
const char *                mc_LIBEXECDIR(void);
char *                      mc_USERCONFIGDIR(const char *subdir);
const char *                mc_EXTHELPERSDIR(void);

#define SYSCONFDIR          mc_SYSCONFDIR()     /* /etc/mc */
#define MC_DATADIR          mc_DATADIR()        /* /usr/share/mc */
#define LOCALEDIR           mc_LOCALEDIR()      /* /usr/share/locale */
#define LIBEXECDIR          mc_LIBEXECDIR()     /* /lib/mc */
#define EXTHELPERSDIR       mc_EXTHELPERSDIR()  /* ???, 4.8.7 */

extern FILE *               mc_popen(const char *cmd, const char *mode);
extern int                  mc_pclose(FILE *file);

#define popen(__cmd,__mode) mc_popen(__cmd, __mode)
#define pclose(__file)      mc_pclose(__file)

#endif


/*
 *  available components
 */
#define HAVE_SLANG 1
#undef  HAVE_SLANG_SLANG_H
#define HAVE_SLANG_H 1
#define HAVE_SYS_PARAM_H 1
#undef  HAVE_SYS_SELECT_H
#define HAVE_SYS_VFS_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_UNISTD_H 1
#define HAVE_TIME_H 1
#define HAVE_UTIME_H 1
#define HAVE_STDARG_H 1
#define HAVE_ASSERT_H 1
#define HAVE_STRING_H 1
#define HAVE_MEMORY_H 1
#define HAVE_PWD_H 1
#define HAVE_GRP_H 1
#undef  HAVE_STRUCT_STATVFS_F_BASETYPE
#undef  HAVE_STRUCT_STATVFS_F_FSTYPENAME
#undef  HAVE_STRUCT_STATFS_F_FSTYPENAME

#define HAVE_LIBMAGIC
#undef  HAVE_ASPELL
#undef  HAVE_SUBSHELL_SUPPORT
#define HAVE_CHARSET 1
#define HAVE_SLANG 1
#undef  HAVE_TEXTMODE_X11_SUPPORT
#undef  HAVE_LIBGPM

#undef  HAVE_REALPATH
#define HAVE_STRCASECMP
#define HAVE_STRNCASECMP
#define HAVE_GETOPT
#if defined(__WATCOMC__)
#define HAVE_STRLCPY
#define HAVE_STRLCAT
#define HAVE_LOCALE_H
#endif

/*
 *  configuration options
 */
#undef  SEARCH_TYPE_PCRE
#define SEARCH_TYPE_GLIB 1

#define LISTMODE_EDITOR 1
#define USE_INTERNAL_EDIT 1
#define USE_DIFF_VIEW 1
#define USE_LIBMAGIC 1
#undef  USE_MAINTAINER_MODE                     /* see: ../lib/logging.c/.h */
#define USE_SLANG 1
#undef  USE_NCURSES
#undef  USE_NCURSESW

#define ENABLE_NLS
#undef  ENABLE_BACKGROUND

#define ENABLE_VFS 1
#define ENABLE_VFS_CPIO 1
#define ENABLE_VFS_TAR 1
#define ENABLE_VFS_SFS 1
#define ENABLE_VFS_EXTFS 1
#define ENABLE_VFS_FTP 1
#undef  ENABLE_VFS_FISH
#undef  ENABLE_VFS_SFTP
#undef  ENABLE_VFS_SMB
#undef  ENABLE_VFS_UNDELFS

#define SIG_ATOMIC_VOLATILE_T int

#endif  /*CONFIG_H_INCLUDED*/



