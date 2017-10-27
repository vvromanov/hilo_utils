#include "ShmBuffer.h"

bool ShmBuffer::Open(const char *name, size_t _size) {
    if (_size) {
        _size = ((_size + MEM_PAGE_SIZE - 1) / MEM_PAGE_SIZE) * MEM_PAGE_SIZE + sizeof(ShmBufferData);
    }
    if (!ShmSimple<ShmBufferData>::OpenMirror(name, _size, sizeof(ShmBufferData))) {
        return false;
    }
    return true;
}

bool ShmBufferData::write(const uint8_t *d, size_t _size) {
    /* prevent buffer from getting completely full or over commited */
    if (free_size() < _size) {
        return false;
    }

    memcpy(circular_data + tail, d, _size);
    tail += _size;
    if (size < tail) {
        tail -= size;
    }
    return true;
}

size_t ShmBufferData::read(uint8_t *d, const size_t data_size) {
    size_t read_size = std::min(data_size, this->data_size());
    memcpy(d, circular_data + head, read_size);
    free(read_size);
    return read_size;
}
