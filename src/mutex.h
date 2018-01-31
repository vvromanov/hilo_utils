#pragma once

#include <pthread.h>
#include <cstdint>
#include "common_utils.h"

#define SRC_LOCATION __FILE__ ":" TO_STRING(__LINE__)

typedef struct {
    pthread_mutex_t mutex;
    uint32_t lock_count;
    pid_t pid;
    char lock_location[64];
} __attribute__((__packed__)) simple_mutex_t;

void simple_mutex_init(simple_mutex_t& m);
void simple_mutex_lock(simple_mutex_t& m, const char *location, int location_len);
#define SIMPLE_LOCK(m) simple_mutex_lock(m, SRC_LOCATION, sizeof(SRC_LOCATION)))

static inline void simple_mutex_unlock(simple_mutex_t& m) {
    m.lock_location[0] = 0;
    pthread_mutex_unlock(&m.mutex);
}
