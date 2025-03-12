/** \file lib/util.h
 *  \brief Header: various utilities
 */

#ifndef MC_UTIL_H
#define MC_UTIL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>           /* uintmax_t */
#include <unistd.h>

#include "lib/global.h"         /* include <glib.h> */

#include "lib/vfs/vfs.h"

/*** typedefs(not structures) and defined constants **********************************************/

#ifndef MAXSYMLINKS
#define MAXSYMLINKS 32
#endif

#define MAX_SAVED_BOOKMARKS 10

#define MC_PTR_FREE(ptr) do { g_free (ptr); (ptr) = NULL; } while (0)

#define mc_return_if_error(mcerror) do { if (mcerror != NULL && *mcerror != NULL) return; } while (0)
#define mc_return_val_if_error(mcerror, mcvalue) do { if (mcerror != NULL && *mcerror != NULL) return mcvalue; } while (0)

#define whitespace(c) ((c) == ' ' || (c) == '\t')
#define whiteness(c) (whitespace (c) || (c) == '\n')

#define MC_PIPE_BUFSIZE BUF_8K
#define MC_PIPE_STREAM_EOF 0
#define MC_PIPE_STREAM_UNREAD -1
#define MC_PIPE_ERROR_CREATE_PIPE -2
#define MC_PIPE_ERROR_PARSE_COMMAND -3
#define MC_PIPE_ERROR_CREATE_PIPE_STREAM -4
#define MC_PIPE_ERROR_READ -5

/* gnulib efa15594e17fc20827dba66414fb391e99905394

 *_GL_CMP (n1, n2) performs a three-valued comparison on n1 vs. n2.
 *  It returns
 *    1  if n1 > n2
 *    0  if n1 == n2
 *    -1 if n1 < n2
 *  The native code   (n1 > n2 ? 1 : n1 < n2 ? -1 : 0)  produces a conditional
 *  jump with nearly all GCC versions up to GCC 10.
 *  This variant      (n1 < n2 ? -1 : n1 > n2)  produces a conditional with many
 *  GCC versions up to GCC 9.
 *  The better code  (n1 > n2) - (n1 < n2)  from Hacker's Delight para 2-9
 *  avoids conditional jumps in all GCC versions >= 3.4.
 */
#define _GL_CMP(n1, n2) (((n1) > (n2)) - ((n1) < (n2)))

/* Difference or zero */
#define DOZ(a, b) ((a) > (b) ? (a) - (b) : 0)

/* flags for shell_execute */
#define EXECUTE_INTERNAL (1 << 0)
#define EXECUTE_AS_SHELL (1 << 2)
#define EXECUTE_HIDE     (1 << 3)

/*** enums ***************************************************************************************/

/* Pathname canonicalization */
/* *INDENT-OFF* */
typedef enum
{
    CANON_PATH_NOCHANGE = 0,
    CANON_PATH_JOINSLASHES = 1L << 0,   /**< Multiple '/'s are collapsed to a single '/' */
    CANON_PATH_REMSLASHDOTS = 1L << 1,  /**< Leading './'s, '/'s and trailing '/.'s are removed */
    CANON_PATH_REMDOUBLEDOTS = 1L << 3, /**< Non-leading '../'s and trailing '..'s are handled by removing
                                             portions of the path */
    CANON_PATH_GUARDUNC = 1L << 4,      /**< Detect and preserve UNC paths: //server/... */
    CANON_PATH_ALL = CANON_PATH_JOINSLASHES | CANON_PATH_REMSLASHDOTS
                   | CANON_PATH_REMDOUBLEDOTS | CANON_PATH_GUARDUNC  /**< All flags */
} canon_path_flags_t;
/* *INDENT-ON* */

enum compression_type
{
    COMPRESSION_NONE,
    COMPRESSION_ZIP,
    COMPRESSION_GZIP,
    COMPRESSION_BZIP,
    COMPRESSION_BZIP2,
    COMPRESSION_LZIP,
    COMPRESSION_LZ4,
    COMPRESSION_LZMA,
    COMPRESSION_LZO,
    COMPRESSION_XZ,
    COMPRESSION_ZSTD,
};

