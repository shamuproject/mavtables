set (SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/unit_tests.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_util.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_DNSLookupError.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_IPAddress.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MAVAddress.cpp"
)
target_sources (unit_tests
    PRIVATE
        ${SOURCES}
)
