#ifndef GR_DLFCN_H_INCLUDED
#define GR_DLFCN_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_dlfcn_h,"$Id: dlfcn.h,v 1.2 2021/11/30 13:06:19 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * <dlfcn.h> for windows
 *
 * Copyright (c) 1998 - 2021, Adam Young.
 * All rights reserved.
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

#include <sys/cdefs.h>
#include <wchar.h>

__BEGIN_DECLS

#define RTLD_LAZY       0x01    /* relocations are performed at an implementation-defined time. */
#define RTLD_NOW        0x02    /* relocations are performed when the object is loaded. */

#define RTLD_GLOBAL     0x04    /* all symbols are available for relocation processing of other modules. */
#define RTLD_LOCAL      0x08    /* all symbols are not made available for relocation processing by other modules. */

LIBW32_API void *       dlopen(const char *file, int mode);
LIBW32_API void *       dlopenA(const char *file, int mode);
LIBW32_API void *       dlopenW(const wchar_t *file, int mode);
LIBW32_API void *       dlsym(void *__restrict handle, const char *__restrict name);
LIBW32_API int          dlclose(void *handle);
LIBW32_API char *       dlerror(void);

__END_DECLS

#endif /*GR_DLFCN_H_INCLUDED*/
