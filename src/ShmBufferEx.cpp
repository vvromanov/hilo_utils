#include <iomanip>
#include "ShmBufferEx.h"
#include "DumpUtils.h"

bool ShmBufferEx::Open(const char *name, size_t _size) {
    if (_size) {
        _size = ((_size + MEM_PAGE_SIZE - 1) / MEM_PAGE_SIZE) * MEM_PAGE_SIZE + sizeof(ShmBufferExData);
    }
    if (!ShmSimple<ShmBufferExData>::OpenMirror(name, _size, sizeof(ShmBufferExData))) {
        return false;
    }
    char cName[NAME_MAX];
    STRNCPY(cName, "shm.queue.");
    STRNCAT(cName, name);
    char *end = cName + strlen(cName);
    strcpy(end, ".Pop");
    cPopHistory.SetName(cName, HistoryVolume);
    strcpy(end, ".Push");
    cPushHistory.SetName(cName, HistoryVolume);
    strcpy(end, ".error.PushFailed");
    cPushFailedHistory.SetName(cName, HistoryCount);
    strcpy(end, ".error.drops");
    cDropHistory.SetName(cName, HistoryCount);
    strcpy(end, ".message_count");
    cMsgCount.SetName(cName, counter_value);

    return true;
}

void ShmBufferEx::UpdateCounters() {
    cPushLocal.TransferTo(cPushHistory);
    cPushFailedLocal.TransferTo(cPushFailedHistory);
    cPopLocal.TransferTo(cPopHistory);
    cMsgCount = Count();
    SHM_WRITE_LOCK;
    GetData()->TransferDrops(cDropHistory);
}

status_t ShmBufferEx::CheckStatus(std::ostream &s, status_format_t format) {
    status_t status = status_ok;
    int full = (DataSize() * 100 / Capacity());
    if (format == status_nagios || full > 80) {
        s << "Queue " << name << " " << full << "% full" << std::endl;
    }
    if (full > 80) {
        status = status_warning;
    }
    if (full > 90) {
        status = status_critical;
    }
    bool drop_is_ok = GetData()->drop_is_ok;
    if ((cPushFailedHistory.GetIntervalCount() + drop_is_ok ? 0 : cDropHistory.GetIntervalCount()) > 0) {
        if ((cPushFailedHistory.GetLastCount() + drop_is_ok ? 0 : cDropHistory.GetLastCount()) > 0) {
            s << "Queue " << name << " now has errors" << std::endl;
            status = std::max(status, status_critical);
        } else {
            s << "Queue " << name << " has errors in last " << HistoryCounterData::HistorySize << " seconds"
              << std::endl;
            status = std::max(status, status_warning);
        }
    }
    return status;
}

void ShmBufferEx::DumpStatHeader(std::ostream &s) {
    s << '|' << std::setw(20) << "Queue" << "|Msg in|       |    cps      |5 min avg cps|    Total    |";
    s << std::endl;
    s << '|' << std::setw(20) << " Name" << "| queue| Used %|MsgSnt|Errors|MsgSnt|Errors|MsgSnt|Errors|";
    s << std::endl;
}

void ShmBufferEx::DumpStat(std::ostream &s) {
    s << '|' << std::setw(20) << name;
    s << '|';
    DumpNumber(s, Count(), 6);
    s << '|';
    int full = (DataSize() * 10000 / Capacity());
    s << std::setw(3) << full / 100 << '.' << std::setw(2) << std::setfill('0')
      << full % 100 << '%' << std::setfill(' ');

    bool drop_is_ok = GetData()->drop_is_ok;
    s << '|';
    DumpNumber(s, cPushHistory.GetLastCount(), 6);
    s << '|';
    DumpNumber(s, (cPushFailedHistory.GetLastCount() + drop_is_ok ? 0 : cDropHistory.GetLastCount()), 6);

    s << '|';
    DumpNumber(s, cPushHistory.GetIntervalCount() / HistoryCounterData::HistorySize, 6);
    s << '|';
    DumpNumber(s, (cPushFailedHistory.GetIntervalCount() + drop_is_ok ? 0 : cDropHistory.GetIntervalCount()) /
                  HistoryCounterData::HistorySize,
               6);

    s << '|';
    DumpNumber(s, cPushHistory.GetTotalCount(), 6);
    s << '|';
    DumpNumber(s, cPushFailedHistory.GetTotalCount() + drop_is_ok ? 0 : cDropHistory.GetTotalCount(), 6);
    s << '|';
    s << std::endl;
}

