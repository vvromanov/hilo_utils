#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include "CDictionary.h"
#include "counters.h"

typedef enum {
    counter_value,
    counter_incremental,
} counter_type_t;


class Counter {
    static counter_t def_value;
    counter_t *ptr = &def_value;
    Counter(const Counter &c);
public:
    Counter() : ptr(&def_value) {};
    Counter(const char *name, counter_type_t type = counter_incremental);

    Counter(counter_t *_ptr) : ptr(_ptr) {};

    void Init(counter_t *p) {
        ptr = p;
    }

    bool IsValid() {
        return (ptr != &def_value);
    }

    bool SetName(const char *name, counter_type_t type = counter_incremental);

    void Set(int64_t value) {
        ptr->Set(value);
    }

    operator int64_t() const { return *ptr; }

    const Counter &operator=(int64_t value) {
        if (ptr == NULL) {
            ptr = &def_value;
        }
        *ptr = value;
        return *this;
    }

    int64_t operator++()     // prefix ++
    {
        return ++(*ptr);
    }

    int64_t operator--()     // prefix --
    {
        return --(*ptr);
    }

    int64_t operator+=(int64_t v) {
        return (*ptr)+=v;
    }

    int64_t operator-=(int64_t v) {
        return (*ptr)-=v;
    }

    static int64_t GetValue(const char *name, counter_type_t type = counter_incremental) {
        Counters &c = (type == counter_value) ? ValueCounters() : IncrementedCounters();
        Counters::index_t index = c.LookupIndex(name);
        if (index != DICTIONARY_INVALID_INDEX) {
            return *c.GetCounterPtr(index);
        } else {
            return -1;
        }
    }
};

class LazyCounter {
    static counter_t def_value;
    counter_t *ptr = &def_value;
    counter_type_t type = counter_incremental;
    std::string name;
    LazyCounter(const Counter &c);
public:
    LazyCounter() {};

    LazyCounter(const char *_name, counter_type_t _type = counter_incremental) : type(_type), name(_name) {};

    void SetName(const char *_name, counter_type_t _type = counter_incremental) {
        Reset();
        type = _type;
        name = _name;
    };

    void Set(int64_t value) {
        Init();
        *ptr = value;
    }

    void Reset() {
        ptr = &def_value;
    }

    operator int64_t() {
        Init();
        return *ptr;
    }

//    operator bool() {
//        Init();
//        return *ptr != 0;
//    }
//
    int64_t Get() {
        Init();
        return *ptr;
    }

    const LazyCounter &operator=(int64_t value) {
        Init();
        *ptr = value;
        return *this;
    }

    int64_t operator++()     // prefix ++
    {
        Init();
        return ++(*ptr);
    }

    int64_t operator--()     // prefix --
    {
        Init();
        return --(*ptr);
    }

    int64_t operator+=(int64_t v) {
        Init();
        return (*ptr)+=v;
    }

    int64_t operator-=(int64_t v) {
        Init();
        return (*ptr)-=v;
    }

protected:
    void Init() {
        if (ptr == &def_value) {
            Counters &c = (type == counter_value) ? ValueCounters() : IncrementedCounters();
            Counters::index_t index = c.GetCounterIndex(name.c_str());
            if (index != DICTIONARY_INVALID_INDEX) {
                ptr = c.GetCounterPtr(index);
            }
        }
    }
};

//class ValueCounter : public Counter {
//public:
//    ValueCounter(const char *name) {
//        Counters::index_t index = ValueCounters().GetCounterIndex(name);
//        if (index != DICTIONARY_INVALID_INDEX) {
//            Init(ValueCounters().GetCounterPtr(index));
//        }
//    }
//};
