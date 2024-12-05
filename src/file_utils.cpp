#include <errno.h>
#include <sys/stat.h>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <climits>
#include "LogBase.h"
#include "file_utils.h"
#include "common_utils.h"

bool is_file_exists(const char *name) {
    struct stat structstat;
    if (stat(name, &structstat) == -1) {
        if (errno != ENOENT) {
            log_write(LOG_LEVEL_ERR_ERRNO, "stat call failed for file %s", name);
        }
        return false;
    }
    return S_ISREG(structstat.st_mode);
}

bool is_dir_exists(const char *name) {
    struct stat structstat;
    if (stat(name, &structstat) == -1) {
        if (errno != ENOENT) {
            log_write(LOG_LEVEL_ERR_ERRNO, "stat call failed for directory %s", name);
        }
        return false;
    }
    return S_ISDIR(structstat.st_mode);
}

bool get_file_size(const char *name, size_t &size) {
    struct stat structstat;
    if (stat(name, &structstat) == -1) {
        if (errno != ENOENT) {
            log_write(LOG_LEVEL_ERR_ERRNO, "stat call failed for file %s", name);
        }
        size = 0;
        return false;
    }
    size = structstat.st_size;
    return true;
}


bool remove_test_file(const char *path) {
    struct stat structstat;
    if (stat(path, &structstat) == -1) {
        if (errno == ENOENT) {
            return true;
        }
        return false;
    }
    if (!S_ISREG(structstat.st_mode)) {
        return false;
    }
    if (0 == remove(path)) {
        return true;
    }
    log_write(LOG_LEVEL_ERR_ERRNO, "Can't remove '%s'", path);
    return false;
}

bool mkdir_for_file(const char *filename, __mode_t mode) {
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;
    STRNCPY(tmp, filename);
    len = strlen(tmp);
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (0 != mkdir(tmp, mode)) {
                if (errno != EEXIST || !is_dir_exists(tmp)) {
                    log_write(LOG_LEVEL_ERR_ERRNO, "mkdir(%s) call failed", tmp);
                    return false;
                }
            }
            *p = '/';
        }
    }
    return true;
}

static const char* get_filename(const char* path) {
    if (path == nullptr) {
        return nullptr;
    }
    const char* p = strrchr(path, '/');
    if (p == nullptr) {
        p = strrchr(path, '\\');
    }
    if (p) {
        return p + 1;
    } else {
        return path;
    }
}

const char *get_ext(const char *path) {
    const char* filename = get_filename(path);
    if (filename != nullptr) {
        const char* p = strrchr(filename, '.');
        if (p) {
            return p + 1;
        }
        else {
            return "";
        }
    } else {
        return nullptr;
    }
}