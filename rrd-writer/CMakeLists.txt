set(TARGET rrd-writer)
project(${TARGET})
add_definitions(-DTX_PROGNAME="${TARGET}")

find_package(LibEv REQUIRED)

if (NOT CYGWIN)
    find_package(Systemd)
endif ()

if (NOT SYSTEMD_FOUND)
    set(SYSD_LIB systemd-daemon)
    set(SYSD_INC ${SYSTEMDDAEMON_INCLUDE_DIR})
else ()
    set(SYSD_LIB ${SYSTEMD_LIBRARIES})
    set(SYSD_INC ${SYSTEMD_INCLUDE_DIRS})
endif ()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIBEV_INCLUDE_DIR}
    ${SYSD_INC}
)

add_executable(${TARGET}
    rrd-writer_main.cpp
    rrd-writer_options.h
    rrd-writer_options.cpp
    rrd_utils.cpp
    rrd_utils.h)

target_link_libraries(${TARGET}
    hilo_utils
    rrd
    ${SYSD_LIB}
    ${LIBEV_SHARED_LIB}
)

install(TARGETS ${TARGET} DESTINATION /opt/telexir/rrd-writer/bin COMPONENT rrd-writer)
