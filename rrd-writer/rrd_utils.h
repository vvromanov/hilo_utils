#pragma once

#include <counters_base.h>

bool rrd_check_connect();
bool rrd_connect();
bool rrd_create_file(const char* filename, int count, const char* ds[]);
bool rrd_create_cpu();
bool rrd_update_cpu();
bool rrd_create_memory();
bool rrd_update_memory();
bool rrd_create_load();
bool rrd_update_load();
bool rrd_update_counters(const char* prefix, counter_type_t type);
bool rrd_update_history_counters(const char* prefix);
