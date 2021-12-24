/* -*- mode: c; indent-width: 4; -*- */
/*
   WIN32 pipe

        mc_popen
        mc_pread
        mc_pclose

   Written by: Adam Young 2015 - 2021

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
#include "lib/utilunix.h"

#include "win32_utl.h"


static const char bin_sh[] = "/bin/sh";


/**
 *  Create pipe and run child process.
 *
 *  @parameter command command line of child process
 *  @paremeter error contains pointer to object to handle error code and message
 *
 *  @return newly created object of mc_pipe_t class in success, NULL otherwise
 */

#if (VERSION_3 >= 27)
mc_pipe_t *
mc_popen (const char *xcommand, gboolean read_out, gboolean read_err, GError ** error)
#else
mc_pipe_t *
mc_popen (const char *xcommand, GError ** error)
#endif
{
#if (VERSION_3 < 27)
    gboolean read_out = 1, read_err = 1;
#endif
    win32_exec_t *args = NULL;
    const char *busybox = getenv("MC_BUSYBOX");
    const char *cmd = NULL;
    mc_pipe_t *p = NULL;
    int x_errno = -1;

    if (error) *error = NULL;

    if (xcommand) {
        char *command;

        while (' ' == *xcommand) ++xcommand;    // consume leading whitespace (if any).
            /* whitespace within "#! xxx" shall be visible; confusing matching logic below */

        if (NULL == (command = my_unquote(xcommand, TRUE))) {
            goto error;
        }

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

                if (exec[0] == 'p') {           // perl/python
                    t_cmd = g_strconcat(exec, " ", command, NULL);
                } else {                        // sh/ash/bash
                    t_cmd = g_strconcat("\"", busybox, "\" ", exec, " ", command, NULL);
                }
                if (NULL == t_cmd) {
                    free((void *)command);
                    x_errno = ENOMEM;
                    goto error;
                }
                cmd = t_cmd;                    // replacement command.
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

    if (0 == w32_exec(args)) {
        x_errno = w32_errno_cnv(GetLastError());
        goto error;
    }

    if (read_out) {                             // reading stdout?
        p->out.fd = 0;
    } else if (args->hOutput) {                 // close stdout
        CloseHandle(args->hOutput);
        args->hOutput = 0;
    }

    if (read_err) {                             // reading stderr?
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
 *  Read stdout and stderr of pipe asynchronously.
 *
 *  @parameter p pipe descriptor
 *
 *  The lengths of read data contain in p->out.len and p->err.len.
 *  Before read, p->xxx.len is an input:
 *
 *      p->xxx.len > 0:    do read stream p->xxx and store data in p->xxx.buf;
 *      p->xxx.len <= 0:   do not read stream p->xxx.
 *
 *  After read, p->xxx.len is an output and contains the following:
 *
 *      p->xxx.len > 0:    an actual length of read data stored in p->xxx.buf;
 *      p->xxx.len ==      MC_PIPE_STREAM_EOF:    EOF of stream p->xxx;
 *      p->xxx.len ==      MC_PIPE_STREAM_UNREAD: stream p->xxx was not read;
 *      p->xxx.len ==      MC_PIPE_ERROR_READ:    reading error, and p->xxx.errno is set appropriately.
 *
 *  @paremeter error contains pointer to object to handle error code and message.
 */

void
mc_pread (mc_pipe_t * p, GError ** error)
{
    win32_exec_t *args = (win32_exec_t *)(p + 1);
    gboolean read_out, read_err;
    HANDLE handles[3] = {0};
    DWORD ret, count = 0;

    if (error) *error = NULL;

    read_out = (p->out.fd >= 0);
    read_err = (p->err.fd >= 0);

    if (! read_out && ! read_err) {
        return;
    }

    if (args->hInput) {                         // close stdin
        CloseHandle(args->hInput);
        args->hInput = 0;
    }

    if (read_out) {                             // stdout
        p->out.len = MC_PIPE_STREAM_UNREAD;
        assert(args->hOutput && INVALID_HANDLE_VALUE != args->hOutput);
        handles[count++] = args->hOutput;
    }

    if (read_err) {                             // stderr
        p->err.len = MC_PIPE_STREAM_UNREAD;
        assert(args->hError && INVALID_HANDLE_VALUE != args->hError);
        handles[count++] = args->hError;
    }

    if (!read_out && !read_err) {
        p->out.len = MC_PIPE_STREAM_UNREAD;
        p->err.len = MC_PIPE_STREAM_UNREAD;
        return;
    }
                                                // select(handles)
    ret = WaitForMultipleObjects(count, handles, FALSE, 2500 /*INFINITE*/);

    if (ret < count) {                          // stream ready?
        int alternative = 1;
        HANDLE hPipe = handles[ret];

        assert(0 == ret || 1 == ret);
        while (hPipe) {
            mc_pipe_stream_t *ps = ((hPipe == args->hOutput) ? &p->out : &p->err);
            size_t buf_len;
            DWORD readcount;

            if ((buf_len = (size_t) ps->len) >= MC_PIPE_BUFSIZE) {
                buf_len = (ps->null_term ? MC_PIPE_BUFSIZE - 1 : MC_PIPE_BUFSIZE);
            }

            if (ReadFile(hPipe, ps->buf, buf_len, &readcount, NULL)) {
                if (readcount) {
                    if (ps->null_term) {        // optional terminator
                        ps->buf[readcount] = '\0';
                    }
                    ps->len = (int)readcount;
                } else {
                    ps->len = 0;
                }

            } else {                            // error conditions
                const DWORD lastError = GetLastError();

                if (ERROR_BROKEN_PIPE == lastError) {
                    ps->len = MC_PIPE_STREAM_EOF;
                } else {
                    ps->len = MC_PIPE_ERROR_READ;
                    ps->error = w32_errno_cnv(lastError);
                }
            }

            hPipe = 0;

            if (read_out && read_err) {         // poll alternative input
                if (alternative) {
                    HANDLE t_handles[2] = {0};

                    alternative = 0;
                    t_handles[0] = (0 == ret ? args->hError : args->hOutput);
                    assert(t_handles[0] != handles[ret]);

                    if (WAIT_OBJECT_0 == (ret = WaitForMultipleObjects(1, t_handles, FALSE, 0 /*NON-BLOCKING*/))) {
                        hPipe = t_handles[0];
                    }
                }
            }
        }

    } else if (ret == WAIT_TIMEOUT) {           // select() timeout
        if (read_out) {
            p->out.len = MC_PIPE_STREAM_EOF;
        }

        if (read_err) {
            p->err.len = MC_PIPE_STREAM_EOF;
        }

    } else {                                    // unknown condition
        mc_propagate_error (error, MC_PIPE_ERROR_READ,
            _("Unexpected error in select() reading data from a child process : %s"),
                w32_errno_cnv(GetLastError()));
    }
}


/** 4.8.27+
 * Reads a line from @stream. Reading stops after an EOL or a newline. If a newline is read,
 * it is appended to the line.
 *
 * @stream mc_pipe_stream_t object
 *
 * @return newly created GString or NULL in case of EOL;
 */

#if (VERSION_3 >= 27)
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
#endif  //VERSION_3


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
