#include <iomanip>
#include "ShmBufferEx.h"
#include "DumpUtils.h"
#include "CDictionary.h"
#include "HistoryCounterData.h"
#include "file_utils.h"

#define CNAME_MESSAGE  "shm.queue.%s.message"
#define CNAME_ERROR    "shm.queue.%s.error"
#define CNAME_DROP     "shm.queue.%s.drop"
#define CNAME_COUNT    "shm.queue.%s.count"
#define CNAME_USED_BP     "shm.queue.%s.used_bp"
#define CNAME_USED     "shm.queue.%s.used"
#define CNAME_CAPACITY     "shm.queue.%s.capacity"

bool ShmBufferEx::Open(const char *name, size_t _size) {
    if (_size) {
        _size = ((_size + MEM_PAGE_SIZE - 1) / MEM_PAGE_SIZE) * MEM_PAGE_SIZE + sizeof(ShmBufferExData);
    }
    if (!ShmSimple<ShmBufferExData>::OpenMirror(name, _size, sizeof(ShmBufferExData))) {
        return false;
    }
    char cName[NAME_MAX];
    snprintf(cName, sizeof(cName), CNAME_MESSAGE, name);
    cMsgHistory.SetName(cName, HistoryVolume);
    snprintf(cName, sizeof(cName), CNAME_DROP, name);
    cDropHistory.SetName(cName, HistoryVolume);
    snprintf(cName, sizeof(cName), CNAME_ERROR, name);
    cErrorHistory.SetName(cName, HistoryVolume);
    snprintf(cName, sizeof(cName), CNAME_COUNT, name);
    cMsgCount.SetName(cName, counter_value);
    snprintf(cName, sizeof(cName), CNAME_USED_BP, name);
    cUsedBp.SetName(cName, counter_value);
    snprintf(cName, sizeof(cName), CNAME_USED, name);
    cUsed.SetName(cName, counter_value);
    snprintf(cName, sizeof(cName), CNAME_CAPACITY, name);
    cCapacity.SetName(cName, counter_value);
    return true;
}

void ShmBufferEx::UpdateCounters() {
    cMsgCount = Count();
    cUsedBp = GetUsedBp();
    cUsed = DataSize();
    cCapacity = Capacity();
    WRITE_LOCK;
    GetData()->TransferDrops(cDropHistory, cErrorHistory);
    GetData()->TransferMsg(cMsgHistory);
}


status_t ShmBufferEx::CheckStatus(std::ostream &s, status_format_t format) {
    queue_stat_t stat;
    GetStat(stat);
    return CheckStatus(stat, s, format);
}

status_t ShmBufferEx::CheckStatus(const queue_stat_t &stat, std::ostream &s, status_format_t format) {
    status_t status = status_ok;
    int full = stat.data_size * 100 / stat.capacity;
    if (format == status_nagios || full > 80) {
        s << "Queue " << stat.name << " " << full << "% full" << std::endl;
    }
    if (full > 80) {
        status = status_warning;
    }
    if (full > 90) {
        status = status_critical;
    }
    if (stat.error_history.interval_count > 0) {
        if (stat.error_history.last_count > 0) {
            s << "Queue " << stat.name << " now has errors" << std::endl;
            status = std::max(status, status_critical);
        } else {
            s << "Queue " << stat.name << " has errors in last " << HistoryCounterData::HistorySize << " seconds"
              << std::endl;
            status = std::max(status, status_warning);
        }
    }
    return status;
}

