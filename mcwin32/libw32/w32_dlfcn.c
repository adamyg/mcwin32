#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_dlfcn_c,"$Id: w32_dlfcn.c,v 1.3 2022/03/16 13:46:59 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <dlfcn.h> - dynamic library loader system calls.
 *
 *  dlopen, dlsym, dlclose and dlerror
 *
 * Copyright (c) 1998 - 2022, Adam Young.
 *
 * This file is part of the Midnight Commander.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 * or (at your option) any later version.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained 
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#include "win32_internal.h"

#include <dlfcn.h>

#include <tailqueue.h>

#define DLERROR_LEN             1024            /* error buffer size */

typedef TAILQ_HEAD(globallibs, globallib)
                    globallibs_t;               /* name global module list */

typedef struct globallib {
    unsigned                    g_magic;
    TAILQ_ENTRY(globallib)      g_node;
    HANDLE                      g_handle;
    unsigned                    g_references;
    char                        g_name[1];
} globallib_t;

__declspec(thread) static char  x_dlerror[DLERROR_LEN];

static unsigned                 x_dlopen;
static CRITICAL_SECTION         x_guard;
static int                      x_modules;
static globallibs_t             x_globals;

static globallib_t *            mod_find(HMODULE handle);
static globallib_t *            mod_pushA(HMODULE handle, const char *file);
static globallib_t *            mod_pushW(HMODULE handle, const wchar_t *file);

static void                     dlerror_setA(const char *file, const char *msg);
static void                     dlerror_lastA(const char *file);
static void                     dlerror_setW(const wchar_t *file, const char *msg);
static void                     dlerror_lastW(const wchar_t *file);

#define HARD_ERRORS             UINT __hardmode;
#define HARD_ERRORS_DISABLE     __hardmode = SetErrorMode (0);
#define HARD_ERRORS_ENABLE      SetErrorMode (__hardmode);

#include <sys/rwlock.h>


