#ifndef LIBW32_UNISTD_H_INCLUDED
#define LIBW32_UNISTD_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <unistd.h> compat header file -
 *
 *      unistd.h - standard symbolic constants and types
 *
 * Copyright (c) 2007, 2012 - 2017 Adam Young.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * The Midnight Commander is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ==end==
 */

#if defined(_MSC_VER)
#if (_MSC_VER != 1200)                          /* MSVC 6 */
#if (_MSC_VER != 1400)                          /* MSVC 8/2005 */
#if (_MSC_VER != 1500)                          /* MSVC 9/2008 */
#if (_MSC_VER != 1600)                          /* MSVC 10/2010 */
#if (_MSC_VER != 1900)                          /* MSVC 19/2015 */
#error utypes.h: untested MSVC c/c++ Version (CL 12.xx - 19.xx) only ...
#endif
#endif
#endif
#endif
#endif
#pragma warning(disable:4115)

#if defined(_POSIX_)
#error unistd.h: _POSIX_ enabled
#endif

#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE                /* disable deprecate warnings */
#endif

#elif defined(__WATCOMC__)
#if (__WATCOMC__ < 1200)
#error unistd.h: old WATCOM Version, upgrade to OpenWatcom ...
#endif

#elif defined(__MINGW32__)

#else
#error unistd.h: unsupported compiler
#endif

#if !defined(_WIN32_WINCE)                      /* require winsock2.h */
#if !defined(_WIN32_WINNT)
#if defined(WIN32) && (WIN32 > 0x100)
#define _WIN32_WINNT WIN32
#else
#define _WIN32_WINNT 0x400                      /* entry level */
#endif
#elif (_WIN32_WINNT) < 0x400
//  Minimum system required Minimum value for _WIN32_WINNT and WINVER
//  Windows 7                                           (0x0601)
//  Windows Server 2008                                 (0x0600)
//  Windows Vista                                       (0x0600)
//  Windows Server 2003 with SP1, Windows XP with SP2   (0x0502)
//  Windows Server 2003, Windows XP _WIN32_WINNT_WINXP  (0x0501)
//
#pragma message("unistd: _WIN32_WINNT < 0400")
#endif
#endif   /*_WIN32_WINCE*/
#ifndef _WIN32
#define _WIN32 WIN32                           /* WIN32 and _WIN32 generally required */
#endif


/*
 *  avoid importing <win32_include.h>
 *      which among others includes <ctype.h>
 */

#include <win32_errno.h>

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */
#include <sys/utypes.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <time.h>                               /* required to replace strfime() */
#include <utime.h>
#include <stddef.h>                             /* offsetof() */
#include <dirent.h>                             /* MAXPATHLENGTH, MAXNAMELENGTH */
#include <limits.h>                             /* _MAX_PATH */
#include <process.h>                            /* getpid, _beginthread */

#include <stdio.h>                              /* FILE */
#include <stdlib.h>
#include <malloc.h>
#include <string.h>                             /* memset, memmove ... */
#include <fcntl.h>
#include <io.h>                                 /* write, read ... */

#if defined(HAVE_CONFIG_H)                      /* stand alone? */
#ifndef  NEED_GETOPT
#define  NEED_GETOPT
#endif
#include <getopt.h>                             /* getopt */
#endif

__BEGIN_DECLS

/*limits*/
#define WIN32_PATH_MAX  1024
#define WIN32_LINK_DEPTH 8

/*fcntl.h*/
#define F_OK            0                       /* 00 Existence only */
#define W_OK            2                       /* 02 Write permission */
#define R_OK            4                       /* 04 Read permission */
#if !defined(X_OK)
#define X_OK            4                       /* 04 Execute permission, not available unless emulated */
#endif
#define A_OK            6                       /* 06 (access) Read and write permission */

#if !defined(O_ACCMODE)                         /* <fcntl.h>, Mask for file access modes */
#define O_ACCMODE       (O_RDONLY|O_WRONLY|O_RDWR)
#endif

#define O_NDELAY        0


/* <stat.h> */
                                                /* de facto standard definitions */
#if defined(S_IFFMT)                            /* type mask */
#if S_IFFMT != 0170000
#error  S_IFFMT redefinition error ...
#endif
#else
#define S_IFFMT         0170000
#endif

