#pragma once

#include <memory>
#include "ShmSimple.h"
#include "CDictionary.h"
#include "counter_t.h"

//#include "counters_base.h"

#define COUNTER_MAX_AMOUNT 5000
#define COUNTER_AVG_NAME_LEN 50
#define COUNTER_NAMES_SIZE ((COUNTER_AVG_NAME_LEN + 1) * COUNTER_MAX_AMOUNT)

#define COUNTERS_SHM_NAME_INCREMENTED   "tx_counters_inc"
#define COUNTERS_SHM_NAME_VALUE         "tx_counters_val"
#define INVALID_COUNTER_VALUE (INT64_MIN)

class CountersData {
public:
    ShmBase::shm_header_t header;
    typedef CDictionary<COUNTER_MAX_AMOUNT, COUNTER_NAMES_SIZE> Dictionary;
    Dictionary dict;

    counter_t counters[COUNTER_MAX_AMOUNT];

    void Init(size_t size) {
        dict.Clear();
    }

    static size_t get_size() {
        return sizeof(CountersData);
    }

//    static const char *get_name() {
//        return COUNTERS_SHM_NAME_INCREMENTED;
//    }
};

typedef enum {
    format_simple,
    format_raw,
    format_table,
    format_nagios,
    format_nagios_5m,
    format_nagios_total,
    format_unknown
} counters_format_t;

counters_format_t str2counters_format(const char *s);

class Counters : public ShmSimple<CountersData> {
public:
    typedef CountersData::Dictionary::name_rec_t name_rec_t;
    typedef CountersData::Dictionary::index_t index_t;
    typedef CountersData::Dictionary::index_info_t index_info_t;

    bool Open(const char *name, const char* suffix) {
        char name_[NAME_MAX];
        STRNCPY(name_, name);
        if (suffix) {
            STRNCAT(name_,suffix);
        }
        return ShmSimple<CountersData>::Open(name_, CountersData::get_size());
    }

    index_t GetCounterIndex(const char *name);
    index_t LookupIndex(const char *name);
    int64_t GetCounterValue(index_t index);
    int64_t GetCounterValue(const char *name);
    counter_t *GetCounterPtr(index_t index);
    void GetCategory(const char *prefix, index_info_t &index_info);

    const char *Lookup(index_t index) {
        return GetData()->dict.Lookup(index);
    }

    index_t Lookup(const char *name) {
        return GetData()->dict.Lookup(name);
    }
    index_t Add(const char *name) {
        return GetData()->dict.Add(name);
    }
    void DumpHtml(std::ostream &s, const char *prefix);
    void Dump(std::ostream &s, const char *prefix, counters_format_t format, bool hide_prefix);
};

extern const char* counters_suffix;
Counters& ValueCounters();
Counters& IncrementedCounters(bool open=true);
//Delete counters for testing purposes
bool CountersClear();