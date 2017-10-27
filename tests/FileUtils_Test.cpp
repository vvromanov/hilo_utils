#include "file_utils.h"
#include "gtest/gtest.h"

#define TEST_FILE "/tmp/file_utils.tst"
#define TEST_FILE_INVALID "/_##/"

TEST(FileUtils, FileExists) {
    FILE *fp = fopen(TEST_FILE, "ab+");
    fclose(fp);
    EXPECT_TRUE(is_file_exists(TEST_FILE));
}

TEST(FileUtils, FileNotExists) {
    remove(TEST_FILE);
    EXPECT_FALSE(is_file_exists(TEST_FILE));
}


TEST(FileUtils, FileNotExistsInvalid) {
    remove(TEST_FILE);
    EXPECT_FALSE(is_file_exists(""));
    EXPECT_FALSE(is_file_exists(TEST_FILE_INVALID));
    EXPECT_FALSE(is_file_exists("../../../../../../../../../some_file.txt"));
}
