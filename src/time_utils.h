#pragma once

#include <cstdint>
#include <ctime>
#include <sys/time.h>

static inline int64_t getTimeMs(struct timeval val) {
    return val.tv_sec * ((int64_t) 1000) + val.tv_usec / 1000;
}

static inline int64_t getTimeUs(void) {
    struct timeval val;
    gettimeofday(&val, nullptr);
    return val.tv_sec * ((int64_t) 1000000) + val.tv_usec;
}

static inline int64_t getTimeMs(void) {
    struct timeval val;
    gettimeofday(&val, nullptr);
    return getTimeMs(val);
}

static inline int64_t getClockUs(void) {
    struct timespec val;
#ifdef CYGWIN
    clock_gettime(CLOCK_REALTIME, &val);
#else
    clock_gettime(CLOCK_MONOTONIC, &val);
#endif
    return val.tv_sec * ((int64_t) 1000000) + val.tv_nsec / 1000;
}

static inline int64_t getClockMs(void) {
    struct timespec val;
#ifdef CYGWIN
    clock_gettime(CLOCK_REALTIME, &val);
#else
    clock_gettime(CLOCK_MONOTONIC, &val);
#endif
    return val.tv_sec * ((int64_t) 1000) + val.tv_nsec / 1000000;
}

const char *get_time_str(time_t t, bool include_date);

//Copy of ODBC TIMESTAMP_STRUCT
typedef struct {
    int16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t hour;
    uint16_t minute;
    uint16_t second;
    uint32_t fraction; // nanoseconds s/1000 000 000
} timestamp_t;

time_t ts2time(const timestamp_t &ts);

static inline int64_t ts2us(const timestamp_t &ts) {
    return ts2time(ts) * 1000000 + ts.fraction / 1000;
}

void time2ts(time_t t, timestamp_t &ts);
