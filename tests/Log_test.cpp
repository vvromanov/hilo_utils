#include <ShmFileBase.h>
#include <ansi_escape_codes.h>
#include "LogBase.h"
#include "gtest/gtest.h"
#include "LogStorage.h"
#include "LogOptions.h"

#define TEST_FILE "/tmp/file_utils.tst"
#define TEST_FILE_INVALID "/_##/"

class LogTestData {
public:
    static const char *get_name() {
        return LOG_STORAGE_SHM_NAME;
    }
};

class Log : public ShmFileBase<LogTestData> {

};

TEST_F(Log, LevelToName) {
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

TEST_F(Log, NameToLevel) {
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

TEST_F(Log, Write) {
    ScopedLogLevel ll(LOG_LEVEL_INFO);
    EXPECT_TRUE(log_write_record(LOG_LEVEL_CRIT, "test", -1));
}


TEST_F(Log, WriteLogLevels) {
    ScopedLogLevel ll(LOG_LEVEL_DISABLED);
    log_write(LOG_LEVEL_PARN, "Paranoid");
    log_write(LOG_LEVEL_DEBUG, "Debug");
    log_write(LOG_LEVEL_INFO, "Info");
    log_write(LOG_LEVEL_NOTICE, "Notice");
    log_write(LOG_LEVEL_WARNING, "Warning");
    log_write(LOG_LEVEL_ERR, "Error");
    log_write(LOG_LEVEL_CRIT, "Critical");
}


TEST_F(Log, Options) {
    bool handled = false;
    EXPECT_EQ(ARGP_ERR_UNKNOWN, log_option_parse('l', const_cast<char*>("DEBUGX"), nullptr, handled));
    EXPECT_EQ(ARGP_ERR_UNKNOWN, log_option_parse(OPT_LOG_SYSLOG_LEVEL, const_cast<char*>("DEBUGX"), nullptr, handled));
    
    auto save_opt_log_level = opt_log_level;
    auto save_opt_log_level_to_syslog = opt_log_level_to_syslog;
    auto save_opt_log_to_console = opt_log_to_console;
    auto save_opt_log_no_colored = opt_log_no_colored;

    opt_log_level = LOG_LEVEL_CRIT;
    handled = false;
    EXPECT_EQ(0, log_option_parse('l', const_cast<char*>("DEBUG"), nullptr, handled));
    EXPECT_EQ(LOG_LEVEL_DEBUG, opt_log_level);
    EXPECT_TRUE(handled);

    opt_log_level_to_syslog = LOG_LEVEL_CRIT;
    handled = false;
    EXPECT_EQ(0, log_option_parse(OPT_LOG_SYSLOG_LEVEL, const_cast<char*>("DEBUG"), nullptr, handled));
    EXPECT_EQ(LOG_LEVEL_DEBUG, opt_log_level_to_syslog);
    EXPECT_TRUE(handled);

    opt_log_no_colored = false;
    handled = false;
    EXPECT_EQ(0, log_option_parse(OPT_LOG_NO_COLORED, nullptr, nullptr, handled));
    EXPECT_TRUE(opt_log_no_colored);
    EXPECT_TRUE(handled);

    opt_log_to_console = true;
    handled = false;
    EXPECT_EQ(0, log_option_parse('s', nullptr, nullptr, handled));
    EXPECT_FALSE(opt_log_to_console);
    EXPECT_TRUE(handled);

    handled = true;
    EXPECT_EQ(0, log_option_parse('x', nullptr, nullptr, handled));
    EXPECT_FALSE(handled);

    opt_log_level = save_opt_log_level;
    opt_log_level_to_syslog = save_opt_log_level_to_syslog;
    opt_log_to_console = save_opt_log_to_console;
    opt_log_no_colored = save_opt_log_no_colored;
    opt_log_no_colored = false;

}
