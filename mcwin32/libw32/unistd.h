#ifndef LIBW32_UNISTD_H_INCLUDED
#define LIBW32_UNISTD_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <unistd.h> compat header file -
 *
 *      unistd.h - standard symbolic constants and types
 *
 * Copyright (c) 2007, 2012 - 2015 Adam Young.
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
#if (_MSC_VER != 1600)                          /* MSVC 10/2010 */
#error unistd.h: Untested MSVC C/C++ Version (CL 12.xx - 16.xx) only ...
#endif
#endif
#endif
#pragma warning(disable:4115)

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
#define _WIN32_WINNT        0x400               /* entry level */
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

#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE                /* disable deprecate warnings */
#endif

/*
 *  avoid importing <win32_include.h>
 *      which among others includes <ctype.h>
 */

#if !defined(WIN32_UNISTD_CLEAN)
#define WIN32_UNISTD_MAP
#endif

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */
#include <sys/utypes.h>
#include <sys/stat.h>
#include <time.h>                               /* required to replace strfime() */
#include <utime.h>
#include <stddef.h>                             /* offsetof() */
#include <dirent.h>                             /* MAXPATHLENGTH, MAXNAMELENGTH */
#include <limits.h>                             /* _MAX_PATH */
#include <process.h>                            /* getpid, _beginthread */

#include <stdio.h>                              /* FILE */
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>                             /* memset, memmove ... */
#include <fcntl.h>
#include <io.h>                                 /* write, read ... */


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

#define __S_ISTYPE(mode,mask) \
                        (((mode) & S_IFFMT) == mask)

#ifndef S_ISDIR
#define S_ISDIR(m)      __S_ISTYPE(m, S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(m)      __S_ISTYPE(m, S_IFREG)
#endif
#ifndef S_ISBLK
#define S_ISBLK(m)      __S_ISTYPE(m, S_IFBLK)
#endif
#define S_ISLNK(m)      __S_ISTYPE(m, S_IFLNK)
#define S_ISSOCK(m)     __S_ISTYPE(m, S_IFSOCK)
#define S_ISFIFO(m)     __S_ISTYPE(m, S_IFIFO)


/*stdio.h*/
#if !defined(STDIN_FILENO)
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2
#endif


/*errno.h, also see sys/socket.h*/
/*
 *  Addition UNIX style errno's, plus Windows Sockets errors redefined
 *  as regular Berkeley error constants.
 *
 *  These are normally commented out in winsock.h in Windows NT to avoid conflicts with errno.h.
 *  MSVC 2010 these are defined, plus others EADDRINUSE ... EWOULDBLOCK.
 */

#if !defined(_MSC_VER) || (_MSC_VER < 1600)
#define EIDRM           100
#define EBADRQC         101
#define ENODATA         102
#define ENONET          103
#define ENOTUNIQ        104
#define ECOMM           105
#define ENOLINK         106
#define ENOMEDIUM       107
#define ENOTSUP         108
#define EBADFD          109
#define ENOMSG          110
#define EBADMSG         111

#if !defined(EALREADY)
#define EALREADY        WSAEALREADY
#elif (EALREADY != WSAEALREADY)
#error  Inconsistent EALREADY definition ....
#endif
#endif

