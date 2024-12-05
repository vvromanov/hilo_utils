#pragma once

#include <string>
#include "stdbool.h"

bool is_file_exists(const char *name);
bool is_dir_exists(const char* name);
bool remove_test_file(const char* path);
bool get_file_size(const char *name, size_t &size);
bool mkdir_for_file(const char* filename, __mode_t mode);
const char* get_ext(const char* path);