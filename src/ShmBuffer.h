#pragma once

#include "ShmSimple.h"

// based on https://github.com/willemt/cbuffer

class ShmBufferData {
public:
    union {
        struct {
            ShmBase::shm_header_t header;
            size_t size;
            uint32_t head;
            uint32_t tail;
        };
        uint8_t fill_page[MEM_PAGE_SIZE];
    };
    uint8_t circular_data[];

    void Init(size_t _size) {
        size = _size - sizeof(ShmBufferData);
        head = tail = 0;
    }

    bool write(const uint8_t *data, const size_t data_size);
    size_t read(uint8_t *data, const size_t data_size);

    uint8_t *peek() {
        if (is_empty()) {
            return NULL;
        }
        return &circular_data[head];
    }

    void free(size_t _size) {
        head += _size;
        if (head >= size) {
            head -= size;
        }
    }

    size_t capacity() const {
        return size - 1;
    }

    size_t data_size() const {
        if (head <= tail) {
            return tail - head;
        } else {
            return size - (head - tail);
        }
    }

    size_t free_size() const {
        return capacity() - data_size();
    }

    bool is_empty() const {
        return head == tail;
    }
};

class ShmBuffer : public ShmSimple<ShmBufferData> {
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

    bool Write(const uint8_t *d, size_t data_size) {
        WRITE_LOCK
        return GetData()->write(d, data_size);
    }

    size_t Read(uint8_t *d, size_t data_size) {
        WRITE_LOCK
        return GetData()->read(d, data_size);
    }

    size_t DataSize() {
        READ_LOCK
        return GetData()->data_size();
    }

    void Free(size_t s) {
        WRITE_LOCK;
        GetData()->free(s);
    }

    bool IsEmpty() {
        return GetData()->is_empty();
    }
};
