#include <counters_base.h>
#include <time_utils.h>
#include <CDictionary.h>
#include <iomanip>
#include <DumpUtils.h>
#include "Uptime.h"

#define CNAME_LAST_ACTIVE  "wd.%s.last_active"
#define CNAME_UPTIME  "wd.%s.uptime"

static Counter last_active;
static Counter uptime;
static bool already_init = false;

void UptimeReset() {
    if (already_init) {
        last_active.Reset();
        uptime.Reset();
        already_init = false;
    }
}

void UptimeUpdate() {
    static int64_t started = getTimeMs();
    if (!already_init) {
        char name[NAME_MAX];
        snprintf(name, sizeof(name), CNAME_LAST_ACTIVE, log_get_worker_name());
        last_active.SetName(name, counter_value);

        snprintf(name, sizeof(name), CNAME_UPTIME, log_get_worker_name());
        uptime.SetName(name, counter_value);
        already_init = true;
    }
    int64_t now = getTimeMs();
    last_active = now / 1000;
    uptime = (now - started) / 1000;

};

status_t UptimeCheckStatus(std::ostream &s, status_format_t format, const char *processes[], int count) {
    status_t status = status_ok;
    for (int i = 0; i < count; i++) {
        char cname[NAME_MAX];
        snprintf(cname, sizeof(cname), CNAME_LAST_ACTIVE, processes[i]);
        int64_t last_active = ValueCounters().GetCounterValue(cname);
        if (last_active == INVALID_COUNTER_VALUE) {
            status = status_critical;
            s << "Process " << processes[i] << " not active!" << std::endl;
        } else {
            int64_t not_active = abs(getTimeMs() / 1000 - last_active);
            if (not_active > 10) {
                status = status_critical;
                s << "Process " << processes[i] << " not active in last " << not_active << " seconds!" << std::endl;
            } else {
                if (not_active > 3) {
                    status = status_warning;
                    s << "Process " << processes[i] << " not active in last " << not_active << " seconds!" << std::endl;
                }
            }
        }
    }
    return status;
}

void UptimeDumpTable(std::ostream &os, const char *processes[], int count) {
    Counters::index_info_t index;
    ValueCounters().GetCategory("wd.", index);
    int width = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(processes[i]);
        if (len > width) {
            width = len;
        }
    }
    os << '|' << std::setw(width) << "Name" << "|    Uptime  | Inactive, s|" << std::endl;
    for (int i = 0; i < count; i++) {
        os << '|' << std::setw(width) << processes[i] << '|';
        char cname[NAME_MAX];
        snprintf(cname, sizeof(cname), CNAME_UPTIME, processes[i]);
        int64_t uptime = ValueCounters().GetCounterValue(cname);
        snprintf(cname, sizeof(cname), CNAME_LAST_ACTIVE, processes[i]);
        int64_t last_active = ValueCounters().GetCounterValue(cname);
        if (uptime == INVALID_COUNTER_VALUE) {
            os << " Not Active!| Not Active!";
        } else {
            DumpTime(os, uptime, 12);
            os << '|';
            DumpNumber(os, getTimeMs() / 1000 - last_active, 12);
        }
        os << '|' << std::endl;
    }
}
