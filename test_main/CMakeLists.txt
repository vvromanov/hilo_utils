cmake_minimum_required(VERSION 3.0.0)
set(TARGET test_main)
project(${TARGET})

enable_testing()
if (CYGWIN)
    set_source_files_properties(gtest-all.cc PROPERTIES COMPILE_FLAGS "-Wno-missing-declarations")
    SET(CYGWIN_GTEST_SRC gtest/gtest-all.cc)
    SET(CYGWIN_GTEST_INC ${CMAKE_CURRENT_SOURCE_DIR}/gtest)
else (CYGWIN)
    find_package(GTest REQUIRED)
    SET(NO_CYGWIN_GTEST_LIB GTest::gtest GTest::gtest_main)
endif (CYGWIN)

add_library(${TARGET} STATIC
    test_main.cpp
    BenchTime.cpp
    BenchTime.h
    ShmFileBase.h
    TestBase.h
    ${CYGWIN_GTEST_SRC}
)

target_include_directories(${TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CYGWIN_GTEST_INC}
)

target_link_libraries(${TARGET}
    hilo_utils
    ${NO_CYGWIN_GTEST_LIB}
)
