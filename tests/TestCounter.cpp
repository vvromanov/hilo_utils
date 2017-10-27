#include <gtest/gtest.h>
#include <sstream>
#include "counters_base.h"

TEST(TestCounter, SetGet) {
    counter_t v = 0;
    Counter c;
    c.Init(&v);
    c.Set(10);
    EXPECT_EQ(10, v);
}

TEST(TestCounter, Assign) {
    counter_t v = 0;
    Counter c;
    c.Init(&v);
    c=10;
    EXPECT_EQ(10, v);
}

TEST(TestCounter, Increment) {
    counter_t v = 0;
    Counter c;
    c.Init(&v);

    ++c;
    EXPECT_EQ(1, v);

    EXPECT_EQ(2, ++c);
    EXPECT_EQ(2, v);
}

TEST(TestCounter, Decrement) {
    counter_t v = 5;
    Counter c;
    c.Init(&v);

    --c;
    EXPECT_EQ(4, v);

    EXPECT_EQ(3, --c);
    EXPECT_EQ(3, v);
}

TEST(TestCounter, Increment_Val) {
    counter_t v = 10;
    Counter c;
    c.Init(&v);

    c+=3;
    EXPECT_EQ(13, v);

    EXPECT_EQ(18, c+=5);
    EXPECT_EQ(18, v);
}

TEST(TestCounter, Decrement_Val) {
    counter_t v = 10;
    Counter c;
    c.Init(&v);

    c-=2;
    EXPECT_EQ(8, v);

    EXPECT_EQ(5, c-=3);
    EXPECT_EQ(5, v);
}

