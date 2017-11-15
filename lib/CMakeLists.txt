# Catch (a unit testing framework)
find_path (CATCH_INCLUDE_DIR catch.hpp HINTS catch/single_include)
if ((NOT CATCH_INCLUDE_DIR) OR (NOT EXISTS "${CATCH_INCLUDE_DIR}"))
    message ("Downloading Catch")
    execute_process (
        COMMAND git submodule update --init
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    )
endif ()
set (CATCH_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/catch/single_include")
set (LCOV_REMOVE_PATTERNS
    ${LCOV_REMOVE_PATTERNS} '${PROJECT_SOURCE_DIR}/lib/catch/*'
    PARENT_SCOPE
)
add_library (Catch INTERFACE)
target_include_directories (Catch INTERFACE "${CATCH_INCLUDE_DIR}")
