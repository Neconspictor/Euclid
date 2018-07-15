# - Try to find Assimp
# Once done, this will define
#
# ASSIMP_FOUND - system has Assimp

IF (${ASSIMP_FOUND})
  message("Assimp library is already imported")
  return()
ENDIF(${ASSIMP_FOUND})

FIND_PATH( ASSIMP_INCLUDE_DIR
        NAMES assimp/mesh.h
        HINTS ${NEX_BASE_LIBRARY_FOLDER}/assimp-4.1.0/include
        DOC "The header include directory for the assimp library"
        )


get_library_path("assimp-4.1.0" ASSIMP_LIB_PATH)

# This library file is only needed on Windows
FIND_LIBRARY( ASSIMP_IMPORTED_LIB_DEBUG assimp
        DOC "The debug version of the imported library ( a *.lib file) for assimp. This option is only necessary on Windows."
        ${ASSIMP_LIB_PATH}/debug
        )

FIND_LIBRARY( ASSIMP_IMPORTED_LIB_RELEASE assimp
        DOC "The release version of the imported library ( a *.lib file) for assimp. This option is only necessary on Windows."
        ${ASSIMP_LIB_PATH}/release
        )

if (MINGW OR CYGWIN)
  FIND_FILE( ASSIMP_LIBRARY_DEBUG libassimp.dll
          DOC "The debug version of the shared assimp library. On Windows this is a *.dll file, on linux a *.so, and
        on Mac OS a *.so or *.dylib file."
          HINTS ${ASSIMP_LIB_PATH}/debug
          )

  FIND_FILE( ASSIMP_LIBRARY_RELEASE libassimp.dll
          DOC "The release version of the shared assimp library. On Windows this is a *.dll file, on linux a *.so, and
        on Mac OS a *.so or *.dylib file."
          HINTS ${ASSIMP_LIB_PATH}/release
          )

elseif(MSVC)
  FIND_FILE( ASSIMP_LIBRARY_DEBUG assimp.dll
          DOC "The debug version of the shared assimp library. On Windows this is a *.dll file, on linux a *.so, and
        on Mac OS a *.so or *.dylib file."
          HINTS ${ASSIMP_LIB_PATH}/debug
          )

  FIND_FILE( ASSIMP_LIBRARY_RELEASE assimp.dll
          DOC "The release version of the shared assimp library. On Windows this is a *.dll file, on linux a *.so, and
        on Mac OS a *.so or *.dylib file."
          HINTS ${ASSIMP_LIB_PATH}/release
          )
endif()


if (ASSIMP_INCLUDE_DIR AND ASSIMP_LIBRARY_DEBUG AND ASSIMP_LIBRARY_RELEASE)
  SET(ASSIMP_FOUND "TRUE")
else()
  SET(ASSIMP_FOUND "FALSE")
endif()

# On windows we need one additional library
IF(WIN32)
  if (ASSIMP_FOUND AND ASSIMP_IMPORTED_LIB_DEBUG AND ASSIMP_IMPORTED_LIB_RELEASE)
    SET(ASSIMP_FOUND "TRUE")
  else()
    SET(ASSIMP_FOUND "FALSE")
  endif()
ENDIF(WIN32)


IF(ASSIMP_FOUND)
  # print find libraries and the include directory
  IF(NOT ASSIMP_FIND_QUIETLY) # ASSIMP_FIND_QUIETLY gets set by cmake if the QUIET option is set in find_package
    MESSAGE(STATUS "Found ASSIMP debug shared library: ${ASSIMP_LIBRARY_DEBUG}")
    MESSAGE(STATUS "Found ASSIMP release shared library: ${ASSIMP_LIBRARY_RELEASE}")
    MESSAGE(STATUS "Found ASSIMP include directory: ${ASSIMP_INCLUDE_DIR}")
    IF (WIN32)
      MESSAGE(STATUS "Found ASSIMP debug imported library: ${ASSIMP_IMPORTED_LIB_DEBUG}")
      MESSAGE(STATUS "Found ASSIMP release imported library: ${ASSIMP_IMPORTED_LIB_RELEASE}")
    ENDIF(WIN32)
  ENDIF(NOT ASSIMP_FIND_QUIETLY)

  # create the library target
  add_library(ASSIMP SHARED IMPORTED GLOBAL)
  set_target_properties(ASSIMP PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ASSIMP_INCLUDE_DIR}")
  set_target_properties(ASSIMP PROPERTIES IMPORTED_LOCATION_DEBUG "${ASSIMP_LIBRARY_DEBUG}")
  set_target_properties(ASSIMP PROPERTIES IMPORTED_LOCATION_RELEASE "${ASSIMP_LIBRARY_RELEASE}")
  set_target_properties(ASSIMP PROPERTIES IMPORTED_IMPLIB_DEBUG "${ASSIMP_IMPORTED_LIB_DEBUG}")
  set_target_properties(ASSIMP PROPERTIES IMPORTED_IMPLIB_RELEASE "${ASSIMP_IMPORTED_LIB_RELEASE}")

ELSE(ASSIMP_FOUND)
  IF(ASSIMP_FIND_REQUIRED)  # ASSIMP_FIND_REQUIRED gets set by cmake if the REQUIRED option was set in find_package
    MESSAGE(FATAL_ERROR "Could not find all necessary parts of the ASSIMP library")
  ENDIF(ASSIMP_FIND_REQUIRED)
ENDIF(ASSIMP_FOUND)