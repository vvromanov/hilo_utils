#include "rrd_utils.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#define HAVE_STDINT_H
#include <rrd.h>
#include <rrd_client.h>
#ifdef __cplusplus
}
#endif

#include "LogBase.h"
#include <SysInfoCpu.h>
#include <file_utils.h>
#include <SysInfoMemory.h>
#include <CDictionary.h>
#include <HistoryCounters.h>
#include "common_utils.h"
#include <sys/sysinfo.h>

#define RRD_CACHED "unix:/run/dtp/rrd-cached.sock"
#define RRD_CACHED_DIR "/var/lib/rrd-writer/rrd"
#define RRD_STEP 2 //seconds

#define RRA1 "RRA:AVERAGE:0.95:2:86400" //1d
#define RRA2 "RRA:AVERAGE:0.95:10:120960" //14d
#define RRA3 "RRA:AVERAGE:0.95:60:129600" //90d
#define RRA4 "RRA:AVERAGE:0.95:600:108000" //750d

static bool rrd_check_status(int status, const char *func, const char* file, const char* update) {
    if (status != 0) {
        log_write(LOG_LEVEL_ERR, "%s: status=%d (%s) file='%s' param='%s'", func, status, rrd_get_error(), file, update);
        rrd_clear_error();
        return false;
    }
    return true;
}

bool rrd_connect() {
    int status;

    rrdc_disconnect();

    rrd_clear_error();
    status = rrdc_connect(RRD_CACHED);
    if (!rrd_check_status(status, "rrdc_connect", "", RRD_CACHED)) {
        return false;
    }
    log_write(LOG_LEVEL_INFO, "rrdcached client: connected to %s", RRD_CACHED);
    return true;
}

bool rrd_check_connect() {
    int status = rrdc_is_connected(RRD_CACHED);
    if (0 == status) {
        return rrd_connect();
    } else {
        return true;
    }
}

bool rrd_create_file(const char *filename, int count, const char *ds[]) {
    if (!rrd_check_connect()) {
        return false;
    }
    const char *argv[100];
    int argc = 0;
    for (int i = 0; i < count; i++) {
        argv[argc++] = ds[i];
    }
    argv[argc++] = RRA1;
    argv[argc++] = RRA2;
    argv[argc++] = RRA3;
#ifdef RRA4
    argv[argc++] = RRA4;
#endif
    int status = rrd_create_r(filename, RRD_STEP, time(NULL) - 10, argc, argv);
    if (!rrd_check_status(status, "rrdc_create", filename,"")) {
        log_write(LOG_LEVEL_ERR, "Can't create %s file", filename);
        return false;
    }
    status = rrdc_flush(filename);
    if (!rrd_check_status(status, "rrdc_flush", filename,"")) {
        return false;
    }
    log_write(LOG_LEVEL_NOTICE, "File %s created", filename);
    return true;
}

bool rrd_create_cpu() {
#define CPU_STAT(n) "DS:" #n ":GAUGE:" TO_STRING(RRD_STEP) ":0:110",
    const char *ds[] = {
            CPU_STATS
    };
#undef CPU_STAT
    return rrd_create_file(RRD_CACHED_DIR "/cpu.rrd", ARRAY_SIZE(ds), ds);
}

bool rrd_create_memory() {
#define MEM_STAT(n) "DS:" #n ":GAUGE:" TO_STRING(RRD_STEP) ":0:1000000000000",
    const char *ds[] = {
            MEM_STATS
    };
#undef MEM_STAT
    return rrd_create_file(RRD_CACHED_DIR "/memory.rrd", ARRAY_SIZE(ds), ds);
}

static bool rrd_update(const char *filename, const char *update) {
    if (!rrd_check_connect()) {
        return false;
    }
    const char *argv[1] = {update};
    int status = rrdc_update(filename, 1, argv);
    log_write(LOG_LEVEL_PARN, "update file %s with %s", filename, update);
    if (!rrd_check_status(status, "rrdc_update", filename, update)) {
        return false;
    }
    return true;
}

bool rrd_update_cpu() {
    cpu_stat_t new_stat;
    static cpu_stat_t prev_stat = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    if (!is_file_exists(RRD_CACHED_DIR "/cpu.rrd")) {
        if (!rrd_create_cpu()) {
            return false;
        }
    }
    if (!cpu_stat_read(new_stat)) {
        return false;
    }
    if (prev_stat.idle != 0xFF) {
        cpu_stat_t delta;
        cpu_stat_diff(new_stat, prev_stat, delta);
        uint64_t total = cpu_stat_total(delta);
        if (total) {
            cpu_stat_update_counters(delta);
            char update[256];
            snprintf(update, sizeof(update), "%lu"
#define CPU_STAT(n) ":%g"
                             CPU_STATS
#undef CPU_STAT
                             "%s", time(NULL),
#define CPU_STAT(n) delta.n*100/(double)total,
                     CPU_STATS
#undef CPU_STAT
                     ""
            );
            rrd_update(RRD_CACHED_DIR "/cpu.rrd", update);
        }
    }
    prev_stat = new_stat;
    return true;
}

bool rrd_update_memory() {
    if (!is_file_exists(RRD_CACHED_DIR "/memory.rrd")) {
        if (!rrd_create_memory()) {
            return false;
        }
    }
    mem_stat_t stat;
    if (!mem_stat_read(stat)) {
        return false;
    }
    char update[256];
    snprintf(update, sizeof(update), "%lu"
#define MEM_STAT(n) ":%ld"
                     MEM_STATS
#undef MEM_STAT
                     "%s", time(NULL),
#define MEM_STAT(n) stat.n*1024,
             MEM_STATS
#undef MEM_STAT
             ""
    );
    return rrd_update(RRD_CACHED_DIR "/memory.rrd", update);
}

