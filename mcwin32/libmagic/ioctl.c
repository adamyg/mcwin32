//
//  ioctl
//

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

int 
w32_ioctl(int fd, unsigned long request, int *value)
{
    if (FIONREAD == request) {
        /*TODO*/
    }
    errno = EINVAL;
    return -1;
}

/*end*/