/*
//  NAME
//      dlopen - gain access to an executable object file
//
//  SYNOPSIS
//      #include <dlfcn.h>
//      void *dlopen(const char *file, int mode);
//
//  DESCRIPTION
//      The dlopen() function shall make an executable object file specified by file
//      available to the calling program. The class of files eligible for this operation
//      and the manner of their construction are implementation-defined, though typically
//      such files are executable objects such as shared libraries, relocatable files, or
//      programs. Note that some implementations permit the construction of dependencies
//      between such objects that are embedded within files. In such cases, a dlopen()
//      operation shall load such dependencies in addition to the object referenced by
//      file. Implementations may also impose specific constraints on the construction of
//      programs that can employ dlopen() and its related services.
//
//      A successful dlopen() shall return a handle which the caller may use on subsequent
//      calls to dlsym() and dlclose(). The value of this handle should not be interpreted
//      in any way by the caller.
//
//      The file argument is used to construct a pathname to the object file. If file
//      contains a slash character, the file argument is used as the pathname for the file.
//      Otherwise, file is used in an implementation-defined manner to yield a pathname.
//
//      If the value of file is 0, dlopen() shall provide a handle on a global symbol
//      object. This object shall provide access to the symbols from an ordered set of
//      objects consisting of the original program image file, together with any objects
//      loaded at program start-up as specified by that process image file (for example,
//      shared libraries), and the set of objects loaded using a dlopen() operation
//      together with the RTLD_GLOBAL flag. As the latter set of objects can change during
//      execution, the set identified by handle can also change dynamically.
//
//      Only a single copy of an object file is brought into the address space, even if
//      dlopen() is invoked multiple times in reference to the file, and even if different
//      pathnames are used to reference the file.
//
//      The mode parameter describes how dlopen() shall operate upon file with respect to
//      the processing of relocations and the scope of visibility of the symbols provided
//      within file. When an object is brought into the address space of a process, it may
//      contain references to symbols whose addresses are not known until the object is
//      loaded. These references shall be relocated before the symbols can be accessed. The
//      mode parameter governs when these relocations take place and may have the following
//      values:
//
//      RTLD_LAZY
//          Relocations shall be performed at an implementation-defined time, ranging from
//          the time of the dlopen() call until the first reference to a given symbol
//          occurs. Specifying RTLD_LAZY should improve performance on implementations
//          supporting dynamic symbol binding as a process may not reference all of the
//          functions in any given object. And, for systems supporting dynamic symbol
//          resolution for normal process execution, this behavior mimics the normal
//          handling of process execution.
//
//      RTLD_NOW
//          All necessary relocations shall be performed when the object is first loaded.
//          This may waste some processing if relocations are performed for functions that
//          are never referenced. This behavior may be useful for applications that need to
//          know as soon as an object is loaded that all symbols referenced during
//          execution are available.
//
//      Any object loaded by dlopen() that requires relocations against global symbols can
//      reference the symbols in the original process image file, any objects loaded at
//      program start-up, from the object itself as well as any other object included in
//      the same dlopen() invocation, and any objects that were loaded in any dlopen()
//      invocation and which specified the RTLD_GLOBAL flag. To determine the scope of
//      visibility for the symbols loaded with a dlopen() invocation, the mode parameter
//      should be a bitwise-inclusive OR with one of the following values:
//
//      RTLD_GLOBAL
//          The object's symbols shall be made available for the relocation processing of
//          any other object. In addition, symbol lookup using dlopen(0, mode) and an
//          associated dlsym() allows objects loaded with this mode to be searched.
//
//      RTLD_LOCAL
//          The object's symbols shall not be made available for the relocation processing
//          of any other object.
//
//      If neither RTLD_GLOBAL nor RTLD_LOCAL are specified, then the default behavior is
//      unspecified.
//
//      If a file is specified in multiple dlopen() invocations, mode is interpreted at
//      each invocation. Note, however, that once RTLD_NOW has been specified all
//      relocations shall have been completed rendering further RTLD_NOW operations
//      redundant and any further RTLD_LAZY operations irrelevant. Similarly, note that
//      once RTLD_GLOBAL has been specified the object shall maintain the RTLD_GLOBAL
//      status regardless of any previous or future specification of RTLD_LOCAL, as long as
//      the object remains in the address space (see dlclose()).
//
//      Symbols introduced into a program through calls to dlopen() may be used in
//      relocation activities. Symbols so introduced may duplicate symbols already defined
//      by the program or previous dlopen() operations. To resolve the ambiguities such a
//      situation might present, the resolution of a symbol reference to symbol definition
//      is based on a symbol resolution order. Two such resolution orders are defined: load
//      or dependency ordering. Load order establishes an ordering among symbol definitions,
//      such that the definition first loaded (including definitions from the image file
//      and any dependent objects loaded with it) has priority over objects added later
//      (via dlopen()). Load ordering is used in relocation processing. Dependency ordering
//      uses a breadth-first order starting with a given object, then all of its
//      dependencies, then any dependents of those, iterating until all dependencies are
//      satisfied. With the exception of the global symbol object obtained via a dlopen()
//      operation on a file of 0, dependency ordering is used by the dlsym() function. Load
//      ordering is used in dlsym() operations upon the global symbol object.
//
//      When an object is first made accessible via dlopen() it and its dependent objects
//      are added in dependency order. Once all the objects are added, relocations are
//      performed using load order. Note that if an object or its dependencies had been
//      previously loaded, the load and dependency orders may yield different resolutions.
//
//      The symbols introduced by dlopen() operations and available through dlsym() are at
//      a minimum those which are exported as symbols of global scope by the object.
//      Typically such symbols shall be those that were specified in (for example) C source
//      code as having extern linkage. The precise manner in which an implementation
//      constructs the set of exported symbols for a dlopen() object is specified by that
//      implementation.
//
//  RETURN VALUE
//      If file cannot be found, cannot be opened for reading, is not of an appropriate
//      object format for processing by dlopen(), or if an error occurs during the process
//      of loading file or relocating its symbolic references, dlopen() shall return NULL.
//      More detailed diagnostic information shall be available through dlerror().
//
//  ERRORS
//      No errors are defined.
*/

