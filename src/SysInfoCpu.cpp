#include <fcntl.h>
#include <strings.h>
#include <cstdio>
#include <cinttypes>
#include <unistd.h>
#include "SysInfoCpu.h"
#include "LogBase.h"
#include "counters_base.h"


#define CPU_STAT(name) LazyCounter cpu_##name("sysinfo.cpu." #name, counter_value);

CPU_STATS
#undef CPU_STAT

void cpu_stat_diff(const cpu_stat_t &r1, const cpu_stat_t &r2, cpu_stat_t &res) {
#define CPU_STAT(name) res.name=r1.name-r2.name; if (res.name>INT32_MAX) res.name=0;
    CPU_STATS
#undef CPU_STAT
}

bool cpu_stat_read(cpu_stat_t &res) {
    char szBuff[255];
    int fd = open("/proc/stat", O_RDONLY, 0);
    if (fd == -1) {
        log_write(LOG_LEVEL_ERR_ERRNO, "Can't open /proc/stat!");
        return false;
    }
    ssize_t len = read(fd, szBuff, sizeof(szBuff) - 1);
    if (len < 0) {
        log_write(LOG_LEVEL_ERR_ERRNO, "Can't read from /proc/stat!");
        close(fd);
        return false;
    }
    szBuff[len] = 0;
    bzero(&res, sizeof(res));
    uint64_t fake;
    int count = sscanf(szBuff,
                       "cpu"
#define CPU_STAT(name) " %" PRId64
                    CPU_STATS
#undef CPU_STAT
                    " %" PRId64,
#define CPU_STAT(name) &res.name,
                       CPU_STATS
#undef CPU_STAT
                       &fake);
    if (count < 8) {
        log_write(LOG_LEVEL_ERR_ERRNO, "Invalid format of /proc/stat!");
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

void cpu_stat_update_counters(const cpu_stat_t &delta) {
    uint64_t total = cpu_stat_total(delta);
    if (total) {
#define CPU_STAT(name) cpu_##name=((delta.name*10000./total)+0.5);
        CPU_STATS
#undef CPU_STAT
    }
}

bool cpu_stat_update_counters() {
    cpu_stat_t new_stat;
    static cpu_stat_t prev_stat = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    if (!cpu_stat_read(new_stat)) {
        return false;
    }
    if (prev_stat.idle != 0xff) {
        cpu_stat_t delta;
        cpu_stat_diff(new_stat, prev_stat, delta);
        cpu_stat_update_counters(delta);
    }
    prev_stat = new_stat;
    return true;
}
