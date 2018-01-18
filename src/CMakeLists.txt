set (SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/util.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/DNSLookupError.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/IPAddress.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MAVAddress.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MAVSubnet.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Packet.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketVersion1.cpp"
)
set (HEADERS
    "${CMAKE_CURRENT_LIST_DIR}/macros.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/util.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/DNSLookupError.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/IPAddress.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/MAVAddress.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/MAVSubnet.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Connection.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Packet.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketVersion1.hpp"
)
target_sources (mavtables 
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/mavtables.cpp"
        ${SOURCES}
        ${HEADERS}
)
target_sources (unit_tests
    PRIVATE
        ${SOURCES}
        ${HEADERS}
)
target_include_directories (mavtables 
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}"
        "${PROJECT_BINARY_DIR}"
)
target_include_directories (unit_tests
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}"
        "${PROJECT_BINARY_DIR}"
)
configure_file (
    "${PROJECT_SOURCE_DIR}/src/config.hpp.in"
    "${PROJECT_BINARY_DIR}/config.hpp"
)
configure_file (
    "${PROJECT_SOURCE_DIR}/src/mavlink.h.in"
    "${PROJECT_BINARY_DIR}/mavlink.h"
)
