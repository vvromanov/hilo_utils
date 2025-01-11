#define FOR_TEST

#include <ShmFileBase.h>
#include <ShmBufferEx.h>
#include <zlib.h>

#define TEST_SHM_SIZE (MEM_PAGE_SIZE*4)

class TestData {
public:
    static const char *get_name() {
        return "test_shm";
    }
};

class ShmBufferExTest : public ShmFileBase<TestData> {

};

TEST_F(ShmBufferExTest, Open) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    EXPECT_TRUE(is_file_exists(TestFileName()));
    EXPECT_EQ(TEST_SHM_SIZE + MEM_PAGE_SIZE, test.GetSize());
}

TEST_F(ShmBufferExTest, OpenInvalidSize) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    test.Close();
    ScopedLogLevel ll(LOG_LEVEL_DISABLED);
    EXPECT_FALSE(test.Open(TestShmName(), TEST_SHM_SIZE * 10));
}

TEST_F(ShmBufferExTest, AlreadyOpen) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    ScopedLogLevel ll(LOG_LEVEL_DISABLED);
    EXPECT_FALSE(test.Open(TestShmName(), TEST_SHM_SIZE));
}

TEST_F(ShmBufferExTest, ReOpen) {
    ShmBufferEx test1, test2;
    EXPECT_TRUE(test1.Open(TestShmName(), TEST_SHM_SIZE));
    EXPECT_TRUE(test2.Open(TestShmName(), 0));
}

TEST_F(ShmBufferExTest, ReOpenFailed) {
    ShmBufferEx test;
    ScopedLogLevel ll(LOG_LEVEL_DISABLED);
    EXPECT_FALSE(test.Open(TestShmName(), 0));
}

/*TEST_F(ShmFileBase, TooBigSize) {
    ShmBase test;
    EXPECT_FALSE(test.Open(TestShmName(), 1024UL * 1024UL * 1024UL * 1024UL * 10)); //10 Tb
}*/

TEST_F(ShmBufferExTest, IsDeleted) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    EXPECT_FALSE(test.IsDeleted());
    EXPECT_EQ(0, remove(TestFileName()));
    EXPECT_TRUE(test.IsDeleted());
}

TEST_F(ShmBufferExTest, CapacityAfterCreate) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    EXPECT_EQ(MEM_PAGE_SIZE - 1, test.Capacity());
}

TEST_F(ShmBufferExTest, FreeSizeAfterCreate) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    EXPECT_EQ(MEM_PAGE_SIZE - 1, test.FreeSize());
}

TEST_F(ShmBufferExTest, DataSizeAfterCreate) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    EXPECT_EQ(0, test.DataSize());
}

TEST_F(ShmBufferExTest, IsEmptyAfterCreate) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    EXPECT_TRUE(test.IsEmpty());
}

TEST_F(ShmBufferExTest, DataSizeAfterWrite) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[100];
    EXPECT_TRUE(test.Write(data, sizeof(data)));
    EXPECT_EQ(sizeof(data), test.DataSize());
}

TEST_F(ShmBufferExTest, IsEmptyAfterWriteRead) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[100];
    EXPECT_TRUE(test.Write(data, sizeof(data)));
    EXPECT_EQ(sizeof(data), test.Read(data, sizeof(data)));
    EXPECT_TRUE(test.IsEmpty());
}

TEST_F(ShmBufferExTest, CantWrite) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[100];
    EXPECT_FALSE(test.Write(data, MEM_PAGE_SIZE + 1));
    EXPECT_EQ(0, test.DataSize());
    EXPECT_TRUE(test.IsEmpty());
}

TEST_F(ShmBufferExTest, CantWriteToFull) { //иначе голова и хвост будут равны
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[100];
    EXPECT_FALSE(test.Write(data, MEM_PAGE_SIZE));
    EXPECT_EQ(0, test.DataSize());
    EXPECT_TRUE(test.IsEmpty());
}

TEST_F(ShmBufferExTest, FillOneWrite) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[MEM_PAGE_SIZE];
    EXPECT_TRUE(test.Write(data, MEM_PAGE_SIZE - 1));
    EXPECT_EQ(MEM_PAGE_SIZE - 1, test.DataSize());
    EXPECT_EQ(0, test.FreeSize());
    EXPECT_FALSE(test.IsEmpty());
}

