#pragma once

#include "ShmBase.h"
#include "counters.h"
#include "HistoryCounterData.h"

#define COUNTERS_SHM_NAME_HISTORY         "tx_counters_history"
#define COUNTER_MAX_AMOUNT_HISTORY 500
#define COUNTER_NAMES_SIZE_HISTORY ((COUNTER_AVG_NAME_LEN + 1) * COUNTER_MAX_AMOUNT_HISTORY)

class HistoryCountersData {
public:
    ShmBase::shm_header_t header;
    typedef CDictionary<COUNTER_MAX_AMOUNT_HISTORY, COUNTER_NAMES_SIZE_HISTORY> Dictionary;
    Dictionary dict;

    HistoryCounterData counters[COUNTER_MAX_AMOUNT_HISTORY];

    void Init(size_t size) {
        dict.Clear();
        for (int i = 0; i < COUNTER_MAX_AMOUNT_HISTORY; i++) {
            counters[i].Init(HistoryUnknown);
        }
    }

    static size_t get_size() {
        return sizeof(HistoryCountersData);
    }

};

class HistoryCounters : public ShmSimple<HistoryCountersData> {
public:
    typedef HistoryCountersData::Dictionary::index_t index_t;
    typedef HistoryCountersData::Dictionary::index_info_t index_info_t;
    typedef HistoryCountersData::Dictionary::name_rec_t name_rec_t;

    HistoryCounters() {};
    HistoryCounters(const HistoryCounters&) = delete;
    HistoryCounters& operator=(const HistoryCounters&) = delete;

    bool Open(const char *name, const char *suffix) {
        char name_[NAME_MAX];
        STRNCPY(name_, name);
        if (suffix) {
            STRNCAT(name_, suffix);
        }
        return ShmSimple<HistoryCountersData>::Open(name_, HistoryCountersData::get_size());
    }

    index_t GetCounterIndex(const char *name);
    index_t LookupIndex(const char *name);
    HistoryCounterData *GetCounterPtr(const char *name);

    void GetCounterInfo(const char *name, history_counter_info_t& info) {
        HistoryCounterData *p=GetCounterPtr(name);
        if (p) {
            p->GetInfo(info);
        } else {
            BZERO_S(info);
        }
    }

    HistoryCounterData *GetCounterPtr(index_t index) {
        return GetData()->counters + index;
    }

    void GetCategory(const char *prefix, index_info_t &index_info) {
        GetData()->dict.GetCategory(prefix, index_info);
    }

    const char *Lookup(index_t index) {
        return GetData()->dict.Lookup(index);
    }

    index_t Lookup(const char *name) {
        return GetData()->dict.Lookup(name);
    }

    void DumpHtml(std::ostream &s, const char *prefix);
    void Dump(std::ostream &s, const char *prefix, counters_format_t format, bool hide_prefix);
};

HistoryCounters &GetHistoryCounters();

//Delete counters for testing purposes
bool HistoryCountersClear();

class HistoryCounter {
    std::string name;
    HistoryCounterData *d = NULL;
    HistoryCounterType_t type;
public:
    HistoryCounter() {
    }

    HistoryCounter(const char *n, HistoryCounterType_t t) {
        name = n;
        type = t;
    }

    void SetName(const char *_name, HistoryCounterType_t t) {
        Reset();
        name = _name;
        type = t;
    };

    void Reset() {
        d = nullptr;
    }

    void AddVolume(int64_t v) {
        Init();
        d->AddVolume(v);
    }

    void AddBatch(int64_t c, int64_t v) {
        Init();
        d->AddBatch(c, v);
    }

    void operator++() {
        Init();
        ++(*d);
    }

    void operator++(int) {
        Init();
        ++(*d);
    }

    int64_t GetIntervalCount() {
        Init();
        return d->GetIntervalCount();
    }

    int64_t GetIntervalVolume() {
        Init();
        return d->GetIntervalVolume();
    }

    int64_t GetLastCount() {
        Init();
        return d->GetLastCount();
    }

    int64_t GetLastVolume() {
        Init();
        return d->GetLastVolume();
    }

    int64_t GetTotalCount() {
        Init();
        return d->GetTotalCount();
    }

    int64_t GetTotalVolume() {
        Init();
        return d->GetTotalVolume();
    }

    void GetInfo(history_counter_info_t& info) {
        Init();
        return d->GetInfo(info);
    }

    void Init() {
        if (nullptr == d) {
            d = GetHistoryCounters().GetCounterPtr(name.c_str());
            d->SetType(type);
        }
    }
};
