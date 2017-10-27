#include "BenchTime.h"

void BenchTime::DumpDelta(std::ostream &s, const BenchTime &start, const BenchTime &stop) {
    s << "Clock/user/sys delta: " << ts2ms(stop.clock) - ts2ms(start.clock) << '/'
      << ts2ms(stop.ru.ru_utime) - ts2ms(start.ru.ru_utime)  << '/' << ts2ms(stop.ru.ru_stime) - ts2ms(start.ru.ru_stime)
      << " ms" << std::endl;
}
