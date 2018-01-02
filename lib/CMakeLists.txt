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


# Pass lcov ignore patters up to parent.
set (
    LCOV_REMOVE_PATTERNS
    ${LCOV_REMOVE_PATTERNS} "${LCOV_REMOVE_LOCAL}"
    PARENT_SCOPE
)
