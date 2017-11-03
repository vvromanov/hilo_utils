#include <arpa/inet.h>
#include "common_utils.h"

bool ConvertString(const char *data, bool &value) {
    bool ok = false;
    if (data && data[0] != 0) {
        if (strcasecmp(data, "y") == 0 || strcasecmp(data, "1") == 0 || strcasecmp(data, "true") == 0) {
            value = true;
            ok = true;
        }
        if (strcasecmp(data, "n") == 0 || strcasecmp(data, "0") == 0 || strcasecmp(data, "false") == 0) {
            value = false;
            ok = true;
        }
    }
    return ok;
}

bool ConvertStringIp(const char *data, in_addr_t &ip_network) {
    struct in_addr tmp;
    bool res = (1 == inet_pton(AF_INET, data, &tmp));
    ip_network = tmp.s_addr;
    return res;
}

bool ConvertStringTime(const char *data, time_t &time) {
    struct tm tm;
    char *p;
    if (strlen(data) > 4 && data[4] == '-') {
        p = strptime(data, DATE_TIME_FORMAT1, &tm);
    } else {
        p = strptime(data, DATE_TIME_FORMAT2, &tm);
    }
    if (NULL==p || *p) {
        return ConvertString(data, time);
    }
    time = mktime(&tm);  // t is now your desired time_t
    return true;
}

std::string ReplaceAll(std::string subject, const std::string &search, const std::string &replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (item.empty()) {
            continue;
        }
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}