#define ENOTINITIALISED WSANOTINITIALISED
#if !defined(EWOULDBLOCK)
#define EWOULDBLOCK     WSAEWOULDBLOCK
#endif
#if !defined(EINPROGRESS)
#define EINPROGRESS     WSAEINPROGRESS
#endif
#if !defined(ENOTSOCK)
#define ENOTSOCK        WSAENOTSOCK
#endif
#if !defined(EDESTADDRREQ)
#define EDESTADDRREQ    WSAEDESTADDRREQ
#endif
#if !defined(EMSGSIZE)
#define EMSGSIZE        WSAEMSGSIZE
#endif
#if !defined(EPROTOTYPE)
#define EPROTOTYPE      WSAEPROTOTYPE
#endif
#if !defined(ENOPROTOOPT)
#define ENOPROTOOPT     WSAENOPROTOOPT
#endif
#if !defined(EPROTONOSUPPORT)
#define EPROTONOSUPPORT WSAEPROTONOSUPPORT
#endif
#if !defined(ESOCKTNOSUPPORT)
#define ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
#endif
#if !defined(EOPNOTSUPP)
#define EOPNOTSUPP      WSAEOPNOTSUPP
#endif
#if !defined(EPFNOSUPPORT)
#define EPFNOSUPPORT    WSAEPFNOSUPPORT
#endif
#if !defined(EAFNOSUPPORT)
#define EAFNOSUPPORT    WSAEAFNOSUPPORT
#endif
#if !defined(EADDRINUSE)
#define EADDRINUSE      WSAEADDRINUSE
#endif
#if !defined(EADDRNOTAVAIL)
#define EADDRNOTAVAIL   WSAEADDRNOTAVAIL
#endif
#if !defined(ENETDOWN)
#define ENETDOWN        WSAENETDOWN
#endif
#if !defined(ENETUNREACH)
#define ENETUNREACH     WSAENETUNREACH
#endif
#if !defined(ENETRESET)
#define ENETRESET       WSAENETRESET
#endif
#if !defined(ECONNABORTED)
#define ECONNABORTED    WSAECONNABORTED
#endif
#if !defined(ECONNRESET)
#define ECONNRESET      WSAECONNRESET
#endif
#if !defined(ENOBUFS)
#define ENOBUFS         WSAENOBUFS
#endif
#if !defined(EISCONN)
#define EISCONN         WSAEISCONN
#endif
#if !defined(ENOTCONN)
#define ENOTCONN        WSAENOTCONN
#endif
#if !defined(ESHUTDOWN)
#define ESHUTDOWN       WSAESHUTDOWN
#endif
#if !defined(ETOOMANYREFS)
#define ETOOMANYREFS    WSAETOOMANYREFS
#endif
#if !defined(ETIMEDOUT)
#define ETIMEDOUT       WSAETIMEDOUT
#endif
#if !defined(ECONNREFUSED)
#define ECONNREFUSED    WSAECONNREFUSED
#endif
#if !defined(ELOOP)
#define ELOOP           WSAELOOP
#endif
#if !defined(ENAMETOOLONG)
#define ENAMETOOLONG    WSAENAMETOOLONG
#endif
#if !defined(EHOSTDOWN)
#define EHOSTDOWN       WSAEHOSTDOWN
#endif
#if !defined(EHOSTUNREACH)
#define EHOSTUNREACH    WSAEHOSTUNREACH
#endif
#if !defined(ENOTEMPTY)
#define ENOTEMPTY       WSAENOTEMPTY
#endif
#define EPROCLIM        WSAEPROCLIM
#define EUSERS          WSAEUSERS
#define EDQUOT          WSAEDQUOT
#define ESTALE          WSAESTALE
#define EREMOTE         WSAEREMOTE

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


int                     sigemptyset (sigset_t *);
int                     sigaction (int, struct sigaction *, struct sigaction *);
#endif

/*shell support*/
#if !defined(WNOHANG)
#define WNOHANG         1
#endif

int                     w32_waitpid (int, int *, int);
int                     w32_kill (int pid, int sig);

#if defined(WIN32_UNISTD_MAP)
#define                 kill(__pid, __val) \
                w32_kill(__pid, __val)
#endif /*WIN32_UNISTD_MAP*/

#if !defined(WEXITSTATUS)
int                     WEXITSTATUS (int status);
int                     WIFEXITED (int status);
int                     WIFSIGNALED (int status);
int                     WTERMSIG (int status);
int                     WCOREDUMP (int status);
#endif

/* <stdlib.h> */
extern int              getsubopt (char **optionp, char * const *tokens, char **valuep);

/* <string.h> */
#if !defined(__WATCOMC__)
extern int              strcasecmp (const char *s1, const char *s2);
extern int              strncasecmp (const char *s1, const char *s2, size_t len);
#endif
#if defined(_MSC_VER) &&(_MSC_VER < 1400)
extern int              strnlen (const char *s, size_t maxlen);
#endif

/* <unistd.h> */
unsigned int            sleep (unsigned int secs);
int                     gettimeofday (struct timeval *tv, struct timezone *tz);
int                     w32_utime (const char *path, const struct utimbuf *times);
int                     w32_gethostname (char *name, size_t namelen);

#if defined(WIN32_UNISTD_MAP)
#if !defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
#define gethostname(__name,__namelen) \
                w32_gethostname (__name, __namelen)
#endif
#endif /*WIN32_UNISTD_MAP*/

