# should we use our own math functions?
option (USE_CLIP_SPACE_ZERO_TO_ONE 
        "Use range [0,1] for clipping space. Default is the OpenGL default: [-1, 1]" OFF) 
				
if (USE_CLIP_SPACE_ZERO_TO_ONE) 
		ADD_DEFINITIONS(-DUSE_CLIP_SPACE_ZERO_TO_ONE)
endif (USE_CLIP_SPACE_ZERO_TO_ONE)

#preprocessor definitions; in order to get defines working as preprocessor definitions, -D has to be put in front
ADD_DEFINITIONS(-DGLEW_STATIC)
ADD_DEFINITIONS(-DNANOGUI_PYTHON)
ADD_DEFINITIONS(-DNANOGUI_SHARED)
ADD_DEFINITIONS(-DNVG_SHARED)
#ADD_DEFINITIONS(-DGLAD_GLAPI_EXPORT)
ADD_DEFINITIONS(-DNANOGUI_GLAD)


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