set(TARGET test_hilo_utils)
project(${TARGET})

enable_testing()
find_package(ZLIB REQUIRED)

add_executable(${TARGET}
    Convert.cpp
    Log_test.cpp
#    ShmBufferEx_Test.cpp
#    ShmBuffer_Test.cpp
#    TestCounter.cpp
#    TestCountersInit.cpp
#    TestCounters.cpp
#    ShmBase_Test.cpp
#    FileUtils_Test.cpp
#    Dictionary_test.cpp
    #TestTimeUtils.cpp
#    DumpUtilsTest.cpp
#    SysInfo.cpp
#    TestUptime.cpp
)

target_link_libraries(${TARGET}
    test_main
    ${ZLIB_LIBRARY}
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

add_test(${TARGET} ${TARGET})
if (NOT ${STANDALONE_BUILD})
    add_dependencies(check ${TARGET})
endif ()
