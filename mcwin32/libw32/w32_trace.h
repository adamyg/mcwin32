#ifndef WIN32_TRACE_H_INCLUDED
#define WIN32_TRACE_H_INCLUDED
/* -*- mode: c; tabs: 4 -*- */
/*
 *  win32 debugging support
 *
 *      win32Trace(x) -
 *           Trace macro. Use double in parenthesis for x. Same args as printf.
 *
 *      win32ASSERT(x) -
 *           assert macro, but will not abort program and output sent to trace routine.
 *
 *      win32APICALL(x) -
 *           Use to enclose a Win32 system call that should return TRUE.
 *
 *      win32APICALL_HANDLE(h,x) -
 *           Use to enclose a Win32 system call that should return a handle.
 */

#include <stdarg.h>

__BEGIN_DECLS

LIBW32_API extern int   w32x_tracing_enabled;

LIBW32_API void         w32_Trace (const char *, ...);
LIBW32_API void         w32_trace (const char *, ...);
LIBW32_API void         w32_tracev (const char *, va_list);
LIBW32_API void         w32_TraceAPICall (const char *name, int line, const char *file);
LIBW32_API void         w32_AssertionFailed (const char *name, int line, const char *file);

LIBW32_API void         w32_SetTrace (int trace);
LIBW32_API void         w32_TraceOn (void);
LIBW32_API void         w32_TraceOff (void);

__END_DECLS

#ifdef HAVE_TRACE

#define win32Trace(x)   if (w32x_tracing_enabled) w32_Trace x;

#define win32ASSERT(x)  if (!(x)) w32_AssertionFailed(#x, __LINE__, __FILE__);

#define win32APICALL(x) if (!(x)) w32_TraceAPICall(#x, __LINE__, __FILE__);

#define win32APICALL_HANDLE(h,x) \
            h=x; if (INVALID_HANDLE_VALUE == h) \
                            w32_TraceAPICall(#h" = "#x, __LINE__, __FILE__);

#define win32APICALL_RC(r,x) \
            r=x; if (!r) w32_TraceAPICall(#r" = "#x, __LINE__, __FILE__);

#define win32APIERROR(x) \
            w32_TraceAPICall(#x, __LINE__, __FILE__);

#include <sys/cdefs.h>

#define SetTrace        w32_SetTrace
#define TraceOn         w32_TraceOn
#define TraceOff        w32_TraceOff

#else
#define win32Trace(x)
#define win32ASSERT(x)
#define win32APICALL(__call) __call
#define win32APIRC(x, rc)
#define win32APIERROR(x)
#define win32APICALL_HANDLE(__handle,__api) __handle=__api;

#define SetTrace(x)
#define TraceOn()
#define TraceOff()
#endif

#endif  /*WIN32_TRACE_H_INCLUDED*/