const char *            getlogin (void);
int                     getlogin_r (char *name, size_t namesize);

int                     w32_getuid (void);
int                     w32_geteuid (void);
int                     w32_getgid (void);
int                     w32_getegid (void);
int                     w32_getgpid (void);

#if defined(WIN32_UNISTD_MAP)
#define getuid()        w32_getuid ()
#define geteuid()       w32_geteuid ()
#define getgid()        w32_getgid ()
#define getegid()       w32_getegid ()
#define getgpid()       w32_getgpid ()
#endif

int                     getgroups (int gidsetsize, gid_t grouplist[]);

/* time.h */
size_t                  w32_strftime (char *buf, size_t buflen, const char *fmt, const struct tm *tm);

#if defined(WIN32_UNISTD_MAP)
#define strftime(a,b,c,d) \
                w32_strftime (a, b, c, d)
#endif /*WIN32_UNISTD_MAP*/

/* i/o */
int                     w32_open (const char *path, int, ...);
int                     w32_stat (const char *path, struct stat *sb);
int                     w32_lstat (const char *path, struct stat *sb);
int                     w32_fstat (int fd, struct stat *sb);
int                     w32_read (int fd, void *buffer, unsigned int cnt);
int                     w32_write (int fd, const void *buffer, unsigned int cnt);
int                     w32_close (int fd);
const char *            w32_strerror (int errnum);
int                     w32_link (const char *from, const char *to);
int                     w32_unlink (const char *fname);

#if defined(WIN32_UNISTD_MAP)
#define open            w32_open
#define stat(a,b)       w32_stat(a, b)
#define lstat(a,b)      w32_lstat(a, b)
#define fstat(a,b)      w32_fstat(a, b)
#define read(a,b,c)     w32_read(a, b, c)
#define write(a,b,c)    w32_write(a, b, c)
#define close(a)        w32_close(a)
#define strerror(a)     w32_strerror(a)
#define g_strerror(a)   w32_strerror(a)         /* must also replace libglib version */
#define link(f,t)       w32_link(f,t)
#define unlink(p)       w32_unlink(p)
#endif /*WIN32_UNISTD_MAP*/

int                     w32_mkdir (const char *fname, int mode);
int                     w32_chdir (const char *fname);
int                     w32_rmdir (const char *fname);
char *                  w32_getcwd (char *path, int size);
char *                  w32_getcwdd (char drive, char *path, int size);

#if defined(WIN32_UNISTD_MAP)
#define mkdir(d,m)      w32_mkdir(d, m)
#define chdir(d)        w32_chdir(d)
#define rmdir(d)        w32_rmdir(d)
#define getcwd(d,s)     w32_getcwd(d,s)
#define utime(p,t)      w32_utime(p,t)

#if defined(_MSC_VER)
#define vsnprintf       _vsnprintf
#define snprintf        _snprintf
#endif
#endif /*WIN32_UNISTD_MAP*/

#if defined(__WATCOMC__)
int                     w32_mkstemp (char *path);
#else
int                     mkstemp (char *path);
#endif
int                     w32_mkstempx (char *path);

int                     ftruncate (int fildes, off_t size);
int                     truncate (const char *path, off_t length);

int                     w32_readlink (const char *path, char *name, int sz);
int                     w32_symlink (const char *from, const char *to);

#if defined(WIN32_UNISTD_MAP)
#define readlink(__path,__name, __sz) \
                w32_readlink (__path, __name, __sz)
#define symlink(__from,__to) \
                w32_symlink (__from, __to)
#endif

int                     chown (const char *, uid_t, gid_t);
int                     mknod (const char *path, int mode, int dev);

#if !defined(F_GETFL)
#define F_GETFL                         1
#define F_SETFL                         2
#endif

int                     fcntl (int fd, int ctrl, int);
int                     w32_fsync (int fildes);


/*string.h*/
#if defined(_MSC_VER)
size_t                  strlcat(char *dst, const char *src, size_t siz);
size_t                  strlcpy(char *dst, const char *src, size_t siz);

#if (_MSC_VER <= 1600)
unsigned long long      strtoull(const char * nptr, char ** endptr, int base);
unsigned long           strtoul(const char * nptr, char ** endptr, int base);
#endif
#endif

__END_DECLS

#endif /*LIBW32_UNISTD_H_INCLUDED*/
