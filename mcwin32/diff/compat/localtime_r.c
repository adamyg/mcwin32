/*
 * localtime_r: WIN32
 */
 
#include "libcompat.h"

//	#if !defined(HAVE_LOCALTIME_R)

struct tm *
localtime_r(const time_t *timep, struct tm *result)
{
	*result = *localtime(timep);
	return result;
}

//	#endif //HAVE_LOCALTIME_R

//end
