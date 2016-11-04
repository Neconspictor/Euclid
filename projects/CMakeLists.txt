#preprocessor definitions; in order to get defines working as preprocessor definitions, -D has to be put in front
ADD_DEFINITIONS(-DGLEW_STATIC)
ADD_DEFINITIONS(-DNEC_BUILD_PATH="${CMAKE_CURRENT_SOURCE_DIR}/")

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /FC" )

# Maps to a solution file (NecEngine.sln). The solution will 
# have all targets (exe, lib, dll) as projects (.vcproj)
project (NecEngine)

set(PLATFORM_TARGET_NAME platform)
set(PLATFORM_DEPENDENCIES_LIBS_DEBUG "")
set(PLATFORM_DEPENDENCIES_LIBS_RELEASE "")

set(ENGINE_TARGET_NAME engine)
set(ENGINE_DEPENDENCIES_LIBS_DEBUG "")
set(ENGINE_DEPENDENCIES_LIBS_RELEASE "")

# Sub-directories where more CMakeLists.txt exist
add_subdirectory(platform)
add_subdirectory(platform_test)
add_subdirectory(engine)
add_subdirectory(engine_test)