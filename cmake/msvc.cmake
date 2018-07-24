message(STATUS "Detected MSVC")

# debug optimization configuration
#set(CMAKE_CXX_FLAGS_DEBUG "-g")

#release optimization configuration
set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ot /Oy")

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /FC" )

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

SET(NEX_COMPILER_NAME msvc)

# find compiler version and set the NeX compiler folder name (used for finding libs)
SET(NEX_COMPILER_FOLDER_NAME "${NEX_COMPILER_NAME}-${MSVC_TOOLSET_VERSION}")