#pragma once

#include <cstdint>
#include <ctime>

int64_t getTimeMs(void);
int64_t getTimeUs(void);
int64_t getClockMs(void);
int64_t getClockUs(void);

const char *get_time_str(time_t t, bool include_date);
