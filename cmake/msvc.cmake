message(STATUS "Detected MSVC")

# debug optimization configuration
#set(CMAKE_CXX_FLAGS_DEBUG "/DEBUG:FULL")

#release optimization configuration
set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ot /Oy /fp:fast /Ob2 /Oi /GT") #/GL
#for RelWithDebInfo we don't set optimizations for improved debugging capabilities set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Ot /Oy /fp:fast /Ob2 /Oi /GT") #/GL
set(CMAKE_CXX_FLAGS_MINSIZEREL "/O2 /Ot /Oy /fp:fast /Ob2 /Oi /GT") #/GL
#set(CMAKE_EXE_LINKER_FLAGS  "/LTCG" CACHE INTERNAL "" FORCE)

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /FC" )

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

SET(EUCLID_COMPILER_NAME msvc)

# find compiler version and set the NeX compiler folder name (used for finding libs)
SET(EUCLID_COMPILER_FOLDER_NAME "${EUCLID_COMPILER_NAME}-${MSVC_TOOLSET_VERSION}")