#ifndef LIBW32_LIBGEN_H_INCLUDED
#define LIBW32_LIBGEN_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libgen_h,"$Id: libgen.h,v 1.6 2024/01/16 15:17:51 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <libgen.h> implementation
 *
 * Copyright (c) 2007, 2012 - 2024 Adam Young.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 * or (at your option) any later version.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 */

#include <sys/cdefs.h>

__BEGIN_DECLS

LIBW32_API char *	w32_basename (char *path);
LIBW32_API char *	w32_dirname (char *path);

__END_DECLS

#endif /*LIBW32_LIBGEN_H_INCLUDED*/
