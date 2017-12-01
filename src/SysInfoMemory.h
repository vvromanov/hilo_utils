#pragma once

#include <cstdint>

#define MEM_STATS \
        MEM_STAT(MemTotal)\
        MEM_STAT(MemFree)\
        MEM_STAT(Buffers) \
        MEM_STAT(Cached)\
        MEM_STAT(SwapCached)\
        MEM_STAT(Used)\
        MEM_STAT(Shmem)\
        MEM_STAT(SReclaimable)\
        MEM_STAT(SUnreclaim)\
        MEM_STAT(SwapTotal)\
        MEM_STAT(SwapFree)
typedef struct {
#define MEM_STAT(name) int64_t name;
    MEM_STATS
#undef MEM_STAT
} mem_stat_t;

bool mem_stat_read(mem_stat_t& ms);
int64_t GetMemoryPart(int percentage);
