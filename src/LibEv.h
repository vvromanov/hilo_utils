#pragma once

#include "ev.h"
#include "LogBase.h"

#define EV_TIMER_PARAMS  __attribute__ ((unused)) EV_P_ __attribute__ ((unused)) ev_timer  *w, __attribute__ ((unused)) int revents
#define EV_IDLE_PARAMS  __attribute__ ((unused)) EV_P_ __attribute__ ((unused)) ev_idle  *w, __attribute__ ((unused)) int revents
#define EV_SIGNAL_PARAMS __attribute__ ((unused)) EV_P_ __attribute__ ((unused)) ev_signal *w, __attribute__ ((unused)) int revents

#define EV_TIMER_PARAMS_NULL  NULL, NULL, 0

#define process_timer_start(timer, handler, after, repeat)   \
    if ((after)>0.00001 || (repeat)>0.00001) {                                              \
        ev_timer_init((timer), &handler, (after), (repeat));            \
        ev_timer_start(EV_DEFAULT_ (timer));                            \
        log_write(LOG_LEVEL_DEBUG, "Start periodic task '%s' every %g seconds", #handler, (double)repeat); \
    }

#define process_idle_start(timer, handler)   \
    ev_idle_init((timer), &handler);            \
    ev_idle_start(EV_DEFAULT_ (timer));                            \
    log_write(LOG_LEVEL_DEBUG, "Start idle task '%s'", #handler);


#define EV_CYCLE(func) \
    int64_t now = getClockUs(); \
    int64_t stop = INT64_MAX; \
    if (w) { \
        stop = now + (w->repeat * (1000000)); \
    } \
    do { \
        if (!func()) { \
            break; \
        } \
    } while (getClockUs() < stop);
