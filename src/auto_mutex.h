#pragma once

#include <pthread.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <mutex.h>

#define READ_LOCK_(m) AutoMutexLock lock((m), SRC_LOCATION, sizeof(SRC_LOCATION));
#define WRITE_LOCK_(m) AutoMutexLock lock((m), SRC_LOCATION, sizeof(SRC_LOCATION));

#define READ_LOCK READ_LOCK_(GetMutex())
#define WRITE_LOCK WRITE_LOCK_(GetMutex())

class AutoMutexLock {
    simple_mutex_t* mutex;
public:
    inline AutoMutexLock(simple_mutex_t& _mutex, const char* location, int location_size) : mutex(&_mutex) {
        simple_mutex_lock(_mutex, location, location_size);
    }

    inline ~AutoMutexLock() {
        unlock();
    }

    inline void unlock() {
        if (mutex) {
            simple_mutex_unlock(*mutex);
            mutex = NULL;
        }
    }
};
