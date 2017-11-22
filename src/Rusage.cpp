#include <counters_base.h>
#include <time_utils.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "Rusage.h"

#define CNAME_UTIME  "process.%s.utime"
#define CNAME_STIME  "process.%s.stime"

static Counter cnt_utime;
static Counter cnt_stime;
static bool already_init = false;
int64_t prev_utime, prev_stime;

void RusageReset() {
    if (already_init) {
        cnt_utime.Reset();
        cnt_stime.Reset();
        already_init = false;
    }
}

static void RusageInit() {
    char name[NAME_MAX];
    snprintf(name, sizeof(name), CNAME_UTIME, program_invocation_short_name);
    cnt_utime.SetName(name, counter_incremental);

    snprintf(name, sizeof(name), CNAME_STIME, program_invocation_short_name);
    cnt_stime.SetName(name, counter_incremental);

    struct rusage usage;
    if (0 != getrusage(RUSAGE_SELF, &usage)) {
        log_write(LOG_LEVEL_WARNING_ERRNO, "getrusage error");
    } else {
        prev_stime = getTimeMs(usage.ru_stime);
        prev_utime = getTimeMs(usage.ru_utime);
    }
    already_init = true;
}

void RusageUpdate() {
    if (!already_init) {
        RusageInit();
        return;
    }
    struct rusage usage;
    if (0 != getrusage(RUSAGE_SELF, &usage)) {
        log_write(LOG_LEVEL_WARNING_ERRNO, "getrusage error");
        return;
    }
    int64_t stime = getTimeMs(usage.ru_stime);
    int64_t utime = getTimeMs(usage.ru_utime);

    cnt_stime += stime - prev_stime;
    cnt_utime += utime - prev_utime;
    prev_stime = stime;
    prev_utime = utime;
};
