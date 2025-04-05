/* -*- mode: c; indent-width: 4; -*- */
/*
   WIN32 pipe

        mc_popen
        mc_pread
        mc_pclose

   Adam Young 2015 - 2025

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <config.h>

#include "libw32.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>                             /* struct sigaction */
#include <limits.h>                             /* INT_MAX */
#include <malloc.h>

#include <sys/stat.h>
#include <stdarg.h>
#include <errno.h>                              /* errno */
#include <string.h>
#include <ctype.h>

#include "lib/global.h"
#include "lib/util.h"

#include "win32_utl.h"
#include "win32_trace.h"

static const char bin_sh[] = "/bin/sh";

static mc_pipe_t *pipe_open (const char *xcommand, gboolean read_out, gboolean read_err, gboolean write_in, GError ** error);
static gboolean pipe_read (mc_pipe_t *p, gboolean fdout, gboolean fderr, GError **error);


/**
 *  Create pipe and run child process.
 *
 *  @parameter command command line of child process
 *  @paremeter error contains pointer to object to handle error code and message
 *
 *  @return newly created object of mc_pipe_t class in success, NULL otherwise
 */

mc_pipe_t *
mc_popen (const char *command, gboolean read_out, gboolean read_err, GError ** error)
{
    return pipe_open (command, read_out, read_err, FALSE, error);
}


int
mc_popen2 (const char *command, int *fds, GError **error)
{
    mc_pipe_t *p;

    if (NULL != (p = pipe_open(command, TRUE, FALSE, TRUE, error))) {
        win32_exec_t *args = (win32_exec_t *)(p + 1);
        int t_fdin = -1, t_fdout = -1;

        if ((t_fdin = _open_osfhandle((long)args->hOutput, _O_NOINHERIT|_O_BINARY|_O_RDONLY)) >= 0) {
            args->hOutput = 0;                  // change ownership
            
            if ((t_fdout = _open_osfhandle((long)args->hInput, _O_NOINHERIT|_O_BINARY|_O_WRONLY)) >= 0) {
                const int handle = (int)args->hProc;

                args->hInput = 0;               // change ownership
                args->hProc = 0;
                mc_pclose (p, NULL);

                fds[0] = t_fdin;
                fds[1] = t_fdout;
                return handle;
            }
            close (t_fdin);
        }
        mc_pclose (p, NULL);
    }
    return -1;
}