static bool rrd_update_counter(Counters &c, Counters::name_rec_t &counter, counter_type_t type) {
    char filename[NAME_MAX];
    STRNCPY(filename, RRD_CACHED_DIR
            "/");
    STRNCAT(filename, c.Lookup(counter.id));
    STRNCAT(filename, ".rrd");
    if (!is_file_exists(filename)) {
        const char *ds_val[] = {
                "DS:value:GAUGE:" TO_STRING(RRD_STEP) ":U:U",
        };
        const char *ds_inc[] = {
                "DS:value:COUNTER:" TO_STRING(RRD_STEP) ":0:1000000000",
        };
        if (!rrd_create_file(filename, 1, ((type == counter_incremental) ? ds_inc : ds_val))) {
            return false;
        }
    }
    char update[256];
    snprintf(update, sizeof(update), "%lu:%ld", time(NULL), c.GetCounterValue(counter.id));
    return rrd_update(filename, update);
}

bool rrd_update_counters(const char *prefix, counter_type_t type) {
    Counters &c = (type == counter_incremental) ? IncrementedCounters() : ValueCounters();
    Counters::index_info_t index_info;
    c.GetCategory(prefix, index_info);
    for (int i = 0; i < index_info.count; i++) {
        if (!rrd_update_counter(c, index_info.index[i], type)) {
            return false;
        }
    }
    return true;
}

static bool rrd_update_history_counter(HistoryCounters::name_rec_t &counter) {
    char filename[NAME_MAX];
    STRNCPY(filename, RRD_CACHED_DIR
            "/");
    STRNCAT(filename, GetHistoryCounters().Lookup(counter.id));
    STRNCAT(filename, ".rrd");
    HistoryCounterData *d = GetHistoryCounters().GetCounterPtr(counter.id);
    if (!is_file_exists(filename)) {
        const char *ds_count[] = {
                "DS:count:COUNTER:" TO_STRING(RRD_STEP) ":0:1000000000",
        };
        const char *ds_volume[] = {
                "DS:count:COUNTER:" TO_STRING(RRD_STEP) ":0:1000000000",
                "DS:volume:COUNTER:" TO_STRING(RRD_STEP) ":0:1000000000",
        };
        const char *ds_call[] = {
                "DS:count:GAUGE:" TO_STRING(RRD_STEP) ":0:1000000000",
                "DS:avg:GAUGE:" TO_STRING(RRD_STEP) ":0:1000000000",
        };
        switch (d->GetType()) {
            case HistoryCount:
                if (!rrd_create_file(filename, ARRAY_SIZE(ds_count), ds_count)) {
                    return false;
                }
                break;
            case HistoryVolume: //Учитываем количество и объем. Например, сколько данных передано по сети
                if (!rrd_create_file(filename, ARRAY_SIZE(ds_volume), ds_volume)) {
                    return false;
                }
                break;
            case HistoryCall: //Учитываем количество и среднее время вызова
                if (!rrd_create_file(filename, ARRAY_SIZE(ds_call), ds_call)) {
                    return false;
                }
                break;
            case HistoryUnknown:
                return true;
        }
    }
    char update[256];
    switch (d->GetType()) {
        case HistoryCount:
            snprintf(update, sizeof(update), "%lu:%ld", time(NULL), d->GetTotalCount());
            break;
        case HistoryVolume: //Учитываем количество и объем. Например, сколько данных передано по сети
            snprintf(update, sizeof(update), "%lu:%ld:%ld", time(NULL), d->GetTotalCount(), d->GetTotalVolume());
            break;
        case HistoryCall: //Учитываем количество и среднее время вызова
            snprintf(update, sizeof(update), "%lu:%ld:%ld", time(NULL), d->GetLastCount(), d->GetLastAvg());
            break;
        case HistoryUnknown:
            return true;
    }
    return rrd_update(filename, update);
}

bool rrd_update_history_counters(const char *prefix) {
    HistoryCounters::index_info_t index_info;
    GetHistoryCounters().GetCategory(prefix, index_info);
    for (int i = 0; i < index_info.count; i++) {
        if (!rrd_update_history_counter(index_info.index[i])) {
            return false;
        }
    }
    return true;
}

bool rrd_create_load() {
    const char *ds[] = {
            "DS:load_1:GAUGE:" TO_STRING(RRD_STEP) ":0:U",
            "DS:load_5:GAUGE:" TO_STRING(RRD_STEP) ":0:U",
            "DS:load_15:GAUGE:" TO_STRING(RRD_STEP) ":0:U",
    };
    return rrd_create_file(RRD_CACHED_DIR "/load.rrd", ARRAY_SIZE(ds), ds);
}

bool rrd_update_load() {
    if (!is_file_exists(RRD_CACHED_DIR "/load.rrd")) {
        if (!rrd_create_load()) {
            return false;
        }
    }
    struct sysinfo si;
    sysinfo(&si);
    char update[128];
    snprintf(update, sizeof(update), "%lu:%g:%g:%g", time(NULL), (double) si.loads[0] / (1 << SI_LOAD_SHIFT),
             (double) si.loads[1] / (1 << SI_LOAD_SHIFT), (double) si.loads[2] / (1 << SI_LOAD_SHIFT));
    return rrd_update(RRD_CACHED_DIR "/load.rrd", update);
}
