# configuration options
option (USE_CLIP_SPACE_ZERO_TO_ONE 
        "Use range [0,1] for clipping space. Default is the OpenGL default: [-1, 1]" OFF) 
option (USE_LEFT_HANDED_COORDINATE_SYSTEM 
        "The program uses a left handed coordinate system. Default is a right handed coordinate system." OFF) 				
				
#preprocessor definitions; in order to get defines working as preprocessor definitions, -D has to be put in front
				
if (USE_CLIP_SPACE_ZERO_TO_ONE) 
		ADD_DEFINITIONS(-DUSE_CLIP_SPACE_ZERO_TO_ONE)
		# TODO implement support in the application
endif (USE_CLIP_SPACE_ZERO_TO_ONE)

if (USE_LEFT_HANDED_COORDINATE_SYSTEM) 
		ADD_DEFINITIONS(-DUSE_LEFT_HANDED_COORDINATE_SYSTEM)
		# TODO implement left handed coordinate system support in the application
endif (USE_LEFT_HANDED_COORDINATE_SYSTEM)

#ADD_DEFINITIONS(-DGLEW_STATIC)
#ADD_DEFINITIONS(-DNANOGUI_PYTHON)
#ADD_DEFINITIONS(-DNANOGUI_SHARED)
#ADD_DEFINITIONS(-DNVG_SHARED)
	#ADD_DEFINITIONS(-DGLAD_GLAPI_EXPORT)
#ADD_DEFINITIONS(-DNANOGUI_GLAD)


ADD_DEFINITIONS(-DNEC_BUILD_PATH="${CMAKE_CURRENT_SOURCE_DIR}/")


#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /FC" )

# Sub-directories where more CMakeLists.txt exist
add_subdirectory(platform)
add_subdirectory(engine)
add_subdirectory(engine_opengl)
#add_subdirectory(test)
add_subdirectory(app)