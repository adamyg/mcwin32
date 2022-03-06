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

#include <sys/cdefs.h>
#include <stdarg.h>

__BEGIN_DECLS

extern void             OutputDebugPrintA (const char *fmt, ...);
extern void             OutputDebugPrintW (const wchar_t *fmt, ...);

extern void             w32_trace (const char *, ...);
extern void             w32_tracev (const char *, va_list);
extern void             w32_trace_api (const char *name, unsigned line, const char *file);
extern void             w32_assertion (const char *name, unsigned line, const char *file);

extern int              w32_trace_enabled (void);
extern void             w32_trace_set (int trace);
extern void             w32_trace_on (void);
extern void             w32_trace_off (void);

__END_DECLS

#ifdef HAVE_TRACE

#define win32Trace(x)   { if (w32_trace_enabled()) w32_trace x; }
#define win32Tracev(x)  { if (w32_trace_enabled()) w32_tracev x; }

#define win32ASSERT(x)  { if (!(x)) w32_assertion(#x, __LINE__, __FILE__); }

#define win32API_CALL(__call) \
            { if (!(__call)) w32_trace_api(#__call, __LINE__, __FILE__); }

#define win32API_CALL_HANDLE(__handle,__api) \
            { __handle=__api; if (INVALID_HANDLE_VALUE == __handle) w32_trace_api(#__handle" = "#__api, __LINE__, __FILE__); }

#define win32API_CALL_RC(r,x) \
            { r=x; if (!r) w32_trace_api(#r" = "#x, __LINE__, __FILE__); }

#define win32API_ERROR(x) \
            w32_trace_api(#x, __LINE__, __FILE__);

#define win32TraceSet   w32_trace_set
#define win32TraceOn    w32_trace_on
#define win32TraceOff   w32_trace_off

#else

#define win32Trace(x)
#define win32Tracev(x)
#define win32ASSERT(x)
#define win32API_CALL(__call) __call
#define win32API_CALL_HANDLE(__handle,__api) __handle=__api;
#define win32API_RC(x, rc)
#define win32API_ERROR(x)
#define win32TraceSet()
#define win32TraceOn() 
#define win32TraceOff()

#endif

#endif  /*WIN32_TRACE_H_INCLUDED*/
