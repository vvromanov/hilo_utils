#pragma once

typedef uint32_t shm_record_size_t;

class ShmBufferRecord {
    shm_record_size_t record_size;
    shm_record_size_t prev_record_size;
    uint8_t data[];
public:

    shm_record_size_t Size() const {
        return record_size;
    }

    shm_record_size_t DataSize() const {
        return record_size - sizeof(ShmBufferRecord);
    }

    void SetSize(shm_record_size_t s) {
        record_size = s;
    }

    shm_record_size_t PrevSize() const {
        return prev_record_size;
    }

    void SetPrevSize(shm_record_size_t s) {
        prev_record_size = s;
    }

    const uint8_t *Data() const {
        return data;
    }

    uint8_t *Data() {
        return data;
    }

    static size_t RecordSize(size_t s) {
        return s + sizeof(ShmBufferRecord);
    }
    size_t DataSize(size_t s) {
        return record_size- sizeof(ShmBufferRecord);
    }
} __attribute__ ((__packed__));
