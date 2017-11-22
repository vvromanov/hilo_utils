#pragma once

#include <Status.h>

void UptimeUpdate();
void UptimeReset();
void UptimeDumpTable(std::ostream &os, const char* processes[], int count);
status_t UptimeCheckStatus(std::ostream &s, status_format_t format, const char *processes[], int count);
