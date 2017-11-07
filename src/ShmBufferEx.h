#pragma once

#include "ShmSimple.h"
#include "ShmBufferRecord.h"
#include "ShmChunks.h"
#include "counters_base.h"
#include "HistoryCounters.h"

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
            volatile uint32_t head;
            volatile uint32_t tail;
            volatile uint64_t head_offset;
            volatile uint64_t tail_offset;
            volatile shm_record_size_t last_record_size;
            volatile uint64_t drop_count;
            volatile uint64_t total_count;
            volatile uint64_t count;
            on_overflov_t on_overflow;
            reader_info_t reader[MaxReaderCount];
        };
        uint8_t fill_page[MEM_PAGE_SIZE];
    };
    uint8_t circular_data[];

    void Init(size_t _size) {
        buffer_size = _size - sizeof(ShmBufferExData);
        head = tail = 0;
        head_offset = tail_offset = 0;
        last_record_size = 0;
        drop_count = 0;
        count = 0;
        total_count = 0;
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

    void free(size_t _size) {
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

    size_t free_size() const {
        return capacity() - data_size();
    }

    bool is_empty() const {
        return head == tail;
    }

    bool Add(const ShmChunks &chunks);

    bool Add(const uint8_t *data, shm_record_size_t size) {
        return Add(ShmChunks(data, size));
    }

    bool GetFirst(uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size, bool delete_record = true);
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
        ++drop_count;
        --count;
        free(rec->Size());
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


class ShmBufferEx : public ShmSimple<ShmBufferExData> {
public:
    bool Open(const char *name, size_t size);

    size_t FreeSize() {
        SHM_READ_LOCK;
        return GetData()->free_size();
    }

    size_t Capacity() {
        SHM_READ_LOCK;
        return GetData()->capacity();
    }

#ifdef FOR_TEST

    //Will be called only form tests
        bool Write(const uint8_t *d, size_t data_size) {
            SHM_WRITE_LOCK
            return GetData()->write(d, data_size);
        }

        size_t Read(uint8_t *d, size_t data_size) {
            SHM_WRITE_LOCK
            return GetData()->read(d, data_size);
        }

        size_t DataSize() {
            SHM_READ_LOCK
            return GetData()->data_size();
        }

        void Free(size_t s) {
            SHM_WRITE_LOCK;
            GetData()->free(s);
        }

        const ShmBufferRecord *getRecord(uint64_t vptr) {
            return GetData()->getRecord(vptr);
        }

#endif

    void SetOverflovBehavior(on_overflov_t d) {
        SHM_WRITE_LOCK;
        GetData()->on_overflow = d;
    }

    uint64_t Count() {
        return GetData()->count;
    }

    uint64_t DropCount() {
        return GetData()->drop_count;
    }

    uint64_t TotalCount() {
        return GetData()->total_count;
    }

    bool Add(const ShmChunks &chunks) {
        SHM_WRITE_LOCK;
        return GetData()->Add(chunks);
    }

    bool Add(const uint8_t *data, shm_record_size_t size) {
        return Add(ShmChunks(data, size));
    }

    bool GetFirst(uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size, bool delete_record = true) {
        SHM_WRITE_LOCK;
        return GetData()->GetFirst(data, max_size, size, delete_record);
    }

    bool Get(vptr_t &pos, direction_t dir, uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size) {
        SHM_WRITE_LOCK;
        return GetData()->Get(pos, dir, data, max_size, size);
    }

    bool Get(vptr_t &pos, vptr_t &lost, uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size) {
        SHM_WRITE_LOCK;
        return GetData()->Get(pos, lost, data, max_size, size);
    }

    bool Get(int reader_index, uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size) {
        SHM_WRITE_LOCK;
        return GetData()->Get(reader_index, data, max_size, size);
    }

    bool GetReaderInfo(int reader_index, ShmBufferExData::reader_info_t &reader_info) {
        SHM_READ_LOCK;
        return GetData()->GetReaderInfo(reader_index, reader_info);
    }

    bool IsEmpty() {
        SHM_READ_LOCK;
        return GetData()->is_empty();
    }

    vptr_t GetBeginVptr() {
        SHM_READ_LOCK;
        return GetData()->GetBeginVptr();
    }

    vptr_t GetLastVptr() {
        SHM_READ_LOCK;
        return GetData()->GetLastVptr();
    }

    vptr_t GetEndVptr() {
        SHM_READ_LOCK;
        return GetData()->GetEndVptr();
    }

    vptr_t GetNextVptr(vptr_t pos) {
        SHM_READ_LOCK;
        return GetData()->GetNextVptr(pos);
    }

    vptr_t GetPrevVptr(vptr_t pos) {
        SHM_READ_LOCK;
        return GetData()->GetPrevVptr(pos);
    }

};



