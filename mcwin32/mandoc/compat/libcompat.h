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

#if !defined(HAVE_ASPRINTF)     /*stdio.h*/
extern int asprintf(char **str, const char *fmt, ...);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_ASPRINTF 1
#endif
#endif /*HAVE_ASPRINTF*/

#if !defined(HAVE_VASPRINTF)    /*stdio.h*/
extern int vasprintf(char **str, const char *fmt, va_list ap);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_VASPRINTF 1
#endif
#endif /*HAVE_VASPRINTF*/

#if !defined(HAVE_ISBLANK) && !defined(isblank)
extern int isblank(int ch);
#if !defined(LIBCOMPAT_SOURCE)
#define HAVE_ISBLANK 1
#endif
#endif

__END_DECLS

#endif /*LIBCOMPACT_H_INCLUDED*/