static mc_pipe_t *
pipe_open (const char *xcommand, gboolean fdout, gboolean fderr, gboolean fdin, GError ** error)
{
    win32_exec_t *args = NULL;
    const char *busybox = mc_BUSYBOX();
    const char *cmd = NULL;
    mc_pipe_t *p = NULL;
    int x_errno = -1;

    if (error) *error = NULL;

    if (xcommand) {
        char *command;

        while (' ' == *xcommand) ++xcommand;    // consume leading whitespace (if any).
            /* whitespace within "#! xxx" shall be visible; confusing matching logic below */

        win32Trace(("mc_popen: in:<%s>", xcommand ? xcommand : ""))
     // my_unquote_test();

        if (NULL == (command = my_unquote(xcommand, TRUE))) {
            goto error;
        }

        win32Trace(("mc_popen: qu:<%s>", command))

        if (busybox && *busybox) {
            const char *space, *exec;

            if (NULL != (space = strchr(command, ' ')) &&
                    space == (command + (sizeof(bin_sh) - 1)) && 0 == strncmp(command, bin_sh, sizeof(bin_sh)-1)) {
                /*
                 *  If </bin/sh> <cmd ...>
                 *  execute as <\"$(busybox)\" sh cmd ...>
                 */
                char *t_cmd;

                if (NULL == (t_cmd = g_strconcat("\"", busybox, "\" sh", space, NULL))) {
                    free((void *)command);
                    x_errno = ENOMEM;
                    goto error;
                }
                cmd = t_cmd;                    // replacement command.

            } else if (NULL != (exec = mc_isscript(command))) {
                /*
                 *  If <#!> </bin/sh | /usr/bin/perl | /usr/bin/python | /usr/bin/env python>
                 */
                char *t_cmd = NULL;

                if (exec[0] == 'p') {           // perl/python.
                    t_cmd = g_strconcat(exec, " ", command, NULL);
                } else {                        // sh/ash/bash.
                    t_cmd = g_strconcat("\"", busybox, "\" ", exec, " ", command, NULL);
                }
                if (NULL == t_cmd) {
                    free((void *)command);
                    x_errno = ENOMEM;
                    goto error;
                }
                cmd = t_cmd;                    // replacement command.

            } else if (space) {
                /*
                 *  If busybox <cmd>, execute as <\"busybox\" sh -c `cmd` ...>
                 */
                unsigned i, count = 0;
                const char **busybox_cmds = mc_busybox_exts(&count);
                const int commandlen = space - command;

                for (i = 0; i < count; ++i)  {
                    if (0 == strncmp(busybox_cmds[i], command, commandlen)) {
                        char *t_cmd = NULL;

                        if (NULL == (t_cmd = g_strconcat("\"", busybox, "\" sh -c \"", command, "\"", NULL))) {
                           free((void *)command);
                           x_errno = ENOMEM;
                           goto error;
                        }
                        cmd = t_cmd;            // replacement command.
                        break;
                    }
                }
            }
        }

        if (NULL == cmd) cmd = g_strdup(command);
        free((void *)command);

        if (NULL == cmd) {
            goto error;
        }
    }

    if (NULL == (p = (mc_pipe_t *)calloc(sizeof(mc_pipe_t) + sizeof(win32_exec_t), 1))) {
        x_errno = ENOMEM;
        goto error;
    }

    p->out.fd = -1;                             // file-descriptors, not used.
    p->err.fd = -1;

    args = (win32_exec_t *)(p + 1);
    args->spawn.cmd = cmd;

    win32Trace(("mc_popen: ex:<%s>", cmd))

    if (0 == w32_exec(args)) {
        x_errno = w32_errno_cnv(GetLastError());
        goto error;
    }

    if (fdout) {                                // reading stdout?
        p->out.fd = 0;
    } else if (args->hOutput) {                 // close stdout
        CloseHandle(args->hOutput);
        args->hOutput = 0;
    }

    if (fderr) {                                // reading stderr?
        p->err.fd = 0;
    } else if (args->hError) {                  // close stderr
        CloseHandle(args->hError);
        args->hError = 0;
    }

    p->out.len = MC_PIPE_BUFSIZE;               // read buffer length.
    p->out.null_term = FALSE;                   // whether buf is null-terminated or not.
    p->out.buf[0] = '\0';

    p->err.len = MC_PIPE_BUFSIZE;               // read buffer length.
    p->err.null_term = FALSE;                   // whether buf is null-terminated or not.
    p->err.buf[0] = '\0';

    return p;

error:;
    if (x_errno > 0) {
        mc_replace_error (error, MC_PIPE_ERROR_CREATE_PIPE, "%s : %s",
                _("Cannot create pipe descriptor"), strerror(x_errno));
    } else {
        mc_replace_error (error, MC_PIPE_ERROR_CREATE_PIPE, "%s",
                _("Cannot create pipe descriptor"));
    }

    if (p) {
        if (args) g_free((void *)args->spawn.cmd);
        free(p);
    }

    return NULL;
}


/**
 * Read stdout and stderr of pipe asynchronously.
 *
 * @parameter p pipe descriptor
 *
 * The lengths of read data contain in p->out.len and p->err.len.
 *
 * Before read, p->xxx.len is an input. It defines the number of data to read.
 * Should not be greater than MC_PIPE_BUFSIZE.
 *
 * After read, p->xxx.len is an output and contains the following:
 *   p->xxx.len > 0: an actual length of read data stored in p->xxx.buf;
 *   p->xxx.len == MC_PIPE_STREAM_EOF: EOF of stream p->xxx;
 *   p->xxx.len == MC_PIPE_STREAM_UNREAD: stream p->xxx was not read;
 *   p->xxx.len == MC_PIPE_ERROR_READ: reading error, and p->xxx.errno is set appropriately.
 *
 * @paremeter error contains pointer to object to handle error code and message
 */

