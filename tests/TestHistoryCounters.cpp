#include <gtest/gtest.h>
#include <TestBase.h>
#include "HistoryCounters.h"
#include "file_utils.h"

#define TEST_COUNTERS_FILE "test_history_counters"
#define TEST_COUNTER "test.level1.level2"

class TestHistoryCounters : public TestBase {
};

TEST_F(TestHistoryCounters, Open) {
    HistoryCounters counters;
    ASSERT_TRUE(counters.Open(TEST_COUNTERS_FILE, NULL));
    counters.Close();
    EXPECT_TRUE(remove_test_file("/dev/shm/" TEST_COUNTERS_FILE));
}

TEST_F(TestHistoryCounters, GetIndex) {
    EXPECT_TRUE(HistoryCountersClear());
    EXPECT_EQ(0, GetHistoryCounters().GetCounterIndex(TEST_COUNTER ".c1"));
    EXPECT_EQ(1, GetHistoryCounters().GetCounterIndex(TEST_COUNTER ".c2"));
}

