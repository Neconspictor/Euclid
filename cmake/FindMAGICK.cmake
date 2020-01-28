# - Try to find Magick++

IF (TARGET Magick)
    message("Magick library is already imported")
    return()
ENDIF(TARGET Magick)

FIND_PATH(MAGICK_INCLUDE_DIR
        NAMES   Magick++.h
        HINTS ${EUCLID_BASE_LIBRARY_FOLDER}/Magick++/include
        DOC "The header include directory for the Magick++ library"
        )

get_library_path("Magick++" MAGICK_LIB_PATH)

FIND_LIBRARY( MAGICK_LIBRARY_DEBUG CORE_DB_Magick++_
        DOC "The debug version of the static Magick library. On Windows this is a *.lib file, on Linux, Unix and
        Mac OS a *.a  file."
        ${MAGICK_LIB_PATH}
        )

FIND_LIBRARY( MAGICK_LIBRARY_RELEASE CORE_DB_Magick++_
        DOC "The release version of the static Magick library. On Windows this is a *.lib file, on Linux, Unix and
        Mac OS a *.a  file."
        ${MAGICK_LIB_PATH}
        )
		
		
FIND_LIBRARY( MAGICKCORE_LIBRARY_DEBUG CORE_DB_MagickCore_
        DOC "The debug version of the static Magick library. On Windows this is a *.lib file, on Linux, Unix and
        Mac OS a *.a  file."
        ${MAGICK_LIB_PATH}
)		
		
FIND_LIBRARY( MAGICKWAND_LIBRARY_DEBUG CORE_DB_MagickWand_
	DOC "The debug version of the static Magick library. On Windows this is a *.lib file, on Linux, Unix and
	Mac OS a *.a  file."
	${MAGICK_LIB_PATH}
)

FIND_LIBRARY( MAGICK_CODERS_LIBRARY_DEBUG CORE_DB_coders_
	DOC "The debug version of the static Magick library. On Windows this is a *.lib file, on Linux, Unix and
	Mac OS a *.a  file."
	${MAGICK_LIB_PATH}
)

FIND_LIBRARY( MAGICK_FILTERS_LIBRARY_DEBUG CORE_DB_filters_
	DOC "The debug version of the static Magick library. On Windows this is a *.lib file, on Linux, Unix and
	Mac OS a *.a  file."
	${MAGICK_LIB_PATH}
)


if (MAGICK_INCLUDE_DIR AND MAGICK_LIBRARY_DEBUG AND MAGICK_LIBRARY_RELEASE)
    SET(MAGICK_FOUND TRUE)
else()
    SET(MAGICK_FOUND FALSE)
endif(MAGICK_INCLUDE_DIR AND MAGICK_LIBRARY_DEBUG AND MAGICK_LIBRARY_RELEASE)


IF(MAGICK_FOUND)

    # print find libraries and the include directory
    IF(NOT MAGICK_FIND_QUIETLY) # MAGICK_FIND_QUIETLY gets set by cmake if the QUIET option is set in find_package
        MESSAGE(STATUS "Found Magick include directory: ${MAGICK_INCLUDE_DIR}")
        MESSAGE(STATUS "Found Magick static debug library: ${MAGICK_LIBRARY_DEBUG}")
        MESSAGE(STATUS "Found Magick static release library: ${MAGICK_LIBRARY_RELEASE}")
    ENDIF(NOT MAGICK_FIND_QUIETLY)

    # create the library target
    add_library(MAGICK STATIC IMPORTED GLOBAL)
    set_target_properties(MAGICK PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MAGICK_INCLUDE_DIR}")
    set_target_properties(MAGICK PROPERTIES IMPORTED_LOCATION_DEBUG "${MAGICK_LIBRARY_DEBUG} ${MAGICKCORE_LIBRARY_DEBUG} ${MAGICKWAND_LIBRARY_DEBUG} ${MAGICK_CODERS_LIBRARY_DEBUG} ${MAGICK_FILTERS_LIBRARY_DEBUG}")
    set_target_properties(MAGICK PROPERTIES IMPORTED_LOCATION_RELEASE "${MAGICK_LIBRARY_RELEASE}")
    set_target_properties(MAGICK PROPERTIES IMPORTED_LOCATION_RELWITHDEBINFO "${MAGICK_LIBRARY_RELEASE}")
    set_target_properties(MAGICK PROPERTIES IMPORTED_LOCATION_MINSIZEREL "${MAGICK_LIBRARY_RELEASE}")

ELSE(MAGICK_FOUND)
    IF(MAGICK_FIND_REQUIRED)  # GLM_FIND_REQUIRED gets set by cmake if the REQUIRED option was set in find_package
        MESSAGE(FATAL_ERROR "Could not find all necessary parts of the Magick library")
    ENDIF(MAGICK_FIND_REQUIRED)
ENDIF(MAGICK_FOUND)