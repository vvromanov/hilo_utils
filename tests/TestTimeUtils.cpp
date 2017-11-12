#include <sys/time.h>
#include <gtest/gtest.h>
#include "time_utils.h"

TEST(TestTimeUtils, GetTimeMs) {
    struct timeval val;
    gettimeofday(&val, NULL);
    int64_t now_ms = val.tv_sec * ((int64_t) 1000) + val.tv_usec / 1000;

    int64_t now_lib_ms = getTimeMs();
    ASSERT_GE(now_lib_ms, now_ms);
    ASSERT_LT(now_ms - now_lib_ms, 10);
}

TEST(TestTimeUtils, GetTimeUs) {
    struct timeval val;
    gettimeofday(&val, NULL);
    int64_t now_us = val.tv_sec * ((int64_t) 1000000) + val.tv_usec;

    int64_t now_lib_us = getTimeUs();
    ASSERT_GE(now_lib_us, now_us);
    ASSERT_LT(now_us - now_lib_us, 10);
}

TEST(TestTimeUtils, GetClockMs) {
    struct timespec val;
    clock_gettime(CLOCK_MONOTONIC, &val);
    int64_t now_ms = val.tv_sec * ((int64_t) 1000) + val.tv_nsec / 1000000;

    int64_t now_lib_ms = getClockMs();
    ASSERT_GE(now_lib_ms, now_ms);
    ASSERT_LT(now_ms - now_lib_ms, 10);
}

TEST(TestTimeUtils, GetClockUs) {
    struct timespec val;
    clock_gettime(CLOCK_MONOTONIC, &val);
    int64_t now_us = val.tv_sec * ((int64_t) 1000000) + val.tv_nsec / 1000;

    int64_t now_lib_us = getClockUs();
    int64_t delta = now_lib_us - now_us;
    ASSERT_TRUE(delta>=0);
    ASSERT_GE(now_lib_us, now_us);
    ASSERT_LT(now_us - now_lib_us, 10);
}

