#pragma once
#ifndef LIBCOMPACT_H_INCLUDED
#define LIBCOMPACT_H_INCLUDED
//
//  libcompat
//

#include "w32config.h"

#if defined(LIBCOMPAT_SOURCE)
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <sys/cdefs.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#include <unistd.h>

__BEGIN_DECLS

//#if !defined(HAVE_STRNLEN)
//extern size_t strnlen(const char *str, size_t maxlen);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_STRNLEN
//#endif
//#endif /*HAVE_STRNLEN*/

#if !defined(HAVE_STRNDUP)
extern char *strndup(const char *str, size_t maxlen);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_STRNDUP
#endif
#endif /*HAVE_STRNDUP*/

//#if !defined(HAVE_STRCATN)
//extern char *strcatn(char *s1, char *s2, int n);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_STRCATN
//#endif
//#endif /*HAVE_STRCATN*/

//#if !defined(HAVE_STRLCPY)
//extern size_t strlcpy(char *dst, const char *src, size_t siz);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_STRLCPY
//#endif
//#endif /*HAVE_STRLCPY*/

//#if !defined(HAVE_STRLCAT)
//extern size_t strlcat(char *dst, const char *src, size_t siz);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_STRLCAT
///#endif
//#endif /*HAVE_STRLCAT*/

//#if !defined(HAVE_STRSEP)
//extern char *strsep(char **stringp, const char *delim);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_STRSEP
//#endif
//#endif /*HAVE_STRSEP*/

#if !defined(HAVE_STRTONUM) /*libbsd*/
extern long long strtonum(const char *numstr, long long minval, long long maxval, const char **errstrp);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_STRTONUM
#endif
#endif /*HAVE_STRTONUM*/

//#if !defined(HAVE_STRCASECMP)
//extern int strcasecmp(const char *s1, const char *s2);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_STRCASECMP
//#endif
//#endif /*HAVE_STRCASECMP*/

//#if !defined(HAVE_STRNCASECMP)
//extern int strncasecmp(const char *s1, const char *s2, size_t len);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_STRNCASECMP
//#endif
//#endif /*HAVE_STRNCASECMP*/

//#if !defined(HAVE_STRTOK_R)
//extern char * strtok_r(char *s, const char *delim, char **lasts);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_STRTOK_R
//#endif
//#endif /*HAVE_STRTOK_R*/

#if !defined(HAVE_BCOPY)
extern void bcopy(const void *s1, void *s2, size_t n);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_BCOPY
#endif
#endif /*HAVE_BCOPY*/

#if !defined(HAVE_BZERO)
extern void bzero(void *s, size_t len);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_BZERO
#endif
#endif /*HAVE_BZERO*/

#if !defined(HAVE_EXPLICIT_BZERO)
extern void explicit_bzero(void *s, size_t len);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_EXPLICIT_BZERO
#endif
#endif /*HAVE_EXPLICIT_BZERO*/

//#if !defined(HAVE_MEMMEM)
//extern void *memmem(const void *h0, size_t k, const void *n0, size_t l);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_MEMMEM
//#endif
//#endif /*HAVE_MEMMEM*/

//#if !defined(HAVE_PUTW)
//extern int putw(int w, FILE *fp);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_PUTW
//#endif
//#endif /*HAVE_PUTW*/

//#if !defined(HAVE_GETW)
//extern int getw(FILE *fp);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_GETW
//#endif
//#endif /*HAVE_GETW*/

//#if !defined(HAVE_INDEX)
//extern char *index(const char *s, int c);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_INDEX
//#endif
//#endif /*HAVE_INDEX*/

//#if !defined(HAVE_RINDEX)
//extern char *rindex(const char *s, int c);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_RINDEX
//#endif
//#endif /*HAVE_RINDEX*/

#if !defined(HAVE_MKSTEMP)
extern int mkstemp(char *path);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_MKSTEMP
#endif
#endif /*HAVE_MKSTEMP*/

#if !defined(HAVE_MKTEMP)
extern char *mktemp(char *path);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_MKTEMP
#endif
#endif /*HAVE_MKTEMP*/
extern char *xmktemp(char *path, char *result, size_t length); /*extension*/

//#if !defined(HAVE_BASENAME)     /*libgen.h*/
//extern char *basename(char *path);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_BASENAME
//#endif
//#endif /*HAVE_BASENAME*/

//#if !defined(HAVE_DIRNAME)      /*libgen.h*/
//extern char *dirname(char *path);
//#if !defined(LIBCOMPAT_SOURCE)
//#define HAVE_DIRNAME
//#endif
//#endif /*HAVE_DIRNAME*/

#if !defined(HAVE_REALLOCARRAY)
extern void *reallocarray(void *optr, size_t nmemb, size_t size);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_REALLOCARRAY
#endif
#endif /*HAVE_REALLOCARRAY*/

#if !defined(HAVE_RECALLOCARRAY)
extern void *recallocarray(void *ptr, size_t oldnmemb, size_t newnmemb, size_t size);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_RECALLOCARRAY
#endif
#endif /*HAVE_RECALLOCARRAY*/

#if !defined(HAVE_ASPRINTF)     /*stdio.h*/
extern int asprintf(char **str, const char *fmt, ...);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_REALLOCARRAY
#endif
#endif /*HAVE_ASPRINTF*/

#if !defined(HAVE_VASPRINTF)    /*stdio.h*/
extern int vasprintf(char **str, const char *fmt, va_list ap);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_VASPRINTF
#endif
#endif /*HAVE_VASPRINTF*/

#if !defined(HAVE_TIMEGM)       /*unistd.h*/
struct tm;
extern time_t timegm(struct tm *tm);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_TIMEGM
#endif
#endif /*HAVE_TIMEGM*/

#if !defined(HAVE_LOCALTIME_R)  /*unistd.h*/
extern struct tm *localtime_r(const time_t *timep, struct tm *result);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_LOCALTIME_R
#endif
#endif /*HAVE_TIMEGM*/

//extern char *xmktemp(char *path, char *result, size_t length);

#if !defined(HAVE_GETLINE)
extern int /*ssize_t*/ getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp);
extern int /*ssize_t*/ getline(char **buf, size_t *bufsiz, FILE *fp);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_GETLINE
#endif
#endif

#if !defined(HAVE_FGETLN)
extern char *fgetln(FILE *fp, size_t *len);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_FGETLN
#endif
#endif /*HAVE_FGETLN*/

#if !defined(HAVE_ALPHASORT)
struct dirent;
extern int alphasort(const struct dirent **d1, const struct dirent **d2);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_ALPHASORT
#endif
#endif /*HAVE_ALPHASORT*/

#if !defined(HAVE_SCANDIR)
struct dirent;
extern int scandir(const char *dirname, struct dirent ***namelist, int (*select)(const struct dirent *), int (*dcomp)(const struct dirent **, const struct dirent **));
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_SCANDIR
#endif
#endif /*HAVE_SCANDIR*/

#if !defined(HAVE_QSORT_R)
extern void qsort_r(void *a, size_t n, size_t es, void *thunk, int (*cmp)(void *, const void *, const void *));
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_QSORT_R
#endif
#endif /*HAVE_QSORT_R*/
    
__END_DECLS

#endif /*LIBCOMPACT_H_INCLUDED*/
