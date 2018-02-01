#include "LogStorage.h"
#include "time_utils.h"
#include "SysInfoMemory.h"

log_source_t log_source = -1;

size_t opt_log_storage_size = std::min(GetMemoryPart(5), (size_t) 500 * 1024 * 1024);

Counters &LogCounters() {
    static Counters *logCounters = NULL;
    if (NULL == logCounters) {
        if (opt_log_level == LOG_LEVEL_PARN) {
            fprintf(stderr, "Open LogCounters\n");
        }
        logCounters = new Counters();
        {
            ScopedLogLevel ll(LOG_LEVEL_DISABLED);
            logCounters->Open(LOG_COUNTERS_SHM_NAME, counters_suffix);
        }
        if (opt_log_level == LOG_LEVEL_PARN) {
            if (logCounters->IsOpened()) {
                fprintf(stderr, "Log counters opened\n");
            } else {
                fprintf(stderr, "Can't open log counters!!!\n");
                delete logCounters;
                logCounters = NULL;
            }
        }
    }
    return *logCounters;
}

ShmBufferEx &LogStorage() {
    static ShmBufferEx *logStorage = NULL;
    if (NULL == logStorage) {
        char name[NAME_MAX];
        STRNCPY(name, LOG_STORAGE_SHM_NAME);
        if (counters_suffix) {
            STRNCAT(name, counters_suffix);
        }
        if (opt_log_level == LOG_LEVEL_PARN) {
            fprintf(stderr, "Open log storage %s size=%lu\n", name, opt_log_storage_size);
        }
        logStorage = new ShmBufferEx();
        {
            ScopedLogLevel ll(LOG_LEVEL_DISABLED);
            logStorage->Open(name, opt_log_storage_size);
        }
        if (!logStorage->IsOpened()) {
            fprintf(stderr, "Can`t open log storage %s size=%lu [E%d] %s\n", name, opt_log_storage_size, errno,
                    strerror(errno));
            delete logStorage;
            logStorage = NULL;
        } else {
            if (opt_log_level == LOG_LEVEL_PARN) {
                fprintf(stderr, "Log storage opened\n");
            }
            logStorage->SetOverflovBehavior(drop_old, true);
        }
    }
    return *logStorage;
}


bool log_write_record(log_level_t level, const char *message, int len) {
    static bool in_log_write = false;
    if (in_log_write) {
        if (opt_log_level == LOG_LEVEL_PARN) {
            fprintf(stderr, "log_write_record - in_log_write\n");
        }
        return false;
    }
    in_log_write = true;
    if (-1 == log_source) {
        if (opt_log_level == LOG_LEVEL_PARN) {
            fprintf(stderr, "log_write_record - update_log_source\n");
        }
        log_source = LogCounters().Add(log_get_worker_name());
    }
//    fprintf(stderr, "log_write_record - construct log_record\n");
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
    //  fprintf(stderr, "log_write_record - call LogStorage().Push(c)\n");
    bool res = LogStorage().Push(c);
    in_log_write = false;
    return res;
}
