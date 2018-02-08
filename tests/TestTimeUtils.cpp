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
test_ts2time(int64_t expected, int16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t min, uint16_t sec) {
    timestamp_t ts;
    ts.year = year;
    ts.month = month;
    ts.day = day;
    ts.hour = hour;
    ts.minute = min;
    ts.second = sec;
    ts.fraction = 0;
    time_t t = ts2time(ts);
    EXPECT_EQ(expected, t);
    return expected == t;
}

static bool
test_timegm(int64_t expected, int16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t min, uint16_t sec) {
    struct tm tm;
    tm.tm_isdst = 0;
    tm.tm_gmtoff = 0;
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    time_t t = timegm(&tm);
    EXPECT_EQ(expected, t);
    return expected == t;
}

static bool
test_time2ts(int64_t expected, int16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t min, uint16_t sec) {
    timestamp_t ts;
    time2ts(expected, ts);
    EXPECT_EQ(year, ts.year);
    EXPECT_EQ(month, ts.month);
    EXPECT_EQ(day, ts.day);
    EXPECT_EQ(hour, ts.hour);
    EXPECT_EQ(min, ts.minute);
    EXPECT_EQ(sec, ts.second);
    EXPECT_EQ(0, ts.fraction);
    return (year == ts.year)
           && (month == ts.month)
           && (day == ts.day)
           && (hour == ts.hour)
           && (min == ts.minute)
           && (sec == ts.second)
           && (0 == ts.fraction);
}

static bool test_DumpTs(int64_t time, const char* expected) {
    timestamp_t ts;
    time2ts(time, ts);
    std::ostringstream os;
    DumpTs(os, ts);
    EXPECT_STREQ(expected, os.str().c_str());
    return os.str().c_str(), expected;
}

#define N(x) (1##x-100)

#define TEST_TIMES \
  TEST_TIME(          0, 1970, 01, 01, 00, 00, 00)\
  TEST_TIME(  536457599, 1986, 12, 31, 23, 59, 59)\
  TEST_TIME( 1234567890, 2009, 02, 13, 23, 31, 30)\
  TEST_TIME( 2147483647, 2038, 01, 19, 03, 14, 07)\
  TEST_TIME( 2147483648, 2038, 01, 19, 03, 14, 08)\
  TEST_TIME( 4294967295, 2106, 02, 07, 06, 28, 15)\
  TEST_TIME( 4294967296, 2106, 02, 07, 06, 28, 16)\
  TEST_TIME( 7726651512, 2214, 11, 06, 20, 05, 12)\
  TEST_TIME(12807349704, 2375, 11, 07, 05, 08, 24)\
  TEST_TIME(17007251228, 2508, 12, 09, 04, 27, 08)\
  TEST_TIME(21832612682, 2661, 11, 06, 06, 38, 02)\
  TEST_TIME(27223353970, 2832, 09, 03, 02, 46, 10)\
  TEST_TIME(30336942789, 2931, 05, 05, 00, 33, 09)\
  TEST_TIME(31972020437, 2983, 02, 25, 12, 47, 17)\


TEST(TestTimeUtils, timegm) {
#define TEST_TIME(tt, year, month, day, hour, minute, second) \
    EXPECT_TRUE(test_timegm(tt, year, N(month), N(day), N(hour), N(minute), N(second)));
    TEST_TIMES;
#undef TEST_TIME
}

TEST(TestTimeUtils, ts2time) {
#define TEST_TIME(tt, year, month, day, hour, minute, second) \
    EXPECT_TRUE(test_ts2time(tt, year, N(month), N(day), N(hour), N(minute), N(second)));
    TEST_TIMES;
#undef TEST_TIME
}

TEST(TestTimeUtils, time2ts) {
#define TEST_TIME(tt, year, month, day, hour, minute, second) \
    EXPECT_TRUE(test_time2ts(tt, year, N(month), N(day), N(hour), N(minute), N(second)));
    TEST_TIMES;
#undef TEST_TIME
}

TEST(TestTimeUtils, DumpTs) {
#define TEST_TIME(tt, year, month, day, hour, minute, second) \
    EXPECT_TRUE(test_DumpTs(tt, #year "/" #month "/" #day " " #hour ":" #minute ":" #second ".000"));
    TEST_TIMES;
#undef TEST_TIME
}

