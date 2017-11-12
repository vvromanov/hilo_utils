#include "time_utils.h"
#include <sys/time.h>
#include <time.h>

int64_t getTimeMs(void) {
    return getTimeUs() / 1000;
}

int64_t getTimeUs(void) {
    struct timeval val;
    gettimeofday(&val, nullptr);
    return val.tv_sec * ((int64_t) 1000000) + val.tv_usec;
}

int64_t getClockMs(void) {
    return getTimeUs() / 1000;
}

int64_t getClockUs(void) {
    struct timespec val;
#ifdef CYGWIN
    clock_gettime(CLOCK_REALTIME, &val);
#else
    clock_gettime(CLOCK_MONOTONIC_COARSE, &val);
#endif
    return val.tv_sec * ((int64_t) 1000000) + val.tv_nsec/1000;
}

const char *get_time_str(time_t t, bool include_date) {
    static char time_str_cache[]="XX-XX-XX ZZ:ZZ:ZZ  ";
    static time_t t_cache = -1;
    if (t != t_cache) {
        t_cache = t;
        if (include_date) {
            strftime(time_str_cache, sizeof(time_str_cache)-1, "%y-%m-%d %H:%M:%S", localtime(&t));
        } else {
            strftime(time_str_cache, sizeof(time_str_cache)-1, "%H:%M:%S", localtime(&t));
        }
    }
    return time_str_cache;
}

