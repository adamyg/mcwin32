#pragma once
/*-
 * diff --- bool.
 */

#if defined(WIN32)
typedef unsigned char bool;
#define true 1
#define false 0

#else
#include <stdbool.h>
#endif

//end
