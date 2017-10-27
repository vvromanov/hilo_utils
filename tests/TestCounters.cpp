#include <gtest/gtest.h>
#include <sstream>
#include <ShmFileBase.h>
#include <counters_base.h>
#include <TestBase.h>
#include "counters.h"
#include "file_utils.h"

#define TEST_COUNTER(n) "test_"#n

class TestCounters : public TestBase {
};

TEST_F(TestCounters, SetGet) {
    Counter counter1(TEST_COUNTER(1));
    Counter counter2(TEST_COUNTER(2));
    ASSERT_TRUE(counter1.IsValid());
    ASSERT_TRUE(counter2.IsValid());
    ASSERT_EQ(0, counter1);
    ASSERT_EQ(0, counter2);
    counter1.Set(1);
    counter2.Set(2);
    ASSERT_EQ(1, counter1);
    ASSERT_EQ(2, counter2);
}

TEST_F(TestCounters, FullFill) {
    for (int i = 0; i < COUNTER_MAX_AMOUNT; ++i) {
        std::ostringstream oss;
        oss << "test_" << i << std::endl;
        Counter counter(oss.str().c_str());
        ASSERT_TRUE(counter.IsValid());
    }

    std::string name("test");
    Counter counter(name.c_str());
    ASSERT_FALSE(counter.IsValid());
}
