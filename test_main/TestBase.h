#pragma once


#include <gtest/gtest.h>
#include <counters.h>

class TestBase: public ::testing::Test{
protected:
    virtual void TearDown() {
        ::testing::Test::SetUp();  // Sets up the base fixture first.
        EXPECT_TRUE(CountersClear());
    }
};