TEST_F(ShmBufferExTest, FillMultipleWritesWithWrap) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[100];
    EXPECT_TRUE(test.Write(data, sizeof(data)));
    EXPECT_EQ(sizeof(data), test.Read(data, sizeof(data)));
    for (int i = 0; i < MEM_PAGE_SIZE - 1; i++) {
        EXPECT_TRUE(test.Write(data, 1));
        EXPECT_EQ(i + 1, test.DataSize());
    }
    EXPECT_EQ(MEM_PAGE_SIZE - 1, test.DataSize());
    EXPECT_EQ(0, test.FreeSize());
    EXPECT_FALSE(test.IsEmpty());
}

TEST_F(ShmBufferExTest, CantReadFromEmpty) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[1];
    EXPECT_EQ(0, test.Read(data, sizeof(data)));
    EXPECT_EQ(0, test.DataSize());
    EXPECT_TRUE(test.IsEmpty());
}


TEST_F(ShmBufferExTest, CantReadTooManyData) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[100];
    EXPECT_TRUE(test.Write(data, sizeof(data) - 1));
    EXPECT_EQ(sizeof(data) - 1, test.Read(data, sizeof(data)));
}

TEST_F(ShmBufferExTest, ReadWriteCycle) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    for (int i = 0; i < 10000; i++) {
        uint8_t data1[11];
        uint8_t data2[11];
        for (int j = 0; j < sizeof(data1); j++) {
            data1[j] = rand() % 256;
        }
        EXPECT_TRUE(test.Write(data1, sizeof(data1)));
        EXPECT_EQ(sizeof(data2), test.Read(data2, sizeof(data2)));
        if (memcmp(data1, data2, sizeof(data1)) != 0) {
            EXPECT_EQ(0, memcmp(data1, data2, sizeof(data1)));
        }
    }
}


TEST_F(ShmBufferExTest, ReadWriteCycle2) {
    ShmBufferEx test1;
    EXPECT_TRUE(test1.Open(TestShmName(), 1));
    ShmBufferEx test2;
    EXPECT_TRUE(test2.Open(TestShmName(), 1));
    for (int i = 0; i < 10000; i++) {
        uint8_t data1[11];
        uint8_t data2[11];
        for (int j = 0; j < sizeof(data1); j++) {
            data1[j] = rand() % 256;
        }
        EXPECT_TRUE(test1.Write(data1, sizeof(data1)));
        EXPECT_EQ(sizeof(data2), test2.Read(data2, sizeof(data2)));
        EXPECT_EQ(0, memcmp(data1, data2, sizeof(data1)));
    }
}

TEST_F(ShmBufferExTest, AddGet) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data1[11];
    uint8_t data2[11];
    for (int j = 0; j < sizeof(data1); j++) {
        data1[j] = rand() % 256;
    }
    EXPECT_TRUE(test.Push(data1, sizeof(data1)));
    EXPECT_TRUE(test.Push(data1, sizeof(data1)));
    EXPECT_EQ(2, test.Count());
    EXPECT_EQ(2, test.TotalCount());

    BZERO_S(data2);
    shm_record_size_t size = 0;
    EXPECT_TRUE(test.Pop(data2, sizeof(data2), size));
    EXPECT_EQ(sizeof(data2), size);
    EXPECT_EQ(0, memcmp(data1, data2, sizeof(data1)));

    EXPECT_EQ(1, test.Count());
    EXPECT_EQ(2, test.TotalCount());


    BZERO_S(data2);
    size = 0;
    EXPECT_TRUE(test.Pop(data2, sizeof(data2), size));
    EXPECT_EQ(sizeof(data2), size);
    EXPECT_EQ(0, memcmp(data1, data2, sizeof(data1)));
    BZERO_S(data2);

    EXPECT_EQ(0, test.Count());
    EXPECT_EQ(2, test.TotalCount());


    EXPECT_FALSE(test.Pop(data2, sizeof(data2), size));
    EXPECT_EQ(0, size);
}

class ShmTestData {
public:
    constexpr static size_t MaxTestDataSize = 10000;
    union { ;
        struct {
            uint32_t hash;
            int64_t vptr;
            int16_t size;
            uint8_t random_data[];
        } h;
        uint8_t data[MaxTestDataSize];
    } __attribute__ ((__packed__));

    ShmTestData() {}

    ShmTestData(size_t s, vptr_t v) {
        h.vptr = v;
        h.size = s;
        for (int i = 0; i < h.size - sizeof(h); i++) {
            h.random_data[i] = rand() % 256;
        }
        update_hash();
    }

