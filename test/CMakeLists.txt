set (SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/unit_tests.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_util.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_DNSLookupError.cpp"
)
target_sources (unit_tests
    PRIVATE
        ${SOURCES}
)
