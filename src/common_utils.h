#pragma once

#include <sstream>
#include <cctype>
#include <algorithm>
#include <functional>
#include <vector>
#include <string>
#include <cstring>
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
#define DATE_TIME_FORMAT_UTC "%Y-%m-%dT%H:%M:%SZ"
#define DATE_TIME_FORMAT1 "%Y-%m-%d %H:%M:%S"
#define DATE_TIME_FORMAT2 "%y/%m/%d %H:%M:%S"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

template<typename T>
bool ConvertString_(const char *data, T &value) {
    if (data == NULL) {
        return false;
    };
    while (isspace(*data)) {
        data++;
    };
    std::istringstream iss(data);
    T ret;
    iss >> std::dec >> ret;
    if (iss.fail() || !iss.eof()) {
        return false;
    }
    value = ret;
    return true;
}

bool ConvertString(const char *s, uint64_t &value);
bool ConvertStringHex(const char *data, uint64_t &value);
bool ConvertString(const char *s, int64_t &value);
bool ConvertString(const char *s, int8_t &value);
bool ConvertString(const char *s, uint8_t &value);
bool ConvertString(const char *s, int16_t &value);
bool ConvertString(const char *s, uint16_t &value);
bool ConvertString(const char *s, int32_t &value);
bool ConvertString(const char *s, uint32_t &value);

bool ConvertString(const char *data, bool &value);
bool ConvertStringIp(const char *data, in_addr_t &ip_network);
bool ConvertStringTime(const char *data, time_t& time);

static inline bool ConvertString(const char* s, float& value) {
    return ConvertString_(s, value);
}

static inline bool ConvertString(const char* s, double& value)
{
    return ConvertString_(s, value);
}

// trim from start
static inline std::string ltrim(const std::string &s) {
    return std::string(std::find_if(s.cbegin(), s.cend(), [](unsigned char ch) {return !std::isspace(ch); }), s.cend());
}

// trim from end
static inline std::string rtrim(const std::string &s) {
    return std::string(s.cbegin(), std::find_if(s.crbegin(), s.crend(), [](unsigned char ch) {return !std::isspace(ch); }).base());
}

// trim from both ends
static inline std::string trim(const std::string &s) {
    return ltrim(rtrim(s));
}

std::string ReplaceAll(std::string subject, const std::string& search, const std::string& replace);
std::vector<std::string> split(const std::string &s, char delim);
