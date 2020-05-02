/* -*- mode: c; indent-width: 4; -*- */

#include "win32_internal.h"

#include <unistd.h>
#include <errno.h>

#include "w32_trace.h"

#ifndef TRACE_FILE
#define TRACE_FILE      "mctrace.log"
#endif

static int              w32x_tracing_started = 0;
static int              w32x_tracing_init = 0;
static FILE *           w32x_trace_f = NULL;

int                     w32x_tracing_enabled = 1;

static void             w32InitTrace (void);
static void __cdecl     w32EndTrace (void);
static const char*      GetLastErrorText (void);


static void
w32InitTrace(void)
{
    if (! w32x_tracing_started)  {
        if (NULL == (w32x_trace_f = fopen(TRACE_FILE, "wt"))) {
            printf("Midnight Commander[DEBUG]: Can't open trace file '" TRACE_FILE "': %s \n", strerror(errno));

        } else if (! w32x_tracing_init) {
            w32x_tracing_init = 0;
//          atexit(w32EndTrace); 
        }
        w32x_tracing_started = 1;
    }
}


static void __cdecl
w32EndTrace(void)
{
    if (w32x_tracing_started) {
        if (w32x_trace_f) {
            fclose (w32x_trace_f);
            w32x_trace_f = NULL;
        }
        w32x_tracing_started = 0;
    }
}


void
w32_Trace (const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    w32_tracev(fmt, ap);
    va_end(ap);
}


void
w32_trace (const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    w32_tracev(fmt, ap);
    va_end(ap);
}


void
w32_tracev (const char *fmt, va_list ap)
{
    const int buflen = 1024 - 2;                /* buffer - (nl+nul) */
    char buffer[1024];
    int len;

    if (!w32x_tracing_started) {
        w32InitTrace();
    }

    len = _vsnprintf(buffer, buflen, fmt, ap);
    if (len < 0 || len > buflen) len = buflen;  /* error/overflow */
    buffer[len] = 0;

    if (w32x_trace_f && len) {
        if (buffer[len-1] != '\n') {
            fprintf (w32x_trace_f, "%s\n", buffer);
        } else {
            fputs (buffer, w32x_trace_f);
        }
    }

#if defined(WIN32) && defined(_DEBUG)           /* also write Output to Debug monitor */
    if (0 == len || buffer[len-1] != '\n') {
        buffer[len++] = '\n', buffer[len] = 0;  /* newline terminate */
    }
    OutputDebugString(buffer);
#endif  //WIN32
}


void
w32_SetTrace (int trace)
{
    w32x_tracing_enabled = trace;
}


void
w32_TraceOn(void)
{
    w32x_tracing_enabled = 1;
}


void
w32_TraceOff(void)
{
    w32x_tracing_enabled = 0;
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
w32_TraceAPICall(
    const char* name, int line, const char* file)
{
    w32_trace("%s(%d): Call to Win32 API Failed. \"%s\".", file, line, name);
    w32_trace("        System Error (%d): %s. ", GetLastError(), GetLastErrorText());
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
w32_AssertionFailed(
    const char* name, int line, const char* file)
{
    w32_trace("%s(%d): Assertion failed! \"%s\".", file, line, name);
}


/*
 *  Retrieves the text associated with the last system error.
 *
 *  Returns pointer to static buffer. Contents valid till next call
 */
static const char *
GetLastErrorText(void)
{
#define MAX_MSG_SIZE    256
    static char szMsgBuf[MAX_MSG_SIZE];
    DWORD dwError, dwRes;

    dwError = GetLastError();
    dwRes = FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwError,
                MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                szMsgBuf,
                MAX_MSG_SIZE,
                NULL);
    if (0 == dwRes) {
        sprintf (szMsgBuf, "FormatMessage failed with %d", GetLastError());
    }
    return szMsgBuf;
}

/*end*/
