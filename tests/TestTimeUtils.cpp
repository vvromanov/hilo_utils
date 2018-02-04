#include <sys/time.h>
#include <gtest/gtest.h>
#include <time_utils.h>
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
    ASSERT_TRUE(delta >= 0);
    ASSERT_GE(now_lib_us, now_us);
    ASSERT_LT(now_us - now_lib_us, 10);
}

static bool
test_time(int64_t expected, int16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t min, uint16_t sec) {
    struct tm tm;
    tm.tm_isdst = 0;
    tm.tm_gmtoff = 0;
    timestamp_t ts;
    ts.year = year;
    tm.tm_year = year - 1900;
    ts.month = month;
    tm.tm_mon = ts.month - 1;
    tm.tm_mday = ts.day = day;
    tm.tm_hour = ts.hour = hour;
    tm.tm_min = ts.minute = min;
    tm.tm_sec = ts.second = sec;
    ts.fraction = 0;
    time_t t = timegm(&tm);
    time_t t2 = ts2time(ts);

//    int64_t result = ts2us(ts);
    EXPECT_EQ(expected, t);
    EXPECT_EQ(expected, t2);
    timestamp_t ts2;
    time2ts(expected, ts2);
    EXPECT_EQ(year, ts2.year);
    EXPECT_EQ(month, ts2.month);
    EXPECT_EQ(day, ts2.day);
    EXPECT_EQ(hour, ts2.hour);
    EXPECT_EQ(min, ts2.minute);
    EXPECT_EQ(sec, ts2.second);
    return expected == t;
}

TEST(TestTimeUtils, ts2utc) {
    EXPECT_TRUE(test_time(0, 1970, 1, 1, 0, 0, 0));
    EXPECT_TRUE(test_time(536457599, 1986, 12, 31, 23, 59, 59));
    EXPECT_TRUE(test_time(1234567890, 2009, 2, 13, 23, 31, 30));
    EXPECT_TRUE(test_time(2147483647, 2038, 1, 19, 3, 14, 7));
    EXPECT_TRUE(test_time(2147483648, 2038, 1, 19, 3, 14, 8));
    EXPECT_TRUE(test_time(4294967295, 2106, 2, 7, 6, 28, 15));
    EXPECT_TRUE(test_time(4294967296, 2106, 2, 7, 6, 28, 16));
    EXPECT_TRUE(test_time(7726651512, 2214, 11, 6, 20, 5, 12));
    EXPECT_TRUE(test_time(12807349704, 2375, 11, 7, 5, 8, 24));
    EXPECT_TRUE(test_time(17007251228, 2508, 12, 9, 4, 27, 8));
    EXPECT_TRUE(test_time(21832612682, 2661, 11, 6, 6, 38, 2));
    EXPECT_TRUE(test_time(27223353970, 2832, 9, 3, 2, 46, 10));
    EXPECT_TRUE(test_time(30336942789, 2931, 5, 5, 0, 33, 9));
    EXPECT_TRUE(test_time(31972020437, 2983, 2, 25, 12, 47, 17));
}