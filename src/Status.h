#pragma once

#include "time_utils.h"
#include "counters_base.h"

typedef enum {
    status_ok = 0,
    status_warning = 1,
    status_critical = 2,
    status_unknown = 3,
} status_t;

const char* status2str(status_t status);

typedef enum {
    status_nagios,
    status_simple,
} status_format_t;

status_format_t str2status_format(const char *s);

#ifdef TX_PROGNAME

static inline void UpdateLiveCounter() {
    static LazyCounter c("wd." TX_PROGNAME ".last_active", counter_value);
    static LazyCounter uptime("wd." TX_PROGNAME ".uptime", counter_value);
    static int64_t started = getTimeMs();
    c = getTimeMs() / 1000;
    uptime = (getTimeMs() - started) / 1000;
};
#endif /* TX_PROGNAME */
