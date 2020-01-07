# - Try to find ImGUI

IF (TARGET IMGUI)
    message("IMGUI library is already imported")
    return()
ENDIF(TARGET IMGUI)


FIND_PATH(IMGUI_ROOT_DIR
        NAMES imgui/imgui.h src/imgui.cpp
        HINTS ${EUCLID_BASE_LIBRARY_FOLDER}/imgui
        DOC "The root directory of the imgui library"
        )

if (IMGUI_ROOT_DIR)
    SET(IMGUI_FOUND TRUE)
else()
    SET(IMGUI_FOUND FALSE)
endif(IMGUI_ROOT_DIR)

IF(IMGUI_FOUND)

    # print find libraries and the include directory
    IF(NOT IMGUI_FIND_QUIETLY) # IMGUI_FIND_QUIETLY gets set by cmake if the QUIET option is set in find_package
        MESSAGE(STATUS "Found ImGUI root directory: ${IMGUI_ROOT_DIR}")
    ENDIF(NOT IMGUI_FIND_QUIETLY)

    set(IMGUI_SOURCES
            "${EUCLID_BASE_LIBRARY_FOLDER}/imgui/imgui/imconfig.h"
            "${EUCLID_BASE_LIBRARY_FOLDER}/imgui/imgui/imgui.h"
            "${EUCLID_BASE_LIBRARY_FOLDER}/imgui/imgui/imgui.cpp"
			"${EUCLID_BASE_LIBRARY_FOLDER}/imgui/imgui/imgui_draw.cpp"
			"${EUCLID_BASE_LIBRARY_FOLDER}/imgui/imgui/imgui_internal.h"
			"${EUCLID_BASE_LIBRARY_FOLDER}/imgui/imgui/imgui_stdlib.cpp"
			"${EUCLID_BASE_LIBRARY_FOLDER}/imgui/imgui/imgui_stdlib.h"
            "${EUCLID_BASE_LIBRARY_FOLDER}/imgui/imgui/imgui_widgets.cpp"
			"${EUCLID_BASE_LIBRARY_FOLDER}/imgui/imgui/imstb_rectpack.h"
			"${EUCLID_BASE_LIBRARY_FOLDER}/imgui/imgui/imstb_textedit.h"
			"${EUCLID_BASE_LIBRARY_FOLDER}/imgui/imgui/imstb_truetype.h"
    )

    # create the library target
    add_library(IMGUI STATIC
            ${IMGUI_SOURCES})

    # This is very important as Visual Studio doesn't compile the source files otherwise!
    #set_source_files_properties(lib/glad/src/glad.c PROPERTIES LANGUAGE CXX)

    target_include_directories(IMGUI PUBLIC ${EUCLID_BASE_LIBRARY_FOLDER}/imgui)
    set_target_properties(IMGUI PROPERTIES FOLDER lib)
	

ELSE(IMGUI_FOUND)
    IF(IMGUI_FIND_REQUIRED)  # IMGUI_FIND_REQUIRED gets set by cmake if the REQUIRED option was set in find_package
        MESSAGE(FATAL_ERROR "Could not find all necessary parts of the ImGUI library")
    ENDIF(IMGUI_FIND_REQUIRED)
ENDIF(IMGUI_FOUND)