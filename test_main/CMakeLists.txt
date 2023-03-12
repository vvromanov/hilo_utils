cmake_minimum_required(VERSION 3.0.0)
set(TARGET test_main)
project(${TARGET})

enable_testing()
find_package(GTest REQUIRED)

add_library(${TARGET} STATIC
        test_main.cpp
        BenchTime.cpp
        BenchTime.h
        ShmFileBase.h
        TestBase.h
        )

target_include_directories(${TARGET} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(${TARGET}
        hilo_utils
        GTest::gtest
        GTest::gtest_main
        )
