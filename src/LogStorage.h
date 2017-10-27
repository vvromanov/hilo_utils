#pragma once


#include <signal.h>
#include <cstdint>
#include <cstdarg>
#include "log.h"
#include "ShmBufferEx.h"
#include "counters.h"

#define LOG_COUNTERS_SHM_NAME   "tx_log_counters"
#define LOG_STORAGE_SHM_NAME    "tx_log_storage"

extern size_t opt_log_storage_size;

typedef int16_t log_source_t;
extern log_source_t log_source;

typedef struct {
    uint64_t timestamp_us;
    pid_t pid;
    log_source_t source;
    log_level_t level:8;
    char message[];
} __attribute__((__packed__)) log_record_t;

Counters &LogCounters();
ShmBufferEx &LogStorage();

bool log_write_record(log_level_t level, const char *message, int len);
