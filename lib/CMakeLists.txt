# Catch (a unit testing framework)
find_path (CATCH_INCLUDE_DIR catch.hpp HINTS catch/single_include)
if ((NOT CATCH_INCLUDE_DIR) OR (NOT EXISTS "${CATCH_INCLUDE_DIR}"))
    execute_process (
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    )
endif ()
set (CATCH_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/catch/single_include")
list (APPEND LCOV_REMOVE_LOCAL '${PROJECT_SOURCE_DIR}/lib/catch/*')
add_library (Catch INTERFACE)
target_include_directories (Catch SYSTEM INTERFACE "${CATCH_INCLUDE_DIR}")


# FakeIt (a mocking library)
find_path (
    FAKEIT_INCLUDE_DIR fakeit.hpp
    HINTS ${CMAKE_CURRENT_LIST_DIR}/fakeit/single_header/catch
)
if ((NOT FAKEIT_INCLUDE_DIR) OR (NOT EXISTS "${FAKEIT_INCLUDE_DIR}"))
    execute_process (
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    )
endif ()
set (FAKEIT_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/fakeit/single_header/catch")
list (APPEND LCOV_REMOVE_LOCAL '${PROJECT_SOURCE_DIR}/lib/fakeit/*')
add_library (FakeIt INTERFACE)
target_include_directories (FakeIt SYSTEM INTERFACE "${FAKEIT_INCLUDE_DIR}")


# fake_clock (a fake TrivialClock used for testing)
find_path (
    FAKE_CLOCK_INCLUDE_DIR fake_clock.hh
    HINTS ${CMAKE_CURRENT_LIST_DIR}/fake_clock
)
if ((NOT FAKE_CLOCK_INCLUDE_DIR) OR (NOT EXISTS "${FAKE_CLOCK_INCLUDE_DIR}"))
    execute_process (
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    )
endif ()
set (FAKE_CLOCK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/fake_clock")
list (APPEND LCOV_REMOVE_LOCAL '${PROJECT_SOURCE_DIR}/lib/fake_clock/*')
add_library (FakeClock STATIC "${FAKE_CLOCK_INCLUDE_DIR}/fake_clock.cc")
target_include_directories (FakeClock SYSTEM INTERFACE "${FAKE_CLOCK_INCLUDE_DIR}")


# MAVLink
find_path (
    MAVLINK_INCLUDE_DIR mavlink
    HINTS ${CMAKE_CURRENT_LIST_DIR}/mavlink
)
if ((NOT MAVLINK_INCLUDE_DIR) OR (NOT EXISTS "${MAVLINK_INCLUDE_DIR}"))
    execute_process (
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    )
endif ()
set (MAVLINK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/mavlink")
list (APPEND LCOV_REMOVE_LOCAL '${PROJECT_SOURCE_DIR}/lib/mavlink/*')
add_library (MAVLink INTERFACE)
target_include_directories (MAVLink SYSTEM INTERFACE "${MAVLINK_INCLUDE_DIR}")


# Pass lcov ignore patterns up to parent.
set (
    LCOV_REMOVE_PATTERNS
    ${LCOV_REMOVE_PATTERNS} "${LCOV_REMOVE_LOCAL}"
    PARENT_SCOPE
)


# Set MAVLink include path from MAVLINK_INCLUDE_PATH environment variable.
if ("$ENV{MAVLINK_INCLUDE_PATH}" STREQUAL "")
    set (
        MAVLINK_INCLUDE_PATH "mavlink/v2.0/common"
        CACHE INTERNAL "Use default MAVLink implementation."
    )
else ()
    set (
        MAVLINK_INCLUDE_PATH "$ENV{MAVLINK_INCLUDE_PATH}"
        CACHE INTERNAL "Copied from environment variable."
    )
endif ()
