#pragma once

#include "LogMonitor.h"
#include "LibEv.h"

class LogMonitorEv: public LogMonitor {
    ev_timer timer;
public:
    LogMonitorEv() {
        initCounters();
        process_timer_start(&timer, periodicLogLevel, 0, 0.1);
        timer.data = this;
    }

    ~LogMonitorEv() {
        ev_timer_stop(EV_DEFAULT_ &timer);
    }

private:
    static void periodicLogLevel(EV_TIMER_PARAMS) {
        LogMonitorEv *lm = (LogMonitorEv *) w->data;
        lm->periodic();
    }
};