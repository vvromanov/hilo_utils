#pragma once

#include <cstdint>
#include "counter_t.h"
#include "auto_mutex.h"
#include <pthread.h>
#include <ostream>

#define VOLATILE_MEMBER

struct HistoryRec {
    int64_t count;
    int64_t sum;
} __attribute__((__packed__));

typedef enum {
    HistoryUnknown,
    HistoryCount, //Просто какое-то событие
    HistoryVolume, //Учитываем количество и объем. Например, сколько данных передано по сети
    HistoryCall //Учитываем количество и среднее время вызова
} HistoryCounterType_t;

typedef struct {
    int64_t last_count;
    int64_t last_summ;
    int64_t interval_count;
    int64_t interval_summ;
    int64_t total_count;
    int64_t total_summ;
} history_counter_info_t;

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
        WRITE_LOCK;
        return total_count;
    }

    int64_t GetTotalVolume() {
        WRITE_LOCK;
        return total_summ;
    }

    int64_t GetIntervalCount() {
        WRITE_LOCK;
        update();
        return interval_count;
    }

    int64_t GetIntervalVolume() {
        WRITE_LOCK;
        update();
        return interval_summ;
    }

    int64_t GetLastCount() {
        WRITE_LOCK;
        update();
        return recs[(rec_index == 0) ? HistorySize : (rec_index - 1)].count;
    }

    int64_t GetLastAvg() {
        WRITE_LOCK;
        update();
        const HistoryRec &rec = recs[(rec_index == 0) ? HistorySize : (rec_index - 1)];
        if (rec.count == 0) {
            return 0;
        } else {
            return rec.sum / rec.count;
        }
    }

    int64_t GetTotalAvg() {
        WRITE_LOCK;
        update();
        if (total_count) {
            return total_summ / total_count;
        } else {
            return 0;
        }
    }

    int64_t GetIntervalAvg() {
        WRITE_LOCK;
        update();
        if (interval_count) {
            return interval_summ / interval_count;
        } else {
            return 0;
        }
    }

    int64_t GetLastVolume() {
        WRITE_LOCK;
        update();
        return recs[(rec_index == 0) ? HistorySize : (rec_index - 1)].sum;
    }

    void GetInfo(history_counter_info_t &info) {
        WRITE_LOCK;
        int last_index = (rec_index == 0) ? HistorySize : (rec_index - 1);
        info.last_count = recs[last_index].count;
        info.last_summ = recs[last_index].sum;
        info.interval_count = interval_count;
        info.interval_summ = interval_summ;
        info.total_count = total_count;
        info.total_summ = total_summ;
    }

    void DumpTable(std::ostream &s);
    void DumpSimple(std::ostream &s);
    void DumpHtml(std::ostream &s);

protected:
    simple_mutex_t mutex;
    HistoryRec recs[HistorySize + 1];
    VOLATILE_MEMBER int64_t current_rec;
    counter_t total_count;
    counter_t total_summ;
    counter_t interval_count;
    counter_t interval_summ;
    VOLATILE_MEMBER int rec_index;
    int64_t started_ms;
    HistoryCounterType_t type;

    inline simple_mutex_t& GetMutex() { return mutex; }

    void update();
} __attribute__((__packed__));

