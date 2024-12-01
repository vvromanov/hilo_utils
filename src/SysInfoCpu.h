#pragma once
#include <cstdint>

#define CPU_STATS \
        CPU_STAT(user)\
        CPU_STAT(nice)\
        CPU_STAT(system)\
        CPU_STAT(idle)\
        CPU_STAT(iowait)\
        CPU_STAT(irq)\
        CPU_STAT(softirq)\
        CPU_STAT(steal)

typedef struct { ;
#define CPU_STAT(name) uint64_t name;
    CPU_STATS
#undef CPU_STAT
} cpu_stat_t;

void cpu_stat_diff(const cpu_stat_t& r1, const cpu_stat_t& r2, cpu_stat_t& res);
bool cpu_stat_read(cpu_stat_t& res);
bool cpu_stat_update_counters();
void cpu_stat_update_counters(const cpu_stat_t& delta);

static inline uint64_t cpu_stat_total(const cpu_stat_t &r) {
        uint64_t total = 0;
#define CPU_STAT(name) total+=r.name;
        CPU_STATS
#undef CPU_STAT
        return total;
}

