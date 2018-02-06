set (SOURCES
    # "${CMAKE_CURRENT_LIST_DIR}/Accept.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Action.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ActionResult.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Call.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Conditional.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Connection.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Chain.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/DNSLookupError.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/GoTo.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/IPAddress.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MAVAddress.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/mavlink.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MAVSubnet.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Packet.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketParser.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketVersion1.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketVersion2.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/RecursionChecker.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Reject.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Rule.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/util.cpp"
)
set (HEADERS
    # "${CMAKE_CURRENT_LIST_DIR}/Accept.hpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Action.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/ActionResult.hpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Call.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Conditional.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Connection.hpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Chain.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/DNSLookupError.hpp"
    # "${CMAKE_CURRENT_LIST_DIR}/GoTo.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/IPAddress.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/macros.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/MAVAddress.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/mavlink.hpp.in"
    "${CMAKE_CURRENT_LIST_DIR}/MAVSubnet.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Packet.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketParser.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketVersion1.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketVersion2.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/RecursionChecker.hpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Reject.hpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Rule.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/util.hpp"
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
    "${PROJECT_SOURCE_DIR}/src/mavlink.hpp.in"
    "${PROJECT_BINARY_DIR}/mavlink.hpp"
)
