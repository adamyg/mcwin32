#ifndef WIN32_SYS_MMAN_H_WIN32
#define WIN32_SYS_MMAN_H_WIN32
/* -*- mode: c; tabs: 4 -*- */
/*
 * win32 mmap implementation
 * 
 * ==end==
 */
#include <stddef.h>                             /* size_t */

/*
 * Protections are chosen from these bits, or-ed together.
 * Note - not all implementations literally provide all possible
 * combinations.  PROT_WRITE is often implemented as (PROT_READ |
 * PROT_WRITE) and (PROT_EXECUTE as PROT_READ | PROT_EXECUTE).
 * However, no implementation will permit a write to succeed
 * where PROT_WRITE has not been set.  Also, no implementation will
 * allow any access to succeed where prot is specified as PROT_NONE.
 */

#define PROT_READ       0x1                     /* pages can be read */
#define PROT_WRITE      0x2                     /* pages can be written */
#define PROT_EXEC       0x4                     /* pages can be executed */
#define PROT_NONE       0x0                     /* pages cannot be accessed */


/* sharing types:  must choose either SHARED or PRIVATE */
#define MAP_SHARED      1                       /* share changes */
#define MAP_PRIVATE     2                       /* changes are private */
#define MAP_TYPE        0xf                     /* mask for share type */


/* other flags to mmap (or-ed in to MAP_SHARED or MAP_PRIVATE) */
#define MAP_FIXED       0x10                    /* user assigns address */
#define MAP_NORESERVE   0x40                    /* don't reserve needed swap area */


/* these flags not yet implemented */
#define MAP_RENAME      0x20                    /* rename private pages to file */



/* return value on failure */
#if !defined (MAP_FAILED)                       /* Failure return value. */
#define MAP_FAILED      ((void *) -1)
#endif


/* flags to msync */
#define MS_SYNC         0x4                     /* wait for msync */
#define MS_ASYNC        0x1                     /* return immediately */
#define MS_INVALIDATE   0x2                     /* invalidate caches */


#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */

__BEGIN_DECLS

extern void * __PDECL   mmap __P((void *addr, size_t len, int prot,
                              int flags, int fildes, off_t off));
extern int __PDECL      mprotect __P((void *addr, size_t len, int prot));
extern int __PDECL      msync __P((void *addr, size_t len, int flags));
extern int __PDECL      munmap __P((void *addr, size_t len));
extern int __PDECL      mlock __P((const void *, size_t));
extern int __PDECL      munlock __P((const void *, size_t));

__END_DECLS

#endif /*WIN32_SYS_MMAN_H_WIN32*/
