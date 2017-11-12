cmake_minimum_required(VERSION 2.8.4)
set(TARGET test_main)
project(${TARGET})


set(TEST_INCS_ALL
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/gtest
        CACHE INTERNAL "TEST_INCS_ALL")

include_directories(
        ${TEST_INCS_ALL}
)

add_library(${TARGET} STATIC
        test_main.cpp
        BenchTime.cpp
        BenchTime.h
        ShmFileBase.h
        TestBase.h
        gtest-all.cc)

target_link_libraries(${TARGET}
        hilo_utils
        )
