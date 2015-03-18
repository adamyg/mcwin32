/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 gi_strerror()
 *
 * Copyright (c) 2007, 2012, Adam Young.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained 
 * online at http://www.opengroup.org/unix/online.html.
 * ==end==
 */

#include "win32_internal.h"
#include <netdb.h>

/*
//  NAME
//      gai_strerror - address and name information error description
//  
//  SYNOPSIS
//      #include <netdb.h>
//      const char *gai_strerror(int ecode);
//  
//  DESCRIPTION
//  
//      The gai_strerror() function shall return a text string describing an error value
//      for the getaddrinfo() and getnameinfo() functions listed in the <netdb.h> header.
//  
//      When the ecode argument is one of the following values listed in the <netdb.h>
//      header:
//  
//          [EAI_AGAIN]
//          [EAI_BADFLAGS]
//          [EAI_FAIL]
//          [EAI_FAMILY]
//          [EAI_MEMORY]
//          [EAI_NONAME]
//          [EAI_OVERFLOW]
//          [EAI_SERVICE]
//          [EAI_SOCKTYPE]
//          [EAI_SYSTEM]
//  
//      the function return value shall point to a string describing the error. If the
//      argument is not one of those values, the function shall return a pointer to a
//      string whose contents indicate an unknown error.
//  
//  RETURN VALUE
//      Upon successful completion, gai_strerror() shall return a pointer to an
//      implementation-defined string.
//  
//  ERRORS
//      No errors are defined.
*/

const char *
#if (defined(_MSC_VER) && (_MSC_VER < 1400)) || \
	defined(__WATCOMC__)
gai_strerror(int ecode)
#else
w32_gai_strerror(int ecode)
#endif
{
    switch (ecode) {
#if defined(EAI_ADDRFAMILY)
    case EAI_ADDRFAMILY:    return "address family for host not supported";       
#endif
    case EAI_AGAIN:         return "temporary failure in name resolution";     
    case EAI_BADFLAGS:      return "invalid flags value";      
    case EAI_FAIL:          return "non-recoverable failure in name resolution";       
    case EAI_FAMILY:        return "address family not supported";     
    case EAI_MEMORY:        return "memory allocation failure";        
#if defined(EAI_NOSECURENAME)
    case EAI_NOSECURENAME:  return "no such host is known securely";
#endif
    case EAI_NONAME:        return "host nor service provided, or not known";
    case EAI_SERVICE:       return "service not supported for socket type";        
    case EAI_SOCKTYPE:      return "socket type not supported";
#if defined(EAI_IPSECPOLICY)
    case EAI_IPSECPOLICY:   return "name based IPSEC policy could not be added";
#endif
#if defined(EAI_SYSTEM)
    case EAI_SYSTEM:        return "system error";
#endif
#if defined(EAI_NODATA) && (EAI_NODATA != EAI_NONAME)
    case EAI_NODATA:        return "no address associated with host";      
#endif
    case WSANOTINITIALISED: return "winsock is not initialised";
    default:
        break;
    }
    return "unknown error, gai_strerror";
}
/*end*/


