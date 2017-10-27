#include <gtest/gtest.h>
#include <counters.h>
#include <ShmFileBase.h>
#include <counters_base.h>
#include <TestBase.h>

#define TEST_COUNTER(n) "test_"#n

class TestCountersInit : public TestBase {
};

#define TEST_NAME1 "Counter1"
#define TEST_NAME2 "Counter2"
#define TEST_NAME3 "Counter3"

TEST_F(TestCountersInit, Init) {
    Counter c1(TEST_NAME1);
    Counter c2(TEST_NAME2);
    Counter c3(TEST_NAME3);

    ASSERT_TRUE(c1.IsValid());
    ASSERT_TRUE(c2.IsValid());
    ASSERT_TRUE(c3.IsValid());
    c1.Set(1);
    c2.Set(2);
    c3.Set(3);
    Counters &counters = IncrementedCounters();
    ASSERT_EQ(c1, counters.GetCounterValue(counters.GetCounterIndex(TEST_NAME1)));
    ASSERT_EQ(c2, counters.GetCounterValue(counters.GetCounterIndex(TEST_NAME2)));
    ASSERT_EQ(c3, counters.GetCounterValue(counters.GetCounterIndex(TEST_NAME3)));
}
