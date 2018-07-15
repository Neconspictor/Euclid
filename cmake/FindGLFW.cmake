# - Try to find GLFW

IF (TARGET GLFW)
    message("GLFW library is already imported")
    return()
ENDIF(TARGET GLFW)

FIND_PATH(GLFW_INCLUDE_DIR
        NAMES   GLFW/glfw3.h
                GLFW/glfw3native.h
        HINTS ${NEX_BASE_LIBRARY_FOLDER}/glfw-3.2.1/include
        DOC "The header include directory for the GLFW 3.2.1 library"
        )

get_library_path("glfw-3.2.1" GLFW_LIB_PATH)

FIND_LIBRARY( GLFW_LIBRARY_DEBUG glfw3
        DOC "The debug version of the static glfw 3.2.1 library. On Windows this is a *.lib file, on Linux, Unix and
        Mac OS a *.a  file."
        ${GLFW_LIB_PATH}/debug
        )

FIND_LIBRARY( GLFW_LIBRARY_RELEASE glfw3
        DOC "The release version of the static glfw 3.2.1 library. On Windows this is a *.lib file, on Linux, Unix and
        Mac OS a *.a  file."
        ${GLFW_LIB_PATH}/release
        )

if (GLFW_INCLUDE_DIR AND GLFW_LIBRARY_DEBUG AND GLFW_LIBRARY_RELEASE)
    SET(GLFW_FOUND TRUE)
else()
    SET(GLFW_FOUND FALSE)
endif(GLFW_INCLUDE_DIR AND GLFW_LIBRARY_DEBUG AND GLFW_LIBRARY_RELEASE)


IF(GLFW_FOUND)

    # print find libraries and the include directory
    IF(NOT GLFW_FIND_QUIETLY) # GLFW_FIND_QUIETLY gets set by cmake if the QUIET option is set in find_package
        MESSAGE(STATUS "Found GLFW 3.2.1 include directory: ${GLFW_INCLUDE_DIR}")
        MESSAGE(STATUS "Found GLFW 3.2.1 static debug library: ${GLFW_LIBRARY_DEBUG}")
        MESSAGE(STATUS "Found GLFW 3.2.1 static release library: ${GLFW_LIBRARY_RELEASE}")
    ENDIF(NOT GLFW_FIND_QUIETLY)

    # create the library target
    add_library(GLFW STATIC IMPORTED GLOBAL)
    set_target_properties(GLFW PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}")
    set_target_properties(GLFW PROPERTIES IMPORTED_LOCATION_DEBUG "${GLFW_LIBRARY_DEBUG}")
    set_target_properties(GLFW PROPERTIES IMPORTED_LOCATION_RELEASE "${GLFW_LIBRARY_RELEASE}")

ELSE(GLFW_FOUND)
    IF(GLFW_FIND_REQUIRED)  # GLM_FIND_REQUIRED gets set by cmake if the REQUIRED option was set in find_package
        MESSAGE(FATAL_ERROR "Could not find all necessary parts of the GLFW 3.2.1 library")
    ENDIF(GLFW_FIND_REQUIRED)
ENDIF(GLFW_FOUND)