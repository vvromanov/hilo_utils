#pragma once


#include <cstdint>
#include <ctime>
#include <sys/time.h>
#include <sys/resource.h>
#include <ostream>

class BenchTime {
public:
    struct timespec clock;
    rusage ru;

    BenchTime() {
        GetTime();
    }

    void GetTime() {
#ifdef CYGWIN
        clock_gettime(CLOCK_REALTIME, &clock);
#else
        clock_gettime(CLOCK_MONOTONIC_RAW, &clock);
#endif
        getrusage(RUSAGE_SELF, &ru);
    }
    static void DumpDelta(std::ostream &s, const BenchTime& start, const BenchTime& stop=BenchTime());
};

inline static uint64_t ts2ms(const struct timespec &val) {
    return val.tv_sec * 1000 + val.tv_nsec / 1000000;
}

inline static uint64_t ts2ms(const struct timeval &val) {
    return val.tv_sec * 1000 + val.tv_usec / 1000;
}

