IF (TARGET Boxer)
    message(STATUS "boxer library is already imported")
    return()
ENDIF(TARGET Boxer)

set(LINUX FALSE)
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   set(LINUX TRUE)
endif()

# Source files
set(BOXER_SRC_DIR "${NEX_BASE_LIBRARY_FOLDER}/boxer/src")
set(BOXER_INC_DIR "${NEX_BASE_LIBRARY_FOLDER}/boxer/include")
set(BOXER_SOURCES "${BOXER_INC_DIR}/boxer/boxer.h")
if (APPLE)
   list(APPEND BOXER_SOURCES
      "${BOXER_SRC_DIR}/boxer_osx.mm"
   )
elseif (WIN32)
   list(APPEND BOXER_SOURCES
      "${BOXER_SRC_DIR}/boxer_win.cpp"
   )
elseif (LINUX)
   list(APPEND BOXER_SOURCES
      "${BOXER_SRC_DIR}/boxer_linux.cpp"
   )
endif ()

# Library definition and features
add_library(Boxer ${BOXER_SOURCES})
target_include_directories(Boxer PUBLIC "${BOXER_INC_DIR}")
target_compile_features(Boxer PUBLIC cxx_strong_enums cxx_nullptr)

# Platform-specific dependencies
if (APPLE)
   find_library(COCOA_LIBRARY Cocoa)
   target_link_libraries(Boxer PUBLIC ${COCOA_LIBRARY})
elseif (LINUX)
   find_package(PkgConfig REQUIRED)
   pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
   target_link_libraries(Boxer PUBLIC ${GTK3_LIBRARIES})
   target_include_directories(Boxer PRIVATE ${GTK3_INCLUDE_DIRS})
endif ()

# Add boxer to the lib folder
set_target_properties(Boxer PROPERTIES FOLDER lib)