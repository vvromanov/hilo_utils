set(TARGET test_hilo_utils)
project(${TARGET})

find_package(ZLIB REQUIRED)

include_directories(
        ${TEST_INCS_ALL}
        ${UTILS_INCS_ALL}
        ${T1HA_INCS_ALL}
)

add_executable(${TARGET}
        Convert.cpp
        Log_test.cpp
        ShmBufferEx_Test.cpp
        ShmBuffer_Test.cpp
        TestCounter.cpp
        TestCountersInit.cpp
        TestCounters.cpp
        ShmBase_Test.cpp
        FileUtils_Test.cpp
        Dictionary_test.cpp
        TestTimeUtils.cpp
        DumpUtilsTest.cpp
)

target_link_libraries(${TARGET}
        test_main
        ${ZLIB_LIBRARY}
        )

add_test(${TARGET} ${TARGET})
if (NOT ${STANDALONE_BUILD})
    add_dependencies(check ${TARGET})
endif()
