#include <CDictionary.h>
#include "CDictionary.h"
#include "gtest/gtest.h"

#define TEST_FILE "/tmp/file_utils.tst"
#define TEST_FILE_INVALID "/_##/"

TEST(Dictionary, Add) {
    CDictionary<> d;
    d.Clear();
    EXPECT_EQ(0, d.Add("e"));
    EXPECT_EQ(1, d.Add("f"));
    EXPECT_EQ(2, d.Add("g"));
    EXPECT_EQ(2, d.Add("g"));
    EXPECT_EQ(0, d.Add("e"));
    EXPECT_EQ(3, d.Add("a"));
    EXPECT_EQ(4, d.Add("b"));
}

TEST(Dictionary, TooManyItems) {
    CDictionary<> d;
    d.Clear();
    for (uint32_t i = 0; i < d.SIZE; i++) {
        char tmp[10];
        snprintf(tmp, sizeof(tmp), "%X", i);
        EXPECT_EQ(i, d.Add(tmp));
    }
    EXPECT_EQ(DICTIONARY_INVALID_INDEX, d.Add("xxx"));
}

TEST(Dictionary, TooLongNames) {
    CDictionary<> d;
    d.Clear();
    int str_size = 0;
    for (uint32_t i = 0; i < d.SIZE; i++) {
        char tmp[1000];
        snprintf(tmp, sizeof(tmp),"%0500X", i);
        str_size += strlen(tmp) + 1;
        if (str_size >= d.NAMES_STORAGE_SIZE) {
            EXPECT_EQ(DICTIONARY_INVALID_INDEX, d.Add(tmp));
            return;
        } else {
            EXPECT_EQ(i, d.Add(tmp));
        }
    }
    ADD_FAILURE();
}

TEST(Dictionary, LookupByName) {
    CDictionary<> d;
    d.Clear();
    EXPECT_EQ(DICTIONARY_INVALID_INDEX, d.Lookup("e"));
    EXPECT_EQ(0, d.Add("e"));
    EXPECT_EQ(1, d.Add("f"));
    EXPECT_EQ(2, d.Add("g"));
    EXPECT_EQ(2, d.Add("g"));
    EXPECT_EQ(0, d.Add("e"));
    EXPECT_EQ(3, d.Add("a"));
    EXPECT_EQ(4, d.Add("b"));
    EXPECT_EQ(DICTIONARY_INVALID_INDEX, d.Lookup("xxx"));
    EXPECT_EQ(0, d.Lookup("e"));
    EXPECT_EQ(1, d.Lookup("f"));
    EXPECT_EQ(2, d.Lookup("g"));
    EXPECT_EQ(3, d.Lookup("a"));
    EXPECT_EQ(4, d.Lookup("b"));
}

TEST(Dictionary, LookupById) {
    CDictionary<> d;
    d.Clear();
    EXPECT_EQ(DICTIONARY_INVALID_INDEX, d.Lookup("e"));
    EXPECT_EQ(0, d.Add("e"));
    EXPECT_EQ(1, d.Add("f"));
    EXPECT_EQ(2, d.Add("g"));
    EXPECT_EQ(2, d.Add("g"));
    EXPECT_EQ(0, d.Add("e"));
    EXPECT_EQ(3, d.Add("a"));
    EXPECT_EQ(4, d.Add("b"));
    EXPECT_EQ(NULL, d.Lookup(DICTIONARY_INVALID_INDEX));
    EXPECT_EQ(NULL, d.Lookup(-3));
    EXPECT_EQ(NULL, d.Lookup(5));
    EXPECT_STREQ("e", d.Lookup(0));
    EXPECT_STREQ("f", d.Lookup(1));
    EXPECT_STREQ("g", d.Lookup(2));
    EXPECT_STREQ("a", d.Lookup(3));
    EXPECT_STREQ("b", d.Lookup(4));
}

TEST(Dictionary, GetIndex) {
    CDictionary<> d;
    d.Clear();
    EXPECT_EQ(0, d.Add("e"));
    EXPECT_EQ(1, d.Add("f"));
    EXPECT_EQ(2, d.Add("g"));
    EXPECT_EQ(3, d.Add("a"));
    EXPECT_EQ(4, d.Add("b"));
    CDictionary<>::index_info_t ii;
    d.GetIndex(ii);
    EXPECT_EQ(5,ii.count);
    EXPECT_STREQ("a", d.Lookup(ii.index[0].id));
    EXPECT_STREQ("b", d.Lookup(ii.index[1].id));
    EXPECT_STREQ("e", d.Lookup(ii.index[2].id));
    EXPECT_STREQ("f", d.Lookup(ii.index[3].id));
    EXPECT_STREQ("g", d.Lookup(ii.index[4].id));
}

TEST(Dictionary, GetCategoryEmpty) {
    CDictionary<> d;
    d.Clear();
    EXPECT_EQ(0, d.Add("e"));
    EXPECT_EQ(1, d.Add("f"));
    EXPECT_EQ(2, d.Add("g"));
    EXPECT_EQ(3, d.Add("a"));
    EXPECT_EQ(4, d.Add("b"));
    CDictionary<>::index_info_t ii;
    d.GetCategory("", ii);
    EXPECT_EQ(5,ii.count);
}

TEST(Dictionary, GetCategoryNull) {
    CDictionary<> d;
    d.Clear();
    EXPECT_EQ(0, d.Add("e"));
    EXPECT_EQ(1, d.Add("f"));
    EXPECT_EQ(2, d.Add("g"));
    EXPECT_EQ(3, d.Add("a"));
    EXPECT_EQ(4, d.Add("b"));
    CDictionary<>::index_info_t ii;
    d.GetCategory(NULL, ii);
    EXPECT_EQ(5,ii.count);
}

TEST(Dictionary, GetCategory) {
    CDictionary<> d;
    d.Clear();
    EXPECT_EQ(0, d.Add("aaaa"));
    EXPECT_EQ(1, d.Add("b1"));
    EXPECT_EQ(2, d.Add("b2"));
    EXPECT_EQ(3, d.Add("b3"));
    EXPECT_EQ(4, d.Add("caaaa"));
    CDictionary<>::index_info_t ii;

    d.GetCategory("a", ii);
    EXPECT_EQ(1,ii.count);
    EXPECT_STREQ("aaaa", d.Lookup(ii.index[0].id));

    d.GetCategory("c", ii);
    EXPECT_EQ(1,ii.count);
    EXPECT_STREQ("caaaa", d.Lookup(ii.index[0].id));

    d.GetCategory("1", ii);
    EXPECT_EQ(0,ii.count);

    d.GetCategory("z", ii);
    EXPECT_EQ(0,ii.count);

    d.GetCategory("b", ii);
    EXPECT_EQ(3,ii.count);
    EXPECT_STREQ("b1", d.Lookup(ii.index[0].id));
    EXPECT_STREQ("b2", d.Lookup(ii.index[1].id));
    EXPECT_STREQ("b3", d.Lookup(ii.index[2].id));
}
