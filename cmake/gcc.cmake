# debug optimization configuration
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# release optimization configuration
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -fomit-frame-pointer")

SET(NEX_COMPILER_NAME gcc)

SET(NEX_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})

# find compiler version and set the NeX compiler folder name (used for finding libs)
SET(NEX_COMPILER_FOLDER_NAME "${NEX_COMPILER_NAME}-${NEX_COMPILER_VERSION}")