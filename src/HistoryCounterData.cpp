#include "HistoryCounterData.h"
#include "common_utils.h"
#include "time_utils.h"
#include "DumpUtils.h"

void HistoryCounterData::Init(HistoryCounterType_t t) {
    simple_mutex_init(mutex);
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

void HistoryCounterData::operator++()
{
    WRITE_LOCK;
    update();
    ++recs[rec_index].count;
    ++total_count;
    ++interval_count;
}

int64_t HistoryCounterData::GetTotalCount()
{
    WRITE_LOCK;
    return total_count;
}

int64_t HistoryCounterData::GetTotalVolume()
{
    WRITE_LOCK;
    return total_summ;
}

int64_t HistoryCounterData::GetIntervalCount()
{
    WRITE_LOCK;
    update();
    return interval_count;
}

int64_t HistoryCounterData::GetIntervalVolume()
{
    WRITE_LOCK;
    update();
    return interval_summ;
}

int64_t HistoryCounterData::GetLastCount()
{
    WRITE_LOCK;
    update();
    return recs[(rec_index == 0) ? HistorySize : (rec_index - 1)].count;
}

int64_t HistoryCounterData::GetLastAvg()
{
    WRITE_LOCK;
    update();
    const HistoryRec& rec = recs[(rec_index == 0) ? HistorySize : (rec_index - 1)];
    if (rec.count == 0) {
        return 0;
    } else {
        return rec.sum / rec.count;
    }
}

int64_t HistoryCounterData::GetTotalAvg()
{
    WRITE_LOCK;
    update();
    if (total_count) {
        return total_summ / total_count;
    } else {
        return 0;
    }
}

int64_t HistoryCounterData::GetIntervalAvg()
{
    WRITE_LOCK;
    update();
    if (interval_count) {
        return interval_summ / interval_count;
    } else {
        return 0;
    }
}

int64_t HistoryCounterData::GetLastVolume()
{
    WRITE_LOCK;
    update();
    return recs[(rec_index == 0) ? HistorySize : (rec_index - 1)].sum;
}

void HistoryCounterData::GetInfo(history_counter_info_t& info)
{
    WRITE_LOCK;
    int last_index = (rec_index == 0) ? HistorySize : (rec_index - 1);
    info.last_count = recs[last_index].count;
    info.last_summ = recs[last_index].sum;
    info.interval_count = interval_count;
    info.interval_summ = interval_summ;
    info.total_count = total_count;
    info.total_summ = total_summ;
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


void HistoryCounterData::DumpSimple(std::ostream &s) {
    switch (GetType()) {
        case HistoryVolume:
            s << ' ' << GetLastCount();
            s << ' ' << GetLastVolume();
            s << ' ' << GetIntervalCount();
            s << ' ' << GetIntervalVolume();
            s << ' ' << GetTotalCount();
            s << ' ' << GetTotalVolume();
            break;
        case HistoryCount:
            s << ' ' << GetLastCount();
            s << ' ' << GetIntervalCount();
            s << ' ' << GetTotalCount();
            break;
        case HistoryCall:
            s << ' ' << GetLastCount();
            s << ' ' << GetLastAvg();
            s << ' ' << GetIntervalCount();
            s << ' ' << GetIntervalAvg();
            s << ' ' << GetTotalCount();
            s << ' ' << GetTotalAvg();
            break;
    }
}

void HistoryCounterData::DumpTable(std::ostream &s) {
    switch (GetType()) {
        case HistoryCount:
            s << '|';
            DumpNumber(s, GetLastCount(), 6);
            s << "|      |";
            DumpNumber(s, GetIntervalCount(), 6);
            s << "|      |";
            DumpNumber(s, GetTotalCount(), 6);
            s << "|      ";
            break;
        case HistoryVolume:
            s << '|';
            DumpNumber(s, GetLastCount(), 6);
            s << '|';
            DumpNumber(s, GetLastVolume(), 6);
            s << '|';
            DumpNumber(s, GetIntervalCount(), 6);
            s << '|';
            DumpNumber(s, GetIntervalVolume(), 6);
            s << '|';
            DumpNumber(s, GetTotalCount(), 6);
            s << '|';
            DumpNumber(s, GetTotalVolume(), 6);
            break;
        case HistoryCall:
            s << '|';
            DumpNumber(s, GetLastCount(), 6);
            s << '|';
            DumpNumber(s, GetLastAvg(), 6);
            s << '|';
            DumpNumber(s, GetIntervalCount(), 6);
            s << '|';
            DumpNumber(s, GetIntervalAvg(), 6);
            s << '|';
            DumpNumber(s, GetTotalCount(), 6);
            s << '|';
            DumpNumber(s, GetTotalAvg(), 6);
            break;
    }
    s << "|" << std::endl;
}

void HistoryCounterData::DumpHtml(std::ostream &s) {
    s << "</td><td align='right'>";
    DumpNumber(s, GetLastCount(), 6);
    s << "</td><td align='right'>";
    switch (GetType()) {
        case HistoryCount:
            s << "&nbsp;";
            break;
        case HistoryVolume:
            DumpNumber(s, GetLastVolume(), 6);
            break;
        case HistoryCall:
            DumpNumber(s, GetLastAvg(), 6);
            break;
    }
    s << "</td><td align='right'>";
    DumpNumber(s, GetIntervalCount(), 6);
    s << "</td><td align='right'>";
    switch (GetType()) {
        case HistoryCount:
            s << "&nbsp;";
            break;
        case HistoryVolume:
            DumpNumber(s, GetIntervalVolume(), 6);
            break;
        case HistoryCall:
            DumpNumber(s, GetIntervalAvg(), 6);
            break;
    }

    s << "</td><td align='right'>";
    DumpNumber(s, GetTotalCount(), 6);
    s << "</td><td align='right'>";
    switch (GetType()) {
        case HistoryCount:
            s << "&nbsp;";
            break;
        case HistoryVolume:
            DumpNumber(s, GetTotalVolume(), 6);
            break;
        case HistoryCall:
            DumpNumber(s, GetTotalAvg(), 6);
            break;
    }
    s << "</td></tr>" << std::endl;
}
