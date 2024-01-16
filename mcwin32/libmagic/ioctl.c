//
//  ioctl
//

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

#if !defined(__MINGW32__) && defined(_WIN32)

#include <Windows.h>

#if defined(_MSC_VER)
#define OSFHANDLE intptr_t
#else
#define OSFHANDLE long
#endif

int
file_pipe(int fildes[2])
{
    HANDLE hReadPipe = 0, hWritePipe = 0;
    const BOOL ret = CreatePipe(&hReadPipe, &hWritePipe, NULL, 0);

    if (ret) {
        fildes[0] = _open_osfhandle((OSFHANDLE)hReadPipe, O_NOINHERIT);
        if (fildes[0] < 0) {
            CloseHandle(hReadPipe), CloseHandle(hWritePipe);
            return -1;
        }
        fildes[1] = _open_osfhandle((OSFHANDLE)hWritePipe, O_NOINHERIT);
        if (fildes[1] < 0) {
            _close(fildes[0]), CloseHandle(hWritePipe);
            return -1;
        }
        return 0;
    }
    errno = EINVAL;
    return -1;
}
#endif  //!__MINGW32__ && _WIN32

int file_ioctl(int fd, unsigned long request, int *value);

int
file_ioctl(int fd, unsigned long request, int *value)
{
    if (FIONREAD == request) {
        /*TODO*/
    }
    errno = EINVAL;
    return -1;
}

/*end*/
