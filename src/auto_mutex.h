#pragma once

#include <pthread.h>
#include <cstdio>
#include <cstring>
#include <cerrno>

#define READ_LOCK AutoMutexLock lock(GetMutex(), __PRETTY_FUNCTION__);
#define WRITE_LOCK AutoMutexLock lock(GetMutex(), __PRETTY_FUNCTION__);

class AutoMutexLock {
    pthread_mutex_t *mutex;
    const char* name;
public:
    inline AutoMutexLock(pthread_mutex_t *_mutex, const char* _name) : mutex(_mutex), name(_name) {
        int res = pthread_mutex_lock(mutex);
        if (0 != res) {
            fprintf(stderr, "Can't lock mutex %s E%d %s", name, res, strerror(res));
        }
    }

    inline ~AutoMutexLock() {
        unlock();
    }

    inline void unlock() {
        if (mutex) {
            int res=pthread_mutex_unlock(mutex);
            if (0 != res) {
                fprintf(stderr, "Can't unlock mutex %s E%d %s", name, res, strerror(res));
            }
            mutex = NULL;
        }
    }
};
