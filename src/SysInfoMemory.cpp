#include <cstdio>
#include <log.h>
#include <string.h>
#include <common_utils.h>
#include <sys/sysinfo.h>
#include "SysInfoMemory.h"

#define MEM_INFO "/proc/meminfo"

#define READ_MEM(name, field) \
        if (strcasecmp(line, name) == 0) { \
            if (!ConvertString(number, ms.field)) { \
                log_write(LOG_LEVEL_ERR, "Can't convert %s string '%s' to number", name, number); \
            } \
        }

bool mem_stat_read(mem_stat_t &ms) {
    memset(&ms, 0xff, sizeof(ms));
    FILE *f = fopen(MEM_INFO, "r");
    if (NULL == f) {
        log_write(LOG_LEVEL_ERR_ERRNO, "Can't read %s", MEM_INFO);
        return false;
    }
    char line[100];
    while (fgets(line, sizeof(line), f) != NULL) {
        char *p = strchr(line, ':');
        if (NULL == p) {
            continue;
        }
        *p = 0;
        p++;
        while (isspace(*p)) ++p;
        const char *number = p;
        while (isdigit(*p)) ++p;
        *p = 0;
#define MEM_STAT(name) READ_MEM(#name, name);
        MEM_STATS
#undef MEM_STAT
    }
    if (fclose(f)) {
        log_write(LOG_LEVEL_ERR_ERRNO, "fclose(%s)", MEM_INFO);
        return NULL;
    }
    ms.Cached -= ms.Shmem;
    ms.Used = (ms.MemTotal + ms.SwapTotal) -
              (ms.MemFree + ms.SwapFree + ms.Buffers + ms.Cached + ms.SReclaimable + ms.SUnreclaim + ms.Shmem);
    return true;
}


size_t GetMemoryPart(int percentage) {
    struct sysinfo info;
    sysinfo(&info);
    return (size_t)info.totalram * info.mem_unit * percentage / 100;
}
