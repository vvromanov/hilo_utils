#include "HistoryCounterData.h"
#include "common_utils.h"
#include "time_utils.h"
#include "DumpUtils.h"

void HistoryCounterData::Init(HistoryCounterType_t t) {
    pthread_mutexattr_t a;
    pthread_mutexattr_init(&a);
    pthread_mutexattr_setpshared(&a, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex, &a);
    pthread_mutexattr_destroy(&a);
    Clear();
    SetType(t);
}


void HistoryCounterData::Clear() {
    WRITE_LOCK;
    BZERO_S(recs);
    current_rec = (getClockMs() / 1000);
    rec_index = current_rec % (HistorySize + 1);
    total_count = 0;
    total_summ = 0;
    interval_count = 0;
    interval_summ = 0;
    started_ms = getTimeMs();
}

void HistoryCounterData::update() {
    WRITE_LOCK;
    int64_t index = (getClockMs() / 1000);
    if ((index - current_rec) > HistorySize) { //Последнее обновление было очень давно. Интервал давно прошел
        memset(recs, 0, sizeof(recs));
        interval_count = 0;
        interval_summ = 0;
        current_rec = index;
    } else {
        while (current_rec != index) {
            ++current_rec;
            rec_index = current_rec % (HistorySize + 1);
            interval_count -= recs[rec_index].count;
            interval_summ -= recs[rec_index].sum;
            recs[rec_index].count = 0;
            recs[rec_index].sum = 0;
        }
    }
}

void HistoryCounterData::AddVolume(int64_t v) {
    WRITE_LOCK;
    update();
    ++recs[rec_index].count;
    recs[rec_index].sum += v;
    ++total_count;
    ++interval_count;
    total_summ += v;
    interval_summ += v;
}

void HistoryCounterData::AddBatch(int64_t c, int64_t v) {
    WRITE_LOCK;
    update();
    recs[rec_index].count += c;
    recs[rec_index].sum += v;
    total_count += c;
    interval_count += c;
    total_summ += v;
    interval_summ += v;
}


void HistoryCounterData::DumpTable(std::ostream &s) {
    s << '|';
    DumpNumber(s, GetLastCount(), 6);
    s << '|';
    if (GetType() == HistoryCount) {
        s << "      ";
    } else {
        DumpNumber(s, GetLastVolume(), 6);
    }
    s << '|';
    DumpNumber(s, GetIntervalCount(), 6);
    s << '|';
    if (GetType() == HistoryCount) {
        s << "      ";
    } else {
        DumpNumber(s, GetIntervalVolume(), 6);
    }
    s << '|';
    DumpNumber(s, GetTotalCount(), 6);
    s << '|';
    if (GetType() == HistoryCount) {
        s << "      ";
    } else {
        DumpNumber(s, GetTotalVolume(), 6);
    }
    s << "|" << std::endl;
}

void HistoryCounterData::DumpHtml(std::ostream &s) {
    s << "</td><td align='right'>";
    DumpNumber(s, GetLastCount(), 6);
    s << "</td><td align='right'>";
    if (GetType() == HistoryCount) {
        s << "&nbsp;";
    } else {
        DumpNumber(s, GetLastVolume(), 6);
    }

    s << "</td><td align='right'>";
    DumpNumber(s, GetIntervalCount(), 6);
    s << "</td><td align='right'>";
    if (GetType() == HistoryCount) {
        s << "&nbsp;";
    } else {
        DumpNumber(s, GetIntervalVolume(), 6);
    }
    s << "</td><td align='right'>";
    DumpNumber(s, GetTotalCount(), 6);
    s << "</td><td align='right'>";
    if (GetType() == HistoryCount) {
        s << "&nbsp;";
    } else {
        DumpNumber(s, GetTotalVolume(), 6);
    }
    s << "</td></tr>" << std::endl;
}
