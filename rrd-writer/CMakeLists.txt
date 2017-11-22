set(TARGET rrd-writer)
project(${TARGET})
add_definitions(-DTX_PROGNAME="${TARGET}")

find_package(LibEv REQUIRED)
find_package(SystemdDaemon)
find_package(Systemd)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HILO_INCS_ALL}
        ${LIBEV_INCLUDE_DIR}
        ${SYSTEMD_INCLUDE_DIRS}
        ${SYSTEMDDAEMON_INCLUDE_DIR}
        ${FILE_WRITER_INCS_ALL}
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
        ${SYSTEMD_LIBRARIES}
        ${SYSTEMDDAEMON_LIBRARY}
        ${LIBEV_LIBRARIES}
        )

install(TARGETS ${TARGET} DESTINATION /opt/telexir/rrd-writer/bin COMPONENT rrd-writer)
