#pragma once

#include <string.h>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <vector>
#include <string>
#include <netinet/in.h>

/* https://stackoverflow.com/questions/12648988/converting-a-defined-constant-number-to-a-string */

#define TO_STRING_(x) #x
#define TO_STRING(x) TO_STRING_(x)

#define BZERO_S(some_struct) memset(&(some_struct), 0, sizeof(some_struct))
#define STRNCPY(destination, source) do { strncpy((destination), (source), sizeof(destination)); destination[sizeof(destination)-1]=0;} while (0)
#define STRNCAT(destination, source) do { strncat((destination), (source), sizeof(destination) - strlen(destination) - 1); destination[sizeof(destination)-1]=0;} while (0)

#define OUT_IPV4_NETWORK(s, addr) s << (int)((addr) & 0xff) << '.' << (int)((addr) >> 8 & 0xff) << '.' << (int)((addr) >> 16 & 0xff) << '.' << (int)((addr) >> 24)

#define CREATE_NETWORK_IP(d1, d2, d3, d4) (((uint32_t)d1)|((uint32_t)d2<<8)|((uint32_t)d3<<16)|((uint32_t)d4<<24))
#define IP_PRINTF_FORMAT "%d.%d.%d.%d"
#define IP_PRINTF_ARG_NETWORK(addr) (int)((addr) & 0xff), (int)((addr) >> 8 & 0xff), (int)((addr) >> 16 & 0xff), (int)((addr) >> 24)

//ISO 8601
#define DATE_TIME_FORMAT1 "%Y-%m-%d %H:%M:%S"
#define DATE_TIME_FORMAT2 "%y/%m/%d %H:%M:%S"

template<typename T>
bool ConvertString(const char *data, T &value) {
    if (data == NULL) {
        return false;
    };
    while (isspace(*data)) {
        data++;
    };
    T ret;
    std::istringstream iss(data);
    if (strncasecmp(data, "0x", 2) == 0) {
        iss.ignore(2);
        iss >> std::hex >> ret;
    } else {
        iss >> std::dec >> ret;
    }
    if (iss.fail() || !iss.eof()) {
        return false;
    }
    value = ret;
    return true;
}

bool ConvertString(const char *data, bool &value);
bool ConvertStringIp(const char *data, in_addr_t &ip_network);
bool ConvertStringTime(const char *data, time_t& time);

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

std::string ReplaceAll(std::string subject, const std::string& search, const std::string& replace);
std::vector<std::string> split(const std::string &s, char delim);
