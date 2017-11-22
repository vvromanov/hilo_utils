#pragma once

#include <evhtp.h>
#include "LogMonitor.h"

class LogMonitorEvent: public LogMonitor {
    struct event *ev;
public:
    LogMonitorEvent(evbase_t *evbase) {
        struct timeval half_sec = {0, 500000};
        ev = event_new(evbase, -1, EV_PERSIST, periodicLogLevel, this);
        event_add(ev, &half_sec);
    }
    ~LogMonitorEvent() {
        event_del(ev);
    }
    static void periodicLogLevel(evutil_socket_t fd, short what, void *arg) {
        LogMonitor *lm = (LogMonitor *) arg;
        lm->periodic();
    }
};
