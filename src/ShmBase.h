#pragma once

#include <cstdint>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include "mutex.h"
#include <limits.h>

#define SHM_LOCATION "/dev/shm/"

#define SHM_SIGNATURE 0x31524958454C4554ULL
#define MEM_PAGE_SIZE 4096

class ShmBase {
protected:
    uint8_t *shm_data_ptr = NULL;
    size_t size = 0;
    size_t map_size = 0;
    int fd = -1;
    ino_t inode = 0;
    int32_t instance = 0;
    char name[NAME_MAX] = {0};
    static int32_t prev_instance;
public:
    static bool ignore_mlock_error;
    typedef enum {
        clean,
        intilializing,
        intilialized,
    } init_state_t;

    typedef struct {
        volatile int64_t signature;
        volatile int32_t init_state;
        simple_mutex_t mutex;
    } __attribute__((__packed__)) shm_header_t;

    ShmBase();

    ~ShmBase() {
        Close();
    }

    bool Open(const char *name, size_t size, bool resize = false);
    bool OpenMirror(const char *name, size_t size, size_t header_size, bool resize = false);

    void Close();

    size_t GetSize() const { return size; };

    uint8_t *GetData() const { return shm_data_ptr; };
    bool IsDeleted() const;

    bool IsOpened() const { return shm_data_ptr != NULL; };
    std::string GetFileName() const;

    ino_t GetINode() const {
        return inode;
    }

    int32_t GetInstance() const {
        return instance;
    }

    const char *GetName() const {
        return name;
    }

protected:
    void clear();
    static bool is_init_needed(shm_header_t &h);
    static void header_init(shm_header_t &h);
    void header_init_done(shm_header_t &h);
    bool openShm(const char *name, size_t size, bool resize);
};

bool ShmFileExists(const char *shm_name, const char *suffix = NULL);
