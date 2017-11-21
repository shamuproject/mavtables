cmake_minimum_required (VERSION 3.3)

# project
project (mavtables)
set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (VERSION_PATCH 2)

# compiler
set (CMAKE_CXX_STANDARD 17)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    add_compile_options (-Werror -Weverything -Wno-c++98-compat)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options (-Werror -Wall -Wextra -Wpedantic)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
endif()

# packages
add_subdirectory (cmake)
find_package (codecov)

# libraries
add_subdirectory (lib)

# mavtables
add_executable (mavtables src/mavtables.cpp)
install (TARGETS mavtables DESTINATION bin)

# unit_tests
add_executable (unit_tests test/CMakeLists.txt)
target_link_libraries (unit_tests Catch FakeIt)
add_test(NAME UnitTests COMMAND unit_tests)

# add sources and evaluate test coverage
add_subdirectory (src)
add_subdirectory (test)
add_subdirectory (doc)
add_coverage (mavtables)
add_coverage (unit_tests)
coverage_evaluate ()
