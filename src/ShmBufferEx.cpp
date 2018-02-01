#include <iomanip>
#include "ShmBufferEx.h"
#include "DumpUtils.h"
#include "file_utils.h"

constexpr static const int prefix_len = strlen(BUFFER_EX_COUNTER_PREFIX);
constexpr static const int suffix_len = strlen(BUFFER_EX_COUNTER_SUFFIX);


bool ShmBufferEx::Open(const char *name, size_t _size) {
    if (_size) {
        _size = ((_size + MEM_PAGE_SIZE - 1) / MEM_PAGE_SIZE) * MEM_PAGE_SIZE + sizeof(ShmBufferExData);
    }
    if (!ShmSimple<ShmBufferExData>::OpenMirror(name, _size, sizeof(ShmBufferExData))) {
        return false;
    }
    char cName[NAME_MAX];
    snprintf(cName, sizeof(cName), BUFFER_EX_CNAME_MESSAGE, name);
    cMsgHistory.SetName(cName, HistoryVolume);
    snprintf(cName, sizeof(cName), BUFFER_EX_CNAME_DROP, name);
    cDropHistory.SetName(cName, HistoryVolume);
    snprintf(cName, sizeof(cName), BUFFER_EX_CNAME_ERROR, name);
    cErrorHistory.SetName(cName, HistoryVolume);
    snprintf(cName, sizeof(cName), BUFFER_EX_CNAME_COUNT, name);
    cMsgCount.SetName(cName, counter_value);
    snprintf(cName, sizeof(cName), BUFFER_EX_CNAME_USED_BP, name);
    cUsedBp.SetName(cName, counter_value);
    snprintf(cName, sizeof(cName), BUFFER_EX_CNAME_USED, name);
    cUsed.SetName(cName, counter_value);
    snprintf(cName, sizeof(cName), BUFFER_EX_CNAME_CAPACITY, name);
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


void ShmBufferEx::DumpStatTable(std::ostream &s, bool asHtml) {
    Counters::index_info_t index_info;

    ValueCounters().GetCategory(BUFFER_EX_COUNTER_PREFIX, index_info);
    if (index_info.count == 0) {
        return;
    }
    DumpStatHeader(s, asHtml);
    for (int i = 0; i < index_info.count; i++) {
        const char *c_name = ValueCounters().Lookup(index_info.index[i].id);
        const char *p = strstr(c_name, BUFFER_EX_COUNTER_SUFFIX);
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
    ValueCounters().GetCategory(BUFFER_EX_COUNTER_PREFIX, index_info);
    if (index_info.count == 0) {
        return status;
    }
    for (int i = 0; i < index_info.count; i++) {
        const char *c_name = ValueCounters().Lookup(index_info.index[i].id);
        const char *p = strstr(c_name, BUFFER_EX_COUNTER_SUFFIX);
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
    STRNCPY(stat.name, name);
    stat.count = Count();
    stat.data_size = DataSize();
    stat.capacity = Capacity();
    cMsgHistory.GetInfo(stat.message_history);
    cDropHistory.GetInfo(stat.drop_history);
    cErrorHistory.GetInfo(stat.error_history);
}

void ShmBufferEx::GetStat(const char *name, queue_stat_t &stat) {
    char cName[NAME_MAX];
    snprintf(cName, sizeof(cName), BUFFER_EX_CNAME_MESSAGE, name);
    GetHistoryCounters().GetCounterInfo(cName, stat.message_history);
    snprintf(cName, sizeof(cName), BUFFER_EX_CNAME_DROP, name);
    GetHistoryCounters().GetCounterInfo(cName, stat.drop_history);
    snprintf(cName, sizeof(cName), BUFFER_EX_CNAME_ERROR, name);
    GetHistoryCounters().GetCounterInfo(cName, stat.error_history);
    snprintf(cName, sizeof(cName), BUFFER_EX_CNAME_COUNT, name);
    stat.count = ValueCounters().GetCounterValue(cName);
    snprintf(cName, sizeof(cName), BUFFER_EX_CNAME_USED, name);
    stat.data_size = ValueCounters().GetCounterValue(cName);
    snprintf(cName, sizeof(cName), BUFFER_EX_CNAME_CAPACITY, name);
    stat.capacity = ValueCounters().GetCounterValue(cName);
}