void ShmBufferEx::DumpStatHeader(std::ostream &s, bool asHtml) {
    if (asHtml) {
        const char *szIndent = "    ";
        s << szIndent << "<table border='1' cellspacing='0'>\n";
        s << szIndent << "  <thead>\n";
        s << szIndent
          << "    <tr><td rowspan='2'>Name</td><td rowspan='2'>Messages<br/>in queue</td><td rowspan='2'>Queue<br/>size</td><td rowspan='2'>Space<br/>used, %</td><td colspan='3'>Last second</td><td colspan='3'>Last 5 minutes</td><td colspan='3'>Total</td></tr>";
        s << szIndent << "    <tr>\n";
        s << szIndent << "      <td>MsgSend</td><td>Volume</td><td>Errors</td>\n";
        s << szIndent << "      <td>MsgSend</td><td>Volume</td><td>Errors</td>\n";
        s << szIndent << "      <td>MsgSend</td><td>Volume</td><td>Errors</td>\n";
        s << szIndent << "    </tr>\n";
        s << szIndent << "  </thead>\n";
        s << szIndent << "  <tbody align='right'>\n";
    } else {
        s << '|' << std::setw(20) << "Queue"
          << "|Msg in|Queue | Space |    Last second     |  Last 5 minutes    |       Total        |";
        s << std::endl;
        s << '|' << std::setw(20) << " Name"
          << "| queue| size | Used %|MsgSnt|Volume|Errors|MsgSnt|Volume|Errors|MsgSnt|Volume|Errors|";
        s << std::endl;
    }
}


#define COUNTER_PREFIX "shm.queue."
#define COUNTER_SUFFIX ".capacity"

constexpr static const int prefix_len = strlen(COUNTER_PREFIX);
constexpr static const int suffix_len = strlen(COUNTER_SUFFIX);

void ShmBufferEx::DumpStatTable(std::ostream &s, bool asHtml) {
    Counters::index_info_t index_info;

    ValueCounters().GetCategory(COUNTER_PREFIX, index_info);
    if (index_info.count == 0) {
        return;
    }
    DumpStatHeader(s, asHtml);
    for (int i = 0; i < index_info.count; i++) {
        const char *c_name = ValueCounters().Lookup(index_info.index[i].id);
        const char *p = strstr(c_name, COUNTER_SUFFIX);
        if (NULL == p) {
            continue;
        }
        c_name += prefix_len;
        int name_len = strlen(c_name);
        queue_stat_t stat;
        STRNCPY(stat.name, c_name);
        stat.name[name_len - suffix_len] = 0;
        char filename[NAME_MAX] = SHM_LOCATION;
        STRNCAT(filename, stat.name);
        if (!is_file_exists(filename)) {
            continue;
        }
        GetStat(stat.name, stat);
        DumpStat(stat, s, asHtml);
    }
    if (asHtml) {
        s << "    </tbody></table>" << std::endl;
    }
}

status_t ShmBufferEx::CheckAllStatus(std::ostream &s, status_format_t format) {
    status_t status = status_ok;
    Counters::index_info_t index_info;
    ValueCounters().GetCategory(COUNTER_PREFIX, index_info);
    if (index_info.count == 0) {
        return status;
    }
    for (int i = 0; i < index_info.count; i++) {
        const char *c_name = ValueCounters().Lookup(index_info.index[i].id);
        const char *p = strstr(c_name, COUNTER_SUFFIX);
        if (NULL == p) {
            continue;
        }
        c_name += prefix_len;
        int name_len = strlen(c_name);
        queue_stat_t stat;
        STRNCPY(stat.name, c_name);
        stat.name[name_len - suffix_len] = 0;
        char filename[NAME_MAX] = SHM_LOCATION;
        STRNCAT(filename, stat.name);
        if (!is_file_exists(filename)) {
            continue;
        }
        GetStat(stat.name, stat);
        status = std::max(status, CheckStatus(stat, s, format));
    }
    return status;
}


