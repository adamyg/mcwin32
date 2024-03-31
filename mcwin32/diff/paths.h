/*
 * paths.h: WIN32
 */

#define _PATH_DEVNULL		"NUL"
#define _PATH_TMP		"/tmp"

#if !defined(MIN)
#define MIN(__a,__b)		(((__a)<(__b))?(__a):(__b))
#define MAX(__a,__b)		(((__a)>(__b))?(__a):(__b))
#endif

#if !defined(PATH_MAX)
#define PATH_MAX		1024
#endif

//end
