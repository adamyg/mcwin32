#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_mmap_c,"$Id: w32_mmap.c,v 1.9 2021/11/30 13:06:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 mmap() system calls.
 *
 * Copyright (c) 2007, 2012 - 2018 Adam Young.
 *
 * This file is part of the Midnight Commander.
 *
 * The Midnight Commander is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * The Midnight Commander is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==end==
 */

#include "win32_internal.h"
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

#define PROT_ALL                (PROT_READ | PROT_WRITE | PROT_EXEC)

/*
//  NAME
//
//      mmap - map pages of memory
//
//  SYNOPSIS
//
//      #include <sys/mman.h>
//
//      void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
//
//  DESCRIPTION
//
//      The mmap() function establishes a mapping between a process' address space and a
//      file or shared memory object. The format of the call is as follows:
//
//          pa=mmap(addr, len, prot, flags, fildes, off);
//
//      The mmap() function establishes a mapping between the address space of the process
//      at an address pa for len bytes to the memory object represented by the file
//      descriptor fildes at offset off for len bytes. The value of pa is an
//      implementation-dependent function of the parameter addr and the values of flags,
//      further described below. A successful mmap() call returns pa as its result. The
//      address range starting at pa and continuing for len bytes will be legitimate for
//      the possible (not necessarily current) address space of the process. The range of
//      bytes starting at off and continuing for len bytes will be legitimate for the
//      possible (not necessarily current) offsets in the file or shared memory object
//      represented by fildes.
//
//      The mapping established by mmap() replaces any previous mappings for those whole
//      pages containing any part of the address space of the process starting at pa and
//      continuing for len bytes.
//
//      If the size of the mapped file changes after the call to mmap() as a result of some
//      other operation on the mapped file, the effect of references to portions of the
//      mapped region that correspond to added or removed portions of the file is
//      unspecified.
//
//      The mmap() function is supported for regular files and shared memory objects.
//      Support for any other type of file is unspecified.
//
//      The parameter prot determines whether read, write, execute, or some combination of
//      accesses are permitted to the data being mapped. The prot should be either
//      PROT_NONE or the bitwise inclusive OR of one or more of the other flags in the
//      following table, defined in the header <sys/mman.h>.
//
//          Symbolic Constant 	Description
//          PROT_READ 	        Data can be read.
//          PROT_WRITE 	        Data can be written.
//          PROT_EXEC 	        Data can be executed.
//          PROT_NONE 	        Data cannot be accessed.
//
//      If an implementation cannot support the combination of access types specified by
//      prot, the call to mmap() fails. An implementation may permit accesses other than
//      those specified by prot; however, the implementation will not permit a write to
//      succeed where PROT_WRITE has not been set or permit any access where PROT_NONE
//      alone has been set. The implementation will support at least the following values
//      of prot: PROT_NONE, PROT_READ, PROT_WRITE, and the inclusive OR of PROT_READ and
//      PROT_WRITE. The file descriptor fildes will have been opened with read permission,
//      regardless of the protection options specified. If PROT_WRITE is specified, the
//      application must have opened the file descriptor fildes with write permission
//      unless MAP_PRIVATE is specified in the flags parameter as described below.
//
//      The parameter flags provides other information about the handling of the mapped
//      data. The value of flags is the bitwise inclusive OR of these options, defined in
//      <sys/mman.h>:
//
//          Symbolic Constant 	Description
//          MAP_SHARED 	        Changes are shared.
//          MAP_PRIVATE         Changes are private.
//          MAP_FIXED 	        Interpret addr exactly.
//
//      MAP_SHARED and MAP_PRIVATE describe the disposition of write references to the
//      memory object. If MAP_SHARED is specified, write references change the underlying
//      object. If MAP_PRIVATE is specified, modifications to the mapped data by the
//      calling process will be visible only to the calling process and will not change the
//      underlying object. It is unspecified whether modifications to the underlying object
//      done after the MAP_PRIVATE mapping is established are visible through the
//      MAP_PRIVATE mapping. Either MAP_SHARED or MAP_PRIVATE can be specified, but not
//      both. The mapping type is retained across fork().
//
//      When MAP_FIXED is set in the flags argument, the implementation is informed that
//      the value of pa must be addr, exactly. If MAP_FIXED is set, mmap() may return
//      MAP_FAILED and set errno to [EINVAL]. If a MAP_FIXED request is successful, the
//      mapping established by mmap() replaces any previous mappings for the process' pages
//      in the range [pa, pa + len).
//
//      When MAP_FIXED is not set, the implementation uses addr in an unspecified manner to
//      arrive at pa. The pa so chosen will be an area of the address space that the
//      implementation deems suitable for a mapping of len bytes to the file. All
//      implementations interpret an addr value of 0 as granting the implementation
//      complete freedom in selecting pa, subject to constraints described below. A
//      non-zero value of addr is taken to be a suggestion of a process address near which
//      the mapping should be placed. When the implementation selects a value for pa, it
//      never places a mapping at address 0, nor does it replace any extant mapping.
//
//      The off argument is constrained to be aligned and sized according to the value
//      returned by sysconf() when passed _SC_PAGESIZE or _SC_PAGE_SIZE. When MAP_FIXED is
//      specified, the argument addr must also meet these constraints. The implementation
//      performs mapping operations over whole pages. Thus, while the argument len need not
//      meet a size or alignment constraint, the implementation will include, in any
//      mapping operation, any partial page specified by the range [pa, pa + len).
//
//      The system always zero-fills any partial page at the end of an object. Further, the
//      system never writes out any modified portions of the last page of an object that
//      are beyond its end. References within the address range starting at pa and
//      continuing for len bytes to whole pages following the end of an object result in
//      delivery of a SIGBUS signal.
//
//      An implementation may deliver SIGBUS signals when a reference would cause an error
//      in the mapped object, such as out-of-space condition.
//
//      The mmap() function adds an extra reference to the file associated with the file
//      descriptor fildes which is not removed by a subsequent close() on that file
//      descriptor. This reference is removed when there are no more mappings to the file.
//
//      The st_atime field of the mapped file may be marked for update at any time between
//      the mmap() call and the corresponding munmap() call. The initial read or write
//      reference to a mapped region will cause the file's st_atime field to be marked for
//      update if it has not already been marked for update.
//
//      The st_ctime and st_mtime fields of a file that is mapped with MAP_SHARED and
//      PROT_WRITE, will be marked for update at some point in the interval between a write
//      reference to the mapped region and the next call to msync() with MS_ASYNC or
//      MS_SYNC for that portion of the file by any process. If there is no such call,
//      these fields may be marked for update at any time after a write reference if the
//      underlying file is modified as a result.
//
//      There may be implementation-dependent limits on the number of memory regions that
//      can be mapped (per process or per system). If such a limit is imposed, whether the
//      number of memory regions that can be mapped by a process is decreased by the use of
//      shmat() is implementation-dependent.
//
//   RETURN VALUE
//
//      Upon successful completion, the mmap() function returns the address at which the
//      mapping was placed (pa); otherwise, it returns a value of MAP_FAILED and sets errno
//      to indicate the error. The symbol MAP_FAILED is defined in the header <sys/mman.h>.
//      No successful return from mmap() will return the value MAP_FAILED.
//
//      If mmap() fails for reasons other than [EBADF], [EINVAL] or [ENOTSUP], some of the
//      mappings in the address range starting at addr and continuing for len bytes may
//      have been unmapped.
//
//   ERRORS
//
//      The mmap() function will fail if:
//
//      [EACCES]
//          The fildes argument is not open for read, regardless of the protection
//          specified, or fildes is not open for write and PROT_WRITE was specified for a
//          MAP_SHARED type mapping.
//
//      [EAGAIN]
//          The mapping could not be locked in memory, if required by mlockall(), due to a
//          lack of resources.
//
//      [EBADF]
//          The fildes argument is not a valid open file descriptor.
//
//      [EINVAL]
//          The addr argument (if MAP_FIXED was specified) or off is not a multiple of the
//          page size as returned by sysconf(), or are considered invalid by the
//          implementation.
//
//      [EINVAL]
//          The value of flags is invalid (neither MAP_PRIVATE nor MAP_SHARED is set).
//
//      [EMFILE]
//          The number of mapped regions would exceed an implementation-dependent limit
//          (per process or per system).
//
//      [ENODEV]
//          The fildes argument refers to a file whose type is not supported by mmap().
//
//      [ENOMEM]
//          MAP_FIXED was specified, and the range [addr, addr + len) exceeds that allowed
//          for the address space of a process; or if MAP_FIXED was not specified and there
//          is insufficient room in the address space to effect the mapping.
//
//      [ENOMEM]
//          The mapping could not be locked in memory, if required by mlockall(), because
//          it would require more space than the system is able to supply.
//
//      [ENOTSUP]
//          The implementation does not support the combination of accesses requested in the prot argument.
//
//      [ENXIO]
//          Addresses in the range [off, off + len) are invalid for the object specified by fildes.
//
//      [ENXIO]
//          MAP_FIXED was specified in flags and the combination of addr, len and off is
//          invalid for the object specified by fildes.
//
//      [EOVERFLOW]
//          The file is a regular file and the value of off plus len exceeds the offset
//          maximum established in the open file description associated with fildes.
//
//   EXAMPLES
//      None.
*/
LIBW32_API void *
mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
    HANDLE  hMapping = INVALID_HANDLE_VALUE;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    void   *region = MAP_FAILED;
    int     ntflags;
    DWORD   ntprot;

    /*
     *  Map prot and flags, to system flags
     */
    if ((flags & MAP_PRIVATE) && (flags & MAP_SHARED) == 0) {
        if ((prot & PROT_ALL) == 0) {           /* NONE */
            ntprot = PAGE_NOACCESS;
            ntflags = FILE_MAP_READ;

        } else {
            if (prot & PROT_EXEC) {
                ntprot = PAGE_EXECUTE_WRITECOPY;
            } else {
                ntprot = PAGE_WRITECOPY;
            }
            ntflags = FILE_MAP_COPY;
        }

    } else if ((flags & MAP_SHARED) && (flags & MAP_PRIVATE) == 0) {
        if ((prot & PROT_ALL) == 0) {           /* NONE */
            ntprot = PAGE_NOACCESS;
            ntflags = FILE_MAP_READ;

        } else {
            if (prot & PROT_WRITE) {            /* WRITE (implied READ) */
                if (prot & PROT_EXEC) {
                    ntprot = PAGE_EXECUTE_READWRITE;
                } else {
                    ntprot = PAGE_READWRITE;
                }
                ntflags = FILE_MAP_WRITE;

            } else {                            /* READONLY */
                if (prot & PROT_EXEC) {
                    ntprot = PAGE_EXECUTE_READ;
                } else {
                    ntprot = PAGE_READONLY;
                }
                ntflags = FILE_MAP_READ;
            }
        }

    } else {    /* SHARED and PRIVATE are not exclusive, or missing */
        errno = EINVAL;
        return MAP_FAILED;
    }

    /*
     *  Convert 'fd' to system handle; unless an ANON mapping.
     */
    if (0 == (flags & MAP_ANONYMOUS) &&         /* extension */
            INVALID_HANDLE_VALUE == (hFile = (HANDLE)_get_osfhandle(fildes))) {
        errno = EBADF;

    /*
     *  Create/open a file-mapping object for the specified file.
     */
    } else if ((hMapping = CreateFileMapping(hFile, 0, ntprot, 0, 0, 0)) == NULL) {
        if (GetLastError() == ERROR_DISK_FULL) {
            errno = ENOMEM;
        } else {
            errno = EINVAL;
        }

    /*
     *  Map the region
     */
    } else {
#if defined(_WINCE)
        (void) addr;
        region = MapViewOfFile(hMapping, ntflags, 0, off, len);
#else
        region = MapViewOfFileEx(hMapping, ntflags, 0, off, len, addr);
#endif

        if (region == (void *)NULL) {
            errno = EINVAL;
            region = MAP_FAILED;

        } else if ((flags & MAP_FIXED) && region != addr) {
            errno = EINVAL;
            (void) UnmapViewOfFile( region );
            region = MAP_FAILED;

        } else if ((flags & PROT_ALL) == 0) {
            (void) mprotect(addr, len, PROT_NONE);
        }

        (void) CloseHandle(hMapping);
    }

    return region;
}


