#include <cstdint>
#include <gtest/gtest.h>
#include <common_utils.h>

TEST(CommonUtils, ReplaceAll)
{
    EXPECT_EQ("1_2__3", ReplaceAll("1__2____3", "__", "_"));
}