LIBW32_API void *
dlopen(const char *file, int mode)
{
#if defined(UTF8FILENAMES)
    if (file && w32_utf8filenames_state()) {
        wchar_t *wfile = NULL;
        void *ret = NULL;

        if (NULL != (wfile = w32_utf2wca(file, NULL))) {
            ret = dlopenW(wfile, mode);
            free((void *)wfile);
        }
        return ret;
    }
#endif  //UTF8FILENAMES

    return dlopenA(file, mode);
}


LIBW32_API void *
dlopenA(const char *file, int mode)
{
    HMODULE hm = 0;

    if (NULL == file || !*file) {
        dlerror_lastA("missing file");
        return NULL;
    }

    if (0 == x_dlopen) {                        // runtime initialisation
        InitializeCriticalSection(&x_guard);
        TAILQ_INIT(&x_globals);
        ++x_dlopen;
    }

    if (NULL == file) {				// global handle
        if (! (hm = GetModuleHandle(NULL))) {
            dlerror_lastA("global handle");
        }
    } else {					// module specific
        HARD_ERRORS
        const char *cursor;
        char t_file[MAX_PATH];
        unsigned i;
                                                // import and convert
        for (cursor = file, i = 0; *cursor && i < sizeof(t_file)-1; ++i, ++cursor) {
            char c;

            if ('/' == (c = *cursor) || '\\' == c) {
                if (i) {                        // compress
                    while (0 != (c = cursor[1]) && ('/' == c || '\\' == c)) {
                        ++cursor;
                    }
                }
                t_file[i] = '\\';
            } else {
                t_file[i] = c;
            }
        }
        t_file[i] = 0;

        HARD_ERRORS_DISABLE
        if (0 == (hm = LoadLibraryA(t_file))) {
            dlerror_lastA(t_file);

        } else if (RTLD_GLOBAL & mode) {        // global
            globallib_t *lib;

            EnterCriticalSection(&x_guard);
            if (NULL != (lib = mod_find(hm))) {
                ++lib->g_references;
            } else {
                lib = mod_pushA(hm, t_file);
            }
            LeaveCriticalSection(&x_guard);
            if (! lib) {
                dlerror_setA(t_file, "memory allocation error");
                (void) FreeLibrary(hm);
                hm = 0;
            }
        }
        HARD_ERRORS_ENABLE
    }
    return (void *)hm;
}


LIBW32_API void *
dlopenW(const wchar_t *file, int mode)
{
    HMODULE hm = 0;

    if (NULL == file || !*file) {
        dlerror_lastA("missing file");
        return NULL;
    }

    if (0 == x_dlopen) {                        // runtime initialisation
        InitializeCriticalSection(&x_guard);
        TAILQ_INIT(&x_globals);
        ++x_dlopen;
    }

    if (NULL == file) {				// global handle
        if (! (hm = GetModuleHandle(NULL))) {
            dlerror_lastA("global handle");
        }

    } else {					// module specific
        HARD_ERRORS
        const wchar_t *cursor;
        wchar_t t_file[MAX_PATH*2];
        unsigned i;
                                                // import and convert
        for (cursor = file, i = 0; *cursor && i < _countof(t_file)-1; ++i, ++cursor) {
            wchar_t c;

            if ('/' == (c = *cursor) || '\\' == c) {
                if (i) {                        // compress
                    while (0 != (c = cursor[1]) && ('/' == c || '\\' == c)) {
                        ++cursor;
                    }
                }
                t_file[i] = '\\';
            } else {
                t_file[i] = c;
            }
        }
        t_file[i] = 0;

        HARD_ERRORS_DISABLE
        if (0 == (hm = LoadLibraryW(t_file))) {
            dlerror_lastW(t_file);

        } else if (RTLD_GLOBAL & mode) {        // global
            globallib_t *lib;

            EnterCriticalSection(&x_guard);
            if (NULL != (lib = mod_find(hm))) {
                ++lib->g_references;
            } else {
                lib = mod_pushW(hm, t_file);
            }
            LeaveCriticalSection(&x_guard);
            if (! lib) {
                dlerror_setW(t_file, "memory allocation error");
                (void) FreeLibrary(hm);
                hm = 0;
            }
        }
        HARD_ERRORS_ENABLE
    }
    return (void *)hm;
}


