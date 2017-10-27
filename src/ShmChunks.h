#pragma once

#include <cstdint>
#include "ShmBufferRecord.h"

class ShmChunks {
    constexpr static const uint32_t MaxChunkCount = 10;
    uint8_t count;
    const void *chunk_data[MaxChunkCount];
    shm_record_size_t chunk_size[MaxChunkCount];
public:
    ShmChunks() {
        count = 0;
    }

    ShmChunks(const void *data, shm_record_size_t size) {
        count = 1;
        chunk_size[0] = size;
        chunk_data[0] = data;
    }

    void Add(const void *data, shm_record_size_t size) {
        chunk_data[count] = data;
        chunk_size[count] = size;
        count++;
    }

    shm_record_size_t Size() const {
        shm_record_size_t s = 0;
        for (int i = 0; i < count; i++) {
            s += chunk_size[i];
        }
        return s;
    }

    void WriteTo(uint8_t *dst) const {
        for (int i = 0; i < count; i++) {
            memcpy(dst, chunk_data[i], chunk_size[i]);
            dst += chunk_size[i];
        }
    }
};

