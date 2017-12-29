#include "LogOptions.h"
#include "LogBase.h"
#include "common_utils.h"

error_t log_option_parse(int key, char *arg, struct argp_state *state, bool &handled) {
    handled = true;
    switch (key) {
        case 'l':
            if (!tryParseLogLevel(arg, opt_log_level)) {
                return ARGP_ERR_UNKNOWN;
            }
            break;
        case OPT_LOG_SYSLOG_LEVEL:
            if (!tryParseLogLevel(arg, opt_log_level_to_syslog)) {
                return ARGP_ERR_UNKNOWN;
            }
            break;
        case OPT_LOG_NO_COLORED:
            opt_log_no_colored=true;
            break;
        case 's':
            opt_log_to_console = false;
            break;
        default:
            handled = false;
    }
    return 0;
}
