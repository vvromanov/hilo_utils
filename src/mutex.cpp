#include "mutex.h"
#include <unistd.h>

void simple_mutex_init(simple_mutex_t &m) {
    pthread_mutexattr_t a;
    pthread_mutexattr_init(&a);
    pthread_mutexattr_setpshared(&a, PTHREAD_PROCESS_SHARED);
    //pthread_mutexattr_settype(&a, PTHREAD_MUTEX_ADAPTIVE_NP);
    pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m.mutex, &a);
    pthread_mutexattr_destroy(&a);
}

void simple_mutex_lock(simple_mutex_t &m, const char *location, int location_len) {
    int pos = location_len - 3;
    while (pos > 0 && location[pos] != '/') {
        --pos;
    }
    if (pos) {
        ++pos;
    }
    pthread_mutex_lock(&m.mutex);
    m.pid = getpid();
    ++m.lock_count;
    STRNCPY(m.lock_location, location + pos);
}
