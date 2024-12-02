#pragma once

#include "ShmSimple.h"
#include "ShmBufferRecord.h"
#include "ShmChunks.h"
#include "counters_base.h"
#include "HistoryCounters.h"
#include "HistoryCounterLocal.h"
#include "Status.h"

#define INVALID_VPTR -1

typedef enum {
    drop_new,
    drop_old,
    return_error,
} on_overflov_t;

typedef enum {
    direction_none,
    direction_next,
    direction_prev,
} direction_t;

typedef int64_t vptr_t;

class ShmBufferExData {
public:
    constexpr static const uint32_t MaxReaderCount = 3;
    typedef struct {
        vptr_t pos;
        vptr_t lost;
    } reader_info_t;

    union {
        struct {
            ShmBase::shm_header_t header;
            size_t buffer_size;
            VOLATILE_MEMBER uint32_t head;
            VOLATILE_MEMBER uint32_t tail;
            VOLATILE_MEMBER uint64_t head_offset;
            VOLATILE_MEMBER uint64_t tail_offset;
            VOLATILE_MEMBER shm_record_size_t last_record_size;
            VOLATILE_MEMBER uint64_t total_drop_count;
            VOLATILE_MEMBER uint64_t total_error_count;
            VOLATILE_MEMBER uint64_t tmp_drop_count;
            VOLATILE_MEMBER uint64_t tmp_drop_volume;
            VOLATILE_MEMBER uint64_t tmp_count;
            VOLATILE_MEMBER uint64_t tmp_volume;
            VOLATILE_MEMBER uint64_t count;
            VOLATILE_MEMBER uint64_t total_count;
            on_overflov_t on_overflow;
            reader_info_t reader[MaxReaderCount];
            bool drop_is_ok;
        };
        uint8_t fill_page[MEM_PAGE_SIZE];
    };
    uint8_t circular_data[];

    void Init(size_t _size) {
        buffer_size = _size - sizeof(ShmBufferExData);
        head = tail = 0;
        head_offset = tail_offset = 0;
        last_record_size = 0;
        tmp_drop_count = 0;
        tmp_drop_volume = 0;
        tmp_count = 0;
        tmp_volume = 0;
        count = 0;
        total_count = 0;
        total_drop_count = 0;
        total_error_count = 0;
        on_overflow = return_error;
        memset(reader, 0, sizeof(reader));
    }

    bool write(const uint8_t *data, const size_t data_size);
    void commit(const size_t data_size);
    size_t read(uint8_t *data, const size_t data_size);

    uint8_t *peek() {
        if (is_empty()) {
            return NULL;
        }
        return &circular_data[head];
    }

    void free_head(size_t _size) {
        head += _size;
        if (head >= buffer_size) {
            head -= buffer_size;
            head_offset += buffer_size;
        }
    }

    size_t capacity() const {
        return buffer_size - 1;
    }

    size_t data_size() const {
        if (head <= tail) {
            return tail - head;
        } else {
            return buffer_size - (head - tail);
        }
    }

    size_t get_used_bp() const {
        return data_size()*10000/capacity();
    }


    size_t free_size() const {
        return capacity() - data_size();
    }

    bool is_empty() const {
        return head == tail;
    }

    bool Push(const ShmChunks &chunks);

    bool Add(const uint8_t *data, shm_record_size_t size) {
        return Push(ShmChunks(data, size));
    }

    bool Pop(uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size, bool delete_record = true);
    bool Get(vptr_t &pos, direction_t dir, uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size);
    bool Get(vptr_t &pos, vptr_t &lost, uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size);
    bool Get(int reader_index, uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size);

    bool GetReaderInfo(int reader_index, reader_info_t &reader_info) {
        if (reader_index < 0 || reader_index >= MaxReaderCount) {
            return false;
        }
        reader_info = reader[reader_index];
        return true;
    }

    vptr_t GetBeginVptr() const {
        return head + head_offset;
    }

    vptr_t GetEndVptr() const {
        return tail + tail_offset;
    }

    vptr_t GetLastVptr() const {
        return tail + tail_offset - last_record_size;
    }

    vptr_t GetNextVptr(vptr_t pos) const {
        if (pos < GetBeginVptr()) {
            return GetBeginVptr();
        }
        if (pos >= GetEndVptr()) {
            return GetEndVptr();
        }
        const ShmBufferRecord *r = getRecord(pos);
        return pos + r->Size();
    }

    vptr_t GetPrevVptr(vptr_t pos) const {
        if (pos <= GetBeginVptr()) {
            return -1;
        }
        vptr_t end = GetEndVptr();
        if (pos > end) {
            return end;
        }
        if (pos == end) {
            return pos - last_record_size;
        } else {
            const ShmBufferRecord *r = getRecord(pos);
            return pos - r->PrevSize();
        }
    }

    const ShmBufferRecord *getRecord(vptr_t vptr) const {
        return (const ShmBufferRecord *) (circular_data + (vptr - head_offset));
    }

    void TransferDrops(HistoryCounter &drop, HistoryCounter &error) {
        drop.AddBatch(tmp_drop_count, tmp_drop_volume);
        if (!drop_is_ok) {
            error.AddBatch(tmp_drop_count, tmp_drop_volume);
        }
        tmp_drop_count = 0;
        tmp_drop_volume = 0;
    }

    void TransferMsg(HistoryCounter &c) {
        c.AddBatch(tmp_count, tmp_volume);
        tmp_count = 0;
        tmp_volume = 0;
    }

protected:
    const ShmBufferRecord *getFirstRecord() {
        return (const ShmBufferRecord *) peek();
    }

    ShmBufferRecord *getNewRecord() {
        return (ShmBufferRecord *) (circular_data + tail);
    }

    bool dropFirstRecord() {
        const ShmBufferRecord *rec = getFirstRecord();
        if (rec == NULL) {
            return false;
        }
        ++tmp_drop_count;
        tmp_drop_volume += rec->Size();
        --count;
        ++total_drop_count;
        if (!drop_is_ok) {
            ++total_error_count;
        }
        free_head(rec->Size());
        return true;
    }

    bool dropFirstRecords(shm_record_size_t for_size) {
        while (free_size() < for_size) {
            if (!dropFirstRecord()) {
                return false;
            }
        }
        return true;
    }
};
