#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/* 
 *  WIN32 config.h
 */

#include <w32config.h>
                            
#define PACKAGE_NAME        "file"
#define PACKAGE_TARNAME     "file"
#define PACKAGE_VERSION     "5.11"
#define PACKAGE_STRING      "file 5.11"
#define PACKAGE_BUGREPORT   "christos@astron.com"
#define PACKAGE_URL         ""

#define VERSION             PACKAGE_VERSION

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
#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1 
#if defined(__WATCOMC__)
#define HAVE_STRLCPY 1
#define HAVE_STRLCAT 1
#define HAVE_LOCALE_H 1
#endif

#endif  /*CONFIG_H_INCLUDED*/



