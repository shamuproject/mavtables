cmake_minimum_required (VERSION 3.0)
project (mavtables)
set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (VERSION_PATCH 1)

configure_file (
    "${PROJECT_SOURCE_DIR}/src/config.hpp.in"
    "${PROJECT_BINARY_DIR}/config.hpp"
    )

include_directories("${PROJECT_BINARY_DIR}")

set(HEADERS
        src/util.hpp
        )

set(SOURCES
        src/mavtables.cpp
        src/util.cpp
        )

add_executable (mavtables ${SOURCES} ${HEADERS})
