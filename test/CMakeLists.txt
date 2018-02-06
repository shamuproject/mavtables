set (SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/test_Accept.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Action.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/test_Action_comparison.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ActionResult.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Call.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Conditional.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_DNSLookupError.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_GoTo.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_IPAddress.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MAVAddress.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_mavlink.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MAVSubnet.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Packet.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_PacketParser.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_PacketVersion1.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_PacketVersion2.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Reject.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/test_Rule.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_util.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/unit_tests.cpp"
)
target_sources (unit_tests
    PRIVATE
        ${SOURCES}
)