static DWORD
WaitForMultiplePipes (DWORD waitcount, HANDLE *handles, DWORD timeout)
{
#define PIPEPEEK_INTERVAL 25
    DWORD h;

    if (waitcount) {
        while (1) {
            for (h = 0; h < waitcount; ++h) {
                DWORD avail = 0;

                if (! PeekNamedPipe(handles[h], NULL, 0, NULL, &avail, NULL)) {
                    if (GetLastError() == ERROR_BROKEN_PIPE) {
                        return h + WAIT_ABANDONED_0;
                    }
                    return WAIT_FAILED;

                } else if (avail > 0) {
                    return h + WAIT_OBJECT_0;
                }
            }

            if (INFINITE != timeout) {
                if (timeout < PIPEPEEK_INTERVAL) return WAIT_TIMEOUT;
                timeout -= PIPEPEEK_INTERVAL;
            }

            SleepEx(PIPEPEEK_INTERVAL, TRUE);
        }
    }
    return WAIT_FAILED;
}


void
mc_pread (mc_pipe_t *p, GError **error)
{
    pipe_read (p, p->out.fd >= 0, p->err.fd >= 0, error);
}


int
mc_pread2 (mc_pipe_t *p, void *buf, size_t len)
{
    if (p->out.fd >= 0) {
        p->out.len = len;                       // buffer length

        if (pipe_read (p, TRUE, FALSE, NULL)) {
            const int ret = p->out.len;
            if (ret >= 0) {
                memcpy (buf, p->out.buf, ret);
                return ret;
            }
        }
    }
    return -1;
}


static gboolean
pipe_read (mc_pipe_t *p, gboolean fdout, gboolean fderr, GError **error)
{
    win32_exec_t *args = (win32_exec_t *)(p + 1);
    mc_pipe_stream_t *streams[3] = {0};
    HANDLE handles[3] = {0};
    ssize_t buflens[3] = {0};
    DWORD waitcnt = 0, ret;
    gboolean rc = FALSE;

    if (error) *error = NULL;

    if (!fdout && !fderr) {
        p->out.len = MC_PIPE_STREAM_UNREAD;
        p->err.len = MC_PIPE_STREAM_UNREAD;
        return FALSE;
    }

    if (args->hInput) {                         // close stdin
        CloseHandle(args->hInput);
        args->hInput = 0;
    }

    if (fdout) {                                // stdout
        assert(args->hOutput && INVALID_HANDLE_VALUE != args->hOutput);
        streams[waitcnt] = &p->out;
        handles[waitcnt] = args->hOutput;
        buflens[waitcnt] = p->out.len;
        p->out.len = MC_PIPE_STREAM_UNREAD;
        p->out.pos = 0;                         // buffer position, see: mc_pstream_get_string()
        ++waitcnt;
    }

    if (fderr) {                                // stderr
        assert(args->hError && INVALID_HANDLE_VALUE != args->hError);
        streams[waitcnt] = &p->err;
        handles[waitcnt] = args->hError;
        buflens[waitcnt] = p->err.len;
        p->err.len = MC_PIPE_STREAM_UNREAD;
        p->err.pos = 0;                         // buffer position, see: mc_pstream_get_string()
        ++waitcnt;
    }
                                                // select(handles)
    ret = WaitForMultiplePipes(waitcnt, handles, 20000 /*20 seconds*/);

    while (1) {                                 // stream ready?
         if (ret >= WAIT_OBJECT_0 && ret < (WAIT_OBJECT_0 + 2)) {
            const unsigned sid = ret - WAIT_OBJECT_0;
            mc_pipe_stream_t *ps = streams[ sid ];
            size_t buflen = buflens[ sid ];
            DWORD count = 0;

            if (buflen >= MC_PIPE_BUFSIZE) {
                buflen = (ps->null_term ? MC_PIPE_BUFSIZE - 1 : MC_PIPE_BUFSIZE);
            }
            assert(buflen && buflen <= MC_PIPE_BUFSIZE);

            ps->error = 0;                      // last error

            if (ReadFile(handles[ sid ], ps->buf, buflen, &count, NULL)) {
                ps->len = (int) count;
                if (ps->null_term) {            // optional terminator
                    ps->buf[count] = '\0';
                }
                rc = TRUE;

            } else {                            // error conditions
                const DWORD rc = GetLastError();
                if (ERROR_BROKEN_PIPE == rc) {
                    ps->len = MC_PIPE_STREAM_EOF;
                } else {
                    ps->len = MC_PIPE_ERROR_READ;
                    mc_propagate_error(error, MC_PIPE_ERROR_READ,
                        _("Unexpected error in reading data from a child process : %s"),
                        unix_error_string(w32_errno_cnv(rc)));
                }
            }

            if (2 == waitcnt--) {               // poll alternative
                const DWORD alt = (WAIT_OBJECT_0 == ret ? 1 : 0);

                ret = WaitForMultiplePipes(1, handles + alt, PIPEPEEK_INTERVAL);
                if (WAIT_OBJECT_0 == ret || WAIT_ABANDONED_0 == ret) {
                    ret += alt;
                    continue;
                }
            }

        } else if (ret >= WAIT_ABANDONED_0 && ret < (WAIT_ABANDONED_0 + 2)) {
                                                // pipe broken
            mc_pipe_stream_t *ps = streams[ret - WAIT_ABANDONED_0];
            if (ps) {
                ps->len = MC_PIPE_STREAM_EOF;   
            }

            if (2 == waitcnt--) {               // poll alternative
                const DWORD alt = (WAIT_ABANDONED_0 == ret ? 1 : 0);

                ret = WaitForMultiplePipes(1, handles + alt, PIPEPEEK_INTERVAL);
                if (WAIT_OBJECT_0 == ret || WAIT_ABANDONED_0 == ret) {
                    ret += alt;
                    continue;
                }
            }

        } else if (ret == WAIT_TIMEOUT) {       // timeout
            if (fdout) p->out.len = MC_PIPE_STREAM_EOF;
            if (fderr) p->err.len = MC_PIPE_STREAM_EOF;

        } else {                                // unknown condition.
            mc_propagate_error (error, MC_PIPE_ERROR_READ,
                _("Unexpected error in wait from a child process : %s"),
                unix_error_string(w32_errno_cnv(GetLastError())));
        }

        break;  // done.
    }

    return rc;
}


