#include <iomanip>
#include "counters.h"
#include "file_utils.h"
#include "DumpUtils.h"

counters_format_t str2counters_format(const char *s) {
    if (NULL == s || 0 == strcasecmp("simple", s)) {
        return format_simple;
    }
    if (0 == strcasecmp("raw", s)) {
        return format_raw;
    }
    if (0 == strcasecmp("table", s)) {
        return format_table;
    }
    if (0 == strcasecmp("nagios", s)) {
        return format_nagios;
    }
    if (0 == strcasecmp("nagios_total", s)) {
        return format_nagios_total;
    }
    if (0 == strcasecmp("nagios_5m", s)) {
        return format_nagios_5m;
    }
    return format_simple;
}


Counters::index_t Counters::GetCounterIndex(const char *name) {
    {
        SHM_READ_LOCK;
        auto index = GetData()->dict.Lookup(name);
        if (index != DICTIONARY_INVALID_INDEX) {
            return index;
        }
    }
    SHM_WRITE_LOCK;
    auto index = GetData()->dict.Lookup(name);
    if (index != DICTIONARY_INVALID_INDEX) {
        return index;
    }
    return GetData()->dict.Add(name);
}

Counters::index_t Counters::LookupIndex(const char *name) {
    SHM_READ_LOCK;
    return GetData()->dict.Lookup(name);
}


int64_t Counters::GetCounterValue(index_t index) {
    return GetData()->counters[index];
}

counter_t *Counters::GetCounterPtr(Counters::index_t index) {
    return GetData()->counters + index;
}

void Counters::GetCategory(const char *prefix, Counters::index_info_t &index_info) {
    GetData()->dict.GetCategory(prefix, index_info);
}

void Counters::Dump(std::ostream &s, const char *prefix, counters_format_t format, bool hide_prefix) {
    index_info_t index_info;
    GetCategory(prefix, index_info);
    int len = (NULL == prefix || !hide_prefix) ? 0 : strlen(prefix);
    int max_len = -1;
    for (int i = 0; i < index_info.count; i++) {
        int len = strlen(Lookup(index_info.index[i].id));
        if (len > max_len) {
            max_len = len;
        }
    }
    max_len-=len;

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
            s << '|' << std::setw(max_len) << "   Name" << "| Value|" << std::endl;
            break;
    }

    for (int i = 0; i < index_info.count; i++) {
        const char *name = Lookup(index_info.index[i].id) + len;
        switch (format) {
            case format_raw:
                if (0 != strcasecmp(Lookup(index_info.index[i].id), prefix)) {
                    continue;
                }
                s << GetCounterValue(index_info.index[i].id) << std::endl;
                break;
            case format_simple:
                s << name << ' ' << GetCounterValue(index_info.index[i].id) << std::endl;
                break;
            case format_table:
                s << '|' << std::setw(max_len) << name << '|';
                DumpNumber(s, GetCounterValue(index_info.index[i].id), 6);
                s << '|' << std::endl;
                break;
            case format_nagios_5m:
            case format_nagios_total:
            case format_nagios: {
                s << ' ' << name << '=' << GetCounterValue(index_info.index[i].id);
                break;
            }
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
    }
}

//void Counters::DumpTable(std::ostream &s, const char *prefix) {
//    index_info_t index_info;
//    GetCategory(prefix, index_info);
//    int max_len = -1;
//    for (int i = 0; i < index_info.count; i++) {
//        int len = strlen(Lookup(index_info.index[i].id));
//        if (len > max_len) {
//            max_len = len;
//        }
//    }
//    s << '|' << std::setw(max_len) << "   Name" << "| Value|" << std::endl;
//    for (int i = 0; i < index_info.count; i++) {
//        s << '|' << std::setw(max_len) << Lookup(index_info.index[i].id) << '|';
//        DumpNumber(s, GetCounterValue(index_info.index[i].id), 6);
//        s << '|' << std::endl;
//    }
//}

void Counters::DumpHtml(std::ostream &s, const char *prefix) {
    s << ""
            "  <table border='1' cellspacing='0'>\n"
            "    <thead><tr><td>Id</td><td>Value</td></tr></thead>\n"
            "    <tbody>\n";
    Counters::index_info_t index_info;
    GetCategory(prefix, index_info);
    for (int i = 0; i < index_info.count; i++) {
        s << "      <tr><td>" << Lookup(index_info.index[i].id) << "</td><td align='right'>"
          << GetCounterValue(index_info.index[i].id) << "</td></tr>\n";
    }
    s << ""
            "    </tbody>\n"
            "  </table>\n";
}


static Counters countersValue;
static Counters countersIncremented;
const char *counters_suffix = NULL;

Counters &ValueCounters() {
    if (!countersValue.IsOpened()) {
        countersValue.Open(COUNTERS_SHM_NAME_VALUE, counters_suffix);
    }
    return countersValue;
}

Counters &IncrementedCounters(bool open) {
    if (!countersIncremented.IsOpened() && open) {
        countersIncremented.Open(COUNTERS_SHM_NAME_INCREMENTED, counters_suffix);
    }
    return countersIncremented;
}

bool CountersClear() {
    char name[NAME_MAX];

    countersIncremented.Close();
    countersValue.Close();

    STRNCPY(name, SHM_LOCATION
            COUNTERS_SHM_NAME_VALUE);
    if (counters_suffix) {
        STRNCAT(name, counters_suffix);
    }
    if (!remove_test_file(name)) {
        return false;
    }

    STRNCPY(name, SHM_LOCATION
            COUNTERS_SHM_NAME_INCREMENTED);
    if (counters_suffix) {
        STRNCAT(name, counters_suffix);
    }
    if (!remove_test_file(name)) {
        return false;
    }
    return true;
}