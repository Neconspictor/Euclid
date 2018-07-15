# - Try to find stb

IF (TARGET STB)
    message("STB library is already imported")
    return()
ENDIF(TARGET STB)

FIND_PATH(STB_INCLUDE_DIR
        NAMES stb/stb_image.h
        HINTS ${NEX_BASE_LIBRARY_FOLDER}/stb/include
        DOC "The header include directory for the stb library"
        )

if (STB_INCLUDE_DIR)
    SET(STB_LIBRARY_FOUND TRUE)
else()
    SET(STB_LIBRARY_FOUND FALSE)
endif(STB_INCLUDE_DIR)


IF(STB_LIBRARY_FOUND)
    set(STB_LIBRARY_FOUND TRUE)
    # print find libraries and the include directory
    IF(NOT STB_FIND_QUIETLY) # STB_FIND_QUIETLY gets set by cmake if the QUIET option is set in find_package
        MESSAGE(STATUS "Found stb include directory: ${STB_INCLUDE_DIR}")
    ENDIF(NOT STB_FIND_QUIETLY)

    # create the library target
    add_library(STB INTERFACE)
    set_target_properties(STB PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${STB_INCLUDE_DIR}")

ELSE(STB_LIBRARY_FOUND)
    IF(STB_FIND_REQUIRED)  # STB_FIND_REQUIRED gets set by cmake if the REQUIRED option was set in find_package
        MESSAGE(FATAL_ERROR "Could not find all necessary parts of the stb library")
    ENDIF(STB_FIND_REQUIRED)
ENDIF(STB_LIBRARY_FOUND)