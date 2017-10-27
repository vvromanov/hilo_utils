#include "counters_base.h"
#include "counters.h"

counter_t Counter::def_value;
counter_t LazyCounter::def_value;

Counter::Counter(const char *name, counter_type_t type) {
    SetName(name, type);
}

bool Counter::SetName(const char *name, counter_type_t type) {
    Counters &counters = (type == counter_incremental) ? IncrementedCounters() : ValueCounters();
    Counters::index_t index = counters.GetCounterIndex(name);
    if (index != DICTIONARY_INVALID_INDEX) {
        ptr = counters.GetCounterPtr(index);
        return true;
    }
    return false;
}
