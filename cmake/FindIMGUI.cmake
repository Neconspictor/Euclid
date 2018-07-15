# - Try to find ImGUI

IF (IMGUI_FOUND)
    message("IMGUI library is already imported")
    return()
ENDIF(IMGUI_FOUND)


FIND_PATH(IMGUI_ROOT_DIR
        NAMES imgui/imgui.h src/imgui.cpp
        HINTS ${NEX_BASE_LIBRARY_FOLDER}/imgui-lib
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
            "libs/imgui-lib/imgui/imconfig.h"
            "libs/imgui-lib/imgui/imgui.h"
            "libs/imgui-lib/imgui/imgui_internal.h"
            "libs/imgui-lib/imgui/stb_rect_pack.h"
            "libs/imgui-lib/imgui/stb_textedit.h"
            "libs/imgui-lib/imgui/stb_truetype.h"
            "libs/imgui-lib/imgui/imgui.cpp"
            "libs/imgui-lib/imgui/imgui_demo.cpp"
            "libs/imgui-lib/imgui/imgui_draw.cpp"
    )

    # create the library target
    add_library(IMGUI STATIC
            ${IMGUI_SOURCES})

    # This is very important as Visual Studio doesn't compile the source files otherwise!
    #set_source_files_properties(lib/glad/src/glad.c PROPERTIES LANGUAGE CXX)

    target_include_directories(IMGUI PUBLIC libs/imgui-lib)
    set_target_properties(IMGUI PROPERTIES FOLDER lib)

ELSE(IMGUI_FOUND)
    IF(IMGUI_FIND_REQUIRED)  # IMGUI_FIND_REQUIRED gets set by cmake if the REQUIRED option was set in find_package
        MESSAGE(FATAL_ERROR "Could not find all necessary parts of the ImGUI library")
    ENDIF(IMGUI_FIND_REQUIRED)
ENDIF(IMGUI_FOUND)