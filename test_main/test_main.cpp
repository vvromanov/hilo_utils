#include <log.h>
#include <counters.h>
#include <file_utils.h>
#include <HistoryCounters.h>
#include "LogStorage.h"
#include "gtest/gtest.h"

#define TEST_SUFFIX "_test"

int main(int argc, char **argv) {
    counters_suffix = TEST_SUFFIX;
    opt_log_storage_size = 5 * 1024 * 1024;
    log_set_worker_name("TEST");
    remove_test_file(SHM_LOCATION COUNTERS_SHM_NAME_INCREMENTED TEST_SUFFIX);
    remove_test_file(SHM_LOCATION COUNTERS_SHM_NAME_VALUE TEST_SUFFIX);
    remove_test_file(SHM_LOCATION COUNTERS_SHM_NAME_HISTORY TEST_SUFFIX);
    remove_test_file(SHM_LOCATION LOG_STORAGE_SHM_NAME TEST_SUFFIX);
    remove_test_file(SHM_LOCATION LOG_COUNTERS_SHM_NAME TEST_SUFFIX);
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    ScopedLogLevel ll(LOG_LEVEL_ERR);
    int retcode = RUN_ALL_TESTS();
    if (std::getenv("GTEST_RETURN_0")) {
        retcode = 0;
    }
    return retcode;
}
