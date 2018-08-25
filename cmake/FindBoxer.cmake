# - Try to find glad

IF (TARGET Boxer)
    message(STATUS "boxer library is already imported")
    return()
ENDIF(TARGET Boxer)

include(${NEX_BASE_LIBRARY_FOLDER}/boxer/CMakeLists.txt)