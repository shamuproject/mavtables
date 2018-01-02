# Catch (a unit testing framework)
find_path (CODECOV_MODULE_DIR Findcodecov.cmake HINTS codecov/cmake)
if ((NOT CODECOV_MODULE_DIR) OR (NOT EXISTS "${CODECOV_INCLUDE_DIR}"))
    execute_process (
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    )
endif ()
set (CODECOV_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}/codecov/cmake")

# Push module paths to parent.
set (CMAKE_MODULE_PATH 
    ${CODECOV_MODULE_DIR}
    ${CMAKE_MODULE_PATH}
    PARENT_SCOPE
)