/*
//  NAME
//      mprotect - set protection of memory mapping
//
//  SYNOPSIS
//      #include <sys/mman.h>
//
//      int mprotect(void *addr, size_t len, int prot);
//
//  DESCRIPTION
//      The function mprotect() changes the access protections to be that specified by prot
//      for those whole pages containing any part of the address space of the process
//      starting at address addr and continuing for len bytes. The parameter prot
//      determines whether read, write, execute, or some combination of accesses are
//      permitted to the data being mapped. The prot argument should be either PROT_NONE or
//      the bitwise inclusive OR of one or more of PROT_READ, PROT_WRITE and PROT_EXEC.
//
//      If an implementation cannot support the combination of access types specified by
//      prot, the call to mprotect() fails.
//
//      An implementation may permit accesses other than those specified by prot; however,
//      no implementation permits a write to succeed where PROT_WRITE has not been set or
//      permits any access where PROT_NONE alone has been set. Implementations will support
//      at least the following values of prot: PROT_NONE, PROT_READ, PROT_WRITE, and the
//      inclusive OR of PROT_READ and PROT_WRITE. If PROT_WRITE is specified, the
//      application must have opened the mapped objects in the specified address range with
//      write permission, unless MAP_PRIVATE was specified in the original mapping,
//      regardless of whether the file descriptors used to map the objects have since been
//      closed.
//
//      The implementation will require that addr be a multiple of the page size as
//      returned by sysconf().
//
//      The behaviour of this function is unspecified if the mapping was not established by
//      a call to mmap().
//
//      When mprotect() fails for reasons other than [EINVAL], the protections on some of
//      the pages in the range [addr, addr + len) may have been changed.
//
//  RETURN VALUE
//      Upon successful completion, mprotect() returns 0. Otherwise, it returns -1 and sets
//      errno to indicate the error.
//
//  ERRORS
//
//      The mprotect() function will fail if:
//
//      [EACCES]
//          The prot argument specifies a protection that violates the access permission
//          the process has to the underlying memory object.
//
//      [EAGAIN]
//          The prot argument specifies PROT_WRITE over a MAP_PRIVATE mapping and there are
//          insufficient memory resources to reserve for locking the private page.
//
//      [EINVAL]
//          The addr argument is not a multiple of the page size as returned by sysconf().
//
//      [ENOMEM]
//          Addresses in the range [addr, addr + len) are invalid for the address space of
//          a process, or specify one or more pages which are not mapped.
//
//      [ENOMEM]
//          The prot argument specifies PROT_WRITE on a MAP_PRIVATE mapping, and it would
//          require more space than the system is able to supply for locking the private
//          pages, if required.
//
//      [ENOTSUP]
//          The implementation does not support the combination of accesses requested in
//          the prot argument.
//
//  EXAMPLES
//      None.
*/
LIBW32_API int
mprotect(void *addr, size_t len, int prot)
{
    DWORD oldprot;
    DWORD ntprot;

    if ((prot & PROT_ALL) == 0) {               /* NONE */
        ntprot = PAGE_NOACCESS;

    } else {
        if (prot & PROT_WRITE) {                /* WRITE, implied READ */
            if (prot & PROT_EXEC) {
                ntprot = PAGE_EXECUTE_READWRITE;
            } else {
                ntprot = PAGE_READWRITE;
            }
        } else {                                /* READONLY */
            if (prot & PROT_EXEC) {
                ntprot = PAGE_EXECUTE_READ;
            } else {
                ntprot = PAGE_READONLY;
            }
        }
    }
    return VirtualProtect(addr, len, ntprot, &oldprot) ? 0 : -1;
}


