cmake_minimum_required (VERSION 3.3)

# project
project (mavtables)
set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (VERSION_PATCH 10)

# compiler
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRE ON)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    add_compile_options ("$<$<CONFIG:DEBUG>:-Werror>")
    add_compile_options (-Wdocumentation -Wno-documentation-unknown-command)
    add_compile_options (-Weverything)
    add_compile_options (-Wno-c++98-compat)
    add_compile_options (-Wno-c++98-c++11-compat-pedantic)
    add_compile_options (-Wno-disabled-macro-expansion)
    add_compile_options (-Wno-padded)
    add_compile_options (-Wno-packed)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options ("$<$<CONFIG:DEBUG>:-Werror>")
    add_compile_options (-Wall -Wextra -Wpedantic)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
endif()

# packages
add_subdirectory (cmake)
find_package (codecov)
find_package (Boost 1.56 REQUIRED COMPONENTS)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

# libraries
add_subdirectory (lib)

# mavtables
add_executable (mavtables src/mavtables.cpp)
target_link_libraries (mavtables ${Boost_LIBRARIES} MAVLink)
install (TARGETS mavtables DESTINATION bin)

# unit_tests
add_executable (unit_tests test/CMakeLists.txt)
target_link_libraries (unit_tests Catch FakeIt ${Boost_LIBRARIES} MAVLink)
add_test(NAME UnitTests COMMAND unit_tests)

# add sources and evaluate test coverage
add_subdirectory (src)
add_subdirectory (test)
add_subdirectory (doc)
add_coverage (mavtables)
add_coverage (unit_tests)
coverage_evaluate ()
