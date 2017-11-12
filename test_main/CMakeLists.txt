cmake_minimum_required(VERSION 2.8.4)
set(TARGET test_main)
project(${TARGET})

find_package(GTest REQUIRED)

set(TEST_INCS_ALL
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GTEST_INCLUDE_DIRS}
        ${UTILS_INCS_ALL}
        CACHE INTERNAL "TEST_INCS_ALL")

include_directories(
        ${TEST_INCS_ALL}
)

add_library(${TARGET} STATIC
        test_main.cpp
        BenchTime.cpp
        BenchTime.h
        ShmFileBase.h
        TestBase.h)

target_link_libraries(${TARGET}
        ${GTEST_LIBRARIES}
        hilo_utils
        )
