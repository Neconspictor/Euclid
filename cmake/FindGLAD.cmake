# - Try to find glad

IF (GLAD_FOUND)
    message("Glad library is already imported")
    return()
ENDIF(GLAD_FOUND)


FIND_PATH(GLAD_INCLUDE_DIR
        NAMES glad/glad.h
        HINTS ${NEX_BASE_LIBRARY_FOLDER}/glad/include
        DOC "The header include directory for the glad library"
        )

FIND_PATH(GLAD_SOURCE_DIR
        NAMES glad.c
        HINTS ${NEX_BASE_LIBRARY_FOLDER}/glad/src
        DOC "The source directory for the glad library"
        )

if (GLAD_INCLUDE_DIR AND GLAD_SOURCE_DIR)
    SET(GLAD_FOUND TRUE)
else()
    SET(GLAD_FOUND FALSE)
endif(GLAD_INCLUDE_DIR AND GLAD_SOURCE_DIR)

IF(GLAD_FOUND)

    # print find libraries and the include directory
    IF(NOT GLAD_FIND_QUIETLY) # GLAD_FIND_QUIETLY gets set by cmake if the QUIET option is set in find_package
        MESSAGE(STATUS "Found GLAD include directory: ${GLAD_INCLUDE_DIR}")
        MESSAGE(STATUS "Found GLAD source directory: ${GLAD_SOURCE_DIR}")
    ENDIF(NOT GLAD_FIND_QUIETLY)

    # create the library target
    add_library(GLAD STATIC
            "libs/glad/include/glad/glad.h"
            "libs/glad/include/KHR/khrplatform.h"
            "libs/glad/src/glad.c"
            )

    # This is very important as Visual Studio doesn't compile the source files otherwise!
    set_source_files_properties(libs/glad/src/glad.c PROPERTIES LANGUAGE CXX)

    target_include_directories(GLAD PUBLIC ${NEX_BASE_LIBRARY_FOLDER}/glad/include)
    set_target_properties(GLAD PROPERTIES FOLDER lib)

ELSE(GLAD_FOUND)
    IF(GLAD_FIND_REQUIRED)  # GLAD_FIND_REQUIRED gets set by cmake if the REQUIRED option was set in find_package
        MESSAGE(FATAL_ERROR "Could not find all necessary parts of the glad library")
    ENDIF(GLAD_FIND_REQUIRED)
ENDIF(GLAD_FOUND)