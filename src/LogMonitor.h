#pragma once

#include "counters_base.h"

class LogMonitor {
    Counter counter;
#define LOG_LEVEL(lname, sname, uname) Counter log_##lname;
    LOG_LEVELS
#undef LOG_LEVEL

public:
    void periodic();

protected:
    LogMonitor() {
        initCounters();
    }

    void initCounters();
};