/*
//  NAME
//      msync - synchronize memory with physical storage
//
//  SYNOPSIS
//      #include <sys/mman.h>
//
//      int msync(void *addr, size_t len, int flags);
//
//  DESCRIPTION
//      The msync() function shall write all modified data to permanent storage locations,
//      if any, in those whole pages containing any part of the address space of the
//      process starting at address addr and continuing for len bytes. If no such storage
//      exists, msync() need not have any effect. If requested, the msync() function shall
//      then invalidate cached copies of data.
//
//      The implementation shall require that addr be a multiple of the page size as returned
//      by sysconf().
//
//      For mappings to files, the msync() function shall ensure that all write operations
//      are completed as defined for synchronized I/O data integrity completion. It is
//      unspecified whether the implementation also writes out other file attributes. When
//      the msync() function is called on MAP_PRIVATE mappings, any modified data shall not
//      be written to the underlying object and shall not cause such data to be made
//      visible to other processes. It is unspecified whether data in MAP_PRIVATE mappings
//      has any permanent storage locations. [SHM|TYM] [Option Start] The effect of msync()
//      on a shared memory object or a typed memory object is unspecified. [Option End] The
//      behavior of this function is unspecified if the mapping was not established by a
//      call to mmap().
//
//      The flags argument is constructed from the bitwise-inclusive OR of one or more of
//      the following flags defined in the <sys/mman.h> header:
//
//          Symbolic Constant           Description
//          MS_ASYNC                    Perform asynchronous writes.
//          MS_SYNC                     Perform synchronous writes.
//          MS_INVALIDATE               Invalidate cached data.
//
//      When MS_ASYNC is specified, msync() shall return immediately once all the write
//      operations are initiated or queued for servicing; when MS_SYNC is specified,
//      msync() shall not return until all write operations are completed as defined for
//      synchronized I/O data integrity completion. Either MS_ASYNC or MS_SYNC is specified,
//      but not both.
//
//      When MS_INVALIDATE is specified, msync() shall invalidate all cached copies of
//      mapped data that are inconsistent with the permanent storage locations such that
//      subsequent references shall obtain data that was consistent with the permanent
//      storage locations sometime between the call to msync() and the first subsequent
//      memory reference to the data.
//
//      If msync() causes any write to a file, the file's st_ctime and st_mtime fields
//      shall be marked for update.
//
//
//  RETURN VALUE
//      Upon successful completion, msync() shall return 0; otherwise, it shall return -1
//      and set errno to indicate the error.
//
//  ERRORS
//
//      The msync() function shall fail if:
//
//      [EBUSY]
//          Some or all of the addresses in the range starting at addr and continuing for
//          len bytes are locked, and MS_INVALIDATE is specified.
//
//      [EINVAL]
//          The value of flags is invalid.
//
//      [EINVAL]
//          The value of addr is not a multiple of the page size {PAGESIZE}.
//
//      [ENOMEM]
//          The addresses in the range starting at addr and continuing for len bytes are
//          outside the range allowed for the address space of a process or specify one or
//          more pages that are not mapped.
*/
LIBW32_API int
msync(void *addr, size_t len, int flags)
{
    BOOL ret;

    if (FALSE == (ret = FlushViewOfFile(addr, len))) {
        errno = EINVAL;

    } else if (MS_SYNC & flags) {
        //TODO - need file handle.
    }

    return (ret ? 0 : -1);
}


