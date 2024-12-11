set(TARGET test_hilo_utils)
project(${TARGET})

enable_testing()
find_package(ZLIB REQUIRED)

add_executable(${TARGET}
    Convert.cpp
    Dictionary_test.cpp
    DumpUtilsTest.cpp
    FileUtils_Test.cpp
    Log_test.cpp
    ShmBase_Test.cpp
    ShmBuffer_Test.cpp
    ShmBufferEx_Test.cpp
    SysInfo.cpp
    TestCounter.cpp
    TestCounters.cpp
    TestCountersInit.cpp
    TestHistoryCounters.cpp
    TestTimeUtils.cpp
    TestUptime.cpp
    ClockOverride.h
)

target_link_libraries(${TARGET}
    test_main
    ${ZLIB_LIBRARY}
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

include(GoogleTest)
add_dependencies(check ${TARGET})

gtest_discover_tests(
    ${TARGET}
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    XML_OUTPUT_DIR gtest-reports
    DISCOVERY_MODE PRE_TEST
)
