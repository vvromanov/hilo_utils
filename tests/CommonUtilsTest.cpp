#include <cstdint>
#include <gtest/gtest.h>
#include <common_utils.h>

TEST(CommonUtils, ReplaceAll)
{
    EXPECT_EQ("1_2__3", ReplaceAll("1__2____3", "__", "_"));
}

TEST(CommonUtils, Split)
{
    std::vector<std::string> res = split(" 1 12 123 1234  12345 ", ' ');
    EXPECT_EQ(5, res.size());
    EXPECT_EQ("1", res[0]);
    EXPECT_EQ("12", res[1]);
    EXPECT_EQ("123", res[2]);
    EXPECT_EQ("1234", res[3]);
    EXPECT_EQ("12345", res[4]);
}
