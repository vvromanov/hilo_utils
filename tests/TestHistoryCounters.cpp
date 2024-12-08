#include <gtest/gtest.h>
#include <TestBase.h>
#include "HistoryCounters.h"
#include "file_utils.h"
#include "ClockOverride.h"

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

static void FillTestData(ClockOverride& co, HistoryCounter& c1, HistoryCounter& c2, HistoryCounter& c3) {
	for (int i = 1; i <= 10 * 60; i++) {
		for (int j = 0; j <= i % 5; j++) {
			c1++;
		}
		c2.AddBatch(i * 2, i * 20);
		c3.AddVolume(i * 100);
		co.AddSeconds(1);
	}
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

static bool HC_TestDump(counters_format_t format, const char* expected, const char* prefix = NULL) {
	EXPECT_TRUE(HistoryCountersClear());
	HistoryCounter c1(TEST_COUNTER ".c1", HistoryCount);
	HistoryCounter c2(TEST_COUNTER ".c2", HistoryCall);
	HistoryCounter c3(TEST_COUNTER ".c3", HistoryVolume);
	HistoryCounter xxx("xxx", HistoryVolume);
	ClockOverride co;
	FillTestData(co, c1, c2, c3);
	std::stringstream ss;
	GetHistoryCounters().Dump(ss, prefix ? prefix : TEST_COUNTER, format, false);
	EXPECT_EQ(expected, ss.str());
	return ss.str() == expected;
}

TEST_F(TestHistoryCounters, DumpSimple) {
	EXPECT_TRUE(HC_TestDump(format_simple,
		"test.level1.level2.c1 1 900 1800\n"
		"test.level1.level2.c2 1200 10 270300 10 360600 10\n"
		"test.level1.level2.c3 1 60000 300 13515000 600 18030000\n"
	));
}

TEST_F(TestHistoryCounters, DumpRaw) {
	EXPECT_TRUE(HC_TestDump(format_raw,
		" 1 60000 300 13515000 600 18030000\n",
		"test.level1.level2.c3"
	));
	EXPECT_TRUE(HC_TestDump(format_raw,
		"",
		"XXX"
	));
}

TEST_F(TestHistoryCounters, DumpTable) {
	EXPECT_TRUE(HC_TestDump(format_table,
		"|              Counter| Last second | Last 5 mins |    Total    |\n"
		"|                 Name| Count|Volume| Count|Volume| Count|Volume|\n"
		"|test.level1.level2.c1|     1|      |   900|      |  1800|      |\n"
		"|test.level1.level2.c2|  1200|    10|270300|    10|360600|    10|\n"
		"|test.level1.level2.c3|     1| 60000|   300|13515k|   600|18030k|\n"
	));
}

TEST_F(TestHistoryCounters, DumpNagios) {
	EXPECT_TRUE(HC_TestDump(format_nagios,
		"OK | test.level1.level2.c1=1 test.level1.level2.c2=1200 test.level1.level2.c2_avg=10 test.level1.level2.c3=1 test.level1.level2.c3_vol=60000\n"
	));
}

TEST_F(TestHistoryCounters, DumpNagios_5m) {
	EXPECT_TRUE(HC_TestDump(format_nagios_5m,
		"OK | test.level1.level2.c1=900 test.level1.level2.c2=270300 test.level1.level2.c2_avg=10 test.level1.level2.c3=300 test.level1.level2.c3_vol=13515000\n"
	));
}

TEST_F(TestHistoryCounters, DumpNagios_total) {
	EXPECT_TRUE(HC_TestDump(format_nagios_total,
		"OK | test.level1.level2.c1=1800 test.level1.level2.c2=360600 test.level1.level2.c2_avg=10 test.level1.level2.c3=600 test.level1.level2.c3_vol=18030000\n"
	));
}

TEST_F(TestHistoryCounters, DumpUnknown) {
	EXPECT_TRUE(HC_TestDump(format_unknown,
		""
	));
}

TEST_F(TestHistoryCounters, DumpHtml) {
	EXPECT_TRUE(HistoryCountersClear());
	HistoryCounter c1(TEST_COUNTER ".c1", HistoryCount);
	HistoryCounter c2(TEST_COUNTER ".c2", HistoryCall);
	HistoryCounter c3(TEST_COUNTER ".c3", HistoryVolume);
	HistoryCounter xxx("xxx", HistoryVolume);
	ClockOverride co;
	FillTestData(co, c1, c2, c3);
	std::stringstream ss;
	GetHistoryCounters().DumpHtml(ss, TEST_COUNTER);
	EXPECT_EQ(
		"  <table border='1' cellspacing='0'>\n"
		"    <thead>\n"
		"      <tr><td rowspan='2'>Counter name</td><td colspan='2'>Last second</td><td colspan='2'>Last 5 mins</td><td colspan='2'>Total</td></tr>\n"
		"      <tr><td>Count</td><td>Volume</td><td>Count</td><td>Volume</td><td>Count</td><td>Volume</td></tr>\n"
		"    </thead>\n"
		"    <tbody align='right'>\n"
		"      <tr><td>test.level1.level2.c1</td><td align='right'>     1</td><td align='right'>&nbsp;</td><td align='right'>   900</td><td align='right'>&nbsp;</td><td align='right'>  1800</td><td align='right'>&nbsp;</td></tr>\n"
		"      <tr><td>test.level1.level2.c2</td><td align='right'>  1200</td><td align='right'>    10</td><td align='right'>270300</td><td align='right'>    10</td><td align='right'>360600</td><td align='right'>    10</td></tr>\n"
		"      <tr><td>test.level1.level2.c3</td><td align='right'>     1</td><td align='right'> 60000</td><td align='right'>   300</td><td align='right'>13515k</td><td align='right'>   600</td><td align='right'>18030k</td></tr>\n"
		"    </tbody>\n"
		"  </table>\n", 
		ss.str());
}