#if defined(__WATCOMC__)                        /* note, defined as 0 */
#undef  S_IFSOCK
#undef  S_ISSOCK
#undef  S_IFLNK
#undef  S_ISLNK
#undef  S_ISBLK
#undef  S_ISFIFO
#endif

#if defined(__MINGW32__)
#undef  S_IFBLK
#undef  S_ISBLK
#endif

#if defined(S_IFSOCK)
#if S_IFSOCK != 0140000
#error  S_IFSOCK redefinition error ...
#endif
#else
#define S_IFSOCK        0140000                 /* socket */
#endif

#if defined(S_IFLNK)
#if S_IFLNK != 0120000
#error  S_IFLNK redefinition error ...
#endif
#else
#define S_IFLNK         0120000                 /* symbolic link */
#endif

#if defined(S_IFREG)                            /* regular file */
#if (S_IFREG != 0100000)
#error  S_IFREG redefinition error ...
#endif
#else
#define S_IFREG         0100000
#endif

#if defined(S_IFBLK)                            /* block device */
#if (S_IFBLK != 0060000)
#error  S_IFBLK redefinition error ...
#endif
#else
#define S_IFBLK         0060000
#endif

#if defined(S_IFDIR)                            /* regular file */
#if (S_IFDIR != 0040000)
#error  S_IFDIR redefinition error ...
#endif
#else
#define S_IFDIR         0040000
#endif

#if defined(S_IFCHR)                            /* character special device */
#if (S_IFCHR != 0020000)
#error  S_IFCHR redefinition error ...
#endif
#else
#define S_IFCHR         0020000
#endif

#if defined(S_IFIFO)                            /* fifo */
#if (S_IFIFO != 0010000)
#error  S_IFIFO redefinition error ...
#endif
#else
#define S_IFIFO         0010000
#endif

#if defined(S_IFFIFO)                           /* fifo??? */
#error  S_IFFIFO is defined ??? ...
#endif

/* de facto standard definitions */
#if !defined(S_ISUID)
#define S_ISUID         0002000                 /* set user id on execution */
#endif

#if !defined(S_ISGID)
#define S_ISGID         0001000                 /* set group id on execution */
#endif

#if defined(S_IRWXU)
#if (S_IRWXU != 0000700)
#error  S_IRWXU redefinition error ...
#endif
#else
#define S_IRWXU         0000700                 /* read, write, execute: owner */
#endif

#if defined(S_IRUSR)
#if (S_IRUSR != 0000400)
#error  S_IRUSR redefinition error ...
#endif
#else
#define S_IRUSR         0000400                 /* read permission: owner */
#define S_IWUSR         0000200                 /* write permission: owner */
#define S_IXUSR         0000100                 /* execute permission: owner */
#endif

#define S_IRWXG         0000070                 /* read, write, execute: group */
#define S_IRGRP         0000040                 /* read permission: group */
#define S_IWGRP         0000020                 /* write permission: group */
#define S_IXGRP         0000010                 /* execute permission: group */

#define S_IRWXO         0000007                 /* read, write, execute: other */
#define S_IROTH         0000004                 /* read permission: other */
#define S_IWOTH         0000002                 /* write permission: other */
#define S_IXOTH         0000001                 /* execute permission: other */

#define __S_ISTYPE(__mode,__mask) \
                        (((__mode) & S_IFFMT) == __mask)

#ifndef S_ISBLK                                 /* test for a block special */
#define S_ISBLK(m)      __S_ISTYPE(m, S_IFBLK)
#endif
#if !defined(__MINGW32__)
#ifndef S_ISFIFO                                /* test for a pipe or FIFO special file */
#define S_ISFIFO(m)     __S_ISTYPE(m, S_IFIFO)
#endif
#endif
#ifndef S_ISDIR                                 /* test for a directory */
#define S_ISDIR(m)      __S_ISTYPE(m, S_IFDIR)
#endif
#ifndef S_ISCHR
#define S_ISCHR(m)      __S_ISTYPE(m, S_IFCHR)
#endif
#ifndef S_ISREG                                 /* test for a regular file */
#define S_ISREG(m)      __S_ISTYPE(m, S_IFREG)
#endif
#ifndef S_ISLNK                                 /* test for a symbolic link */
#define S_ISLNK(m)      __S_ISTYPE(m, S_IFLNK)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(m)     __S_ISTYPE(m, S_IFSOCK) /* test for a socket */
#endif