bool ShmBufferExData::write(const uint8_t *d, size_t _size) {
    /* prevent buffer from getting completely full or over commited */
    if (free_size() < _size) {
        return false;
    }

    memcpy(circular_data + tail, d, _size);
    tail += _size;
    if (buffer_size < tail) {
        tail -= buffer_size;
        tail_offset += buffer_size;
    }
    return true;
}

void ShmBufferExData::commit(const size_t data_size) {
    tail += data_size;
    if (buffer_size < tail) {
        tail -= buffer_size;
        tail_offset += buffer_size;
    }
}

size_t ShmBufferExData::read(uint8_t *d, const size_t data_size) {
    size_t read_size = std::min(data_size, this->data_size());
    memcpy(d, circular_data + head, read_size);
    free(read_size);
    return read_size;
}

bool ShmBufferExData::Push(const ShmChunks &chunks) {
    shm_record_size_t rec_size = ShmBufferRecord::RecordSize(chunks.Size());
    if (0 == rec_size) {
        return true;
    }
    if (free_size() < rec_size) {
        switch (on_overflow) {
            case return_error:
                return false;
            case drop_new:
                ++drop_msg_count;
                drop_msg_volume += rec_size;
                return true;
            case drop_old:
                if (!dropFirstRecords(rec_size)) {
                    return false;
                }
                break;
        }
    }
    ShmBufferRecord *r = getNewRecord();
    r->SetSize(rec_size);
    r->SetPrevSize(count ? last_record_size : 0);
    last_record_size = rec_size;
    chunks.WriteTo(r->Data());
    commit(rec_size);
    ++count;
    ++total_count;
    return true;
}

bool ShmBufferExData::GetFirst(uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size, bool delete_record) {
    size = 0;
    if (0 == count) {
        return false;
    }
    const ShmBufferRecord *r = getFirstRecord();
    size = r->DataSize();
    if (size > max_size) {
        return false;
    }
    memcpy(data, r->Data(), size);
    if (delete_record) {
        free(r->Size());
        count--;
    }
    return true;
}


bool
ShmBufferExData::Get(vptr_t &pos, direction_t dir, uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size) {
    size = 0;
    if (0 == count) {
        return false;
    }
    if (pos < GetBeginVptr()) {
        return false;
    }
    if (pos > GetLastVptr()) {
        return false;
    }
    const ShmBufferRecord *r = getRecord(pos);
    size = r->Size();
    if (size > max_size) {
        return false;
    }
    memcpy(data, r->Data(), r->Size());
    switch (dir) {
        case direction_none:
            break;
        case direction_next:
            pos = GetNextVptr(pos);
            break;
        case direction_prev:
            pos = GetPrevVptr(pos);
            break;
    }
    return true;
}

bool
ShmBufferExData::Get(vptr_t &pos, vptr_t &lost, uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size) {
    size = 0;
    lost = 0;
    if (0 == count) {
        return false;
    }
    const vptr_t begin = GetBeginVptr();
    if (pos < begin) {
        lost += begin - pos;
        pos = begin;
    }
    if (pos > GetLastVptr()) {
        return false;
    }
    const ShmBufferRecord *r = getRecord(pos);
    size = r->Size() - sizeof(ShmBufferRecord);
    if (size > max_size) {
        lost += size;
        pos = GetNextVptr(pos);
        size = 0;
        return true;
    }
    memcpy(data, r->Data(), size);
    pos = GetNextVptr(pos);
    return true;
}

bool ShmBufferExData::Get(int reader_index, uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size) {
    size = 0;
    if (reader_index < 0 || reader_index >= MaxReaderCount) {
        return false;
    }
    if (0 == count) {
        return false;
    }
    const vptr_t begin = GetBeginVptr();
    const vptr_t end = GetEndVptr();
    const vptr_t last = GetLastVptr();
    (void) end;
    reader_info_t &r = reader[reader_index];
    if (r.pos < begin) {
        r.lost += begin - r.pos;
        r.pos = begin;
    }
    if (r.pos > last) {
        return false;
    }
    const ShmBufferRecord *rec = getRecord(r.pos);
    size = rec->DataSize();
    if (size > max_size) {
        r.lost += size;
        size = 0;
    } else {
        memcpy(data, rec->Data(), size);
    }
    r.pos = GetNextVptr(r.pos);
    return true;
}
