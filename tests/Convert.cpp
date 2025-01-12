#include <cstdint>
#include <gtest/gtest.h>
#include <common_utils.h>

TEST(Convert, Ip) {
    uint32_t ip_network;
    EXPECT_FALSE(ConvertStringIp(nullptr, ip_network));
    EXPECT_TRUE(ConvertStringIp("11.12.13.14", ip_network));
    EXPECT_EQ(CREATE_NETWORK_IP(11, 12, 13, 14), ip_network);
}

TEST(Convert, Int8) {
    int8_t v;
    EXPECT_FALSE(ConvertString(nullptr, v));
    EXPECT_FALSE(ConvertString("", v));
    EXPECT_FALSE(ConvertString("-129", v));
    EXPECT_FALSE(ConvertString("128", v));

    EXPECT_TRUE(ConvertString("-128", v));
    EXPECT_EQ(INT8_MIN, v);
    EXPECT_TRUE(ConvertString("0", v));
    EXPECT_EQ(0, v);
    EXPECT_TRUE(ConvertString("127", v));
    EXPECT_EQ(INT8_MAX, v);
}

TEST(Convert, UInt8) {
    uint8_t v;
    EXPECT_FALSE(ConvertString(nullptr, v));
    EXPECT_FALSE(ConvertString("", v));
    EXPECT_FALSE(ConvertString("256", v));
    EXPECT_FALSE(ConvertString("-1", v));

    EXPECT_TRUE(ConvertString("0", v));
    EXPECT_EQ(0, v);
    EXPECT_TRUE(ConvertString("255", v));
    EXPECT_EQ(UINT8_MAX, v);
}

TEST(Convert, Int16) {
    int16_t v;
    EXPECT_FALSE(ConvertString(nullptr, v));
    EXPECT_FALSE(ConvertString("", v));
    EXPECT_FALSE(ConvertString("-327679", v));
    EXPECT_FALSE(ConvertString("32768", v));

    EXPECT_TRUE(ConvertString("-32768", v));
    EXPECT_EQ(INT16_MIN, v);
    EXPECT_TRUE(ConvertString("0", v));
    EXPECT_EQ(0, v);
    EXPECT_TRUE(ConvertString("32767", v));
    EXPECT_EQ(INT16_MAX, v);
}

TEST(Convert, UInt16) {
    uint16_t v;
    EXPECT_FALSE(ConvertString(nullptr, v));
    EXPECT_FALSE(ConvertString("", v));
    EXPECT_FALSE(ConvertString("65536", v));
    EXPECT_FALSE(ConvertString("-1", v));

    EXPECT_TRUE(ConvertString("0", v));
    EXPECT_EQ(0, v);
    EXPECT_TRUE(ConvertString("65535", v));
    EXPECT_EQ(UINT16_MAX, v);
}

TEST(Convert, Int32) {
    int32_t v;
    EXPECT_FALSE(ConvertString(nullptr, v));
    EXPECT_FALSE(ConvertString("", v));
    EXPECT_FALSE(ConvertString("-2147483649", v));
    EXPECT_FALSE(ConvertString("2147483648", v));

    EXPECT_TRUE(ConvertString("-2147483648", v));
    EXPECT_EQ(INT32_MIN, v);
    EXPECT_TRUE(ConvertString("0", v));
    EXPECT_EQ(0, v);
    EXPECT_TRUE(ConvertString("2147483647", v));
    EXPECT_EQ(INT32_MAX, v);
}

TEST(Convert, UInt32) {
    uint32_t v;
    EXPECT_FALSE(ConvertString(nullptr, v));
    EXPECT_FALSE(ConvertString("", v));
    EXPECT_FALSE(ConvertString("4294967296", v));
    EXPECT_FALSE(ConvertString("-1", v));

    EXPECT_TRUE(ConvertString("0", v));
    EXPECT_EQ(0, v);
    EXPECT_TRUE(ConvertString("4294967295", v));
    EXPECT_EQ(UINT32_MAX, v);

    EXPECT_TRUE(ConvertString("0x1000", v));
    EXPECT_EQ(0x1000, v);

    EXPECT_TRUE(ConvertString("0xBADF00D1", v));
    EXPECT_EQ(0xBADF00D1, v);
}

TEST(Convert, Int64) {
    int64_t v;
    EXPECT_FALSE(ConvertString(nullptr, v));
    EXPECT_FALSE(ConvertString("", v));
    EXPECT_FALSE(ConvertString("-", v));
    EXPECT_FALSE(ConvertString("-9223372036854775809", v));
    EXPECT_FALSE(ConvertString("9223372036854775808", v));
    EXPECT_FALSE(ConvertString("0xabcdefgh", v));
    EXPECT_FALSE(ConvertString("0x10000000000000000", v));

    EXPECT_TRUE(ConvertString("-9223372036854775808", v));
    EXPECT_EQ(INT64_MIN, v);

    EXPECT_TRUE(ConvertString("9223372036854775807", v));
    EXPECT_EQ(INT64_MAX, v);

    EXPECT_TRUE(ConvertString("0X123456789ABCDEF", v));
    EXPECT_EQ(0x123456789ABCDEF, v);

    EXPECT_TRUE(ConvertString("0x123456789abcdef", v));
    EXPECT_EQ(0x123456789abcdef, v);

    EXPECT_TRUE(ConvertString("0x7FFFFFFFFFFFFFF", v));
    EXPECT_EQ(0x7ffffffffffffff, v);
}

