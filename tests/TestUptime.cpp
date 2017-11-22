#include <gtest/gtest.h>
#include <log.h>
#include <Uptime.h>

static const char *processes[] = {"test", "test2", "test3", "not_active"};

TEST(Uptime, Update) {
    log_set_worker_name("test");
    UptimeReset();
    UptimeUpdate();

    log_set_worker_name("test2");
    UptimeReset();
    UptimeUpdate();

    log_set_worker_name("test3");
    UptimeReset();
    UptimeUpdate();

    std::stringstream os;
    UptimeDumpTable(os, processes, 4);
    EXPECT_STREQ(""
                         "|      Name|    Uptime  | Inactive, s|\n"
                         "|      test|     0:00:00|           0|\n"
                         "|     test2|     0:00:00|           0|\n"
                         "|     test3|     0:00:00|           0|\n"
                         "|not_active| Not Active!| Not Active!|\n", os.str().c_str());
}
