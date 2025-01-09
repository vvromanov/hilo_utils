#include <strings.h>
#include <cerrno>
#include "LogBase.h"
#include "common_utils.h"
#include "LogStorage.h"
#include "ansi_escape_codes.h"
#include "counters_base.h"

log_level_t opt_log_level = LOG_LEVEL_INFO;
log_level_t opt_log_level_to_syslog = LOG_LEVEL_ERR;
bool opt_log_no_colored = false;
bool opt_log_to_console = true;
FILE *log_file = stdout;
static bool syslog_opened;
static char log_worker_name[100] = "";

int64_t log_counters[0x10];

const char *log_level_to_name(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_CRIT:
        case LOG_LEVEL_CRIT_ERRNO:
            return "CRIT";
        case LOG_LEVEL_ERR:
        case LOG_LEVEL_ERR_ERRNO:
            return "ERRR";
        case LOG_LEVEL_WARNING:
        case LOG_LEVEL_WARNING_ERRNO:
            return "WARN";
        case LOG_LEVEL_NOTICE:
            return "NOTI";
        case LOG_LEVEL_INFO:
            return "INFO";
        case LOG_LEVEL_DEBUG:
            return "DEBG";
        case LOG_LEVEL_PARN:
            return "PARN";
        case LOG_LEVEL_DISABLED:
            return "DISABLED";
        case LOG_LEVEL_RESERVED:
            return "RESERVED";
    }
    return "???";
}

const char *log_level_to_color(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_DISABLED:
        case LOG_LEVEL_RESERVED:
        case LOG_LEVEL_CRIT:
        case LOG_LEVEL_CRIT_ERRNO:
            return ANSI_BOLD_YELLOW_ON_RED;
        case LOG_LEVEL_ERR:
        case LOG_LEVEL_ERR_ERRNO:
            return ANSI_BOLD_RED;
        case LOG_LEVEL_WARNING:
        case LOG_LEVEL_WARNING_ERRNO:
            return ANSI_BOLD_YELLOW;
        case LOG_LEVEL_NOTICE:
            return ANSI_BOLD_CYAN;
        case LOG_LEVEL_INFO:
            return ANSI_BOLD_GREEN;
        case LOG_LEVEL_DEBUG:
            return ANSI_BOLD_BLUE;
        case LOG_LEVEL_PARN:
            return ANSI_GRAY;
    }
    return "";
}

bool tryParseLogLevel(const char *name, log_level_t &level) {
    if (0 == strcasecmp(name, "EMRG") || 0 == strcasecmp(name, "EMERG") || 0 == strcasecmp(name, "EMERGENCY")) {
        level = LOG_LEVEL_CRIT;
        return true;
    }
    if (0 == strcasecmp(name, "ALRT") || 0 == strcasecmp(name, "ALERT")) {
        level = LOG_LEVEL_CRIT;
        return true;
    }
    if (0 == strcasecmp(name, "CRIT") || 0 == strcasecmp(name, "CRITICAL")) {
        level = LOG_LEVEL_CRIT;
        return true;
    }
    if (0 == strcasecmp(name, "ERR") || 0 == strcasecmp(name, "ERRR") || 0 == strcasecmp(name, "ERROR")) {
        level = LOG_LEVEL_ERR;
        return true;
    }
    if (0 == strcasecmp(name, "WARN") || 0 == strcasecmp(name, "WARNING")) {
        level = LOG_LEVEL_WARNING;
        return true;
    }
    if (0 == strcasecmp(name, "NOTICE") || 0 == strcasecmp(name, "NOTI")) {
        level = LOG_LEVEL_NOTICE;
        return true;
    }
    if (0 == strcasecmp(name, "INFO")) {
        level = LOG_LEVEL_INFO;
        return true;
    }
    if (0 == strcasecmp(name, "DBG") || 0 == strcasecmp(name, "DEBG") || 0 == strcasecmp(name, "DEBUG")) {
        level = LOG_LEVEL_DEBUG;
        return true;
    }
    if (0 == strcasecmp(name, "PARN") || 0 == strcasecmp(name, "PARANOID")) {
        level = LOG_LEVEL_PARN;
        return true;
    }
    if (0 == strcasecmp(name, "DISABLED")) {
        level = LOG_LEVEL_DISABLED;
        return true;
    }
    return false;
}