/* stdout or stderr stream of child process */
typedef struct
{
    /* file descriptor */
    int fd;
    /* data read from fd */
    char buf[MC_PIPE_BUFSIZE];
    /* current position in @buf (used by mc_pstream_get_string()) */
    size_t pos;
    /* positive: length of data in buf;
     * MC_PIPE_STREAM_EOF: EOF of fd;
     * MC_PIPE_STREAM_UNREAD: there was not read from fd;
     * MC_PIPE_ERROR_READ: reading error from fd.
     */
    ssize_t len;
    /* whether buf is null-terminated or not */
    gboolean null_term;
    /* error code in case of len == MC_PIPE_ERROR_READ */
    int error;
} mc_pipe_stream_t;

/* Pipe descriptor for child process */
typedef struct
{
    /* PID of child process */
    GPid child_pid;
    /* stdout of child process */
    mc_pipe_stream_t out;
    /* stderr of child process */
    mc_pipe_stream_t err;
} mc_pipe_t;

/* sighandler_t is GNU extension */
#ifndef HAVE_SIGHANDLER_T
typedef void (*sighandler_t) (int);
#endif

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

extern struct sigaction startup_handler;

/*** declarations of public functions ************************************************************/

int is_printable (int c);

/* Quote the filename for the purpose of inserting it into the command
 * line.  If quote_percent is 1, replace "%" with "%%" - the percent is
 * processed by the mc command line. */
char *name_quote (const char *c, gboolean quote_percent);

/* returns a duplicate of c. */
char *fake_name_quote (const char *c, gboolean quote_percent);

/* path_trunc() is the same as str_trunc() but
 * it deletes possible password from path for security
 * reasons. */
const char *path_trunc (const char *path, size_t trunc_len);

/* return a static string representing size, appending "K" or "M" for
 * big sizes.
 * NOTE: uses the same static buffer as size_trunc_sep. */
const char *size_trunc (uintmax_t size, gboolean use_si);

/* return a static string representing size, appending "K" or "M" for
 * big sizes. Separates every three digits by ",".
 * NOTE: uses the same static buffer as size_trunc. */
const char *size_trunc_sep (uintmax_t size, gboolean use_si);

/* Print file SIZE to BUFFER, but don't exceed LEN characters,
 * not including trailing 0. BUFFER should be at least LEN+1 long.
 *
 * Units: size units (0=bytes, 1=Kbytes, 2=Mbytes, etc.) */
void size_trunc_len (char *buffer, unsigned int len, uintmax_t size, int units, gboolean use_si);
const char *string_perm (mode_t mode_bits);

const char *extension (const char *);
const char *unix_error_string (int error_num);
const char *skip_separators (const char *s);
const char *skip_numbers (const char *s);
char *strip_ctrl_codes (char *s);

/* Replaces "\\E" and "\\e" with "\033". Replaces "^" + [a-z] with
 * ((char) 1 + (c - 'a')). The same goes for "^" + [A-Z].
 * Returns a newly allocated string. */
char *convert_controls (const char *s);

/* overwrites passwd with '\0's and frees it. */
void wipe_password (char *passwd);

char *diff_two_paths (const vfs_path_t * vpath1, const vfs_path_t * vpath2);

/* Returns the basename of fname. The result is a pointer into fname. */
const char *x_basename (const char *fname);

char *load_mc_home_file (const char *from, const char *filename, char **allocated_filename,
                         size_t *length);

/* uid/gid managing */
void init_groups (void);
void destroy_groups (void);
int get_user_permissions (struct stat *buf);

void init_uid_gid_cache (void);
const char *get_group (gid_t gid);
const char *get_owner (uid_t uid);

/* Returns a copy of *s until a \n is found and is below top */
const char *extract_line (const char *s, const char *top, size_t *len);

