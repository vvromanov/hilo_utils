#include "LogStorage.h"
#include "time_utils.h"

log_source_t log_source = -1;
size_t opt_log_storage_size = 500 * 1024 * 1024;

Counters &LogCounters() {
    static Counters logCounters;
    if (!logCounters.IsOpened()) {
        ScopedLogLevel ll(LOG_LEVEL_DISABLED);
        logCounters.Open(LOG_COUNTERS_SHM_NAME, counters_suffix);
    }
    return logCounters;
}

ShmBufferEx &LogStorage() {
    static ShmBufferEx logStorage;
    if (!logStorage.IsOpened()) {
        ScopedLogLevel ll(LOG_LEVEL_DISABLED);
        char name[NAME_MAX];
        STRNCPY(name, LOG_STORAGE_SHM_NAME);
        if (counters_suffix) {
            STRNCAT(name, counters_suffix);
        }
        logStorage.Open(name, opt_log_storage_size);
        logStorage.SetOverflovBehavior(drop_old);
    }
    return logStorage;
}


static void update_log_source() {
    log_source = LogCounters().Add(log_get_worker_name());
}

bool log_write_record(log_level_t level, const char *message, int len) {
    static bool in_log_write = false;
    if (in_log_write) {
        return false;
    }
    in_log_write = true;
    if (-1 == log_source) {
        update_log_source();
    }
    log_record_t hdr;
    hdr.level = level;
    hdr.pid = getpid();
    hdr.timestamp_us = getTimeUs();
    hdr.source = log_source;
    ShmChunks c(&hdr, sizeof(hdr));
    if (len < 0) {
        len = strlen(message);
    }
    c.Add(message, len);
    bool res = LogStorage().Add(c);
    in_log_write = false;
    return res;
}
