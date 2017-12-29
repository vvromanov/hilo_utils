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
    return true;
}

bool is_dir_exist(const char *name) {
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
        return false;
    }
    size = structstat.st_size;
    return true;
}


bool remove_test_file(const char *path) {
    if (!is_file_exists(path)) {
        return true;
    }
    if (0 == remove(path)) {
        return true;
    }
    log_write(LOG_LEVEL_ERR_ERRNO, "Can't remove '%s'", path);
    return false;
}

bool touch(const std::string &pathname) {
    int fd = open(pathname.c_str(),
                  O_WRONLY | O_CREAT | O_NOCTTY | O_NONBLOCK,
                  0666);
    if (fd < 0) // Couldn't open that path.
    {
        log_write(LOG_LEVEL_ERR_ERRNO, "%s: Couldn't open() path %s", __PRETTY_FUNCTION__, pathname.c_str());
        return false;
    }
    close(fd);
    int rc = utimensat(AT_FDCWD,
                       pathname.c_str(),
                       nullptr,
                       0);
    if (rc) {
        log_write(LOG_LEVEL_ERR_ERRNO, "%s: Couldn't utimensat() path %s", __PRETTY_FUNCTION__, pathname.c_str());
        return false;
    }
    return true;
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
                if (errno != EEXIST || !is_dir_exist(tmp)) {
                    log_write(LOG_LEVEL_ERR_ERRNO, "mkdir(%s) call failed", tmp);
                    return false;
                }
            }
            *p = '/';
        }
    }
    return true;
}

const char *get_ext(const char *filename) {
    const char *p = strrchr(filename, '.');
    if (p) {
        return p + 1;
    } else {
        return "";
    }
}