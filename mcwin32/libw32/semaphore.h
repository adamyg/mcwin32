#ifndef LIBW32_SEMAPHORE_H_INCLUDED
#define LIBW32_SEMAPHORE_H_INCLUDED

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#ifndef _TIMESPEC_DEFINED
#define _TIMESPEC_DEFINED
struct timespec {
    long tv_sec;
    long tv_nsec;
};
#endif

#ifndef _SEM_T_DEFINED
#define _SEM_T_DEFINED
typedef HANDLE sem_t;
#endif

#define _POSIX_SEMAPHORES
#define SEM_FAILED	(-1)

LIBW32_API int		sem_init(sem_t *sem, int pshared, unsigned int value);
LIBW32_API int		sem_destroy(sem_t *sem);
LIBW32_API int		sem_trywait(sem_t *sem);
LIBW32_API int		sem_wait(sem_t * sem);
LIBW32_API int		sem_timedwait(sem_t *sem, const struct timespec *abstime);
LIBW32_API int		sem_post(sem_t *sem);
LIBW32_API int		sem_post_multiple(sem_t *sem, int count);
LIBW32_API int		sem_open(const char *name, int oflag, int mode, unsigned int value);
LIBW32_API int		sem_close(sem_t *sem);
LIBW32_API int		sem_unlink(const char *name);
LIBW32_API int		sem_getvalue(sem_t *sem, int *sval);

__END_DECLS

#endif /*LIBW32_SEMAPHORE_H_INCLUDED*/
