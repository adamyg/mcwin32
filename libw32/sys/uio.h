#ifndef SYS_UIO_H_INCLUDED
#define SYS_UIO_H_INCLUDED
/*
 *  win32 sys/uio.h
 *
 */

#include <stddef.h>             /* size_t */
#include <limits.h>             /* INT_MAX */
#include <cscdefs.h>            /* __PDECL */

#define IOV_MAX                 64
#define SSIZE_MAX		INT_MAX

__BEGIN_DECLS

typedef struct iovec {
    void *     iov_base;
    int        iov_len;
} iovec_t;

extern  size_t __PDECL          readv(int, const struct iovec *, int);
extern  size_t __PDECL          writev(int, const struct iovec *, int);

__END_DECLS

#endif  /* SYS_UIO_H_INCLUDED */
