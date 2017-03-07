/* -*- indent-width: 4; -*- */
/*
   WIN32 pipe

        mc_popen
        mc_pread
        mc_pclose

   Written by: Adam Young 2015 - 2017

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

static const char       bin_sh[] = "/bin/sh";


/**
 *  Create pipe and run child process.
 *
 *  @parameter command command line of child process
 *  @paremeter error contains pointer to object to handle error code and message
 *
 *  @return newly created object of mc_pipe_t class in success, NULL otherwise
 */

mc_pipe_t *
mc_popen (const char *command, GError ** error)
{
    win32_exec_t *args = NULL;
    const char *busybox = getenv("MC_BUSYBOX");
    int x_errno = -1;
    char **argv[32 + 1] = {0};
    char *cmd = NULL;
    int in = -1;
    mc_pipe_t *p = NULL;

    if (error) *error = NULL;

    if (busybox && *busybox) {
        /*
         *  If </bin/sh> <cmd ...>
         *  execute as <\"$(busybox)\" sh cmd ...>
         */
        const char *space;

        if (NULL != (space = strchr(command, ' ')) &&
                space == (command + (sizeof(bin_sh) - 1)) && 0 == strncmp(command, bin_sh, sizeof(bin_sh)-1)) {
            char *t_cmd;

            if (NULL != (t_cmd = g_strconcat("\"", busybox, "\" sh", space, NULL))) {
                cmd = t_cmd;                    // replacement command.
            }
        }
    }

    if ((NULL == cmd && (NULL == (cmd = g_strdup(command)))) ||
            NULL == (p =
                calloc(sizeof(mc_pipe_t) + sizeof(win32_exec_t), 1))) {
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
    free(p);

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

    read_out = (args->hOutput && p->out.len > 0);
    read_err = (args->hError  && p->err.len > 0);

    if (! read_out && ! read_err) {
        return;
    }

    if (args->hInput) {                         // close stdin
        CloseHandle(args->hInput);
        args->hInput = 0;
    }

    if (read_out) {                             // stdout
        p->out.len = MC_PIPE_STREAM_UNREAD;
        handles[count++] = args->hOutput;
    }

    if (read_err) {                             // stderr
        p->err.len = MC_PIPE_STREAM_UNREAD;
        handles[count++] = args->hError;
    }
                                                // select(handles)
    ret = WaitForMultipleObjects(count, handles, FALSE, 2500 /*INFINITE*/);

    if (ret < count) {                          // stream ready?
        HANDLE hPipe = handles[ret];
        mc_pipe_stream_t *ps = ((hPipe == args->hOutput) ? &p->out : &p->err);
        size_t len;
        DWORD readcount;

        if ((len = (size_t) ps->len) > MC_PIPE_BUFSIZE) {
            len = MC_PIPE_BUFSIZE;
        }

        if (ReadFile(hPipe, ps->buf, (ps->null_term ? len-1 : len), &readcount, NULL) && readcount) {
            if (ps->null_term) {                // optional terminator
                ps->buf[readcount] = '\0';
            }
            ps->len = (int)readcount;

        } else {                                // error conditions
            const DWORD lastError = GetLastError();

            if (ERROR_BROKEN_PIPE == lastError) {
                ps->len = MC_PIPE_STREAM_EOF;

            } else {
                ps->len = MC_PIPE_ERROR_READ;
                ps->error = w32_errno_cnv(lastError);
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