void ShmBufferEx::DumpStat(const queue_stat_t &stat, std::ostream &s, bool asHtml) {
    if (asHtml) {
        s << "    <tr><td>";
    } else {
        s << '|' << std::setw(20);
    }
    s << stat.name;
    if (asHtml) {
        s << "</td>" << std::endl << "      <td>";
    } else {
        s << '|';
    }
    DumpNumber(s, stat.count, 6);
    if (asHtml) {
        s << "</td>" << std::endl << "      <td>";
    } else {
        s << '|';
    }
    DumpNumber(s, stat.capacity, 6);
    if (asHtml) {
        s << "</td>" << std::endl << "      <td>";
    } else {
        s << '|';
    }

    int full = (stat.data_size * 10000 / stat.capacity);
    s << std::setw(3) << full / 100 << '.' << std::setw(2) << std::setfill('0')
      << full % 100 << '%' << std::setfill(' ');

    if (asHtml) {
        s << "</td>" << std::endl << "      <td>";
    } else {
        s << '|';
    }
    DumpNumber(s, stat.message_history.last_count, 6);
    if (asHtml) {
        s << "</td>" << std::endl << "      <td>";
    } else {
        s << '|';
    }
    DumpNumber(s, stat.message_history.last_summ, 6);
    if (asHtml) {
        s << "</td>" << std::endl << "      <td>";
    } else {
        s << '|';
    }
    DumpNumber(s, stat.error_history.last_count, 6);

    if (asHtml) {
        s << "</td>" << std::endl << "      <td>";
    } else {
        s << '|';
    }
    DumpNumber(s, stat.message_history.interval_count, 6);
    if (asHtml) {
        s << "</td>" << std::endl << "      <td>";
    } else {
        s << '|';
    }
    DumpNumber(s, stat.message_history.interval_summ, 6);
    if (asHtml) {
        s << "</td>" << std::endl << "      <td>";
    } else {
        s << '|';
    }
    DumpNumber(s, stat.error_history.interval_count, 6);
    if (asHtml) {
        s << "</td>" << std::endl << "      <td>";
    } else {
        s << '|';
    }
    DumpNumber(s, stat.message_history.total_count, 6);
    if (asHtml) {
        s << "</td>" << std::endl << "      <td>";
    } else {
        s << '|';
    }
    DumpNumber(s, stat.message_history.total_summ, 6);
    if (asHtml) {
        s << "</td>" << std::endl << "      <td>";
    } else {
        s << '|';
    }
    DumpNumber(s, stat.error_history.total_count, 6);
    if (asHtml) {
        s << "</td>" << std::endl << "    </tr>";
    } else {
        s << '|';
    }
    s << std::endl;
}

void ShmBufferEx::GetStat(queue_stat_t &stat) {
    UpdateCounters();
    STRNCPY(stat.name, name.c_str());
    stat.count = Count();
    stat.data_size = DataSize();
    stat.capacity = Capacity();
    cMsgHistory.GetInfo(stat.message_history);
    cDropHistory.GetInfo(stat.drop_history);
    cErrorHistory.GetInfo(stat.error_history);
}

void ShmBufferEx::GetStat(const char *name, queue_stat_t &stat) {
    char cName[NAME_MAX];
    snprintf(cName, sizeof(cName), CNAME_MESSAGE, name);
    GetHistoryCounters().GetCounterInfo(cName, stat.message_history);
    snprintf(cName, sizeof(cName), CNAME_DROP, name);
    GetHistoryCounters().GetCounterInfo(cName, stat.drop_history);
    snprintf(cName, sizeof(cName), CNAME_ERROR, name);
    GetHistoryCounters().GetCounterInfo(cName, stat.error_history);
    snprintf(cName, sizeof(cName), CNAME_COUNT, name);
    stat.count = ValueCounters().GetCounterValue(cName);
    snprintf(cName, sizeof(cName), CNAME_USED, name);
    stat.data_size = ValueCounters().GetCounterValue(cName);
    snprintf(cName, sizeof(cName), CNAME_CAPACITY, name);
    stat.capacity = ValueCounters().GetCounterValue(cName);
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
    free_head(read_size);
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
                ++tmp_drop_count;
                ++total_drop_count;
                if (!drop_is_ok) {
                    ++total_error_count;
                }
                tmp_drop_volume += rec_size;
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
    ++tmp_count;
    tmp_volume += rec_size;
    return true;
}

bool ShmBufferExData::Pop(uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size, bool delete_record) {
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
        free_head(r->Size());
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

