set(TARGET hilo_utils)
project(${TARGET})

set(HILO_INCS_ALL ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE INTERNAL "HILO_INCS_ALL")

include_directories(
        ${HILO_INCS_ALL}
)

add_library(${TARGET} STATIC
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
        )

add_subdirectory(test_main)
add_subdirectory(tests)