void log_set_worker_name(const char *name) {
    if (0 != strcmp(log_worker_name, name)) {
        STRNCPY(log_worker_name, name);
        log_source = -1;
        if (syslog_opened) {
            closelog();
            syslog_opened = false;
        }
    }
}

void log_set_worker_name(int worker) {
    char tmp[NAME_MAX];
    snprintf(tmp, sizeof(tmp), "%s_W%d", program_invocation_short_name, worker);
    log_set_worker_name(tmp);
}

const char *log_get_worker_name() {
    if (log_worker_name[0]) {
        return log_worker_name;
    } else {
        return program_invocation_short_name;
    }
}

void log_write(log_level_t level, const char *format, ...) {
    if (!is_log_enabled(level)) {
        return;
    }
    va_list va;
    va_start(va, format);
    log_writev(level, format, va);
    va_end(va);
}

static int syslog_level(log_level_t l) {
    switch (l & LOG_LEVEL_MASK) {
        case LOG_LEVEL_PARN:
            return -1;
        case LOG_LEVEL_DEBUG:
            return LOG_DEBUG;
        case LOG_LEVEL_INFO:
            return LOG_INFO;
        case LOG_LEVEL_NOTICE:
            return LOG_NOTICE;
        case LOG_LEVEL_WARNING:
            return LOG_WARNING;
        case LOG_LEVEL_ERR:
            return LOG_ERR;
        case LOG_LEVEL_CRIT:
            return LOG_CRIT;
        default:
            return -1;
    }
}

void log_write_str(log_level_t level, const char *string, int len) {
    if (!is_log_enabled(level)) {
        return;
    }
    if (len < 0) {
        len = strlen(string);
    }
    while (len > 0 && string[len - 1] == '\n') {
        --len;
    }
    level = (log_level_t) (level & LOG_LEVEL_MASK);
    ++log_counters[level];
    log_write_record(level, string, len);
    if (opt_log_to_console) {
        if (opt_log_no_colored) {
            fprintf(log_file, "%s | %*s\n", log_level_to_name(level), len, string);
        } else {
            fprintf(log_file, "%s%s | %*s" ANSI_ATTR_RESET "\n", log_level_to_color(level), log_level_to_name(level),
                    len,
                    string);
        }
    }
    if (level <= opt_log_level_to_syslog) {
        int l = syslog_level(level);

        if (!syslog_opened) {
            syslog_opened = true;
            openlog(log_get_worker_name(), LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
        }
        syslog(l, "%*s", len, string);
    }
}

void log_writev(log_level_t level, const char *format, va_list ap) {
    static bool in_log_writev = false;
    if (!is_log_enabled(level)) {
        return;
    }
    if (in_log_writev) {
        char tmp[1024];
        int len = vsnprintf(tmp, sizeof(tmp) - 1, format, ap);
        if ((level & LOG_ERRNO) == LOG_ERRNO) {
            char etmp[30];
            int elen = snprintf(etmp, sizeof(etmp), " | E%d - ", errno);
            strncpy(tmp + len, etmp, sizeof(tmp) - len - 1);
            len += elen;
            strncpy(tmp + len, strerror(errno), sizeof(tmp) - len - 1);
        }
        log_write_str(level, tmp, len);
    } else {
        static __thread char tmp[LOG_MAX_MSG_LEN + 1];
        in_log_writev = true;
        int len = vsnprintf(tmp, sizeof(tmp) - 1, format, ap);
        if ((level & LOG_ERRNO) == LOG_ERRNO) {
            char etmp[30];
            int elen = snprintf(etmp, sizeof(etmp), " | E%d - ", errno);
            strncpy(tmp + len, etmp, sizeof(tmp) - len - 1);
            len += elen;
            const char *szErr = strerror(errno);
            strncpy(tmp + len, szErr, sizeof(tmp) - len - 1);
            len += strlen(szErr);
        }
        log_write_str(level, tmp, len);
        in_log_writev = false;
    }
}

ScopedLogLevel::ScopedLogLevel(log_level_t level)
{
    prev_log_level = opt_log_level;
    opt_log_level = (log_level_t)(level & LOG_LEVEL_MASK);
}

ScopedLogLevel::~ScopedLogLevel()
{
    opt_log_level = prev_log_level;
}
