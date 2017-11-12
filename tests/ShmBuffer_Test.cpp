#include <ShmFileBase.h>
#include <ShmBuffer.h>

#define TEST_SHM_SIZE (MEM_PAGE_SIZE*4)

class TestData {
public:
    static const char *get_name() {
        return "test_shm";
    }
};

class ShmBufferTest : public ShmFileBase<TestData> {

};

TEST_F(ShmBufferTest, Open) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    EXPECT_TRUE(is_file_exists(TestFileName()));
    ScopedLogLevel ll(LOG_LEVEL_DISABLED);
    EXPECT_EQ(TEST_SHM_SIZE + MEM_PAGE_SIZE, test.GetSize());
}

TEST_F(ShmBufferTest, OpenInvalidSize) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    test.Close();
    ScopedLogLevel ll(LOG_LEVEL_DISABLED);
    EXPECT_FALSE(test.Open(TestShmName(), TEST_SHM_SIZE * 10));
}

TEST_F(ShmBufferTest, AlreadyOpen) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    ScopedLogLevel ll(LOG_LEVEL_DISABLED);
    EXPECT_FALSE(test.Open(TestShmName(), TEST_SHM_SIZE));
}

TEST_F(ShmBufferTest, ReOpen) {
    ShmBuffer test1, test2;
    EXPECT_TRUE(test1.Open(TestShmName(), TEST_SHM_SIZE));
    EXPECT_TRUE(test2.Open(TestShmName(), 0));
}

TEST_F(ShmBufferTest, ReOpenFailed) {
    ShmBuffer test;
    ScopedLogLevel ll(LOG_LEVEL_DISABLED);
    EXPECT_FALSE(test.Open(TestShmName(), 0));
}

/*TEST_F(ShmFileBase, TooBigSize) {
    ShmBase test;
    EXPECT_FALSE(test.Open(TestShmName(), 1024UL * 1024UL * 1024UL * 1024UL * 10)); //10 Tb
}*/

TEST_F(ShmBufferTest, IsDeleted) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    EXPECT_FALSE(test.IsDeleted());
    EXPECT_EQ(0, remove(TestFileName()));
    EXPECT_TRUE(test.IsDeleted());
}

TEST_F(ShmBufferTest, CapacityAfterCreate) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    EXPECT_EQ(MEM_PAGE_SIZE - 1, test.Capacity());
}

TEST_F(ShmBufferTest, FreeSizeAfterCreate) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    EXPECT_EQ(MEM_PAGE_SIZE - 1, test.FreeSize());
}

TEST_F(ShmBufferTest, DataSizeAfterCreate) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    EXPECT_EQ(0, test.DataSize());
}

TEST_F(ShmBufferTest, IsEmptyAfterCreate) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    EXPECT_TRUE(test.IsEmpty());
}

TEST_F(ShmBufferTest, DataSizeAfterWrite) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[100];
    EXPECT_TRUE(test.Write(data, sizeof(data)));
    EXPECT_EQ(sizeof(data), test.DataSize());
}

TEST_F(ShmBufferTest, IsEmptyAfterWriteRead) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[100];
    EXPECT_TRUE(test.Write(data, sizeof(data)));
    EXPECT_EQ(sizeof(data), test.Read(data, sizeof(data)));
    EXPECT_TRUE(test.IsEmpty());
}

TEST_F(ShmBufferTest, CantWrite) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[100];
    EXPECT_FALSE(test.Write(data, MEM_PAGE_SIZE + 1));
    EXPECT_EQ(0, test.DataSize());
    EXPECT_TRUE(test.IsEmpty());
}

TEST_F(ShmBufferTest, CantWriteToFull) { //иначе голова и хвост будут равны
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[100];
    EXPECT_FALSE(test.Write(data, MEM_PAGE_SIZE));
    EXPECT_EQ(0, test.DataSize());
    EXPECT_TRUE(test.IsEmpty());
}

TEST_F(ShmBufferTest, FillOneWrite) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[MEM_PAGE_SIZE];
    EXPECT_TRUE(test.Write(data, MEM_PAGE_SIZE - 1));
    EXPECT_EQ(MEM_PAGE_SIZE - 1, test.DataSize());
    EXPECT_EQ(0, test.FreeSize());
    EXPECT_FALSE(test.IsEmpty());
}

TEST_F(ShmBufferTest, FillMultipleWritesWithWrap) {
    ShmBuffer test;
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

TEST_F(ShmBufferTest, CantReadFromEmpty) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[1];
    EXPECT_EQ(0, test.Read(data, sizeof(data)));
    EXPECT_EQ(0, test.DataSize());
    EXPECT_TRUE(test.IsEmpty());
}


TEST_F(ShmBufferTest, CantReadTooManyData) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    uint8_t data[100];
    EXPECT_TRUE(test.Write(data, sizeof(data) - 1));
    EXPECT_EQ(sizeof(data) - 1, test.Read(data, sizeof(data)));
}

TEST_F(ShmBufferTest, ReadWriteCycle) {
    ShmBuffer test;
    EXPECT_TRUE(test.Open(TestShmName(), 1));
    for (int i = 0; i < 10000; i++) {
        uint8_t data1[11];
        uint8_t data2[11];
        for (int j = 0; j < sizeof(data1); j++) {
            data1[j] = rand() % 256;
        }
        EXPECT_TRUE(test.Write(data1, sizeof(data1)));
        EXPECT_EQ(sizeof(data2), test.Read(data2, sizeof(data2)));
        if (memcmp(data1, data2, sizeof(data1))!=0) {
            EXPECT_EQ(0, memcmp(data1, data2, sizeof(data1)));
        }
    }
}


TEST_F(ShmBufferTest, ReadWriteCycle2) {
    ShmBuffer test1;
    EXPECT_TRUE(test1.Open(TestShmName(), 1));
    ShmBuffer test2;
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
