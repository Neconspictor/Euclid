# - Try to find gli

IF (TARGET GLI)
    message(STATUS "GLI library is already imported")
    return()
ENDIF(TARGET GLI)

FIND_PATH(GLI_INCLUDE_DIR
        NAMES gli/gli.hpp
        HINTS ${EUCLID_BASE_LIBRARY_FOLDER}/gli/include
        DOC "The header include directory for the gli library"
        )

if (GLI_INCLUDE_DIR)
    SET(GLI_FOUND "TRUE")
else()
    SET(GLI_FOUND "FALSE")
endif(GLI_INCLUDE_DIR)

IF(GLI_FOUND)

    # print find libraries and the include directory
    IF(NOT GLI_FIND_QUIETLY) # GLI_FIND_QUIETLY gets set by cmake if the QUIET option is set in find_package
        MESSAGE(STATUS "Found GLI include directory: ${GLI_INCLUDE_DIR}")
    ENDIF(NOT GLI_FIND_QUIETLY)

    # create the library target
    add_library(GLI INTERFACE IMPORTED GLOBAL)
    set_target_properties(GLI PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLI_INCLUDE_DIR}")
    #if (MSVC)
    target_compile_definitions(GLI INTERFACE GLI_FORCE_CTOR_INIT)
    #target_compile_definitions(GLM INTERFACE GLM_HAS_CXX11_STL)

    #endif()

ELSE(GLI_FOUND)
    IF(GLI_FIND_REQUIRED)  # GLI_FIND_REQUIRED gets set by cmake if the REQUIRED option was set in find_package
        MESSAGE(FATAL_ERROR "Could not find all necessary parts of the GLI library")
    ENDIF(GLI_FIND_REQUIRED)
ENDIF(GLI_FOUND)