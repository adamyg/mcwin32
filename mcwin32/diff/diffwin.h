#pragma once
/*-
 * diff --- Window support.
 */

#include "diffbool.h"

#if defined(__GNUC__)   /*BOOST_GCC_VERSION equiv*/
#if !defined(GCC_VERSION)
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif
#endif

bool iscolorconsole(void);

//end
