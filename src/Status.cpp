#include <cstring>
#include "Status.h"

const char* status2str(status_t status) {
    switch (status) {
        case status_ok:
            return "OK";
        case status_warning:
            return "WARNING";
        case status_unknown:
            return "UNKNOWN";
        case status_critical:
            return "CRITICAL";
    }
    return "UNKNOWN";
}

status_format_t str2status_format(const char *s) {
    if (NULL == s || 0 == strcasecmp(s, "simple")) {
        return status_simple;
    }
    if (0 == strcasecmp(s, "nagios")) {
        return status_nagios;
    }
    return status_simple;
}