/*
//  NAME
//      dlsym - obtain the address of a symbol from a dlopen object
//
//  SYNOPSIS
//      #include <dlfcn.h>
//      void *dlsym(void *restrict handle, const char *restrict name);
//
//  DESCRIPTION
//      The dlsym() function shall obtain the address of a symbol defined within an object
//      made accessible through a dlopen() call. The handle argument is the value returned
//      from a call to dlopen() (and which has not since been released via a call to
//      dlclose()), and name is the symbol's name as a character string.
//
//      The dlsym() function shall search for the named symbol in all objects loaded
//      automatically as a result of loading the object referenced by handle (see
//      dlopen()). Load ordering is used in dlsym() operations upon the global symbol
//      object. The symbol resolution algorithm used shall be dependency order as described
//      in dlopen().
//
//      The RTLD_DEFAULT and RTLD_NEXT flags are reserved for future use.
//
//  RETURN VALUE
//      If handle does not refer to a valid object opened by dlopen(), or if the named
//      symbol cannot be found within any of the objects associated with handle, dlsym()
//      shall return NULL. More detailed diagnostic information shall be available through
//      dlerror().
//
//  ERRORS
//      No errors are defined.
*/
LIBW32_API void *
dlsym(void *__restrict handle, const char *__restrict name)
{
    HMODULE hm = (HMODULE) handle;
    FARPROC symbol;

    if (NULL == (symbol = GetProcAddress(hm, name))) {
        if (x_modules) {
            HANDLE hModule = GetModuleHandle(NULL);

            if (hm == hModule) {
                globallibs_t *libs = &x_globals;
                globallib_t *lib;

                EnterCriticalSection(&x_guard);
                TAILQ_FOREACH(lib, libs, g_node) {
                    if (NULL != (symbol = GetProcAddress(lib->g_handle, name))) {
                        break;
                    }
                }
                LeaveCriticalSection(&x_guard);
            }
            CloseHandle(hModule);
        }
    }
    return (void *)symbol;
}


/*
//  NAME
//      dlclose - close a dlopen object
//
//  SYNOPSIS
//      #include <dlfcn.h>
//      int dlclose(void *handle);
//
//  DESCRIPTION
//      The dlclose() function shall inform the system that the object referenced by a
//      handle returned from a previous dlopen() invocation is no longer needed by the
//      application.
//
//      The use of dlclose() reflects a statement of intent on the part of the process, but
//      does not create any requirement upon the implementation, such as removal of the
//      code or symbols referenced by handle. Once an object has been closed using
//      dlclose() an application should assume that its symbols are no longer available to
//      dlsym(). All objects loaded automatically as a result of invoking dlopen() on the
//      referenced object shall also be closed if this is the last reference to it.
//
//      Although a dlclose() operation is not required to remove structures from an address
//      space, neither is an implementation prohibited from doing so. The only restriction
//      on such a removal is that no object shall be removed to which references have been
//      relocated, until or unless all such references are removed. For instance, an object
//      that had been loaded with a dlopen() operation specifying the RTLD_GLOBAL flag
//      might provide a target for dynamic relocations performed in the processing of other
//      objects-in such environments, an application may assume that no relocation, once
//      made, shall be undone or remade unless the object requiring the relocation has
//      itself been removed.
//
//  RETURN VALUE
//      If the referenced object was successfully closed, dlclose() shall return 0. If the
//      object could not be closed, or if handle does not refer to an open object,
//      dlclose() shall return a non-zero value. More detailed diagnostic information shall
//      be available through dlerror().
//
//  ERRORS
//      No errors are defined.
*/
LIBW32_API int
dlclose(void *handle)
{
    HMODULE hm = (HMODULE) handle;

    if (! FreeLibrary(hm)) {
        globallib_t *lib;

        if (NULL != (lib = mod_find(hm))) {
            dlerror_lastA(lib->g_name);
        } else {
            dlerror_lastA(NULL);
        }
        return -1;

    } else {
        globallib_t *lib;

        EnterCriticalSection(&x_guard);
        if (NULL != (lib = mod_find(hm))) {
            if (0 != --lib->g_references) {
                lib = NULL;
            } else {
                TAILQ_REMOVE(&x_globals, lib, g_node);
                --x_modules;
            }
        }
        LeaveCriticalSection(&x_guard);
        free(lib);
    }
    return 0;
}