/** 4.8.27+
 * Reads a line from @stream. Reading stops after an EOL or a newline. If a newline is read,
 * it is appended to the line.
 *
 * @stream mc_pipe_stream_t object
 *
 * @return newly created GString or NULL in case of EOL;
 */

GString *
mc_pstream_get_string (mc_pipe_stream_t * ps)
{
    char *s;
    size_t size, i;
    gboolean escape = FALSE;

    g_return_val_if_fail (ps != NULL, NULL);

    if (ps->len < 0)
        return NULL;

    size = ps->len - ps->pos;

    if (size == 0)
        return NULL;

    s = ps->buf + ps->pos;

    if (s[0] == '\0')
        return NULL;

    /* find ’\0’ or unescaped ’\n’ */
    for (i = 0; i < size && !(s[i] == '\0' || (s[i] == '\n' && !escape)); i++)
        escape = s[i] == '\\' ? !escape : FALSE;

    if (i != size && s[i] == '\n')
        i++;

    ps->pos += i;

    return g_string_new_len (s, i);
}


/**
 *  Close pipe and destroy pipe descriptor.
 *
 *  @paremeter p pipe descriptor
 *  @paremeter error contains pointer to object to handle error code and message
 */

void
mc_pclose (mc_pipe_t * p, GError ** error)
{
    win32_exec_t *args = (win32_exec_t *)(p + 1);

    if (error) *error = NULL;

    if (args->hInput)  CloseHandle(args->hInput);
    if (args->hOutput) CloseHandle(args->hOutput);
    if (args->hError)  CloseHandle(args->hError);
    if (args->hProc) {
        (void) w32_child_wait(args->hProc, NULL, FALSE);
    }
    g_free((void *)args->spawn.cmd);
    free(p);
}

/*end*/
