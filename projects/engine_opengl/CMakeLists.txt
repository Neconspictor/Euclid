set(
    ENGINE_OPENGL_SOURCES 
    
    ###header files###
    pch_engine_opengl.hpp
		
    #antialiasing
    antialiasing/SMAA_GL.hpp
	antialiasing/SMAA_GL.cpp
    
    #drawing
    drawing/ModelDrawerGL.hpp
	drawing/ModelDrawerGL.cpp
    
    #mesh
    mesh/MeshGL.hpp
	mesh/MeshGL.cpp
    mesh/MeshFactoryGL.hpp
	mesh/MeshFactoryGL.cpp
	
    #model
    model/AssimpModelLoader.hpp
	model/AssimpModelLoader.cpp
    model/ModelGL.hpp
	model/ModelGL.cpp
    model/ModelManagerGL.hpp
	model/ModelManagerGL.cpp
    
	#post_processing/blur
	post_processing/blur/GaussianBlurGL.hpp
	post_processing/blur/GaussianBlurGL.cpp
	
    #renderer
    renderer/RendererOpenGL.hpp
	renderer/RendererOpenGL.cpp
    
    #shader
	shader/DepthMapShaderGL.hpp
	shader/DepthMapShaderGL.cpp
    shader/NormalsShaderGL.hpp
	shader/NormalsShaderGL.cpp
    shader/PBRShaderGL.hpp
	shader/PBRShaderGL.cpp
	shader/PhongTexShaderGL.hpp
	shader/PhongTexShaderGL.cpp
    shader/ScreenShaderGL.hpp
	shader/ScreenShaderGL.cpp
    shader/ShaderGL.hpp
	shader/ShaderGL.cpp
	shader/ShadowShaderGL.hpp
	shader/ShadowShaderGL.cpp
    shader/ShaderManagerGL.hpp
	shader/ShaderManagerGL.cpp
    shader/SimpleColorShaderGL.hpp
	shader/SimpleColorShaderGL.cpp
    shader/SimpleExtrudeShaderGL.hpp
	shader/SimpleExtrudeShaderGL.cpp
    shader/SkyBoxShaderGL.hpp
	shader/SkyBoxShaderGL.cpp
	#shader/post_processing/blur
    shader/post_processing/blur/GaussianBlurShaderGL.hpp
	shader/post_processing/blur/GaussianBlurShaderGL.cpp
	
	#shading_model
	shading_model/PBR_DeferredGL.hpp
	shading_model/PBR_DeferredGL.cpp
	shading_model/ShadingModelFactoryGL.cpp
	shading_model/ShadingModelFactoryGL.hpp
	shading_model/ShadingModelFactoryGL.cpp
 
    
    #texture
    texture/ImageLoaderGL.hpp
	texture/ImageLoaderGL.cpp
    texture/TextureGL.hpp
    texture/TextureGL.cpp
	texture/TextureManagerGL.hpp
	texture/TextureManagerGL.cpp
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${ENGINE_SOURCES})
assign_source_group(${ENGINE_OPENGL_SOURCES})

enable_precompiled_headers(pch_engine_opengl.hpp ENGINE_OPENGL_SOURCES)

# Properties->C/C++->General->Additional Include Directories
include_directories (
    ./ 
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
	./ 
    ${CMAKE_SOURCE_DIR}/libs/glm/include 
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
    ${CMAKE_SOURCE_DIR}/libs/assimp-3.3.1/include
)

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