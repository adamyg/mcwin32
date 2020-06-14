/*-
 * Copyright (c) 2000 Citrus Project,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  $NetBSD: libintl.h,v 1.4 2011/10/14 22:42:01 joerg Exp $
 */

#ifndef __LIBINTL_H_DEFINED__
#define __LIBINTL_H_DEFINED__

#include <sys/cdefs.h>
#ifndef __format_arg
#define __format_arg(__cnt)	/**/
#endif
#ifndef __packed
#define __packed		/**/
#endif

#if defined(LIBINTL_STATIC)
#   define LIBINTL_LINKAGE
#   define LIBINTL_ENTRY
#elif defined(_WIN32)
#   if defined(__LIBINTL_BUILD)
#       define LIBINTL_LINKAGE __declspec(dllexport)
#   else
#       define LIBINTL_LINKAGE __declspec(dllimport)
#   endif
#   define LIBINTL_ENTRY __cdecl
#else
#   define LIBINTL_LINKAGE
#   define LIBINTL_ENTRY
#endif

__BEGIN_DECLS
LIBINTL_LINKAGE char * LIBINTL_ENTRY gettext(const char *) __format_arg(1);
LIBINTL_LINKAGE char * LIBINTL_ENTRY dgettext(const char *, const char *) __format_arg(2);
LIBINTL_LINKAGE char * LIBINTL_ENTRY dcgettext(const char *, const char *, int) __format_arg(2);
LIBINTL_LINKAGE char * LIBINTL_ENTRY ngettext(const char *, const char *, unsigned long int)
			                    __format_arg(1) __format_arg(2);
LIBINTL_LINKAGE char * LIBINTL_ENTRY dngettext(const char *, const char *, const char *, unsigned long int)
			                    __format_arg(2) __format_arg(3);
LIBINTL_LINKAGE char * LIBINTL_ENTRY dcngettext(const char *, const char *, const char *, unsigned long int, int) 
			                    __format_arg(2) __format_arg(3);
LIBINTL_LINKAGE char * LIBINTL_ENTRY textdomain(const char *);
LIBINTL_LINKAGE char * LIBINTL_ENTRY bindtextdomain(const char *, const char *);
LIBINTL_LINKAGE char * LIBINTL_ENTRY bind_textdomain_codeset(const char *, const char *);
__END_DECLS

#endif /*__LIBINTL_H_DEFINED__*/