#ifndef S_ISVTX
#define S_ISVTX         0                       /* on directories, restricted deletion flag; not supported */
#endif

/*stdio.h*/
#if !defined(STDIN_FILENO)
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2
#endif

/*signal.h*/
#define SIGCHLD         -101
#define SIGWINCH        -102
#define SIGPIPE         -103

#if !defined(__MINGW32__)
typedef struct {
    unsigned            junk;
} sigset_t;

struct sigaction {
    void              (*sa_handler)(int);
#define SA_RESTART                      0x01
    unsigned            sa_flags;
    sigset_t            sa_mask;
};

LIBW32_API int          sigemptyset (sigset_t *);
LIBW32_API int          sigaction (int, struct sigaction *, struct sigaction *);
#endif

/*shell support*/
#if !defined(WNOHANG)
#define WNOHANG         1
#endif

LIBW32_API int          w32_waitpid (int, int *, int);
LIBW32_API int          w32_kill (int pid, int sig);

#if defined(WIN32_UNISTD_MAP)
#define                 kill(__pid, __val) \
                w32_kill(__pid, __val)
#endif /*WIN32_UNISTD_MAP*/

#if !defined(WEXITSTATUS)
LIBW32_API int          WEXITSTATUS (int status);
LIBW32_API int          WIFEXITED (int status);
LIBW32_API int          WIFSIGNALED (int status);
LIBW32_API int          WTERMSIG (int status);
LIBW32_API int          WCOREDUMP (int status);
LIBW32_API int          WIFSTOPPED(int status);
#endif

/* <stdlib.h> */
LIBW32_API int          getsubopt (char **optionp, char * const *tokens, char **valuep);

/* <string.h> */
#if defined(_MSC_VER)
LIBW32_API int          strcasecmp(const char *s1, const char *s2);
LIBW32_API int          strncasecmp(const char *s1, const char *s2, size_t len);
#endif

#if (defined(_MSC_VER) && (_MSC_VER < 1400)) || \
            defined(__MINGW32__) || defined(__WATCOMC__)
#define NEED_STRNLEN                            /*see: w32_string.c*/
#endif
#if defined(NEED_STRNLEN)
LIBW32_API size_t       strnlen(const char *s, size_t maxlen);
#endif

/* <unistd.h> */
LIBW32_API int          gettimeofday (struct timeval *tv, struct timezone *tz);
LIBW32_API int          w32_utime (const char *path, const struct utimbuf *times);
LIBW32_API int          w32_gethostname (char *name, size_t namelen);
LIBW32_API int          w32_getdomainname (char *name, size_t namelen);

LIBW32_API const char * getlogin (void);
LIBW32_API int          getlogin_r (char *name, size_t namesize);

LIBW32_API void         setprogname (const char *name);
LIBW32_API const char * getprogname (void);

LIBW32_API int          issetugid (void);

LIBW32_API int          w32_getuid (void);
LIBW32_API int          w32_geteuid (void);
LIBW32_API int          w32_getgid (void);
LIBW32_API int          w32_getegid (void);
LIBW32_API int          w32_getgpid (void);

#if defined(WIN32_UNISTD_MAP)
#define getuid()        w32_getuid ()
#define geteuid()       w32_geteuid ()
#define getgid()        w32_getgid ()
#define getegid()       w32_getegid ()
#define getgpid()       w32_getgpid ()
#endif

LIBW32_API int          getgroups (int gidsetsize, gid_t grouplist[]);

/* time.h */
LIBW32_API unsigned int sleep (unsigned int secs);
LIBW32_API unsigned int w32_sleep (unsigned int secs);
LIBW32_API size_t       w32_strftime (char *buf, size_t buflen, const char *fmt, const struct tm *tm);

#if defined(WIN32_UNISTD_MAP)
#define strftime(a,b,c,d) \
                w32_strftime (a, b, c, d)