TEST(Convert, UInt64) {
    uint64_t v;
    EXPECT_FALSE(ConvertString(nullptr, v));
    EXPECT_FALSE(ConvertString("", v));
    EXPECT_FALSE(ConvertString("-", v));
    EXPECT_FALSE(ConvertString("18446744073709551616", v));
    EXPECT_FALSE(ConvertString("0xabcdefgh", v));
    EXPECT_FALSE(ConvertString("0x100000000000000000", v));

    EXPECT_TRUE(ConvertString("0", v));
    EXPECT_EQ(0, v);

    EXPECT_TRUE(ConvertString("18446744073709551615", v));
    EXPECT_EQ(UINT64_MAX, v);

    EXPECT_TRUE(ConvertString("0X123456789ABCDEF", v));
    EXPECT_EQ(0x123456789ABCDEF, v);

    EXPECT_TRUE(ConvertString("0x123456789abcdef", v));
    EXPECT_EQ(0x123456789abcdef, v);

    EXPECT_TRUE(ConvertString("0x7FFFFFFFFFFFFFF", v));
    EXPECT_EQ(0x7ffffffffffffff, v);

    EXPECT_TRUE(ConvertString("0xFFFFFFFFFFFFFFF", v));
    EXPECT_EQ(0xfffffffffffffff, v);

    EXPECT_TRUE(ConvertString("0x1000", v));
    EXPECT_EQ(0x1000, v);

    EXPECT_TRUE(ConvertString("0xBADF00D1BADF00D1", v));
    EXPECT_EQ(0xBADF00D1BADF00D1, v);

}

TEST(Convert, Float32) {
    float v;
    EXPECT_FALSE(ConvertString(nullptr, v));
    EXPECT_FALSE(ConvertString("", v));
    EXPECT_FALSE(ConvertString("1t", v));
    EXPECT_FALSE(ConvertString("1e1000", v));

    EXPECT_TRUE(ConvertString("12345678", v));
    EXPECT_FLOAT_EQ(12345678, v);
    EXPECT_TRUE(ConvertString("1e-1", v));
    EXPECT_FLOAT_EQ(0.1, v);
    EXPECT_TRUE(ConvertString("1e2", v));
    EXPECT_FLOAT_EQ(100, v);
}

TEST(Convert, Float64) {
    double v;
    EXPECT_FALSE(ConvertString(nullptr, v));
    EXPECT_FALSE(ConvertString("", v));
    EXPECT_FALSE(ConvertString("1t", v));
    EXPECT_FALSE(ConvertString("1e1000", v));

    EXPECT_TRUE(ConvertString("12345678", v));
    EXPECT_DOUBLE_EQ(12345678, v);
    EXPECT_TRUE(ConvertString("1e-1", v));
    EXPECT_DOUBLE_EQ(0.1, v);
    EXPECT_TRUE(ConvertString("1e2", v));
    EXPECT_DOUBLE_EQ(100, v);
}


TEST(Convert, Bool) {
    bool v;
    EXPECT_FALSE(ConvertString(nullptr, v));
    EXPECT_TRUE(ConvertString("yes", v));
    EXPECT_TRUE(v);
    EXPECT_TRUE(ConvertString("true", v));
    EXPECT_TRUE(v);
    EXPECT_TRUE(ConvertString("on", v));
    EXPECT_TRUE(v);
    EXPECT_TRUE(ConvertString("1", v));
    EXPECT_TRUE(v);
    EXPECT_TRUE(ConvertString("no", v));
    EXPECT_FALSE(v);
    EXPECT_TRUE(ConvertString("false", v));
    EXPECT_FALSE(v);
    EXPECT_TRUE(ConvertString("off", v));
    EXPECT_FALSE(v);
    EXPECT_TRUE(ConvertString("0", v));
    EXPECT_FALSE(v);
    EXPECT_FALSE(ConvertString(NULL, v));
    EXPECT_FALSE(ConvertString("xxx", v));
}


TEST(Convert, Time) {
    time_t v;
    time_t t = 0U;
    struct tm lt = {0};

    localtime_r(&t, &lt);

    EXPECT_FALSE(ConvertStringTime(nullptr, v));
    EXPECT_TRUE(ConvertStringTime("0x1000", v));
    EXPECT_EQ(0x1000, v);

    EXPECT_TRUE(ConvertStringTime("0xBADF00D1", v));
    EXPECT_EQ(0xBADF00D1, v);

    int64_t jan2_seconds = 24 * 60 * 60 - lt.tm_gmtoff;
    EXPECT_TRUE(ConvertStringTime("1970-01-02 00:00:00", v));
    EXPECT_EQ(jan2_seconds, v);

    EXPECT_TRUE(ConvertStringTime("70/01/02 00:00:00", v));
    EXPECT_EQ(jan2_seconds, v);
}
