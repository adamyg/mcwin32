/* -*- mode: c; indent-width: 4; -*- */

#include "win32_internal.h"
#include <unistd.h>
#include <errno.h>
#include "w32_trace.h"

#define TRACE_FILE      "mctrace.log"

static int              w32x_tracing_started = 0;
static FILE *           w32x_trace_f = NULL;

int                     w32x_tracing_enabled = 1;

static void             w32InitTrace (void);
static void             w32EndTrace (void);
static const char*      GetLastErrorText (void);
static char *           visbuf (const char *buf);


static void
w32InitTrace(void)
{
    if (! w32x_tracing_started)  {
        if (NULL != (w32x_trace_f = fopen(TRACE_FILE, "wt"))) {
            printf("Midnight Commander[DEBUG]: Can't open trace file '" TRACE_FILE "': %s \n", strerror(errno));
        }
        atexit (&w32EndTrace);
        w32x_tracing_started = 1;
    }
}


static void
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
    char buffer[1024];
    char *vp;

    if (!w32x_tracing_started) {
        w32InitTrace();
    }

    va_start(ap, fmt);
    _vsnprintf(buffer, sizeof(buffer), fmt, ap);
    buffer[sizeof(buffer)-1]=0;
    vp = buffer;
#ifdef WIN32                                    /* Write Output to Debug monitor also */
    OutputDebugString (vp);
#if !defined(_MSC_VER) || (_MSC_VER > 800)
        OutputDebugString ("\n");
#endif
#endif

    if (w32x_trace_f) {
        fprintf (w32x_trace_f, "%s\n", vp);
    }
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
    w32_Trace("%s(%d): Call to Win32 API Failed. \"%s\".", file, line, name);
    w32_Trace("        System Error (%d): %s. ", GetLastError(), GetLastErrorText());
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
    w32_Trace("%s(%d): Assertion failed! \"%s\".", file, line, name);
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
