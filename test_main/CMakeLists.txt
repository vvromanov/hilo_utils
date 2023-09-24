cmake_minimum_required(VERSION 3.0.0)
set(TARGET test_main)
project(${TARGET})

set_source_files_properties(gtest-all.cc PROPERTIES COMPILE_FLAGS "-Wno-missing-declarations")

add_library(${TARGET} STATIC
        test_main.cpp
        BenchTime.cpp
        BenchTime.h
        ShmFileBase.h
        TestBase.h
        gtest/gtest-all.cc
        )

target_include_directories(${TARGET} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/gtest
        )

target_link_libraries(${TARGET}
        hilo_utils
        )
