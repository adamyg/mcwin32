/*	$NetBSD: citrus_types.h,v 1.3 2003/10/27 00:12:42 lukem Exp $	*/

/*-
 * Copyright (c)2003 Citrus Project,
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
 */

#ifndef _CITRUS_TYPES_H_
#define _CITRUS_TYPES_H_

#if HAVE_NBTOOL_CONFIG_H
#include "nbtool_config.h"
#endif

typedef uint32_t	_citrus_wc_t;
typedef uint32_t	_citrus_index_t;
typedef uint32_t	_citrus_csid_t;
#define _CITRUS_CSID_INVALID	((_citrus_csid_t)-1)

#if defined(__WATCOMC__) || defined(_MSC_VER) || defined(__MINGW32__)
#define WCHAR_T		_citrus_wc_t
#elif defined(SIZEOF_WCHAR_T)
#if (SIZEOF_WCHAR_T >= 4)
#define WCHAR_T		wchar_t
#else
#define WCHAR_T		_citrus_wc_t
#endif
#else
#error  Unknown wchar_t definition ...
#endif

#endif
