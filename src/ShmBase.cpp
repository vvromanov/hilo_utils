#define __STDC_FORMAT_MACROS

#include "ShmBase.h"
#include "log.h"
#include "common_utils.h"
#include "file_utils.h"
#include <sys/mman.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>
#include <inttypes.h>
#include <cerrno>
#include <stdio.h>
#include <climits>

int32_t ShmBase::prev_instance;

ShmBase::ShmBase() {
    clear();
}


bool ShmBase::openShm(const char *name, size_t _size, bool resize) {
    struct stat file_stat;

    if (fd > 0) {
        log_write(LOG_LEVEL_CRIT, "Shm [%s] already opened", name);
        return false;
    }
    clear();
    this->name = name;
    fd = -1;
    if (_size == 0) {
        fd = shm_open(name, O_RDWR, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
        if (fd < 0) {
            log_write(LOG_LEVEL_ERR_ERRNO, "shm_open(%s) failed", name);
//            fprintf(stderr, "shm_open(%s) failed. E%d - %s\n", name, errno, strerror(errno));
            return false;
        }
    }
    if (fd < 0) {
        fd = shm_open(name, (O_CREAT | O_RDWR), (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
        if (fd < 0) {
            log_write(LOG_LEVEL_ERR_ERRNO, "shm_open(O_CREAT, %s) failed", name);
//            fprintf(stderr, "shm_open(O_CREAT, %s) failed. E%d - %s\n", name, errno, strerror(errno));
            return false;
        }
    }
    fchmod(fd, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));

    if (fstat(fd, &file_stat)) {
        log_write(LOG_LEVEL_ERR_ERRNO, "fstat(%s) failed", name);
//        fprintf(stderr, "fstat(%s) failed. E%d - %s\n", name, errno, strerror(errno));
        Close();
        return false;
    }
    if (_size == 0) {
        _size = file_stat.st_size;
    } else {
        if (!resize) {
            size_t realsize = file_stat.st_size;
            if (realsize && _size != realsize) {
                log_write(LOG_LEVEL_ERR, "Invalid shm [%s] size=%zu instead of %zu", name, realsize, _size);
//                fprintf(stderr, "Invalid shm [%s] size=%zu instead of %zu\n", name, realsize, _size);
                Close();
                return false;
            }
        }
        if (ftruncate(fd, _size) < 0) {
            log_write(LOG_LEVEL_ERR_ERRNO, "ftruncate(%s, %zu) failed.", name, _size);
//            fprintf(stderr, "ftruncate(%s, %zu) failed. E%d - %s\n", name, size, errno, strerror(errno));
            Close();
            return false;
        }
    }
    size = _size;
    ++prev_instance;
    instance = prev_instance;
    inode = file_stat.st_ino;
    log_write(LOG_LEVEL_DEBUG, "Open Shm [%s]. Inode=%" PRIu64 ", size=%zu, instance=%d", name, file_stat.st_ino, size,
              instance);

    return true;
}

bool ShmBase::Open(const char *name, size_t _size, bool resize) {
    if (!openShm(name, _size, resize)) {
        return false;
    }
    shm_data_ptr = (uint8_t *) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm_data_ptr == MAP_FAILED) {
        log_write(LOG_LEVEL_ERR_ERRNO, "mmap(%s) failed. size=%zu", name, size);
//        fprintf(stderr, "mmap(%s) failed. size=%zu. E%d - %s\n", name, size, errno, strerror(errno));
        Close();
        return false;
    }
    map_size = size;
#ifndef NO_MLOCK
        if (mlock(shm_data_ptr, size) != 0) {
            log_write(LOG_LEVEL_WARNING_ERRNO, "mlock(%s) failed. size=%zu", name, size);
        }
#endif
    return true;
}

bool ShmBase::OpenMirror(const char *name, size_t _size, size_t header_size, bool resize) {
    if (!openShm(name, _size, resize)) {
        return false;
    }
    errno = 0;
    shm_data_ptr = (uint8_t *) mmap(NULL, size * 2 - header_size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (shm_data_ptr == MAP_FAILED) {
        log_write(LOG_LEVEL_ERR_ERRNO, "mmap(%s) failed. size=%zu", name, size * 2 - header_size);
        Close();
        return false;
    }
    map_size = size * 2 - header_size;

    uint8_t *data1 = (uint8_t *) mmap(shm_data_ptr, size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
    if (data1 == MAP_FAILED) {
        log_write(LOG_LEVEL_ERR_ERRNO, "mmap(%s) failed. size=%zu", name, size);
        Close();
        return false;
    }
    if (data1 != shm_data_ptr) {
        log_write(LOG_LEVEL_ERR, "mmap(%s) return invalid address. size=%zu", name, size);
        Close();
        return false;
    }
    uint8_t *data2 = (uint8_t *) mmap(shm_data_ptr + size, size - header_size, PROT_READ | PROT_WRITE,
                                      MAP_FIXED | MAP_SHARED,
                                      fd, header_size);
    if (data2 == MAP_FAILED) {
        log_write(LOG_LEVEL_ERR_ERRNO, "mmap(%s) failed. size=%zu", name, size);
        Close();
        return false;
    }
    if (data2 != shm_data_ptr + size) {
        log_write(LOG_LEVEL_ERR, "mmap(%s) return invalid address. size=%zu", name, size);
        Close();
        return false;
    }
#ifndef NO_MLOCK
        if (mlock(shm_data_ptr, size) != 0) {
            log_write(LOG_LEVEL_WARNING_ERRNO, "mlock(%s) failed. size=%zu", name, size);
        }
#endif
    return true;
}

void ShmBase::Close() {
    if (shm_data_ptr && shm_data_ptr != MAP_FAILED) {
#ifndef __CYGWIN__
        if (munlock(shm_data_ptr, map_size) != 0) {
            log_write(LOG_LEVEL_ERR_ERRNO, "munlock(%s) failed", name.c_str());
        }
#endif
        munmap(shm_data_ptr, map_size);
        close(fd);
    }
    clear();
}

void ShmBase::clear() {
    shm_data_ptr = NULL;
    size = 0;
    map_size = 0;
    fd = 0;
    instance = 0;
    inode = 0;
    name.clear();
}

bool ShmBase::IsDeleted() const {
    struct stat sb;
    std::string file_name(SHM_LOCATION);
    file_name += name;
    if (0 != stat(file_name.c_str(), &sb)) {
        return true;
    }
    return inode != sb.st_ino;
}

bool ShmBase::is_init_needed(ShmBase::shm_header_t &h) {
    if (h.init_state == ShmBase::intilialized) {
        return false;
    }
    if (__sync_bool_compare_and_swap(&h.init_state, ShmBase::clean, ShmBase::intilializing)) {
        return true;
    }
    while (h.init_state != ShmBase::intilialized) {
        usleep(1);
    }
    return false;
}


void ShmBase::header_init_done(ShmBase::shm_header_t &h) {
    if (!__sync_bool_compare_and_swap(&h.init_state, ShmBase::intilializing, ShmBase::intilialized)) {
        fprintf(stderr, "Init state flag in invalid status [%s]!!", name.c_str());
        exit(EXIT_FAILURE);
    }
}

void ShmBase::header_init(ShmBase::shm_header_t &h) {
    h.signature = SHM_SIGNATURE;
    pthread_mutexattr_t a;
    pthread_mutexattr_init(&a);
    pthread_mutexattr_setpshared(&a, PTHREAD_PROCESS_SHARED);
    //pthread_mutexattr_settype(&a, PTHREAD_MUTEX_ADAPTIVE_NP);
    pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&h.mutex, &a);
    pthread_mutexattr_destroy(&a);
}

std::string ShmBase::GetFileName() const {
    std::string r(SHM_LOCATION);
    r += name;
    return r;
}

bool ShmFileExists(const char *shm_name, const char *suffix) {
    char name[NAME_MAX + 1];
    STRNCPY(name, SHM_LOCATION);
    STRNCAT(name, shm_name);
    if (suffix) {
        STRNCAT(name, suffix);
    }
    return is_file_exists(name);
}

