# configuration options
option (USE_CLIP_SPACE_ZERO_TO_ONE 
        "Use range [0,1] for clipping space. Default is the OpenGL default: [-1, 1]" OFF) 
option (USE_LEFT_HANDED_COORDINATE_SYSTEM 
        "The program uses a left handed coordinate system. Default is a right handed coordinate system." OFF) 	
option (USE_SCREEN_SPACE_UPPER_LEFT_ORIGIN 
        "Uses the upper left corner as the origin in screen space. Default is the OpenGL default (lower left corner)" OFF) 		
option (USE_EUCLID_ALL_OPTIMIZATIONS 
        "The program will be compiled with all optimizations enabled (e.g. no OpenGL debug context)" OFF) 	
        
				
#preprocessor definitions; in order to get defines working as preprocessor definitions, -D has to be put in front
				#GLM_FORCE_DEPTH_ZERO_TO_ONE
if (USE_CLIP_SPACE_ZERO_TO_ONE) 
		ADD_DEFINITIONS(-DUSE_CLIP_SPACE_ZERO_TO_ONE -DGLM_FORCE_DEPTH_ZERO_TO_ONE)
		# TODO implement support in the application
endif (USE_CLIP_SPACE_ZERO_TO_ONE)

if (USE_LEFT_HANDED_COORDINATE_SYSTEM) 
		ADD_DEFINITIONS(-DUSE_LEFT_HANDED_COORDINATE_SYSTEM)
		# TODO implement left handed coordinate system support in the application
endif (USE_LEFT_HANDED_COORDINATE_SYSTEM)

if (USE_SCREEN_SPACE_UPPER_LEFT_ORIGIN) 
		ADD_DEFINITIONS(-DUSE_SCREEN_SPACE_UPPER_LEFT_ORIGIN)
		# TODO implement support in the application
endif (USE_SCREEN_SPACE_UPPER_LEFT_ORIGIN)



if (USE_EUCLID_ALL_OPTIMIZATIONS) 
        #CMAKE_BUILD_TYPE
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DEUCLID_ALL_OPTIMIZATIONS")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DEUCLID_ALL_OPTIMIZATIONS")
endif (USE_EUCLID_ALL_OPTIMIZATIONS)

#ADD_DEFINITIONS(-DGLEW_STATIC)
#ADD_DEFINITIONS(-DNANOGUI_PYTHON)
#ADD_DEFINITIONS(-DNANOGUI_SHARED)
#ADD_DEFINITIONS(-DNVG_SHARED)
	#ADD_DEFINITIONS(-DGLAD_GLAPI_EXPORT)
#ADD_DEFINITIONS(-DNANOGUI_GLAD)


ADD_DEFINITIONS(-DEUCLID_BUILD_PATH="${CMAKE_CURRENT_SOURCE_DIR}/")

# we want to use math oeprators
ADD_DEFINITIONS(-DIMGUI_DEFINE_MATH_OPERATORS)


#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /FC" )

# Sub-directories where more CMakeLists.txt exist
add_subdirectory(engine)
add_subdirectory(engine_opengl)
add_subdirectory(euclid)