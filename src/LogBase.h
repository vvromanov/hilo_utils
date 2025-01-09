#pragma once

#include <syslog.h>
#include <cstdarg>
#include <cstdint>

#define LOG_ERRNO 0x10
#define LOG_LEVEL_MASK 0xF

#define LOG_MAX_MSG_LEN 100'000

typedef enum {
    LOG_LEVEL_DISABLED = 0,     /* Logging disabled */
    LOG_LEVEL_RESERVED = 1,     /* reserved */
    LOG_LEVEL_CRIT = 2,         /* critical conditions */
    LOG_LEVEL_ERR = 3,          /* error conditions */
    LOG_LEVEL_WARNING = 4,      /* warning conditions */
    LOG_LEVEL_NOTICE = 5,       /* normal but significant condition */
    LOG_LEVEL_INFO = 6,         /* informational */
    LOG_LEVEL_DEBUG = 7,        /* debug-level messages */
    LOG_LEVEL_PARN = 8,         /* paranoid-level messages */
    LOG_LEVEL_WARNING_ERRNO = LOG_ERRNO | LOG_LEVEL_WARNING,
    LOG_LEVEL_ERR_ERRNO = LOG_ERRNO | LOG_LEVEL_ERR,
    LOG_LEVEL_CRIT_ERRNO = LOG_ERRNO | LOG_LEVEL_CRIT,
} log_level_t;

#define LOG_LEVELS \
    LOG_LEVEL(paranoid, PARN, PARN) \
    LOG_LEVEL(debug, DEBG, DEBUG) \
    LOG_LEVEL(info, INFO, INFO) \
    LOG_LEVEL(notice, NOTI, NOTICE) \
    LOG_LEVEL(warning, WARN, WARNING) \
    LOG_LEVEL(error, ERRR, ERR) \
    LOG_LEVEL(critical, CRIT, CRIT)

extern log_level_t opt_log_level;
extern log_level_t opt_log_level_to_syslog;
extern bool opt_log_to_console;
extern bool opt_log_no_colored;

extern int64_t log_counters[0x10];

void log_set_worker_name(const char *name);
void log_set_worker_name(int worker);
const char *log_get_worker_name();

const char *log_level_to_name(log_level_t level);
const char *log_level_to_color(log_level_t level);
bool tryParseLogLevel(const char *name, log_level_t &level);

inline bool is_log_enabled(log_level_t level) {
    return (level & LOG_LEVEL_MASK) <= opt_log_level;
}

void log_write(log_level_t level, const char *format, ...) __attribute__((format(printf, 2, 3)));
void log_writev(log_level_t level, const char *format, va_list ap);
void log_write_str(log_level_t level, const char *string, int len);

class ScopedLogLevel {
    log_level_t prev_log_level;
public:
    ScopedLogLevel(log_level_t level);
    ~ScopedLogLevel();
};

