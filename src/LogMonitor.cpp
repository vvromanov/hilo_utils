#include <LogStorage.h>
#include "LogMonitor.h"

#define INIT_C(c) snprintf(name,sizeof(name),"log.%s.level." #c, program_invocation_short_name); log_##c.SetName(name);

void LogMonitor::initCounters() {
    char name[NAME_MAX];
    STRNCPY(name, "settings.log.");
    STRNCAT(name, program_invocation_short_name);
    STRNCAT(name, ".level");
    counter.SetName(name, counter_value);
    counter = opt_log_level;
#define LOG_LEVEL(lname, sname, uname) snprintf(name,sizeof(name),"log.%s.level." #lname, program_invocation_short_name); log_##lname.SetName(name);
    LOG_LEVELS
#undef LOG_LEVEL
}

void LogMonitor::periodic() {
    log_level_t l = (log_level_t) (int64_t) counter;
    if (opt_log_level != l) {
        log_write(LOG_LEVEL_NOTICE, "Change log level %s->%s", log_level_to_name(opt_log_level),
                  log_level_to_name(l));
        opt_log_level = l;
    }
#define LOG_LEVEL(lname, sname, uname) log_##lname+=log_counters[LOG_LEVEL_##uname];log_counters[LOG_LEVEL_##uname]=0;
    LOG_LEVELS
#undef LOG_LEVEL
    LogStorage().UpdateCounters();
}
