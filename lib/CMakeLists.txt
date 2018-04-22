# Catch (a unit testing framework)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_path (CATCH_INCLUDE_DIR catch.hpp HINTS catch/single_include)
    if ((NOT CATCH_INCLUDE_DIR) OR (NOT EXISTS "${CATCH_INCLUDE_DIR}"))
        execute_process (
            COMMAND git submodule update --init catch
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
        )
    endif ()
    set (CATCH_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/catch/single_include")
    list (APPEND LCOV_REMOVE_LOCAL '${PROJECT_SOURCE_DIR}/lib/catch/*')
    add_library (Catch INTERFACE)
    target_include_directories (Catch SYSTEM INTERFACE "${CATCH_INCLUDE_DIR}")
endif ()


# FakeIt (a mocking library)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_path (
        FAKEIT_INCLUDE_DIR fakeit.hpp
        HINTS ${CMAKE_CURRENT_LIST_DIR}/fakeit/single_header/catch
    )
    if ((NOT FAKEIT_INCLUDE_DIR) OR (NOT EXISTS "${FAKEIT_INCLUDE_DIR}"))
        execute_process (
            COMMAND git submodule update --init fakeit
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
        )
    endif ()
    set (
        FAKEIT_INCLUDE_DIR
        "${CMAKE_CURRENT_LIST_DIR}/fakeit/single_header/catch"
    )
    list (APPEND LCOV_REMOVE_LOCAL '${PROJECT_SOURCE_DIR}/lib/fakeit/*')
    add_library (FakeIt INTERFACE)
    target_include_directories (FakeIt SYSTEM INTERFACE "${FAKEIT_INCLUDE_DIR}")
endif ()


# fake_clock (a fake TrivialClock used for testing)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_path (
        FAKE_CLOCK_INCLUDE_DIR fake_clock.hh
        HINTS ${CMAKE_CURRENT_LIST_DIR}/fake_clock
    )
    if ((NOT FAKE_CLOCK_INCLUDE_DIR) OR 
        (NOT EXISTS "${FAKE_CLOCK_INCLUDE_DIR}"))
        execute_process (
            COMMAND git submodule update --init fake_clock
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
        )
    endif ()
    set (FAKE_CLOCK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/fake_clock")
    list (APPEND LCOV_REMOVE_LOCAL '${PROJECT_SOURCE_DIR}/lib/fake_clock/*')
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_library (FakeClock STATIC "${FAKE_CLOCK_INCLUDE_DIR}/fake_clock.cc")
        target_include_directories (
            FakeClock SYSTEM INTERFACE "${FAKE_CLOCK_INCLUDE_DIR}")
        target_compile_options(FakeClock PRIVATE "-Wno-reserved-id-macro")
    endif ()
endif ()


# Set MAVLink dialect from DIALECT environment variable.
if ("$ENV{DIALECT}" STREQUAL "")
    set (
        MAVLINK_DIALECT "ardupilotmega"
        CACHE INTERNAL "Use default (ArduPilot) MAVLink dialect."
    )
else ()
    set (
        MAVLINK_DIALECT "$ENV{DIALECT}"
        CACHE INTERNAL "Dialect set in environment variable."
    )
endif ()


# Set MAVLink include path from MDIR environment variable.
if ("$ENV{MDIR}" STREQUAL "")
    set (
        MAVLINK_INCLUDE_DIR
        "${PROJECT_SOURCE_DIR}/lib/mavlink/mavlink/v2.0/${MAVLINK_DIALECT}"
        CACHE INTERNAL "Use default MAVLink implementation."
    )
else ()
    set (
        MAVLINK_INCLUDE_DIR "$ENV{MDIR}/${MAVLINK_DIALECT}"
        CACHE INTERNAL "Copied from environment variable."
    )
endif ()
MESSAGE("Using MAVLink implementation at ${MAVLINK_INCLUDE_DIR}.")


# Pull down mavlink if it does not exist.
if (NOT EXISTS "${MAVLINK_INCLUDE_DIR}")
    execute_process (
        COMMAND git submodule update --init mavlink/mavlink/v2.0
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    )
endif ()

# Create MAVLink library.
list (APPEND LCOV_REMOVE_LOCAL '${MAVLINK_INCLUDE_DIR}../')
add_library (MAVLink INTERFACE)
target_include_directories (MAVLink SYSTEM INTERFACE "${MAVLINK_INCLUDE_DIR}")


# PEGTL (Parser Expression Grammar Template Library)
find_path (
    PEGTL_INCLUDE_DIR pegtl.hpp
    HINTS ${CMAKE_CURRENT_LIST_DIR}/pegtl/include/tao
)
if ((NOT PEGTL_INCLUDE_DIR) OR (NOT EXISTS "${PEGTL_INCLUDE_DIR}"))
    execute_process (
        COMMAND git submodule update --init pegtl
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    )
endif ()
set (PEGTL_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/pegtl/include/tao/")
list (APPEND LCOV_REMOVE_LOCAL '${PROJECT_SOURCE_DIR}/lib/pegtl/*')
add_library (PEGTL INTERFACE)
target_include_directories (PEGTL SYSTEM INTERFACE "${PEGTL_INCLUDE_DIR}")


# Pass lcov ignore patterns up to parent.
set (
    LCOV_REMOVE_PATTERNS
    ${LCOV_REMOVE_PATTERNS} "${LCOV_REMOVE_LOCAL}"
    PARENT_SCOPE
)
