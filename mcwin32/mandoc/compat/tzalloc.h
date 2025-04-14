//
//  tzalloc
//

struct LC_Time_Locale {
    // locale -k LC_TIME
    const char *abday[7];           // "Sun;Mon;Tue;Wed;Thu;Fri;Sat"
    const char *day[7];             // "Sunday;Monday;Tuesday;Wednesday;Thursday;Friday;Saturday"
    const char *abmon[12];          // "Jan;Feb;Mar;Apr;May;Jun;Jul;Aug;Sep;Oct;Nov;Dec"
    const char *mon[12];            // "January;February;March;April;May;June;July;August;September;October;November;December"
    const char *am_pm[2];           // "AM;PM"
    const char *d_t_fmt;            // "%a %d %b %Y %r %Z"
    const char *d_fmt;              // "%m/%d/%Y"
    const char *t_fmt;              // "%r"
    const char *t_fmt_ampm;         // "%I:%M:%S %p"
    const char *date_fmt;           // "%a %d %b %Y %r %Z"
    const char *time_codeset;       // "UTF-8"
};

typedef void * timezone_t;

#define _TIME_LOCALE(loc) \
    ((const struct LC_Time_Locale *)loc)
    
#define isleap(y) \
    (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

#define isleap_sum(a, b) \
    isleap((a) % 400 + (b) % 400)

timezone_t tzalloc(const char *zone);

const char *tzgetname(timezone_t tz, int isdst);

long tzgetgmtoff(timezone_t tz, int isdst);

void tzfree(timezone_t tz);

locale_t _current_locale(void);

//end
