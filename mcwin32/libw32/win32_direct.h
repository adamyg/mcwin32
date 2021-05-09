#ifndef LIBW32_DIRECT_H_INCLUDED
#define LIBW32_DIRECT_H_INCLUDED

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#define DIR_MAGIC               0x57333264      // W32d

#define DIR_FISHPF              0x0001
#define DIR_FISUTF8             0x0002
#define DIR_FHAVESTATS          0x0040

#define PATH_SEP                '/'
#define PATH_SEP_STR            "/"
#define PATH_SEP2               '\\'
#define PATH_SEP_STR2           "\\"
#define IS_PATH_SEP(c)          ((c) == PATH_SEP || (c) == PATH_SEP2)

DIR *                   w32_dir_alloc(void);
void                    w32_dir_free(DIR *dp);

struct _dirlist *       w32_dir_pushA(DIR *dp, const char *filename);
struct _dirlist *       w32_dir_pushW(DIR *dp, const wchar_t *filename);

DIR *                   w32_unc_populateA(const char *servername);
DIR *                   w32_unc_populateW(const wchar_t *servername);

DIR *                   w32_unc_opendirA(const char *dirname);
DIR *                   w32_unc_opendirW(const wchar_t *dirname);
struct dirent *         w32_unc_readdirA(DIR *dp);
struct dirent *         w32_unc_readdirW(DIR *dp);
int                     w32_unc_closedir(DIR *dp);

int                     w32_unc_validA(const char *path);
int                     w32_unc_validW(const wchar_t *path);

int                     w32_unc_rootA(const char *path, int *length);
int                     w32_unc_rootW(const wchar_t *path, int *length);

__END_DECLS

#endif /*LIBW32_DIRECT_H_INCLUDED*/
