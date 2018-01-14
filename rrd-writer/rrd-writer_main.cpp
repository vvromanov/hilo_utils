#include <argp.h>
#include "LogBase.h"
#include <cstdlib>
#include <SignalWatcher.h>
#include <LibEv.h>
#include <ShmBufferEx.h>
#include <file_utils.h>
#include <systemd/sd-daemon.h>
#include "version.h"
#include <LogMonitorEv.h>
#include <Uptime.h>
#include <Rusage.h>
#include "rrd-writer_options.h"
#include "rrd_utils.h"

#define RRD_DIRECTORY "/var/lib/rrd-writer"

static void mainPeriodic(EV_TIMER_PARAMS) {
    static time_t prev_run_end;
    if (rrd_check_connect()) {
        if (prev_run_end == time(NULL)) {
            return;
        }
        rrd_update_cpu();
        rrd_update_memory();
        rrd_update_load();
        rrd_update_history_counters(NULL);
        for (int i = 0; i < custom_prefix_count; i++) {
            switch (custom_prefix[i].type) {
                case prefix_incremented:
                    rrd_update_counters(custom_prefix[i].name, counter_incremental);
                    break;
                case prefix_value:
                    rrd_update_counters(custom_prefix[i].name, counter_value);
                    break;
                case prefix_history:
                    rrd_update_history_counters(custom_prefix[i].name);
                    break;
            }
        }
        prev_run_end = time(NULL);
    }
}

static void updateCounters(EV_TIMER_PARAMS) {
    UptimeUpdate();
    RusageUpdate();
    sd_notifyf(0, ""
                       "READY=1\n"
                       "WATCHDOG=1\n"
                       "STATUS=Writing rrd data to %s\n"
                       "MAINPID=%lu",
               RRD_DIRECTORY,
               (unsigned long) getpid());
}

int main(int argc, char **argv) {
    argp_program_version = VERSION_STR;
    opt_log_level = LOG_LEVEL_INFO;
    rrd_writer_options_parse(argc, argv);
    InitSignalWatcher();
    LogMonitorEv lm;
    ev_timer timer_periodic;
    process_timer_start(&timer_periodic, mainPeriodic, 0, 1);

    ev_timer timer_update;
    process_timer_start(&timer_update, updateCounters, 0, 0.2);
    log_write(LOG_LEVEL_INFO, VERSION_STR " started");
    ev_loop(EV_DEFAULT, 0);
    return EXIT_SUCCESS;
}

