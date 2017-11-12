#include <ShmFileBase.h>
#include "ShmBase.h"
#include "gtest/gtest.h"
#include "file_utils.h"
#include "log.h"

#define TEST_SHM_SIZE 4000U

class TestData {
public:
    static const char *get_name() {
        return "test_shm";
    }
};

class ShmBaseTest : public ShmFileBase<TestData> {

};

TEST_F(ShmBaseTest, Open) {
    ShmBase test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    EXPECT_TRUE(is_file_exists(TestFileName()));
    EXPECT_NE((uint8_t *) NULL, test.GetData());
    EXPECT_EQ(TEST_SHM_SIZE, test.GetSize());
}

TEST_F(ShmBaseTest, PageSize) {
    EXPECT_EQ(MEM_PAGE_SIZE, sysconf(_SC_PAGE_SIZE));
}

TEST_F(ShmBaseTest, OpenMirror) {
    ShmBase test;
    EXPECT_TRUE(test.OpenMirror(TestShmName(), MEM_PAGE_SIZE * 3, MEM_PAGE_SIZE));
    EXPECT_TRUE(is_file_exists(TestFileName()));
    EXPECT_NE((uint8_t *) NULL, test.GetData());
    EXPECT_EQ(MEM_PAGE_SIZE * 3, test.GetSize());
    uint8_t *data = test.GetData();
    data[MEM_PAGE_SIZE] = 33;
    EXPECT_EQ(33, data[MEM_PAGE_SIZE + MEM_PAGE_SIZE * 2]);
}

TEST_F(ShmBaseTest, OpenMirror2) {
    ShmBase test1;
    EXPECT_TRUE(test1.OpenMirror(TestShmName(), MEM_PAGE_SIZE * 3, MEM_PAGE_SIZE));
    ShmBase test2;
    EXPECT_TRUE(test2.OpenMirror(TestShmName(), MEM_PAGE_SIZE * 3, MEM_PAGE_SIZE));
    uint8_t *data1 = test1.GetData();
    uint8_t *data2 = test2.GetData();
    data1[0] = 33;
    EXPECT_EQ(33, data2[0]);
}

TEST_F(ShmBaseTest, OpenInvalidSize) {
    ShmBase test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    ScopedLogLevel ll(LOG_LEVEL_DISABLED);
    test.Close();
    EXPECT_FALSE(test.Open(TestShmName(), TEST_SHM_SIZE + 1));
}

TEST_F(ShmBaseTest, AlreadyOpen) {
    ShmBase test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    ScopedLogLevel ll(LOG_LEVEL_DISABLED);
    EXPECT_FALSE(test.Open(TestShmName(), TEST_SHM_SIZE));
}

TEST_F(ShmBaseTest, ReOpen) {
    ShmBase test1, test2;
    EXPECT_TRUE(test1.Open(TestShmName(), TEST_SHM_SIZE));
    EXPECT_TRUE(test2.Open(TestShmName(), 0));
}

TEST_F(ShmBaseTest, ReOpenFailed) {
    ShmBase test;
    ScopedLogLevel ll(LOG_LEVEL_DISABLED);
    EXPECT_FALSE(test.Open(TestShmName(), 0));
}

/*TEST_F(ShmFileBase, TooBigSize) {
    ShmBase test;
    EXPECT_FALSE(test.Open(TestShmName(), 1024UL * 1024UL * 1024UL * 1024UL * 10)); //10 Tb
}*/

TEST_F(ShmBaseTest, IsDeleted) {
    ShmBase test;
    EXPECT_TRUE(test.Open(TestShmName(), TEST_SHM_SIZE));
    EXPECT_FALSE(test.IsDeleted());
    EXPECT_EQ(0, remove(TestFileName()));
    EXPECT_TRUE(test.IsDeleted());
}
