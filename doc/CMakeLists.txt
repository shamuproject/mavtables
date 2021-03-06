# From: https://vicrucann.github.io/tutorials/quick-cmake-doxygen/

# make documentation build as an option and set it to ON by default
option (BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
find_package (Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set (DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/../doc/Doxyfile.in)
    set (DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file (${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    # add doc target
    add_custom_target (doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
else (DOXYGEN_FOUND)
  message ("Doxygen needs to be installed to generate documentation.")
endif (DOXYGEN_FOUND)
