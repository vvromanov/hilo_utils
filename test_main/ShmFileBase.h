#pragma once

#include <gtest/gtest.h>
#include <ShmBase.h>
#include <file_utils.h>
#include "TestBase.h"

template<typename T>
class ShmFileBase : public TestBase {
    std::string filename;
    std::string shm_name;
protected:
    constexpr static const char* suffix="_test";

    ShmFileBase() {
        shm_name=T::get_name();
        shm_name += suffix;

        filename = SHM_LOCATION;
        filename += shm_name;
        shm_name = "/" + shm_name;
    }

    const char *TestFileName() const {
        return filename.c_str();
    }

    const char *TestShmName() const {
        return shm_name.c_str();
    }

    virtual void SetUp() {
        ::testing::Test::SetUp();  // Sets up the base fixture first.
        TearDown();
        EXPECT_FALSE(is_file_exists(filename.c_str()));
    }

    virtual void TearDown() {
        TestBase::TearDown();
        ASSERT_TRUE(remove_test_file(filename.c_str()));
    }
};

