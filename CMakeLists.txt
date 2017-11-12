cmake_minimum_required (VERSION 3.0)
project (mavtables)
set (mavtables_VERSION_MAJOR 0)
set (mavtables_VERSION_MINOR 0)
set (mavtables_VERSION_PATCH 1)

configure_file (
    "${PROJECT_SOURCE_DIR}/src/config.hpp.in"
    "${PROJECT_BINARY_DIR}/config.hpp"
    )

include_directories("${PROJECT_BINARY_DIR}")

add_executable (mavtables src/mavtables.cpp)
