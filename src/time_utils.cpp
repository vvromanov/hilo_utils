#include "time_utils.h"
#include <sys/time.h>
#include <time.h>
#include <iomanip>

bool clock_override;
int64_t clock_override_us;

//static __inline__ uint64_t rdtsc(void)
//{
//    unsigned hi, lo;
//    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
//    return ( lo)|(((uint64_t)hi)<<32);
//}

const char *get_time_str(time_t t, bool include_date) {
    static char time_str_cache[] = "XX-XX-XX ZZ:ZZ:ZZ  ";
    static time_t t_cache = -1;
    if (t != t_cache) {
        t_cache = t;
        if (include_date) {
            strftime(time_str_cache, sizeof(time_str_cache) - 1, "%y-%m-%d %H:%M:%S", localtime(&t));
        } else {
            strftime(time_str_cache, sizeof(time_str_cache) - 1, "%H:%M:%S", localtime(&t));
        }
    }
    return time_str_cache;
}

constexpr static inline int32_t is_leap(int32_t year) {
    return (year % 400 == 0) ? 1 : ((year % 100 == 0) ? 0 : ((year % 4 == 0) ? 1 : 0));
}

constexpr static inline int32_t days_from_0(int32_t year) {
    return 365 * (year - 1) + ((year - 1) / 400) - ((year - 1) / 100) + ((year - 1) / 4);
}

inline int32_t days_from_1970(int32_t year) {
    constexpr static const int days_from_0_to_1970 = days_from_0(1970);
    return days_from_0(year) - days_from_0_to_1970;
}

inline int32_t days_from_1jan(int32_t year, int32_t month, int32_t day) {
    static const int32_t days[2][13] =
            {
                    {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
                    {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
            };
    return days[is_leap(year)][month] + day - 1;
}

time_t ts2time(const timestamp_t &t) {
    int day_of_year = days_from_1jan(t.year, t.month, t.day);
    int days_since_epoch = days_from_1970(t.year) + day_of_year;
    constexpr time_t seconds_in_day = 3600 * 24;
    return seconds_in_day * days_since_epoch + 3600 * t.hour + 60 * t.minute + t.second;
}


void time2ts(time_t t, timestamp_t &ts) {
    struct tm tm;
    gmtime_r(&t, &tm);
    ts.fraction = 0;
    ts.second = tm.tm_sec;
    ts.minute = tm.tm_min;
    ts.hour = tm.tm_hour;
    ts.day = tm.tm_mday;
    ts.month = tm.tm_mon + 1;
    ts.year = tm.tm_year + 1900;
}

void us2ts(int64_t us, timestamp_t &ts) {
    time2ts(us / 1000'000, ts);
    ts.fraction = (us % 1000'000) * 1000;
}

void DumpTs(std::ostream &os, const timestamp_t &ts) {
    char tmp[100];
    snprintf(tmp, sizeof(tmp), "%d/%02d/%02d %02d:%02d:%02d.%03d", ts.year, ts.month, ts.day, ts.hour, ts.minute,
             ts.second, ts.fraction / 1000'000);
    os<<tmp;
}
