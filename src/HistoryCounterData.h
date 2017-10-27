#pragma once

#include <cstdint>
#include "counter_t.h"
#include "auto_mutex.h"
#include <pthread.h>
#include <ostream>

struct HistoryRec {
    volatile int64_t count;
    volatile int64_t sum;
} __attribute__((__packed__));

typedef enum {
    HistoryUnknown,
    HistoryCount, //Просто какое-то событие
    HistoryVolume, //Учитываем количество и объем. Например, сколько данных передано по сети
    HistoryCall //Учитываем количество и среднее время вызова
} HistoryCounterType_t;

class HistoryCounterData {
public:
    constexpr static uint32_t HistorySize = 60 * 5;

    void Init(HistoryCounterType_t t);

    void SetType(HistoryCounterType_t t) { type = t; }

    void Clear();
    void AddVolume(int64_t v);
    void AddBatch(int64_t c, int64_t v);

    HistoryCounterType_t GetType() const {
        return type;
    }

    void operator++() {
        WRITE_LOCK;
        update();
        ++recs[rec_index].count;
        ++total_count;
        ++interval_count;
    }

    int64_t GetTotalCount() {
        return total_count;
    }

    int64_t GetTotalVolume() {
        return total_summ;
    }

    int64_t GetIntervalCount() {
        update();
        return interval_count;
    }

    int64_t GetIntervalVolume() {
        update();
        return interval_summ;
    }

    int64_t GetLastCount() {
        update();
        return recs[(rec_index == 0) ? HistorySize : (rec_index - 1)].count;
    }

    int64_t GetLastVolume() {
        update();
        return recs[(rec_index == 0) ? HistorySize : (rec_index - 1)].sum;
    }

    void DumpTable(std::ostream &s);
    void DumpHtml(std::ostream &s);

protected:
    pthread_mutex_t mutex;
    HistoryRec recs[HistorySize + 1];
    volatile int64_t current_rec;
    counter_t total_count;
    counter_t total_summ;
    counter_t interval_count;
    counter_t interval_summ;
    volatile int rec_index;
    int64_t started_ms;
    HistoryCounterType_t type;

    pthread_mutex_t *GetMutex() { return &mutex; }

    void update();
} __attribute__((__packed__));

