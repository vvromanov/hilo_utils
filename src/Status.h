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
