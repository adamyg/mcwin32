/*
 *  win32 work-arounds
 */

typedef unsigned int u_int32_t;
typedef int ssize_t;

#define _POSIX2_RE_DUP_MAX              255
#define _DIAGASSERT(__x)                /*not used*/
#define __UNCONST(__s)                  ((char *) __s)

#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE	/* disable deprecate warnings */
#endif

#define snprintf _snprintf
#define vsnprintf _csnprintf