/*
//  NAME
//      munmap - unmap pages of memory
//
//  SYNOPSIS
//      #include <sys/mman.h>
//
//      int munmap(void *addr, size_t len);
//
//  DESCRIPTION
//      The munmap() function shall remove any mappings for those entire pages containing
//      any part of the address space of the process starting at addr and continuing for
//      len bytes. Further references to these pages shall result in the generation of a
//      SIGSEGV signal to the process. If there are no mappings in the specified address
//      range, then munmap() has no effect.
//
//      The implementation shall require that addr be a multiple of the page size {
//      PAGESIZE}.
//
//      If a mapping to be removed was private, any modifications made in this address
//      range shall be discarded.
//
//      [ML|MLR] [Option Start] Any memory locks (see mlock() and mlockall() ) associated
//      with this address range shall be removed, as if by an appropriate call to
//      munlock(). [Option End]
//
//      [TYM] [Option Start] If a mapping removed from a typed memory object causes the
//      corresponding address range of the memory pool to be inaccessible by any process in
//      the system except through allocatable mappings (that is, mappings of typed memory
//      objects opened with the POSIX_TYPED_MEM_MAP_ALLOCATABLE flag), then that range of
//      the memory pool shall become deallocated and may become available to satisfy future
//      typed memory allocation requests.
//
//      A mapping removed from a typed memory object opened with the
//      POSIX_TYPED_MEM_MAP_ALLOCATABLE flag shall not affect in any way the availability
//      of that typed memory for allocation. [Option End]
//
//      The behavior of this function is unspecified if the mapping was not established by
//      a call to mmap().
//
//  RETURN VALUE
//
//      Upon successful completion, munmap() shall return 0; otherwise, it shall return -1
//      and set errno to indicate the error.
//
//  ERRORS
//
//      The munmap() function shall fail if:
//
//      [EINVAL]
//          Addresses in the range [addr, addr+len) are outside the valid range for the
//          address space of a process.
//
//      [EINVAL]
//          The len argument is 0.
//
//      [EINVAL]
//          The addr argument is not a multiple of the page size as returned by sysconf().
*/
LIBW32_API int
munmap(void *addr, size_t len)
{
    BOOL ret;

    (void) len;
    if (FALSE == (ret = UnmapViewOfFile(addr))) {
        errno = EINVAL;
    }
    return (ret ? 0 : -1);
}


LIBW32_API int
mlock(const void *addr, size_t len)
{
    BOOL ret;

    if (FALSE == (ret = VirtualLock((LPVOID)addr, len))) {
        errno = EINVAL;
    }
    return (ret ? 0 : -1);
}


LIBW32_API int
munlock(const void *addr, size_t len)
{
    BOOL ret;

    if (FALSE == (ret = VirtualUnlock((LPVOID)addr, len))) {
        errno = EINVAL;
    }
    return (ret ? 0 : -1);
}


//int mlock2(const void *addr, size_t len, int flags);
//int mlockall(int flags);
//int munlockall(void);

/*end*/