    void Clear() {
        memset(data, 0, sizeof(data));
    }

    bool Check() const {
        return h.hash == crc32(0, data + sizeof(h.hash), h.size - sizeof(h.hash));
    }

    size_t Size() {
        return h.size;
    }

    void SetVptr(vptr_t v) {
        h.vptr = v;
        update_hash();
    }

    vptr_t Vptr() const {
        return h.vptr;
    }

    const uint8_t *Data() const {
        return data;
    }

    uint8_t *Data() {
        return data;
    }

private:
    void update_hash() {
        h.hash = crc32(0, data + sizeof(h.hash), h.size - sizeof(h.hash));
    }
}  __attribute__ ((__packed__));

TEST_F(ShmBufferExTest, Pop) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    size_t free_size = test.FreeSize();
    EXPECT_EQ(MEM_PAGE_SIZE - 1, free_size);
    ShmTestData d1(111, 0);
    EXPECT_TRUE(test.Push(d1.Data(), d1.Size()));
    d1.SetVptr(1);
    EXPECT_TRUE(test.Push(d1.Data(), d1.Size()));
    d1.SetVptr(2);
    EXPECT_TRUE(test.Push(d1.Data(), d1.Size()));

    ShmTestData d2;
    shm_record_size_t size;
    EXPECT_TRUE(test.Pop(d2.Data(), ShmTestData::MaxTestDataSize, size));
    EXPECT_TRUE(d2.Check());
    EXPECT_EQ(0, d2.Vptr());
    EXPECT_EQ(d2.Size(), size);

    d2.Clear();
    EXPECT_TRUE(test.Pop(d2.Data(), ShmTestData::MaxTestDataSize, size));
    EXPECT_TRUE(d2.Check());
    EXPECT_EQ(1, d2.Vptr());
    EXPECT_EQ(d2.Size(), size);

    d2.Clear();
    EXPECT_TRUE(test.Pop(d2.Data(), ShmTestData::MaxTestDataSize, size));
    EXPECT_TRUE(d2.Check());
    EXPECT_EQ(2, d2.Vptr());
    EXPECT_EQ(d2.Size(), size);

    d2.Clear();
    EXPECT_FALSE(test.Pop(d2.Data(), ShmTestData::MaxTestDataSize, size));
}

TEST_F(ShmBufferExTest, Peek) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    size_t free_size = test.FreeSize();
    EXPECT_EQ(MEM_PAGE_SIZE - 1, free_size);
    ShmTestData d1(111, 0);
    EXPECT_TRUE(test.Push(d1.Data(), d1.Size()));
    d1.SetVptr(1);
    EXPECT_TRUE(test.Push(d1.Data(), d1.Size()));
    d1.SetVptr(2);
    EXPECT_TRUE(test.Push(d1.Data(), d1.Size()));

    ShmTestData d2;
    shm_record_size_t size;
    EXPECT_TRUE(test.Pop(d2.Data(), ShmTestData::MaxTestDataSize, size, false));
    EXPECT_TRUE(d2.Check());
    EXPECT_EQ(0, d2.Vptr());
    EXPECT_EQ(d2.Size(), size);

    d2.Clear();
    EXPECT_TRUE(test.Pop(d2.Data(), ShmTestData::MaxTestDataSize, size, false));
    EXPECT_TRUE(d2.Check());
    EXPECT_EQ(0, d2.Vptr());
    EXPECT_EQ(d2.Size(), size);
}

TEST_F(ShmBufferExTest, Fill) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    size_t free_size = test.FreeSize();
    EXPECT_EQ(MEM_PAGE_SIZE - 1, free_size);
    ShmTestData d1(111, 0);
    while (free_size >= d1.Size()) {
        d1.SetVptr(test.Count());
        EXPECT_TRUE(test.Push(d1.Data(), d1.Size()));
        free_size = test.FreeSize();
    }
    EXPECT_FALSE(test.Push(d1.Data(), d1.Size()));

    test.SetOverflovBehavior(drop_new, false);
    EXPECT_EQ(0, test.TotalDropCount());
    EXPECT_TRUE(test.Push(d1.Data(), d1.Size()));
    EXPECT_EQ(1, test.TotalDropCount());

    shm_record_size_t size;
    ShmTestData d2;
    EXPECT_TRUE(test.Pop(d2.Data(), ShmTestData::MaxTestDataSize, size, false));
    EXPECT_EQ(0, d2.Vptr());

    test.SetOverflovBehavior(drop_old, false);
    EXPECT_TRUE(test.Push(d1.Data(), d1.Size()));
    EXPECT_EQ(2, test.TotalDropCount());
    d2.Clear();
    EXPECT_TRUE(test.Pop(d2.Data(), ShmTestData::MaxTestDataSize, size, false));
    EXPECT_EQ(1, d2.Vptr());
}

