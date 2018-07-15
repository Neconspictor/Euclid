# - Try to find glm

IF (GLM_FOUND)
    message("GLM library is already imported")
    return()
ENDIF(GLM_FOUND)

FIND_PATH(GLM_INCLUDE_DIR
        NAMES glm/glm.hpp
        HINTS ${NEX_BASE_LIBRARY_FOLDER}/glm
        DOC "The header include directory for the glm library"
        )

if (GLM_INCLUDE_DIR)
    SET(GLM_FOUND TRUE)
else()
    SET(GLM_FOUND FALSE)
endif(GLM_INCLUDE_DIR)

IF(GLM_FOUND)

    # print find libraries and the include directory
    IF(NOT Glm_FIND_QUIETLY) # GLM_FIND_QUIETLY gets set by cmake if the QUIET option is set in find_package
        MESSAGE(STATUS "Found GLM include directory: ${GLM_INCLUDE_DIR}")
    ENDIF(NOT Glm_FIND_QUIETLY)

    # create the library target
    add_library(GLM INTERFACE IMPORTED GLOBAL)
    set_target_properties(GLM PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLM_INCLUDE_DIR}")

ELSE(GLM_FOUND)
    IF(Glm_FIND_REQUIRED)  # GLM_FIND_REQUIRED gets set by cmake if the REQUIRED option was set in find_package
        MESSAGE(FATAL_ERROR "Could not find all necessary parts of the GLM library")
    ENDIF(Glm_FIND_REQUIRED)
ENDIF(GLM_FOUND)