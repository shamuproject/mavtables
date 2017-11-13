cmake_minimum_required (VERSION 3.0.2)

set (CMAKE_CXX_STANDARD 17)

# project
project (mavtables)
set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (VERSION_PATCH 1)

add_subdirectory (lib)

# mavtables
add_executable (mavtables src/mavtables.cpp)

# unit_tests
add_executable (unit_tests test/CMakeLists.txt)
target_link_libraries (unit_tests Catch)
enable_testing()
add_test(NAME UnitTests COMMAND unit_tests)

add_subdirectory (src)
add_subdirectory (test)