TEST_F(ShmBufferExTest, ReadWriteCycleEx2) {
    ShmBufferEx test1;
    EXPECT_TRUE(test1.Open(TestShmName(), 1));
    ShmBufferEx test2;
    EXPECT_TRUE(test2.Open(TestShmName(), 1));
    for (int i = 0; i < 10000; i++) {
        ShmTestData d1(111, i);
        EXPECT_TRUE(test1.Push(d1.Data(), d1.Size()));

        ShmTestData d2;
        shm_record_size_t size = 0;
        EXPECT_FALSE(test2.Pop(d2.Data(), d1.Size() - 1, size));
        EXPECT_EQ(d1.Size(), size);

        size = 0;
        EXPECT_TRUE(test2.Pop(d2.Data(), ShmTestData::MaxTestDataSize, size));
        EXPECT_EQ(d1.Size(), size);
        EXPECT_TRUE(d2.Check());
        EXPECT_EQ(i, d2.Vptr());

        EXPECT_FALSE(test2.Pop(d2.Data(), ShmTestData::MaxTestDataSize, size));
        EXPECT_EQ(0, size);
    }
}


TEST_F(ShmBufferExTest, GetVptr) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    EXPECT_EQ(0, test.GetBeginVptr());
    EXPECT_EQ(0, test.GetEndVptr());
    ShmTestData d1(100, test.GetEndVptr());

    EXPECT_TRUE(test.Push(d1.Data(), d1.Size()));
    EXPECT_EQ(0, test.GetBeginVptr());
    EXPECT_EQ(ShmBufferRecord::RecordSize(d1.Size()), test.GetEndVptr());
    size_t free_size = test.FreeSize();
    while (free_size >= d1.Size()) {
        d1.SetVptr(test.GetEndVptr());
        EXPECT_TRUE(test.Push(d1.Data(), d1.Size()));
        free_size = test.FreeSize();
        EXPECT_EQ(0, test.GetBeginVptr());
        EXPECT_EQ(test.TotalCount() * ShmBufferRecord::RecordSize(d1.Size()), test.GetEndVptr());
    }
    test.SetOverflovBehavior(drop_old, false);

    for (int i = 0; i < 1000; i++) {
        d1.SetVptr(test.GetEndVptr());
        EXPECT_TRUE(test.Push(d1.Data(), d1.Size()));
        EXPECT_EQ(test.TotalDropCount() * ShmBufferRecord::RecordSize(d1.Size()), test.GetBeginVptr());
        EXPECT_EQ(test.TotalCount() * ShmBufferRecord::RecordSize(d1.Size()), test.GetEndVptr());
    }

}


static bool testRecordByVPtr(ShmBufferEx &test, uint64_t vptr) {
    const ShmBufferRecord *r = test.getRecord(vptr);
    const ShmTestData *pd = (const ShmTestData *) r->Data();
    EXPECT_EQ(vptr, pd->Vptr());
    EXPECT_TRUE(pd->Check());
    return pd->Check();
}


TEST_F(ShmBufferExTest, GetByVptrFull) {
    //При заполненом buffere
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    EXPECT_EQ(0, test.GetBeginVptr());
    EXPECT_EQ(0, test.GetEndVptr());

    test.SetOverflovBehavior(drop_old, false);
    ShmTestData td(111, test.GetEndVptr());
    for (int i = 0; i < 1000; i++) {
        td.SetVptr(test.GetEndVptr());
        EXPECT_TRUE(test.Push(td.Data(), td.Size()));
        EXPECT_EQ(test.TotalDropCount() * ShmBufferRecord::RecordSize(td.Size()), test.GetBeginVptr());
        EXPECT_EQ(test.TotalCount() * ShmBufferRecord::RecordSize(td.Size()), test.GetEndVptr());

        for (uint64_t j = test.GetBeginVptr();
             j < test.GetBeginVptr() + test.DataSize(); j += +ShmBufferRecord::RecordSize(td.Size())) {
            EXPECT_TRUE(testRecordByVPtr(test, j));
        }
    }
}

