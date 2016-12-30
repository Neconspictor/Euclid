#preprocessor definitions; in order to get defines working as preprocessor definitions, -D has to be put in front
ADD_DEFINITIONS(-DGLEW_STATIC)
ADD_DEFINITIONS(-DNEC_BUILD_PATH="${CMAKE_CURRENT_SOURCE_DIR}/")

#wxwidget
#ADD_DEFINITIONS(-DWINVER=0x0400)
#ADD_DEFINITIONS(-D__WXMSW__)
#ADD_DEFINITIONS(-D_WINDOWS)
#ADD_DEFINITIONS(-DWXUSINGDLL)
#ADD_DEFINITIONS(-DwxUSE_GUI=1 )
#ADD_DEFINITIONS(-D_UNICODE)
#ADD_DEFINITIONS(-DSDL_MAIN_HANDLED)



SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /FC" )

# Maps to a solution file (NecEngine.sln). The solution will 
# have all targets (exe, lib, dll) as projects (.vcproj)
project (NecEngine)

# Sub-directories where more CMakeLists.txt exist
add_subdirectory(platform)
add_subdirectory(engine)
#add_subdirectory(test)
add_subdirectory(app)