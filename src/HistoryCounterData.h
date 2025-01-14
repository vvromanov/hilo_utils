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

    void operator++();
    int64_t GetTotalCount();
    int64_t GetTotalVolume();
    int64_t GetIntervalCount();
    int64_t GetIntervalVolume();    
    int64_t GetLastCount();
    int64_t GetLastAvg();
    int64_t GetTotalAvg();
    int64_t GetIntervalAvg();
    int64_t GetLastVolume();
    void GetInfo(history_counter_info_t& info);

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

