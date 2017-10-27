#pragma once
#include <pthread.h>

#define READ_LOCK AutoMutexLock lock(GetMutex());
#define WRITE_LOCK AutoMutexLock lock(GetMutex());

class AutoMutexLock {
    pthread_mutex_t* mutex;
public:
    inline AutoMutexLock(pthread_mutex_t* _mutex) : mutex(_mutex) {
        pthread_mutex_lock(mutex);
    }

    inline ~AutoMutexLock() {
        unlock();
    }

    inline void unlock() {
        if (mutex) {
            pthread_mutex_unlock(mutex);
            mutex = NULL;
        }
    }
};
