function(get_assimp_lib_for_current_config destiny_var)
    if (WIN32)
		IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
			SET(${destiny_var} ${ASSIMP_LIBRARY_DEBUG})
		ELSE()
			SET(${destiny_var} ${ASSIMP_LIBRARY_RELEASE})
		ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)	
	else()
		IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
			SET(${destiny_var} ${ASSIMP_IMPORTED_LIB_DEBUG})
		ELSE()
			SET(${destiny_var} ${ASSIMP_IMPORTED_LIB_DEBUG})
		ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    endif()
	
	message("destiny_var = ${${destiny_var}}")
endfunction(get_assimp_lib_for_current_config)


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
	
	#gui
	gui/ImGUI_GL.hpp
	gui/ImGUI_GL.cpp
	gui/NanoGUI.hpp
	gui/NanoGUI.cpp
    
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
    
	#post_processing
	post_processing/HBAO_GL.hpp
	post_processing/HBAO_GL.cpp
	post_processing/SSAO_GL.hpp
	post_processing/SSAO_GL.cpp
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
	
	
	#window_system/
    #window_system/glfw/
    window_system/glfw/InputGLFW.hpp
	window_system/glfw/InputGLFW.cpp
    window_system/glfw/WindowGLFW.hpp
	window_system/glfw/WindowGLFW.cpp
    window_system/glfw/SubSystemProviderGLFW.hpp
	window_system/glfw/SubSystemProviderGLFW.cpp
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${ENGINE_SOURCES})
assign_source_group(${ENGINE_OPENGL_SOURCES})

enable_precompiled_headers(pch_engine_opengl.hpp ENGINE_OPENGL_SOURCES)


# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_library (engine_opengl ${ENGINE_OPENGL_SOURCES} ${ENGINE_OPENGL_EXTERNAL_SOURCES})
target_include_directories (engine_opengl PUBLIC 
	./ 
)

#nanogui
#find_library(nanogui_release NAMES nanogui HINTS ${CMAKE_SOURCE_DIR}/libs/nanogui/x64/Release)
#find_library(nanogui_debug NAMES nanogui HINTS ${CMAKE_SOURCE_DIR}/libs/nanogui/x64/Debug)

#nanogui
#target_link_libraries(platform PUBLIC debug ${nanogui_debug})
#target_link_libraries(platform PUBLIC optimized ${nanogui_release})

target_link_libraries(engine_opengl PUBLIC engine)

find_package(ASSIMP REQUIRED)
find_package(Boost 1.67 EXACT REQUIRED)
find_package(GLAD REQUIRED)
find_package(GLFW 3.2.1 EXACT REQUIRED)
find_package(GLM REQUIRED)
find_package(Opengl REQUIRED)

target_link_libraries(engine_opengl

        PUBLIC

        ASSIMP
		Boost::boost
		GLAD
		GLFW
        GLM
		
		${OPENGL_LIBRARIES}
)		