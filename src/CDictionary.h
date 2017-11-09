#pragma once
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include "common_utils.h"
#include "ShmBase.h"

#define DICTIONARY_INVALID_INDEX  -1

template<uint32_t DictionarySize = 500, uint32_t DictionaryStorageSize = 65000>
class CDictionary {
public:
    enum {
        NAMES_STORAGE_SIZE = DictionaryStorageSize,
        SIZE = DictionarySize,
        NAME_MAX_LEN = 128,
    } dict_consts_t;

    typedef int32_t index_t;
    typedef uint32_t offset_t;

    typedef struct {
        index_t id;
        offset_t name_offset;
    } name_rec_t;

    typedef struct {
        index_t count;
        name_rec_t index[DictionarySize];
    } index_info_t;

    typedef struct {
        const CDictionary<DictionarySize, DictionaryStorageSize> &dict;
        const char *name;
    } search_rec_t;

    void Clear();
    index_t Add(const char *name);
    index_t Lookup(const char *name) const;
    const char *Lookup(index_t index) const;
    const char *GetString(offset_t offset) const;
    void GetIndex(index_info_t &index) const;
    void GetCategory(const char *prefix, index_info_t &index) const;
    static bool compare(const name_rec_t &r, const search_rec_t &t);
private:
    index_info_t index_info;
    uint32_t names_used; //Использованое хранилище имен
    char names_storage[DictionaryStorageSize];
    offset_t str_index[DictionarySize];
};

typedef struct {
    const CDictionary<> &dict;
    const char *name;
} search_rec_t;

template<uint32_t DictionarySize, uint32_t DictionaryStorageSize>
bool CDictionary<DictionarySize, DictionaryStorageSize>::compare(
        const typename CDictionary<DictionarySize, DictionaryStorageSize>::name_rec_t &r,
        const typename CDictionary<DictionarySize, DictionaryStorageSize>::search_rec_t &t) {
    return strcasecmp(t.dict.Lookup(r.id), t.name) < 0;
}

template<uint32_t DictionarySize, uint32_t DictionaryStorageSize>
void CDictionary<DictionarySize, DictionaryStorageSize>::Clear() {
    index_info.count = 0;
    names_used = 0;
}

template<uint32_t DictionarySize, uint32_t DictionaryStorageSize>
typename CDictionary<DictionarySize, DictionaryStorageSize>::index_t
CDictionary<DictionarySize, DictionaryStorageSize>::Add(const char *name) {
    int len = strlen(name);
    if (names_used + len + 1 > NAMES_STORAGE_SIZE) {
        return DICTIONARY_INVALID_INDEX;
    }
    if (index_info.count >= SIZE) {
        return DICTIONARY_INVALID_INDEX;
    }
    search_rec_t r = {*this, name};
    name_rec_t *p = std::lower_bound(index_info.index, index_info.index + index_info.count, r, compare);
    if (p != (index_info.index + index_info.count)) {
        if (strcasecmp(names_storage + p->name_offset, name) == 0) {
            return p->id;
        }
        memmove(p + 1, p, ((index_info.index + index_info.count) - p) * sizeof(*p));
    }
    p->id = index_info.count;
    p->name_offset = str_index[index_info.count] = names_used;
    index_info.count++;
    memcpy(names_storage + names_used, name, len + 1);
    names_used += len + 1;
    return p->id;
}

template<uint32_t DictionarySize, uint32_t DictionaryStorageSize>
typename CDictionary<DictionarySize, DictionaryStorageSize>::index_t
CDictionary<DictionarySize, DictionaryStorageSize>::Lookup(const char *name) const {
    search_rec_t r = {*this, name};
    const name_rec_t *p = std::lower_bound(index_info.index, index_info.index + index_info.count, r, compare);
    if (p != (index_info.index + index_info.count) && strcasecmp(names_storage + p->name_offset, name) == 0) {
        return p->id;
    } else {
        return DICTIONARY_INVALID_INDEX;
    }
}

template<uint32_t DictionarySize, uint32_t DictionaryStorageSize>
const char *CDictionary<DictionarySize, DictionaryStorageSize>::Lookup(
        typename CDictionary<DictionarySize, DictionaryStorageSize>::index_t index) const {
    if (index < 0 || index >= index_info.count) {
        return NULL;
    }
    return names_storage + str_index[index];
}

template<uint32_t DictionarySize, uint32_t DictionaryStorageSize>
void CDictionary<DictionarySize, DictionaryStorageSize>::GetIndex(
        CDictionary<DictionarySize, DictionaryStorageSize>::index_info_t &index_) const {
    index_.count = this->index_info.count;
    memcpy(index_.index, this->index_info.index, sizeof(index_.index[0]) * index_.count);
}

template<uint32_t DictionarySize, uint32_t DictionaryStorageSize>
void CDictionary<DictionarySize, DictionaryStorageSize>::GetCategory(const char *prefix,
                                                                     CDictionary<DictionarySize, DictionaryStorageSize>::index_info_t &index_) const {
    if (prefix == NULL || prefix[0] == 0) {
        GetIndex(index_);
    } else {
        char prefix_[NAME_MAX_LEN + 1];
        STRNCPY(prefix_, prefix);
        search_rec_t r = {*this, prefix_};
        const name_rec_t *p1 = std::lower_bound(index_info.index, index_info.index + index_info.count, r, compare);
        STRNCAT(prefix_, "\xFF");
        const name_rec_t *p2 = std::lower_bound(index_info.index, index_info.index + index_info.count, r, compare);
        index_.count = p2 - p1;
        memmove(index_.index, p1, index_.count * sizeof(index_.index[0]));
    }
}

template<uint32_t DictionarySize, uint32_t DictionaryStorageSize>
const char* CDictionary<DictionarySize, DictionaryStorageSize>::GetString(CDictionary<DictionarySize, DictionaryStorageSize>::offset_t offset) const {
    return names_storage + offset;
}
