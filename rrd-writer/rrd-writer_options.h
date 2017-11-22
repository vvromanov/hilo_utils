#pragma once

#include <climits>
#include <counters_base.h>

extern bool opt_verbose;
#define MAX_CUSTOM_COUNT 100

typedef enum {
    prefix_incremented,
    prefix_value,
    prefix_history
} prefix_type_t;

typedef struct {
    prefix_type_t type;
    char name[NAME_MAX];
} custom_prefix_t;

extern custom_prefix_t custom_prefix[MAX_CUSTOM_COUNT];
extern int custom_prefix_count;

#define OPT_VALUE_SUFFIX            4
#define OPT_PREFIX_INCREMENTED      5
#define OPT_PREFIX_VALUE            6
#define OPT_PREFIX_HISTORY          7

void rrd_writer_options_parse(int argc, char **argv);
