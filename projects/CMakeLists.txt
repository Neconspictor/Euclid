#preprocessor definitions; in order to get defines working as preprocessor definitions, -D has to be put in front
ADD_DEFINITIONS(-DGLEW_STATIC)

# Maps to a solution file (NecEngine.sln). The solution will 
# have all targets (exe, lib, dll) as projects (.vcproj)
project (NecEngine)

# Sub-directories where more CMakeLists.txt exist
add_subdirectory(platform)
add_subdirectory(engine)