/* System call wrappers */
MC_MOCKABLE sighandler_t my_signal (int signum, sighandler_t handler);
MC_MOCKABLE int my_sigaction (int signum, const struct sigaction *act, struct sigaction *oldact);
MC_MOCKABLE pid_t my_fork (void);
MC_MOCKABLE int my_execvp (const char *file, char *const argv[]);
MC_MOCKABLE char *my_get_current_dir (void);

/* Process spawning */
int my_system (int flags, const char *shell, const char *command);
int my_systeml (int flags, const char *shell, ...);
int my_systemv (const char *command, char *const argv[]);
int my_systemv_flags (int flags, const char *command, char *const argv[]);

mc_pipe_t *mc_popen (const char *command, gboolean read_out, gboolean read_err, GError ** error);
#if defined(WIN32) //WIN32, shell
int mc_popen2 (const char *command, int *fds, GError **error);
#endif
void mc_pread (mc_pipe_t * p, GError ** error);
void mc_pclose (mc_pipe_t * p, GError ** error);

GString *mc_pstream_get_string (mc_pipe_stream_t * ps);

MC_MOCKABLE void my_exit (int status);
void save_stop_handler (void);

/* Tilde expansion */
char *tilde_expand (const char *directory);

void canonicalize_pathname_custom (char *path, canon_path_flags_t flags);

char *mc_realpath (const char *path, char *resolved_path);

/* Looks for "magic" bytes at the start of the VFS file to guess the
 * compression type. Side effect: modifies the file position. */
enum compression_type get_compression_type (int fd, const char *name);
const char *decompress_extension (int type);

GList *list_append_unique (GList * list, char *text);

/* Position saving and restoring */
/* Load position for the given filename */
void load_file_position (const vfs_path_t * filename_vpath, long *line, long *column,
                         off_t * offset, GArray ** bookmarks);
/* Save position for the given filename */
void save_file_position (const vfs_path_t * filename_vpath, long line, long column, off_t offset,
                         GArray * bookmarks);


/* if ch is in [A-Za-z], returns the corresponding control character,
 * else returns the argument. */
extern int ascii_alpha_to_cntrl (int ch);

#undef Q_
const char *Q_ (const char *s);

gboolean mc_util_make_backup_if_possible (const char *file_name, const char *backup_suffix);
gboolean mc_util_restore_from_backup_if_possible (const char *file_name, const char *backup_suffix);
gboolean mc_util_unlink_backup_if_possible (const char *file_name, const char *backup_suffix);

MC_MOCKABLE char *guess_message_value (void);

char *mc_build_filename (const char *first_element, ...);
char *mc_build_filenamev (const char *first_element, va_list args);

const char *mc_get_profile_root (void);

/* *INDENT-OFF* */
void mc_propagate_error (GError ** dest, int code, const char *format, ...) G_GNUC_PRINTF (3, 4);
void mc_replace_error (GError ** dest, int code, const char *format, ...) G_GNUC_PRINTF (3, 4);
/* *INDENT-ON* */

gboolean mc_time_elapsed (gint64 * timestamp, gint64 delay);

/* --------------------------------------------------------------------------------------------- */
/*** inline functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static inline gboolean
exist_file (const char *name)
{
    return (access (name, R_OK) == 0);
}

/* --------------------------------------------------------------------------------------------- */

static inline gboolean
is_exe (mode_t mode)
{
    return ((mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0);
}

/* --------------------------------------------------------------------------------------------- */
/**
 * Canonicalize path with CANON_PATH_ALL.
 *
 * @param path path to file
 * @param flags canonicalization flags
 *
 * All modifications of @path are made in place.
 * Well formed UNC paths are modified only in the local part.
 */
#if defined(WIN32) //WIN32, path
void canonicalize_pathname (char *path);
#else
static inline void
canonicalize_pathname (char *path)
{
    canonicalize_pathname_custom (path, CANON_PATH_ALL);
}
#endif
/* --------------------------------------------------------------------------------------------- */

#endif /* MC_UTIL_H */
