set (SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/unit_tests.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_util.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_DNSLookupError.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_IPAddress.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MAVAddress.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MAVSubnet.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Packet.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_PacketVersion1.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_PacketVersion2.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_PacketParser.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Action.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Accept.cpp"
)
target_sources (unit_tests
    PRIVATE
        ${SOURCES}
)
