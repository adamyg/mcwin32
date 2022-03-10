/* -*- mode: c; indent-width: 4; -*- */

#include "win32_internal.h"
#include "win32_trace.h"

#include <sys/time.h>
#include <unistd.h>
#include <errno.h>


#ifndef TRACE_FILE
#define TRACE_FILE      "mctrace.log"
#endif

static int              x_trace_enabled = 1;
static int              x_tracing_started = 0;
static int              x_tracing_init = 0;
static FILE *           x_trace_file = NULL;

static void             w32_trace_init(void);
static void __cdecl     w32_trace_close(void);

static const char *     GetErrorText(DWORD dwError, char *buffer, unsigned buflen);


void
OutputDebugPrintA(const char *fmt, ...)
{
    struct timeval tv;
    char out[512];
    va_list ap; 
    int prefix;

    va_start(ap, fmt);
    w32_gettimeofday(&tv, NULL);
    prefix = sprintf(out, "%lu.%03u: ", tv.tv_sec, tv.tv_usec / 1000);
    vsprintf_s(out + prefix, _countof(out) - prefix, fmt, ap);
    va_end(ap); 
    OutputDebugStringA(out);
}


void
OutputDebugPrintW(const wchar_t *fmt, ...)
{
    struct timeval tv;
    wchar_t out[512];
    va_list ap; 
    int prefix;

    va_start(ap, fmt);
    w32_gettimeofday(&tv, NULL);
    prefix = swprintf(out, _countof(out), L"%lu.%03u: ", tv.tv_sec, tv.tv_usec / 1000);
    vswprintf(out + prefix, _countof(out) - prefix, fmt, ap);
    out[_countof(out) - 1] = 0;
    va_end(ap); 
    OutputDebugStringW(out);
}


static void
w32_trace_init(void)
{
    if (! x_tracing_started) {
#if !defined(_DEBUG)
        if (NULL == getenv("MC_TRACE")) {
            return; //optional
        }
#endif  //_DEBUG

        if (NULL == (x_trace_file = fopen(TRACE_FILE, "wt"))) {
            printf("Midnight Commander[DEBUG]: cannot open trace file '" TRACE_FILE "': %s \n", strerror(errno));

        } else if (! x_tracing_init) {
            x_tracing_init = 0;
            // atexit(w32_trace_close);
        }
        x_tracing_started = 1;
    }
}


static void __cdecl
w32_trace_close(void)
{
    if (x_trace_file) {
        fclose (x_trace_file);
        x_trace_file = NULL;
    }
    x_tracing_started = 0;
}


int
w32_trace_enabled(void)
{
    return x_trace_enabled;
}


void
w32_trace(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    w32_tracev(fmt, ap);
    va_end(ap);
}


void
w32_tracev(const char *fmt, va_list ap)
{
    const int buflen = 1024 - 2;                /* buffer - (nl+nul) */
    char buffer[1024];
    int len;

    if (!x_tracing_started) {
        w32_trace_init();
    }

    len = _vsnprintf(buffer, buflen, fmt, ap);
    if (len < 0 || len > buflen) len = buflen;  /* error/overflow */
    buffer[len] = 0;

    if (x_trace_file && len) {
        if (buffer[len-1] != '\n') {
            fprintf(x_trace_file, "%s\n", buffer);
        } else {
            fputs(buffer, x_trace_file);
        }
    }

#if defined(_DEBUG)                             /* also write Output to debug monitor */
    if (0 == len || buffer[len-1] != '\n') {
        buffer[len++] = '\n', buffer[len] = 0;  /* newline terminate */
    }
    OutputDebugStringA(buffer);
#endif  //WIN32
}


void
w32_trace_set(int trace)
{
    x_trace_enabled = trace;
}


void
w32_trace_on(void)
{
    x_trace_enabled = 1;
}


void
w32_trace_off(void)
{
    x_trace_enabled = 0;
}


/*
 *  Report a System call failure.
 *
 *      name - text containing the source code that called the offending API func
 *      line, file - place of "name" in code
 *
 *  See Also:  definition of win32APICALL macro.
 */
void
w32_trace_api(const char *name, unsigned line, const char *file)
{
    const DWORD dwError = GetLastError();
    char buffer[256];

    w32_trace("%s(%u): Win32 API failed. \"%s\".", file, line, name);
    w32_trace("  Error (%u): %s", (unsigned)dwError, GetErrorText(dwError, buffer, sizeof(buffer)));
}


/*
 *  Report a logical condition failure. (e.g. a bad argument to a func)
 *
 *      name - text containing the logical condition
 *      line, file - place of "name" in code
 *
 *  See Also:  definition of ASSERT macro.
 */
void
w32_assertion(const char *name, unsigned line, const char* file)
{
    w32_trace("%s(%u): Assertion failed! \"%s\".", file, line, name);
}


/*
 *  Retrieves the text associated with the last system error.
 *
 *  Returns pointer to static buffer. Contents valid till next call
 */
static const char *
GetErrorText(DWORD dwError, char *buffer, unsigned buflen)
{
    const DWORD rc = FormatMessageA(
                        FORMAT_MESSAGE_FROM_SYSTEM,  NULL,  dwError,
                        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                        buffer, buflen, NULL);
    if (0 == rc) {
        _snprintf(buffer, buflen - 1, "unknown error");
    }
    return buffer;
}

/*end*/
