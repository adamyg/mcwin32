#ifndef LIBW32_NETDB_H_INCLUDED
#define LIBW32_NETDB_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_netdb_h,"$Id: netdb.h,v 1.7 2022/02/17 16:04:58 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <netdb.h> implementation
 *
 * Copyright (c) 2007, 2012 - 2022 Adam Young.
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
