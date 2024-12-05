#include "file_utils.h"
#include "gtest/gtest.h"
#include <ftw.h>

#define TEST_FILE "/tmp/file_utils.tst"
#define TEST_FILE_INVALID "/root/*"
#define TEST_DIR "/tmp/level1/level2/level3"


TEST(FileUtils, RemoveTestFile) {
    FILE* fp = fopen(TEST_FILE, "ab+");
    fclose(fp);
    EXPECT_TRUE(remove_test_file(TEST_FILE));
    EXPECT_TRUE(remove_test_file(TEST_FILE));
    EXPECT_FALSE(remove_test_file(TEST_FILE_INVALID));
}

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
    EXPECT_FALSE(is_file_exists(""));
    EXPECT_FALSE(is_file_exists(TEST_FILE_INVALID));
    EXPECT_FALSE(is_file_exists("../../../../../../../../../some_file.txt"));
}

TEST(FileUtils, FileSize) {
    FILE* fp = fopen(TEST_FILE, "ab+");
    fclose(fp);
    size_t size=123;
    EXPECT_TRUE(get_file_size(TEST_FILE, size));
    EXPECT_EQ(0, size);
    fp = fopen(TEST_FILE, "ab+");
    fwrite("0123456789", 1, 10, fp);
    fclose(fp);
    EXPECT_TRUE(get_file_size(TEST_FILE, size));
    EXPECT_EQ(10, size);
}

TEST(FileUtils, FileSizeNotExists) {
    EXPECT_TRUE(remove_test_file(TEST_FILE));
    size_t size = 123;
    EXPECT_FALSE(get_file_size(TEST_FILE, size));
    EXPECT_EQ(0, size);
}

TEST(FileUtils, FileSizeInvalid) {
    size_t size = 123;
    EXPECT_FALSE(get_file_size("", size));
    EXPECT_EQ(0, size);

    size = 123;
    EXPECT_FALSE(get_file_size(TEST_FILE_INVALID, size));
    EXPECT_EQ(0, size);

    size = 123;
    EXPECT_FALSE(get_file_size("../../../../../../../../../some_file.txt", size));
    EXPECT_EQ(0, size);
}

TEST(FileUtils, DirExists) {
    EXPECT_TRUE(is_dir_exists("."));
    EXPECT_TRUE(is_dir_exists("/"));
}

TEST(FileUtils, DirNotExists) {
    EXPECT_TRUE(remove_test_file(TEST_FILE));
    EXPECT_FALSE(is_dir_exists(TEST_FILE));
}


TEST(FileUtils, DirNotExistsInvalid) {
    EXPECT_TRUE(remove_test_file(TEST_FILE));
    EXPECT_FALSE(is_dir_exists(""));
    EXPECT_FALSE(is_dir_exists(TEST_FILE_INVALID));
    EXPECT_FALSE(is_dir_exists("../../../../../../../../../some_file.txt"));
}


static int unlink_cb(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf)
{
    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

static bool rmrf(const char* path)
{
    return 0 == nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

TEST(FileUtils, mkdir_for_file) {
    rmrf(TEST_DIR);
    EXPECT_FALSE(is_dir_exists(TEST_DIR));
    EXPECT_TRUE(mkdir_for_file(TEST_DIR "/file.txt", 0755));
    EXPECT_TRUE(is_dir_exists(TEST_DIR));
    EXPECT_TRUE(rmrf(TEST_DIR));
}

TEST(FileUtils, GetExt) {
    EXPECT_TRUE(remove_test_file(TEST_FILE));
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
