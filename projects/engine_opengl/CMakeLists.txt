set(
    ENGINE_OPENGL_SOURCES 
    
    ###header files###
    include/pch_engine_opengl.hpp
		
    #antialiasing
    include/antialiasing/SMAA_GL.hpp
    
    #drawing
    include/drawing/ModelDrawerGL.hpp
    
    #mesh
    include/mesh/MeshGL.hpp
    include/mesh/MeshFactoryGL.hpp
    
    
    #model
    include/model/AssimpModelLoader.hpp
    include/model/ModelGL.hpp
    include/model/ModelManagerGL.hpp
    
	#post_processing/blur
	include/post_processing/blur/GaussianBlurGL.hpp
	
    #renderer
    include/renderer/RendererOpenGL.hpp
    
    #shader
	include/shader/DepthMapShaderGL.hpp
    include/shader/NormalsShaderGL.hpp
    include/shader/PBRShaderGL.hpp
	include/shader/PhongTexShaderGL.hpp
    include/shader/ScreenShaderGL.hpp
    include/shader/ShaderGL.hpp
	include/shader/ShadowShaderGL.hpp
    include/shader/ShaderManagerGL.hpp
    include/shader/SimpleColorShaderGL.hpp
    include/shader/SimpleExtrudeShaderGL.hpp
    include/shader/SkyBoxShaderGL.hpp
	#shader/post_processing/blur
    include/shader/post_processing/blur/GaussianBlurShaderGL.hpp
 
    
    #texture
    include/texture/ImageLoaderGL.hpp
    include/texture/TextureGL.hpp
    include/texture/TextureManagerGL.hpp
    
    ###source files###
    
    #antialiasing
    src/antialiasing/SMAA_GL.cpp

    
    #drawing
    src/drawing/ModelDrawerGL.cpp

    
    #mesh
    src/mesh/MeshGL.cpp
    src/mesh/MeshFactoryGL.cpp
    
    
    #model
    src/model/AssimpModelLoader.cpp
    src/model/ModelGL.cpp
    src/model/ModelManagerGL.cpp
	
	#post_processing/blur
	src/post_processing/blur/GaussianBlurGL.cpp
	
    #renderer
    src/renderer/RendererOpenGL.cpp
    
    #shader
	src/shader/DepthMapShaderGL.cpp
    src/shader/NormalsShaderGL.cpp
    src/shader/PBRShaderGL.cpp
	src/shader/PhongTexShaderGL.cpp
    src/shader/ScreenShaderGL.cpp
    src/shader/ShaderGL.cpp
    src/shader/ShaderManagerGL.cpp
	src/shader/ShadowShaderGL.cpp
    src/shader/SimpleColorShaderGL.cpp
    src/shader/SimpleExtrudeShaderGL.cpp
    src/shader/SkyBoxShaderGL.cpp
	#shader/post_processing/blur
    src/shader/post_processing/blur/GaussianBlurShaderGL.cpp
	
    
    #texture
    src/texture/ImageLoaderGL.cpp
    src/texture/TextureGL.cpp
    src/texture/TextureManagerGL.cpp
    
    #${CMAKE_SOURCE_DIR}/libs/glad/src/glad.c
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${ENGINE_SOURCES})
assign_source_group(${ENGINE_OPENGL_SOURCES})

enable_precompiled_headers(include/pch_engine_opengl.hpp ENGINE_OPENGL_SOURCES)

# Properties->C/C++->General->Additional Include Directories
include_directories (
    include/ 
    ${CMAKE_SOURCE_DIR}/libs/glm/include  
    #${CMAKE_SOURCE_DIR}/libs/soil2/include
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
    ${CMAKE_SOURCE_DIR}/libs/assimp-3.3.1/include
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_library (engine_opengl ${ENGINE_OPENGL_SOURCES})
target_include_directories (engine_opengl PUBLIC 
include/ 
    ${CMAKE_SOURCE_DIR}/libs/glm/include 
    #${CMAKE_SOURCE_DIR}/libs/soil2/include
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
    ${CMAKE_SOURCE_DIR}/libs/assimp-3.3.1/include
)


#set additional library directories
#link_directories(.)

#choose the right libraries by absolute paths!
#find_library(libglew32_lib NAMES libglew32 HINTS ${CMAKE_SOURCE_DIR}/libs/glew/x64/Release)
#find_library(soil2_lib NAMES soil2 HINTS ${CMAKE_SOURCE_DIR}/libs/soil2/x64/Release)

#boost libraries
find_library(libboost_program_options_release NAMES libboost_program_options-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_program_options_debug NAMES libboost_program_options-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

#assimp library
find_library(assimp_release NAMES assimp-vc140-mt HINTS ${CMAKE_SOURCE_DIR}/libs/assimp-3.3.1/x64/Release)
find_library(assimp_debug NAMES assimp-vc140-mt HINTS ${CMAKE_SOURCE_DIR}/libs/assimp-3.3.1/x64/Debug)


# Properties->Linker->Input->Additional Dependencies
target_link_libraries(engine_opengl PUBLIC opengl32)
target_link_libraries(engine_opengl PUBLIC glu32)


target_link_libraries(engine_opengl PUBLIC engine)

#boost libraries
target_link_libraries(engine_opengl PUBLIC debug ${libboost_program_options_debug})
target_link_libraries(engine_opengl PUBLIC optimized ${libboost_program_options_release})

#assimp library
target_link_libraries(engine_opengl PUBLIC debug ${assimp_debug})
target_link_libraries(engine_opengl PUBLIC optimized ${assimp_release})