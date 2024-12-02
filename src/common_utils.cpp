#include <cstring>
#include "common_utils.h"
#include <arpa/inet.h>
#include <climits>
#include <cstdint>

#ifdef __CYGWIN__
extern "C" {
    char *strptime(const char *__restrict, const char *__restrict, struct tm *__restrict);
}
#endif
bool ConvertString(const char *data, bool &value) {
    bool ok = false;
    if (data && data[0] != 0) {
        if (strcasecmp(data, "y") == 0 || strcasecmp(data, "1") == 0 || strcasecmp(data, "true") == 0 || strcasecmp(data, "on") == 0 || strcasecmp(data, "yes") == 0) {
            value = true;
            ok = true;
        }
        if (strcasecmp(data, "n") == 0 || strcasecmp(data, "0") == 0 || strcasecmp(data, "false") == 0|| strcasecmp(data, "off") == 0 || strcasecmp(data, "no") == 0) {
            value = false;
            ok = true;
        }
    }
    return ok;
}

bool ConvertStringHex(const char *s, uint64_t &value) {
    if (NULL == s) {
        return false;
    }

    uint64_t n = 0;

    constexpr const uint64_t nmax = UINT64_MAX / 16;
    constexpr const uint8_t cmax = UINT64_MAX - UINT64_MAX / 16 * 16;

    for (;;) {
        uint8_t c;
        switch (*s) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                c = (*s - '0');
                break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                c = (*s - 'a' + 10);
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                c = (*s - 'A' + 10);
                break;
            default:
                return false;
        }
        if (n < nmax || (n == nmax && c <= cmax)) {
            n = n * 16 + c;
        } else {
            return false;
        }
        ++s;
        // \0$
        if (*s == '\0') {
            break;
        }
    }
    value = n;
    return true;
}

bool ConvertString(const char *s, uint64_t &value) {
    if (NULL == s) {
        return false;
    }
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        return ConvertStringHex(s + 2, value);
    }

    unsigned long n = 0;

    constexpr const uint64_t nmax = UINT64_MAX / 10;
    constexpr const uint8_t cmax = UINT64_MAX - UINT64_MAX / 10 * 10;

    // [0-9]
    if (*s < '0' || *s > '9') {
        return false;
    }
    for (;;) {
        unsigned c = *s - '0';
        // resulting integer is representable as a long
        if (n < nmax || (n == nmax && c <= cmax)) {
            n = n * 10 + c;
        } else {
            return false;
        }
        ++s;
        // \0$
        if (*s == '\0') {
            break;
        }
        // [0-9]+
        if (*s < '0' || *s > '9') {
            return false;
        }
    }
    value = n;
    return true;
}

bool ConvertString(const char *s, int64_t &value) {
    if (NULL == s) {
        return false;
    }
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        union {
            int64_t i;
            uint64_t u;
        } d;
        if (ConvertStringHex(s + 2, d.u)) {
            value = d.i;
            return true;
        }
        return false;
    }
    int plus;

    // LONG_(MAX|MIN) w/o LSD
    unsigned long nmax;

    // LSD of LONG_(MAX|MIN); used for possible overflow detection
    unsigned cmax;
    unsigned long n = 0;

    // ^-?
    if (*s == '-') {
        ++s;
        plus = 0;
        nmax = -(LONG_MIN / 10);
        cmax = -(LONG_MIN + LONG_MIN / -10 * 10);
    } else {
        plus = 1;
        nmax = LONG_MAX / 10;
        cmax = LONG_MAX - LONG_MAX / 10 * 10;
    }
    // [0-9]
    if (*s < '0' || *s > '9') {
        return false;
    }
    for (;;) {
        unsigned c = *s - '0';
        // resulting integer is representable as a long
        if (n < nmax || (n == nmax && c <= cmax)) {
            n = n * 10 + c;
        } else {
            return false;
        }
        ++s;
        // \0$
        if (*s == '\0') {
            break;
        }
        // [0-9]+
        if (*s < '0' || *s > '9') {
            return false;
        }
    }
    value = plus ? (long) n : n * -1L;
    return true;
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
    if (NULL == p || *p) {
        return ConvertString(data, time);
    }
    time = mktime(&tm);  // t is now your desired time_t
    if (time == -1) {
        fprintf(stderr, "Date=%s tm_year=%d", data, tm.tm_year);
    }
    return true;
}

bool ConvertString(const char *s, int8_t &value) {
    int64_t v;
    if (!ConvertString(s, v)) {
        return false;
    }
    if (v > INT8_MAX || v<INT8_MIN) {
        return false;
    }
    value = v;
    return true;
}

bool ConvertString(const char *s, uint8_t &value) {
    uint64_t v;
    if (!ConvertString(s, v)) {
        return false;
    }
    if (v > UINT8_MAX) {
        return false;
    }
    value = v;
    return true;
}

bool ConvertString(const char *s, int16_t &value) {
    int64_t v;
    if (!ConvertString(s, v)) {
        return false;
    }
    if (v > INT16_MAX || v<INT16_MIN) {
        return false;
    }
    value = v;
    return true;
}

bool ConvertString(const char *s, uint16_t &value) {
    uint64_t v;
    if (!ConvertString(s, v)) {
        return false;
    }
    if (v > UINT16_MAX) {
        return false;
    }
    value = v;
    return true;
}

bool ConvertString(const char *s, int32_t &value) {
    int64_t v;
    if (!ConvertString(s, v)) {
        return false;
    }
    if (v > INT32_MAX || v<INT32_MIN) {
        return false;
    }
    value = v;
    return true;
}

bool ConvertString(const char *s, uint32_t &value) {
    uint64_t v;
    if (!ConvertString(s, v)) {
        return false;
    }
    if (v > UINT32_MAX) {
        return false;
    }
    value = v;
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
