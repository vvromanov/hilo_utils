cmake_minimum_required(VERSION 2.8.7)

set(TARGET hilo_utils)
project(${TARGET})

enable_language(C)
enable_language(CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
find_package(LibEv REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

get_directory_property(hasParent PARENT_DIRECTORY)
if (hasParent)
    set(STANDALONE_BUILD 0)
else ()
    set(STANDALONE_BUILD 1)
    enable_testing()
    set(CTEST_CUSTOM_PRE_TEST "rm -vf /dev/shm/tx_*; rm -vf /dev/mqueue/tx_*")
    set(CTEST_CUSTOM_POST_TEST "rm -vf /dev/shm/tx_*; rm -vf /dev/mqueue/tx_*")

    if (CMAKE_CXX_COMPILER_ID MATCHES GNU)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat-security")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wwrite-strings")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmax-errors=20")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -Wmissing-declarations")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-qual")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-strict-aliasing")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finline-functions-called-once")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-packed-bitfield-compat")

        set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g3")
    endif ()

    if (COVERAGE)
        if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
            message(FATAL_ERROR "Coverage requires -DCMAKE_BUILD_TYPE=Debug Current value=${CMAKE_BUILD_TYPE}")
        endif ()

        message(STATUS "Setting coverage compiler flags")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb3 -O0 --coverage -fprofile-arcs -ftest-coverage")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}     -g -ggdb3 -O0 --coverage -fprofile-arcs -ftest-coverage")

        add_definitions(-DCOVERAGE_TEST)
    endif ()

    if (TRAVIS)
        add_definitions(-DNO_MLOCK)
    else ()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=leak -fstack-protector-strong -static-libasan")
    endif ()
    add_definitions(-DTX_VERSION_VERSION_STR="1.0.0")
    add_definitions(-DTX_VERSION_BUILD_STR="0")
    add_definitions(-DTX_VERSION_VARIANT="${CMAKE_BUILD_TYPE}")
endif ()

set(HILO_INCS_ALL ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE INTERNAL "HILO_INCS_ALL")

include_directories(
        ${HILO_INCS_ALL}
)

add_library(${TARGET} STATIC
        src/version.h
        src/ansi_escape_codes.h
        src/auto_mutex.h
        src/time_utils.h
        src/time_utils.cpp
        src/common_utils.h
        src/common_utils.cpp
        src/DumpUtils.cpp
        src/DumpUtils.h
        src/file_utils.h
        src/file_utils.cpp
        src/Status.cpp
        src/LibEv.h

        #Shared Memory
        src/ShmBase.h
        src/ShmBase.cpp
        src/ShmSimple.h
        src/ShmBuffer.cpp
        src/ShmBuffer.h
        src/ShmBufferEx.cpp
        src/ShmBufferEx.h
        src/ShmBufferRecord.h
        src/ShmChunks.h

        #Dictionary
        src/CDictionary.h

        #counters
        src/counter_t.h
        src/counters.cpp
        src/counters_base.cpp
        src/counters_base.h
        src/HistoryCounterData.cpp
        src/HistoryCounterData.h
        src/HistoryCounters.cpp
        src/HistoryCounters.h
        src/HistoryCounterLocal.h

        #log
        src/log.cpp
        src/log.h
        src/LogStorage.cpp
        src/LogStorage.h
        src/LogMonitorEv.h
        src/LogMonitorEvent.h
        src/LogMonitor.h
        src/LogMonitor.cpp
        src/LogOptions.cpp
        src/LogOptions.h

        #Monitoring
        src/Uptime.cpp
        src/Uptime.h
        src/SysInfoMemory.cpp
        src/SysInfoMemory.h
        src/SysInfoCpu.cpp
        src/SysInfoCpu.h
        src/Rusage.h
        src/Rusage.cpp
        src/SignalWatcher.cpp
        src/SignalWatcher.h
        )

target_link_libraries(${TARGET}
        pthread
        rt
        )

add_subdirectory(rrd-writer)
add_subdirectory(test_main)
add_subdirectory(tests)
