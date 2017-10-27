#pragma once

#include <stdint.h>

class counter_t {
    volatile int64_t v;
public:
    counter_t() {
        Set(0);
    }

    counter_t(int64_t value) {
        Set(value);
    }

    void Set(int64_t value) {
        __sync_lock_test_and_set(&v, value);
    }

    operator int64_t() const {
        return __sync_add_and_fetch(const_cast<volatile int64_t *>(&v), 0);
    }

    int64_t operator++()     // prefix ++
    {
        return __sync_add_and_fetch(&v, 1);
    }

    int64_t operator--()     // prefix --
    {
        return __sync_sub_and_fetch(&v, 1);
    }

    int64_t operator+=(int64_t value) {
        return __sync_add_and_fetch(&v, value);
    }

    int64_t operator-=(int64_t value) {
        return __sync_sub_and_fetch(&v, value);
    }

    const counter_t &operator=(int64_t value) {
        v = value;
        return *this;
    };
} __attribute__((__packed__));