static void fill_data(ShmBufferEx &test) {
    test.SetOverflovBehavior(drop_old, false);
    for (int i = 0; i < 1000; i++) {
        ShmTestData td(100 + rand() % 100, test.GetEndVptr());
        EXPECT_TRUE(test.Push(td.Data(), td.Size()));
    }
    test.UpdateCounters();
}


TEST_F(ShmBufferExTest, GetNextVptr) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    fill_data(test);
    vptr_t pos = test.GetBeginVptr();
    while (pos < test.GetEndVptr()) {
        EXPECT_TRUE(testRecordByVPtr(test, pos));
        pos = test.GetNextVptr(pos);
    }
}

TEST_F(ShmBufferExTest, GetPrevVptr) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    fill_data(test);
    vptr_t pos = test.GetEndVptr();
    pos = test.GetPrevVptr(pos);
    while (pos >= test.GetBeginVptr()) {
        EXPECT_TRUE(testRecordByVPtr(test, pos));
        pos = test.GetPrevVptr(pos);
    }
}

TEST_F(ShmBufferExTest, GetNextVptrEmpty) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    vptr_t pos = test.GetBeginVptr();
    while (pos < test.GetEndVptr()) {
        EXPECT_TRUE(testRecordByVPtr(test, pos));
        pos = test.GetNextVptr(pos);
    }
}

TEST_F(ShmBufferExTest, GetPrevVptrEmpty) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    vptr_t pos = test.GetEndVptr();
    pos = test.GetPrevVptr(pos);
    while (pos >= test.GetBeginVptr()) {
        EXPECT_TRUE(testRecordByVPtr(test, pos));
        pos = test.GetPrevVptr(pos);
    }
}

static bool testGet(ShmBufferEx &test, vptr_t& vptr, direction_t dir) {
    ShmTestData pd;
    shm_record_size_t size=0;
    vptr_t saved_ptr=vptr;
    EXPECT_TRUE(test.Get(vptr,dir,pd.Data(),ShmTestData::MaxTestDataSize,size));
    EXPECT_EQ(saved_ptr, pd.Vptr());
    EXPECT_TRUE(pd.Check());
    return pd.Check();
}


TEST_F(ShmBufferExTest, GetNext) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    fill_data(test);
    vptr_t pos = test.GetBeginVptr();
    int count=0;
    while (pos < test.GetEndVptr()) {
        count++;
        EXPECT_TRUE(testGet(test, pos, direction_next));
    }
    EXPECT_EQ(count,test.Count());
}

TEST_F(ShmBufferExTest, GetPrev) {
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    fill_data(test);
    vptr_t pos = test.GetEndVptr();
    pos = test.GetPrevVptr(pos);
    int count=0;
    while (pos >= test.GetBeginVptr()) {
        count++;
        EXPECT_TRUE(testGet(test, pos, direction_prev));
    }
    EXPECT_EQ(count,test.Count());
}

TEST_F(ShmBufferExTest, CheckStatusNagios)
{
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    EXPECT_TRUE(is_file_exists(TestFileName()));
    {
        std::ostringstream s;
        EXPECT_EQ(status_ok, test.CheckStatus(s, status_nagios, false));
        EXPECT_EQ("Queue test_shm_test 0% full\n", s.str());
    }
    {
        std::ostringstream s;
        fill_data(test);
        EXPECT_EQ(status_critical, test.CheckStatus(s, status_nagios, false));
        EXPECT_EQ("Queue test_shm_test 99% full\nQueue test_shm_test has errors in last 300 seconds\n", s.str());
    }
}

TEST_F(ShmBufferExTest, CheckStatusSimple)
{
    ShmBufferEx test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    EXPECT_TRUE(is_file_exists(TestFileName()));
    {
        std::ostringstream s;
        EXPECT_EQ(status_ok, test.CheckStatus(s, status_nagios, false));
        EXPECT_EQ("Queue test_shm_test 0% full\n", s.str());
    }
    
    {
        std::ostringstream s;
        fill_data(test);
        EXPECT_EQ(status_critical, test.CheckStatus(s, status_nagios, false));
        EXPECT_EQ("Queue test_shm_test 99% full\nQueue test_shm_test has errors in last 300 seconds\n", s.str());
    }
}

