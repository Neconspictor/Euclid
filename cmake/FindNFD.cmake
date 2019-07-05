# - Try to find nfd

IF (TARGET NFD)
    message("NFD library is already imported")
    return()
ENDIF(TARGET NFD)


FIND_PATH(NFD_ROOT_DIR
        NAMES include/nfd/nfd.h
        HINTS ${NEX_BASE_LIBRARY_FOLDER}/nfd
        DOC "The root directory of the nfd library"
        )

if (NFD_ROOT_DIR)
    SET(NFD_FOUND TRUE)
else()
    SET(NFD_FOUND FALSE)
endif(NFD_ROOT_DIR)

IF(NFD_FOUND)

    # print find libraries and the include directory
    IF(NOT NFD_FIND_QUIETLY) # NFD_FIND_QUIETLY gets set by cmake if the QUIET option is set in find_package
        MESSAGE(STATUS "Found NFD root directory: ${NFD_ROOT_DIR}")
    ENDIF(NOT NFD_FIND_QUIETLY)

    set(NFD_SOURCES
            "${NEX_BASE_LIBRARY_FOLDER}/nfd/include/nfd/nfd.h"
            "${NEX_BASE_LIBRARY_FOLDER}/nfd/src/common.h"
            "${NEX_BASE_LIBRARY_FOLDER}/nfd/src/nfd_common.cpp"
            "${NEX_BASE_LIBRARY_FOLDER}/nfd/src/nfd_common.h"
            #"${NEX_BASE_LIBRARY_FOLDER}/nfd/src/nfd_gtk.c"
            "${NEX_BASE_LIBRARY_FOLDER}/nfd/src/nfd_win.cpp"
            #"${NEX_BASE_LIBRARY_FOLDER}/nfd/src/nfd_zenity.c"
            "${NEX_BASE_LIBRARY_FOLDER}/nfd/src/simple_exec.h"
    )

    # create the library target
    add_library(NFD STATIC
            ${NFD_SOURCES})

    # This is very important as Visual Studio doesn't compile the source files otherwise!
    #set_source_files_properties(lib/glad/src/glad.c PROPERTIES LANGUAGE CXX)

    target_include_directories(NFD PUBLIC 
        ${NEX_BASE_LIBRARY_FOLDER}/nfd/include
    )
    set_target_properties(NFD PROPERTIES FOLDER lib)

ELSE(NFD_FOUND)
    IF(NFD_FIND_REQUIRED)  # NFD_FIND_REQUIRED gets set by cmake if the REQUIRED option was set in find_package
        MESSAGE(FATAL_ERROR "Could not find all necessary parts of the NFD library")
    ENDIF(NFD_FIND_REQUIRED)
ENDIF(NFD_FOUND)