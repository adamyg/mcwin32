#ifndef LIBW32_NETDB_H_INCLUDED
#define LIBW32_NETDB_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_netdb_h,"$Id: netdb.h,v 1.12 2025/02/16 12:04:04 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <netdb.h> implementation
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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

#include <sys/socket.h>                         /* winsock etc */
#include <sys/cdefs.h>

__BEGIN_DECLS

#if (defined(_MSC_VER) && (_MSC_VER < 1400)) || \
	defined(__WATCOMC__)
LIBW32_API const char * gai_strerror(int ecode);
#endif
LIBW32_API const char * w32_gai_strerror(int ecode);

__END_DECLS

#endif /*LIBW32_NETDB_H_INCLUDED*/
