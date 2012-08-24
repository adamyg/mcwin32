#ifndef WIN32_SYS_MOUNT_H
#define WIN32_SYS_MOUNT_H

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 mount() implementation
 *
 * ==end==
 */

#include <sys/cdefs.h>

#define MOUNT_UFS   0
#define MOUNT_NFS   1
#define MOUNT_PC    2
#define MOUNT_MFS   3
#define MOUNT_LO    4
#define MOUNT_TFS   5
#define MOUNT_TMP   6

#define MNT_WAIT    0
#define MNT_NOWAIT  1

__BEGIN_DECLS

struct statfs;

extern int                  getmntinfo(struct statfs **mntbufp, int flags);

__END_DECLS

#endif
