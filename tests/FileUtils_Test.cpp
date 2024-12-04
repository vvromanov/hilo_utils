#include "file_utils.h"
#include "gtest/gtest.h"

#define TEST_FILE "/tmp/file_utils.tst"
#define TEST_FILE_INVALID "/root/*"

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

TEST(FileUtils, DirExists) {
    EXPECT_TRUE(is_dir_exists("."));
    EXPECT_TRUE(is_dir_exists("/"));
}

TEST(FileUtils, DirNotExists) {
    remove(TEST_FILE);
    EXPECT_FALSE(is_dir_exists(TEST_FILE));
}


TEST(FileUtils, DirNotExistsInvalid) {
    remove(TEST_FILE);
    EXPECT_FALSE(is_dir_exists(""));
    EXPECT_FALSE(is_dir_exists(TEST_FILE_INVALID));
    EXPECT_FALSE(is_dir_exists("../../../../../../../../../some_file.txt"));
}

TEST(FileUtils, GetExt) {
    remove(TEST_FILE);
    EXPECT_STREQ(nullptr, get_ext(NULL));
    EXPECT_STREQ("", get_ext(""));
    EXPECT_STREQ("", get_ext("filename"));
    EXPECT_STREQ("ext", get_ext("filename.ext"));
    EXPECT_STREQ("ext", get_ext("dir/filename.ext"));
    EXPECT_STREQ("ext", get_ext("dir\\filename.ext"));
    EXPECT_STREQ("", get_ext("dir/filename"));
    EXPECT_STREQ("", get_ext("dir.ext/filename"));
    EXPECT_STREQ("", get_ext("dir.ext\\filename"));
}
