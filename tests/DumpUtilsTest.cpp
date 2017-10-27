#include "gtest/gtest.h"
#include "DumpUtils.h"

TEST(DumpUtils, GetIndent) {
    const char *szIndent = GetIndentStr(0);
    EXPECT_STREQ("", szIndent);

    szIndent = GetIndentStr(1);
    EXPECT_STREQ(" ", szIndent);

    szIndent = GetIndentStr(2);
    EXPECT_STREQ("  ", szIndent);

    szIndent = GetIndentStr(10);
    EXPECT_STREQ("          ", szIndent);
}

TEST(DumpUtils, GetIndentEx) {
    const char *szIndent = GetIndentStrEx(0, true);
    EXPECT_STREQ("", szIndent);
    szIndent = GetIndentStrEx(1, true);
    EXPECT_STREQ(NBSP, szIndent);

    szIndent = GetIndentStrEx(2, true);
    EXPECT_STREQ(NBSP
                         NBSP, szIndent);

    szIndent = GetIndentStrEx(2, false);
    EXPECT_STREQ("  ", szIndent);

    szIndent = GetIndentStrEx(10, true);
    EXPECT_STREQ(NBSP
                         NBSP
                         NBSP
                                 NBSP
                         NBSP
                                 NBSP
                         NBSP
                                 NBSP
                         NBSP
                         NBSP, szIndent);
}

static bool TestDumpN(int64_t n, int w, const char *c) {
    std::stringstream s;
    DumpNumber(s, n, w);
    EXPECT_STREQ(c, s.str().c_str());
    return s.str().compare(c) == 0;
}

TEST(DumpUtils, DumpNumber) {
    EXPECT_TRUE(TestDumpN(0, 1, "0"));
    EXPECT_TRUE(TestDumpN(0, 3, "  0"));
    EXPECT_TRUE(TestDumpN(0, 4, "   0"));
    EXPECT_TRUE(TestDumpN(9999, 4, "9999"));
    EXPECT_TRUE(TestDumpN(10000, 4, " 10k"));
    EXPECT_TRUE(TestDumpN(10000, 5, "10000"));
    EXPECT_TRUE(TestDumpN(100000, 5, " 100k"));
    EXPECT_TRUE(TestDumpN(1000000, 5, "1000k"));
    EXPECT_TRUE(TestDumpN(10000000, 5, "10.0M"));

    EXPECT_TRUE(TestDumpN(10000, 6, " 10000"));
    EXPECT_TRUE(TestDumpN(100000, 6, "100000"));
    EXPECT_TRUE(TestDumpN(1000000, 6, " 1000k"));
    EXPECT_TRUE(TestDumpN(10000000, 6, "10000k"));
}

static bool TestDumpTime(int64_t n, int w, const char *c) {
    std::stringstream s;
    DumpTime(s, n, w);
    EXPECT_STREQ(c, s.str().c_str());
    return s.str().compare(c) == 0;
}

TEST(DumpUtils, DumpTime) {
    EXPECT_TRUE(TestDumpTime(0, 1, "0"));
    EXPECT_TRUE(TestDumpTime(0, 2, " 0"));
    EXPECT_TRUE(TestDumpTime(0, 3, "  0"));
    EXPECT_TRUE(TestDumpTime(0, 4, "0:00"));
    EXPECT_TRUE(TestDumpTime(0, 5, " 0:00"));
    EXPECT_TRUE(TestDumpTime(0, 6, "  0:00"));
    EXPECT_TRUE(TestDumpTime(0, 7, "0:00:00"));
    EXPECT_TRUE(TestDumpTime(59, 7, "0:00:59"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60, 7, "0:59:59"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 3 * (60 * 60), 7, "3:59:59"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 33 * (60 * 60), 7, "  33:59"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 100 * (60 * 60), 7, " 100:59"));

    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 3 * (60 * 60), 8, " 3:59:59"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 33 * (60 * 60), 8, "33:59:59"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 100 * (60 * 60), 8, "  100:59"));

    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 3 * (60 * 60), 9, "  3:59:59"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 33 * (60 * 60), 9, " 33:59:59"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 100 * (60 * 60), 9, "100:59:59"));

    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 10000 * (60 * 60), 10, "  10000:59"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 100000 * (60 * 60), 10, " 100000:59"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 1000000ULL * (60 * 60), 10, "1000000:59"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 10000000ULL * (60 * 60), 10, "  10000000"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 100000000ULL * (60 * 60), 10, " 100000000"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 1000000000ULL * (60 * 60), 10, "1000000000"));
    EXPECT_TRUE(TestDumpTime(59 + 59 * 60 + 10000000000ULL * (60 * 60), 10, " 10000000k"));
}