/*
//  NAME
//      dlerror - get diagnostic information
//
//  SYNOPSIS
//      #include <dlfcn.h>
//      char *dlerror(void);
//
//  DESCRIPTION
//      The dlerror() function shall return a null-terminated character string (with no
//      trailing <newline>) that describes the last error that occurred during dynamic
//      linking processing. If no dynamic linking errors have occurred since the last
//      invocation of dlerror(), dlerror() shall return NULL. Thus, invoking dlerror() a
//      second time, immediately following a prior invocation, shall result in NULL being
//      returned.
//
//      The dlerror() function need not be reentrant. A function that is not required to be
//      reentrant is not required to be thread-safe.
//
//  RETURN VALUE
//      If successful, dlerror() shall return a null-terminated character string; otherwise,
//      NULL shall be returned.
//
//  ERRORS
//      No errors are defined.
*/
LIBW32_API char *
dlerror(void)
{
    return x_dlerror;
}


static globallib_t *
mod_find(HMODULE handle)
{
    globallibs_t *libs = &x_globals;
    globallib_t *lib;

    TAILQ_FOREACH(lib, libs, g_node) {
        if (handle == lib->g_handle) {
            return lib;
        }
    }
    return NULL;
}


static globallib_t *
mod_pushA(HMODULE handle, const char *file)
{
    const size_t len = strlen(file);
    globallibs_t *libs = &x_globals;
    globallib_t *lib;

    if (NULL != (lib = calloc(sizeof(globallib_t) + len, 1))) {
        lib->g_handle = handle;
        lib->g_references = 1;
        memcpy(lib->g_name, file, len + 1);
        TAILQ_INSERT_TAIL(libs, lib, g_node);
        ++x_modules;
        return lib;
    }
    return NULL;
}


static globallib_t *
mod_pushW(HMODULE handle, const wchar_t *file)
{
    globallib_t *lib = NULL;
    char *t_file = NULL;

    if (file && NULL != (t_file = w32_wc2utfa(file, NULL))) {
        lib = mod_pushA(handle, t_file); 
        free(t_file);
    }
    return lib;
}


static void
dlerror_setA(const char *file, const char *msg)
{
    if (msg) {
        _snprintf(x_dlerror, sizeof(x_dlerror), "%s : %s", file, msg);
    } else {
        strncpy(x_dlerror, file, sizeof(x_dlerror));
    }
    x_dlerror[sizeof(x_dlerror)-1] = 0;
}


static void
dlerror_lastA(const char *file)
{
    x_dlerror[0] = 0;

    if (file && *file) {
        int len;

        len = _snprintf(x_dlerror, sizeof(x_dlerror), "%s : ", file);
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0,
            x_dlerror, sizeof(x_dlerror) - len, NULL);
        x_dlerror[sizeof(x_dlerror)-1] = 0;
    }
}


static void
dlerror_setW(const wchar_t *file, const char *msg)
{
    char *t_file = NULL;

    x_dlerror[0] = 0;
    if ( NULL != (t_file = w32_wc2utfa(file, NULL))) {
        dlerror_setA(t_file, msg);
        free(t_file);
    }
}


static void
dlerror_lastW(const wchar_t *file)
{
    char *t_file = NULL;

    x_dlerror[0] = 0;
    if (NULL != (t_file = w32_wc2utfa(file, NULL))) {
        dlerror_lastA(t_file);
        free(t_file);
    }
}

/*end*/
