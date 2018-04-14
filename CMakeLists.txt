cmake_minimum_required (VERSION 3.8.2) # required for C++17

# project
project (mavtables)
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_PATCH 5)

# set prefix
set (PREFIX "${CMAKE_INSTALL_PREFIX}")
configure_file (
    "${PROJECT_SOURCE_DIR}/mavtables.service.in"
    "${PROJECT_BINARY_DIR}/mavtables.service"
)

# compiler
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRE ON)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    add_compile_options ("$<$<CONFIG:DEBUG>:-Werror>")
    add_compile_options (-Wdocumentation -Wno-documentation-unknown-command)
    add_compile_options (-Weverything)
    add_compile_options (-Wno-c++98-compat-pedantic)
    add_compile_options (-Wno-disabled-macro-expansion)
    add_compile_options (-Wno-padded)
    add_compile_options (-Wno-packed)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options ("$<$<CONFIG:DEBUG>:-Werror>")
    add_compile_options (-Wall -Wextra -Wpedantic -Wno-psabi)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
endif()

# debug only
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (TEST_VIRTUAL virtual)
endif ()

# packages
add_subdirectory (cmake)
find_package (Threads)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_package (codecov)
endif ()
find_package (Boost 1.54 REQUIRED COMPONENTS filesystem program_options)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

# libraries
add_subdirectory (lib)

# mavtables
add_executable (mavtables src/mavtables.cpp)
target_link_libraries (mavtables
    MAVLink 
    PEGTL
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})
install (TARGETS mavtables DESTINATION bin CONFIGURATIONS Release)
install (
    FILES "${CMAKE_SOURCE_DIR}/examples/mavtables.conf"
    DESTINATION etc
    CONFIGURATIONS Release
)
install (
    FILES "${CMAKE_BINARY_DIR}/mavtables.service"
    DESTINATION lib/systemd/system
    CONFIGURATIONS Release
)

# unit_tests
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable (unit_tests test/unit_tests/CMakeLists.txt)
    target_link_libraries (unit_tests
        Catch
        FakeIt
        FakeClock
        MAVLink
        PEGTL
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT})
    add_test(NAME UnitTests COMMAND unit_tests)
endif ()

# add sources and evaluate test coverage
add_subdirectory (src)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory (test/unit_tests)
endif ()
add_subdirectory (doc)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_coverage (mavtables)
    add_coverage (unit_tests)
    coverage_evaluate ()
endif ()
