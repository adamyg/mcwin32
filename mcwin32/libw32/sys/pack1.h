/* -*- mode: c; indent-width: 4; -*-
 * $Id: pack1.h,v 1.6 2025/03/06 16:59:48 cvsuser Exp $
 * ==noguard==
 *
 * win32 declaration helpers
 *
 * Copyright (c) 1998 - 2024, Adam Young.
 * All rights reserved.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
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
 *
 * Usage:
 *
 *      #include <sys/cdefs.h>
 *
 *      #include <sys/pack1.h>
 *      struct __packed_pre__ mypackedstruct {
 *              :
 *      } __packed_post__;
 *      #include <sys/pack0.h>
 *
 */

#include <sys/cdefs.h>                          /* __packed_pre__ and __packed_post__ */

#if !(defined(lint) || defined(_lint))
#   if defined(_MSC_VER) && (_MSC_VER >= 800)
#       ifndef __SYS_PACK1_H_INCLUDED__
#           pragma warning(disable:4103)
#       endif
#       pragma pack(1)
#   elif defined(__WATCOMC__) || defined(__BORLANDC__) || defined(__PARADIGM__)
#       pragma pack(1)
#   endif
#endif
#define __SYS_PACK1_H_INCLUDED__
