#pragma once

#include "HistoryCounters.h"

class HistoryCounterLocal {
    int64_t count;
    int64_t volume;
public:
    HistoryCounterLocal() : count(0), volume(0) {
    }

    void operator++() {
        ++count;
    }

    void AddVolume(int64_t _volume) {
        ++count;
        volume += _volume;
    }

    int64_t Count() const {
        return count;
    }

    int64_t Volume() const {
        return volume;
    }

    void TransferTo(HistoryCounter &c) {
        c.AddBatch(count, volume);
        count = 0;
        volume = 0;
    }

    void TransferTo(HistoryCounterData &c) {
        c.AddBatch(count, volume);
        count = 0;
        volume = 0;
    }
};
