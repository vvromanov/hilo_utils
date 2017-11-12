#include <ShmFileBase.h>
#include <ansi_escape_codes.h>
#include "log.h"
#include "gtest/gtest.h"
#include "LogStorage.h"

#define TEST_FILE "/tmp/file_utils.tst"
#define TEST_FILE_INVALID "/_##/"

class LogTestData {
public:
    static const char *get_name() {
        return LOG_STORAGE_SHM_NAME;
    }
};

class TxLog : public ShmFileBase<LogTestData> {

};

TEST_F(TxLog, LevelToName) {
    EXPECT_STREQ("CRIT", log_level_to_name(LOG_LEVEL_CRIT));
    EXPECT_STREQ("ERRR", log_level_to_name(LOG_LEVEL_ERR));
    EXPECT_STREQ("WARN", log_level_to_name(LOG_LEVEL_WARNING));
    EXPECT_STREQ("NOTI", log_level_to_name(LOG_LEVEL_NOTICE));
    EXPECT_STREQ("INFO", log_level_to_name(LOG_LEVEL_INFO));
    EXPECT_STREQ("DEBG", log_level_to_name(LOG_LEVEL_DEBUG));
    EXPECT_STREQ("PARN", log_level_to_name(LOG_LEVEL_PARN));
    EXPECT_STREQ("???", log_level_to_name((log_level_t) -1));
}

static bool TestLogName(const char *name, log_level_t l) {
    log_level_t tmp;
    bool res;
    EXPECT_TRUE(res = tryParseLogLevel(name, tmp));
    EXPECT_EQ(l, tmp);
    return res & (l == tmp);
}

TEST_F(TxLog, NameToLevel) {
    EXPECT_TRUE(TestLogName("EMRG", LOG_LEVEL_CRIT));
    EXPECT_TRUE(TestLogName("ALRT", LOG_LEVEL_CRIT));
    EXPECT_TRUE(TestLogName("CRIT", LOG_LEVEL_CRIT));
    EXPECT_TRUE(TestLogName("ERRR", LOG_LEVEL_ERR));
    EXPECT_TRUE(TestLogName("WARN", LOG_LEVEL_WARNING));
    EXPECT_TRUE(TestLogName("NOTI", LOG_LEVEL_NOTICE));
    EXPECT_TRUE(TestLogName("INFO", LOG_LEVEL_INFO));
    EXPECT_TRUE(TestLogName("DEBG", LOG_LEVEL_DEBUG));
    EXPECT_TRUE(TestLogName("PARN", LOG_LEVEL_PARN));
}

TEST_F(TxLog, Write) {
    ScopedLogLevel ll(LOG_LEVEL_PARN);
    EXPECT_TRUE(log_write_record(LOG_LEVEL_CRIT, "test", -1));
}


TEST_F(TxLog, WriteLogLevels) {
    ScopedLogLevel ll(LOG_LEVEL_DISABLED);
    log_write(LOG_LEVEL_PARN, "Paranoid");
    log_write(LOG_LEVEL_DEBUG, "Debug");
    log_write(LOG_LEVEL_INFO, "Info");
    log_write(LOG_LEVEL_NOTICE, "Notice");
    log_write(LOG_LEVEL_WARNING, "Warning");
    log_write(LOG_LEVEL_ERR, "Error");
    log_write(LOG_LEVEL_CRIT, "Critical");
}


//TEST(Log, LinesCount) {
//    log_lines_count[LOG_ERR] = 0;
//    log_write(LOG_LEVEL_ERR, "test");
//    EXPECT_EQ(1u, log_lines_count[LOG_ERR]);
//    log_write(LOG_LEVEL_ERR_ERRNO, "test");
//    EXPECT_EQ(2u, log_lines_count[LOG_ERR]);
//}
