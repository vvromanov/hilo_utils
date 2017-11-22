#include <gtest/gtest.h>
#include "SysInfoCpu.h"
#include "SysInfoMemory.h"

TEST(SysInfo, GetCpu) {
    cpu_stat_t stat;
    EXPECT_TRUE(cpu_stat_read(stat));
}

TEST(SysInfo, GetMemory) {
    mem_stat_t stat;
    EXPECT_TRUE(mem_stat_read(stat));
}
