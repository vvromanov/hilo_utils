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


TEST_F(TestHistoryCounters, GetCategory) {
    EXPECT_TRUE(HistoryCountersClear());
    HistoryCounter c1(TEST_COUNTER ".c1", HistoryCount);
    HistoryCounter c2(TEST_COUNTER ".c2", HistoryCall);
    HistoryCounter c3(TEST_COUNTER ".c3", HistoryVolume);
    ++c1;
    ++c2;
    c3++;

    HistoryCounters::index_info_t index_info;
    GetHistoryCounters().GetCategory("", index_info);
    EXPECT_EQ(3, index_info.count);
    EXPECT_TRUE(HistoryCountersClear());
}

TEST_F(TestHistoryCounters, AddBatch) {
    EXPECT_TRUE(HistoryCountersClear());
    HistoryCounter c1(TEST_COUNTER ".c1", HistoryCount);
    HistoryCounter c2(TEST_COUNTER ".c2", HistoryCall);
    HistoryCounter c3(TEST_COUNTER ".c3", HistoryVolume);
    c1.AddBatch(10, 20);
    c2.AddBatch(10, 20);
    c3.AddBatch(10, 20);
    std::stringstream ss;
    GetHistoryCounters().Dump(ss, "", format_simple, false);
    EXPECT_EQ(
        "test.level1.level2.c1 0 10 10\n"
        "test.level1.level2.c2 0 0 10 2 10 2\n"
        "test.level1.level2.c3 0 0 10 20 10 20\n",
        ss.str());
    GetHistoryCounters().Dump(ss, "", format_raw, false);
    EXPECT_EQ(
        "test.level1.level2.c1 0 10 10\n"
        "test.level1.level2.c2 0 0 10 2 10 2\n"
        "test.level1.level2.c3 0 0 10 20 10 20\n",
        ss.str());
    EXPECT_TRUE(HistoryCountersClear());
}

TEST_F(TestHistoryCounters, AddVolume) {
    EXPECT_TRUE(HistoryCountersClear());
    HistoryCounter c1(TEST_COUNTER ".c1", HistoryCount);
    c1.AddVolume(10);
    c1.AddVolume(100);
    c1.AddVolume(100);
    EXPECT_TRUE(HistoryCountersClear());
}
