set (SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/App.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Accept.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Action.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Call.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Chain.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/config_grammar.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ConfigParser.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Connection.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ConnectionPool.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/DNSLookupError.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Filesystem.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Filter.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/GoTo.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/If.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Interface.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/InterfaceThreader.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/InvalidPacketIDError.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/IPAddress.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Logger.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MAVAddress.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/mavlink.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MAVSubnet.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Options.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Packet.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketParser.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketVersion1.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketVersion2.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketQueue.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PartialSendError.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/QueuedPacket.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/RecursionError.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/RecursionGuard.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Reject.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Rule.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/semaphore.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/SerialInterface.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/SerialPort.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/UDPInterface.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/UDPSocket.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/UnixSerialPort.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/UnixSyscalls.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/UnixUDPSocket.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/utility.cpp"
)

set (HEADERS
    "${CMAKE_CURRENT_LIST_DIR}/Accept.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Action.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/AddressPool.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Call.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Chain.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/config_grammar.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/ConfigParser.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Connection.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/ConnectionFactory.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/ConnectionPool.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/DNSLookupError.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Filesystem.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Filter.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/GoTo.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/If.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Interface.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/InterfaceThreader.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/InvalidPacketIDError.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/IPAddress.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/macros.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/MAVAddress.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/mavlink.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/MAVSubnet.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Options.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Packet.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketParser.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketQueue.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketVersion1.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/PacketVersion2.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/parse_tree.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/PartialSendError.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/QueuedPacket.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/RecursionData.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/RecursionError.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/RecursionGuard.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Reject.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Rule.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/semaphore.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/SerialInterface.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/SerialPort.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/UDPInterface.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/UDPSocket.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/UnixSerialPort.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/UnixSyscalls.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/UnixUDPSocket.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/utility.hpp"
)

target_sources (mavtables 
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/mavtables.cpp"
        ${SOURCES}
        ${HEADERS}
)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_sources (unit_tests
        PRIVATE
            ${SOURCES}
            ${HEADERS}
    )
endif ()

target_include_directories (mavtables 
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}"
        "${PROJECT_BINARY_DIR}"
)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_include_directories (unit_tests
        PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}"
            "${PROJECT_BINARY_DIR}"
    )
endif ()

configure_file (
    "${PROJECT_SOURCE_DIR}/src/config.hpp.in"
    "${PROJECT_BINARY_DIR}/config.hpp"
)
