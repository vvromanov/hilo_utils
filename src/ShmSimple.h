#pragma once

#include "ShmBase.h"
#include "auto_mutex.h"
#include "log.h"
#include <climits>
#include <sys/stat.h>
#include "common_utils.h"

#define SHM_READ_LOCK AutoMutexLock lock(GetMutex());
#define SHM_WRITE_LOCK AutoMutexLock lock(GetMutex());

template<typename T>
class ShmSimple : public ShmBase {
public:
    bool Open(const char *suffix = NULL) {
        char name[NAME_MAX];
        STRNCPY(name, T::get_name());
        if (suffix) {
            STRNCAT(name, suffix);
        }
        bool res = Open(name, T::get_size());
        is_opened = true;
        return res;
    }

    bool Open(const char *name, ssize_t size) {
        if (size < 0) {
            char fname[NAME_MAX] = SHM_LOCATION;
            STRNCAT(fname, name);
            struct stat stat_buf;
            if (0 != stat(fname, &stat_buf)) {
                return false;
            }
            size = stat_buf.st_size;
        }
        if (!ShmBase::Open(name, size, false)) {
            return false;
        }
        if (is_init_needed(GetData()->header)) {
            log_write(LOG_LEVEL_INFO, "Init shm data [%s]", name);
            header_init(GetData()->header);
            GetData()->Init(size);
            header_init_done(GetData()->header);
        }
        is_opened = true;
        return true;
    }

    bool OpenMirror(const char *name, ssize_t size, size_t header_size) {
        if (size < 0) {
            char fname[NAME_MAX] = SHM_LOCATION;
            STRNCAT(fname, name);
            struct stat stat_buf;
            if (0 != stat(fname, &stat_buf)) {
                return false;
            }
            size = stat_buf.st_size;
        }
        if (!ShmBase::OpenMirror(name, size, header_size, false)) {
            return false;
        }
        if (is_init_needed(GetData()->header)) {
            log_write(LOG_LEVEL_INFO, "Init shm data [%s]", name);
            header_init(GetData()->header);
            GetData()->Init(size);
            header_init_done(GetData()->header);
        }
        is_opened = true;
        return true;
    }

    static std::string GetFileName(const char *suffix = NULL) {
        std::string r(SHM_LOCATION);
        r += T::get_name();
        if (suffix) {
            r += suffix;
        }
        return r;
    }

    bool IsOpened() const {
        return is_opened;
    };

    void Close() {
        if (IsOpened()) {
            ShmBase::Close();
            is_opened = false;
        }
    }

    pthread_mutex_t *GetMutex() { return &(GetData()->header.mutex); }

protected:
    T *GetData() const { return ((T *) (shm_data_ptr)); };
    bool is_opened = false;
};