#endif /*WIN32_UNISTD_MAP*/

/* i/o */
LIBW32_API int          w32_open (const char *path, int, ...);
LIBW32_API int          w32_stat (const char *path, struct stat *sb);
LIBW32_API int          w32_lstat (const char *path, struct stat *sb);
LIBW32_API int          w32_fstat (int fd, struct stat *sb);
LIBW32_API int          w32_read (int fd, void *buffer, size_t cnt);
LIBW32_API int          w32_write (int fd, const void *buffer, size_t cnt);
LIBW32_API int          w32_close (int fd);
LIBW32_API const char * w32_strerror (int errnum);
LIBW32_API int          w32_link (const char *from, const char *to);
LIBW32_API int          w32_unlink (const char *fname);

#if defined(WIN32_UNISTD_MAP)
#define open            w32_open
#define stat(a,b)       w32_stat(a, b)
#define lstat(a,b)      w32_lstat(a, b)
#define fstat(a,b)      w32_fstat(a, b)
#define read(a,b,c)     w32_read(a, b, c)
#define write(a,b,c)    w32_write(a, b, c)
#define close(a)        w32_close(a)
#define link(f,t)       w32_link(f,t)
#define unlink(p)       w32_unlink(p)
#endif /*WIN32_UNISTD_MAP*/

#if defined(WIN32_UNISTD_MAP) || \
    defined(WIN32_SOCKET_MAP_FD) || defined(WIN32_SOCKET_MAP_NATIVE)
#define strerror(a)     w32_strerror(a)
#endif

LIBW32_API int          w32_mkdir (const char *fname, int mode);
LIBW32_API int          w32_chdir (const char *fname);
LIBW32_API int          w32_rmdir (const char *fname);
LIBW32_API char *       w32_getcwd (char *path, int size);
LIBW32_API char *       w32_getcwdd (char drive, char *path, int size);

#if defined(WIN32_UNISTD_MAP)
#define mkdir(d,m)      w32_mkdir(d, m)
#define chdir(d)        w32_chdir(d)
#define rmdir(d)        w32_rmdir(d)
#define getcwd(d,s)     w32_getcwd(d,s)
#define utime(p,t)      w32_utime(p,t)

#if defined(_MSC_VER)
#ifndef vsnprintf
#define vsnprintf       _vsnprintf
#endif
#ifndef snprintf
#define snprintf        _snprintf
#endif
#endif /*_MSC_VER*/
#endif /*WIN32_UNISTD_MAP*/

LIBW32_API int          w32_mkstemp(char *path);
#if defined(_MSC_VER)
LIBW32_API int          mkstemp (char *path);
#endif
LIBW32_API int          w32_mkstempx (char *path);

LIBW32_API int          ftruncate (int fildes, off_t size);
LIBW32_API int          truncate (const char *path, off_t length);

LIBW32_API int          w32_readlink (const char *path, char *name, int sz);
LIBW32_API int          w32_symlink (const char *from, const char *to);

LIBW32_API char *       w32_realpath (const char *path, char *resolved_path /*PATH_MAX*/);

#if defined(WIN32_UNISTD_MAP)
#define readlink(__path,__name, __sz) \
                w32_readlink (__path, __name, __sz)
#define symlink(__from,__to) \
                w32_symlink (__from, __to)
#endif

LIBW32_API int          chown (const char *, uid_t, gid_t);
LIBW32_API int          mknod (const char *path, int mode, int dev);

#if !defined(F_GETFL)
#define F_GETFL                         1
#define F_SETFL                         2
#endif

LIBW32_API int          fcntl (int fd, int ctrl, int);
LIBW32_API int          w32_fsync (int fildes);

/*string.h*/
LIBW32_API char *       strsep (char **stringp, const char *delim);
#if defined(_MSC_VER)
LIBW32_API size_t       strlcat (char *dst, const char *src, size_t siz);
LIBW32_API size_t       strlcpy (char *dst, const char *src, size_t siz);
#if (_MSC_VER <= 1600)
LIBW32_API unsigned long long strtoull (const char * nptr, char ** endptr, int base);
#endif
#endif

__END_DECLS

#endif /*LIBW32_UNISTD_H_INCLUDED*/
