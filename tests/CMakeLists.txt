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
)

target_link_libraries(${TARGET}
    test_main
    ${ZLIB_LIBRARY}
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

add_test(
    NAME ${TARGET} 
    COMMAND ${TARGET} --gtest_output=xml:../test_report_${TARGET}.xml
)
if (NOT ${STANDALONE_BUILD})
    add_dependencies(check ${TARGET})
endif ()
