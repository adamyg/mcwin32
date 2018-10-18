#ifndef LIBW32_MSVC_UTIME_H_INCLUDED
#define LIBW32_MSVC_UTIME_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_msvc_utime_h,"$Id: utime.h,v 1.4 2018/09/29 02:22:55 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */

#if (_MSC_VER >= 1800)
#include <sys/utime.h>
#endif

#endif /*LIBW32_MSVC_UTIME_H_INCLUDED*/
