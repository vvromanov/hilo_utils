#pragma once

#include "ShmBufferExData.h"

#define BUFFER_EX_COUNTER_PREFIX "shm.queue."
#define BUFFER_EX_COUNTER_SUFFIX ".capacity"
#define BUFFER_EX_CNAME_MESSAGE  BUFFER_EX_COUNTER_PREFIX "%s.message"
#define BUFFER_EX_CNAME_ERROR    BUFFER_EX_COUNTER_PREFIX "%s.error"
#define BUFFER_EX_CNAME_DROP     BUFFER_EX_COUNTER_PREFIX "%s.drop"
#define BUFFER_EX_CNAME_COUNT    BUFFER_EX_COUNTER_PREFIX "%s.count"
#define BUFFER_EX_CNAME_USED_BP  BUFFER_EX_COUNTER_PREFIX "%s.used_bp"
#define BUFFER_EX_CNAME_USED     BUFFER_EX_COUNTER_PREFIX "%s.used"
#define BUFFER_EX_CNAME_CAPACITY BUFFER_EX_COUNTER_PREFIX "%s" BUFFER_EX_COUNTER_SUFFIX

typedef struct {
    char name[NAME_MAX];
    size_t count;
    size_t data_size;
    size_t capacity;
    history_counter_info_t drop_history;
    history_counter_info_t message_history;
    history_counter_info_t error_history;
} queue_stat_t;

class ShmBufferEx : public ShmSimple<ShmBufferExData> {
    HistoryCounter cMsgHistory;
    HistoryCounter cErrorHistory;
    HistoryCounter cDropHistory;
    HistoryCounterLocal cPushFailedLocal;

    LazyCounter cMsgCount;
    LazyCounter cUsedBp;
    LazyCounter cUsed;
    LazyCounter cCapacity;
public:
    bool Open(const char *name, size_t size);

    size_t FreeSize() {
        READ_LOCK;
        return GetData()->free_size();
    }

    size_t Capacity() {
        READ_LOCK;
        return GetData()->capacity();
    }

    size_t DataSize() {
        READ_LOCK;
        return GetData()->data_size();
    }

    size_t GetUsedBp() {
        READ_LOCK;
        return GetData()->get_used_bp();
    }


#ifdef FOR_TEST

    //Will be called only form tests
        bool Write(const uint8_t *d, size_t data_size) {
            WRITE_LOCK
            return GetData()->write(d, data_size);
        }

        size_t Read(uint8_t *d, size_t data_size) {
            WRITE_LOCK
            return GetData()->read(d, data_size);
        }

        void Free(size_t s) {
            WRITE_LOCK;
            GetData()->free_head(s);
        }

        const ShmBufferRecord *getRecord(uint64_t vptr) {
            return GetData()->getRecord(vptr);
        }

#endif

    void SetOverflovBehavior(on_overflov_t d, bool drop_is_ok) {
        WRITE_LOCK;
        GetData()->on_overflow = d;
        GetData()->drop_is_ok = drop_is_ok;
    }

    uint64_t Count() {
        return GetData()->count;
    }

    uint64_t TotalCount() {
        return GetData()->total_count;
    }

    uint64_t TotalDropCount() {
        return GetData()->total_drop_count;
    }

    uint64_t TotalErrorCount() {
        return GetData()->total_error_count;
    }

    bool Push(const ShmChunks &chunks) {
        bool res;
        {
            WRITE_LOCK;
            res = GetData()->Push(chunks);
        }
        if (!res) {
            ++cPushFailedLocal;
            return false;
        }
        return true;
    }

    bool Push(const uint8_t *data, shm_record_size_t size) {
        return Push(ShmChunks(data, size));
    }

    bool Pop(uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size, bool delete_record = true) {
        ShmBufferExData *p = GetData();
        if (p->count == 0) {
            size = 0;
            return false;
        }
        WRITE_LOCK;
        return p->Pop(data, max_size, size, delete_record);
    }

    bool Get(vptr_t &pos, direction_t dir, uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size) {
        ShmBufferExData *p = GetData();
        if (p->count == 0) {
            size = 0;
            return false;
        }
        WRITE_LOCK;
        return p->Get(pos, dir, data, max_size, size);
    }

    bool Get(vptr_t &pos, vptr_t &lost, uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size) {
        ShmBufferExData *p = GetData();
        if (p->count == 0) {
            size = 0;
            return false;
        }
        WRITE_LOCK;
        return p->Get(pos, lost, data, max_size, size);
    }

    bool Get(int reader_index, uint8_t *data, shm_record_size_t max_size, shm_record_size_t &size) {
        ShmBufferExData *p = GetData();
        if (p->count == 0) {
            size = 0;
            return false;
        }
        WRITE_LOCK;
        return p->Get(reader_index, data, max_size, size);
    }

    bool GetReaderInfo(int reader_index, ShmBufferExData::reader_info_t &reader_info) {
        READ_LOCK;
        return GetData()->GetReaderInfo(reader_index, reader_info);
    }

    bool IsEmpty() {
        return GetData()->is_empty();
    }

    vptr_t GetBeginVptr() {
        READ_LOCK;
        return GetData()->GetBeginVptr();
    }

    vptr_t GetLastVptr() {
        READ_LOCK;
        return GetData()->GetLastVptr();
    }

    vptr_t GetEndVptr() {
        READ_LOCK;
        return GetData()->GetEndVptr();
    }

    vptr_t GetNextVptr(vptr_t pos) {
        READ_LOCK;
        return GetData()->GetNextVptr(pos);
    }

    vptr_t GetPrevVptr(vptr_t pos) {
        READ_LOCK;
        return GetData()->GetPrevVptr(pos);
    }

    static void DumpStatHeader(std::ostream &s, bool asHtml);
    void GetStat(queue_stat_t& stat);
    static void GetStat(const char* name, queue_stat_t& stat);
    static void DumpStat(const queue_stat_t& stat, std::ostream &s, bool asHtml);
    static void DumpStatTable(std::ostream &s, bool asHtml);
    static status_t CheckAllStatus(std::ostream &s, status_format_t format, const char* ignore_full_names[], int ignore_full_names_count);
    status_t CheckStatus(std::ostream &s, status_format_t format, bool ignore_full);
    static status_t CheckStatus(const queue_stat_t& stat, std::ostream &s, status_format_t format, bool ignore_full);
    void UpdateCounters();
};



