//
//  tzalloc
//

#if !defined(LIBCOMPAT_SOURCE)
#define LIBCOMPAT_SOURCE
#endif

#include "libcompat.h"

#include <sys/cdefs.h>
#include "tzalloc.h"

/*
//  NAME
//      tzset, tzalloc, tzgetname, tzgetgmtoff, tzfree - initialize time conversion information
//
//  SYNOPSIS
//      #include <time.h>
//
//      timezone_t
//      tzalloc(const char *zone);
//
//      void
//      tzfree(timezone_t restrict tz);
//
//      const char *
//      tzgetname(timezone_t restrict tz, int isdst);
//
//      long
//      tzgetgmtoff(timezone_t restrict tz, int isdst);
//
//      void
//      tzset(void);
//
//  DESCRIPTION
//      The tzalloc() function takes as an argument a timezone name and returns a timezone_t
//      object suitable to be used in the ctime_rz(), localtime_rz(), and mktime_z() functions.
//
//      If zone is not a valid timezone description, or if the object cannot be allocated,
//      tzalloc() returns a NULL pointer and sets errno.
//
//      A NULL pointer may be passed to tzalloc() instead of a timezone name, to refer to
//      the current system timezone. An empty timezone string indicates
//      coordinated Universal Time (UTC).
//
//      Note that instead of setting the environment variable TZ, and globally changing the
//      behavior of the calling program, one can use multiple timezones at the same time by using
//      separate timezone_t objects allocated by tzalloc() and calling the "z" variants of the
//      functions. The tzfree() function deallocates tz, which was previously allocated by
//      tzalloc(). This invalidates any tm_zone pointers that tz was used to set. The function
//      tzgetname() returns the name for the given tz. If isdst is 0, the call is equivalent to
//      tzname[0]. If isdst is set to 1 the call is equivalent to tzname[1]. The return values
//      for both tzgetname() and tzgmtoff() correspond to the latest time for which data is
//      available, even if that refers to a future time. Finally, the tzgetgmtoff() function acts
//      like tzgetname() only it returns the offset in seconds from GMT for the timezone. If
//      there is no match, then -1 is returned and errno is set to ESRCH. The tzset() function
//      acts like tzalloc(getenv("TZ")), except it saves any resulting timezone object into
//      internal storage that is accessed by localtime(), localtime_r(), and mktime(). The
//      anonymous shared timezone object is freed by the next call to tzset(). If the implied
//      call to tzalloc() fails, tzset() falls back on Universal Time (UT). If TZ is NULL, the
//      best available approximation to local (wall clock) time, as specified by the tzfile(5)
//      format file /etc/localtime is used by localtime(3). If TZ appears in the environment but
//      its value is the empty string, UT is used, with the abbreviation "UTC" and without leap
//      second correction; please see ctime(3). If TZ is nonnull and nonempty:
//
//      -   if the value begins with a colon, it is used as a pathname of a file
//          from which to read the time conversion information;
//
//      -   if the value does not begin with a colon, it is first used as the
//          pathname of a file from which to read the time conversion
//          information, and, if that file cannot be read, is used directly as a
//          specification of the time conversion information.
//
//      When TZ is used as a pathname, if it begins with a slash, it is used as an absolute
//      pathname; otherwise, it is used as a pathname relative to /usr/share/zoneinfo.
//      The file must be in the format specified in tzfile(5).
//
*/

timezone_t
tzalloc(const char *zone)
{
        return NULL;
}


const char *
tzgetname(timezone_t tz, int isdst)
{
        return "";
}


long
tzgetgmtoff(timezone_t tz, int isdst)
{
        return 0;
}


void
tzfree(timezone_t tz)
{
}


locale_t
_current_locale()
{
        static struct LC_Time_Locale lc_time_locale = {
                    // 
                    //  locale -k LC_TIME
                    //
                    { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" },
                    { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" },
                    { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" },
                    { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" },
                    { "AM", "PM" },
                    "%a %d %b %Y %r %Z",
                    "%m/%d/%Y",
                    "%r",
                    "%I:%M:%S %p",
                    "%a %d %b %Y %r %Z",
                    "UTF-8"
                };
        return (locale_t)(&lc_time_locale);
}

//end
