#include "HistoryCounters.h"
#include "DumpUtils.h"
#include "file_utils.h"

static HistoryCounters historyCounters;

HistoryCounters &GetHistoryCounters() {
    if (!historyCounters.IsOpened()) {
        historyCounters.Open(COUNTERS_SHM_NAME_HISTORY, counters_suffix);
    }
    return historyCounters;
}

bool HistoryCountersClear() {
    char name[NAME_MAX];
    historyCounters.Close();
    STRNCPY(name, SHM_LOCATION COUNTERS_SHM_NAME_HISTORY);
    if (counters_suffix) {
        STRNCAT(name, counters_suffix);
    }
    if (!remove_test_file(name)) {
        return false;
    }
    return true;
}

HistoryCounterData *HistoryCounters::GetCounterPtr(const char *name) {
    WRITE_LOCK
    index_t index = GetData()->dict.Add(name);
    if (index == DICTIONARY_INVALID_INDEX) {
        return NULL;
    }
    return GetData()->counters + index;
}

HistoryCounters::index_t HistoryCounters::GetCounterIndex(const char *name) {
    {
        READ_LOCK;
        auto index = GetData()->dict.Lookup(name);
        if (index != DICTIONARY_INVALID_INDEX) {
            return index;
        }
    }
    WRITE_LOCK;
    auto index = GetData()->dict.Lookup(name);
    if (index != DICTIONARY_INVALID_INDEX) {
        return index;
    }
    return GetData()->dict.Add(name);
}

void HistoryCounters::Dump(std::ostream &s, const char *prefix, counters_format_t format, bool hide_prefix) {
    index_info_t index_info;
    GetCategory(prefix, index_info);
    int len = ((NULL == prefix) || !hide_prefix) ? 0 : strlen(prefix);
    int max_len = -1;
    for (int i = 0; i < index_info.count; i++) {
        int len = strlen(Lookup(index_info.index[i].id));
        if (len > max_len) {
            max_len = len;
        }
    }
    max_len -= len;

    switch (format) {
        case format_nagios:
        case format_nagios_total:
        case format_nagios_5m:
            s << "OK |";
            break;
        case format_simple:
        case format_raw:
            break;
        case format_table:
            s << '|' << std::setw(max_len) << "Counter" << "| Last second | Last 5 mins |    Total    |" << std::endl;
            s << '|' << std::setw(max_len) << "   Name" << "| Count|Volume| Count|Volume| Count|Volume|" << std::endl;
            break;
        case format_unknown:
            break;
    }
    for (int i = 0; i < index_info.count; i++) {
        const char *name = Lookup(index_info.index[i].id) + len;
        HistoryCounterData *c = GetCounterPtr(index_info.index[i].id);
        switch (format) {
            case format_nagios:
                s << ' ' << name;
                s << '=' << c->GetLastCount();
                switch (c->GetType()) {
                    case HistoryCount:
                    case HistoryUnknown:
                        break;
                    case HistoryVolume:
                        s << ' ' << name;
                        s << "_vol=" << c->GetLastVolume();
                        break;
                    case HistoryCall:
                        s << ' ' << name;
                        s << "_avg=" << c->GetLastAvg();
                        break;
                }
                break;
            case format_nagios_total:
                s << ' ' << name;
                s << '=' << c->GetTotalCount();
                switch (c->GetType()) {
                    case HistoryCount:
                    case HistoryUnknown:
                        break;
                    case HistoryVolume:
                        s << ' ' << name;
                        s << "_vol=" << c->GetTotalVolume();
                        break;
                    case HistoryCall:
                        s << ' ' << name;
                        s << "_avg=" << c->GetTotalAvg();
                        break;
                }
                break;
            case format_nagios_5m:
                s << ' ' << name;
                s << '=' << c->GetIntervalCount();
                switch (c->GetType()) {
                    case HistoryCount:
                    case HistoryUnknown:
                        break;
                    case HistoryVolume:
                        s << ' ' << name;
                        s << "_vol=" << c->GetIntervalVolume();
                        break;
                    case HistoryCall:
                        s << ' ' << name;
                        s << "_avg=" << c->GetIntervalAvg();
                        break;
                }
                break;
            case format_simple:
                s << name;
                c->DumpSimple(s);
                s << std::endl;
                break;
            case format_raw:
                if (0 != strcasecmp(Lookup(index_info.index[i].id), prefix)) {
                    continue;
                }
                c->DumpSimple(s);
                s << std::endl;
                break;
            case format_table:
                s << '|' << std::setw(max_len) << name;
                c->DumpTable(s);
                break;
            case format_unknown:
                break;
        }
    }
    switch (format) {
        case format_nagios:
        case format_nagios_total:
        case format_nagios_5m:
            s << std::endl;
            break;
        case format_simple:
        case format_raw:
        case format_table:
            break;
        case format_unknown:
            break;
    }
}

void HistoryCounters::DumpHtml(std::ostream &s, const char *prefix) {
    index_info_t index_info;
    GetCategory(prefix, index_info);
    s << ""
            "  <table border='1' cellspacing='0'>\n"
            "    <thead>\n"
            "      <tr><td rowspan='2'>Counter name</td><td colspan='2'>Last second</td><td colspan='2'>Last 5 mins</td><td colspan='2'>Total</td></tr>\n"
            "      <tr><td>Count</td><td>Volume</td><td>Count</td><td>Volume</td><td>Count</td><td>Volume</td></tr>\n"
            "    </thead>\n"
            "  <tbody align='right'>\n";
    for (int i = 0; i < index_info.count; i++) {
        s << "<tr><td>" << Lookup(index_info.index[i].id);
        HistoryCounterData *c = GetCounterPtr(index_info.index[i].id);
        c->DumpHtml(s);
    }
    s << ""
            "    </tbody>\n"
            "  </table>\n";
}
