#pragma once

#include <argp.h>
#include <cerrno>
#include <cstdint>

#define OPT_LOG_SYSLOG_LEVEL    10000
#define OPT_LOG_NO_COLORED      10001

#define LOG_OPTIONS \
    {"log-level",           'l',                    "INFO",     0, \
        "Specifies log level (DEBUG, INFO, NOTICE, WARNING, ERROR, CRITICAL)", 0}, \
    {"syslog-log-level",    OPT_LOG_SYSLOG_LEVEL,   "ERROR",   0, \
        "Specifies log level for syslog messages (DEBUG, INFO, NOTICE, WARNING, ERROR, CRITICAL)", 0}, \
    {"silent",              's',                    0,          OPTION_ARG_OPTIONAL, \
        "Don't duplicate logs to stderr", 0}, \
    {"no-colored-log",      OPT_LOG_NO_COLORED,        0,          OPTION_ARG_OPTIONAL, \
        "Don't use ansi codes for colored log", 0},

error_t log_option_parse(int key, char *arg, struct argp_state *state, bool &handled);

#define PARSE_LOG_OPTIONS  \
    {\
        bool handled; \
        error_t res = log_option_parse(key, arg, state, handled); \
        if (handled) { \
            return res; \
        } \
    }
