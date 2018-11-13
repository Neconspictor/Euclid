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
		
    #nex/opengl/antialiasing
    nex/opengl/antialiasing/SMAA_GL.hpp
	nex/opengl/antialiasing/SMAA_GL.cpp
    
    #nex/opengl/drawing
    nex/opengl/drawing/ModelDrawerGL.hpp
	nex/opengl/drawing/ModelDrawerGL.cpp
	
	#nex/opengl/gui
	nex/opengl/gui/ImGUI_GL.hpp
	nex/opengl/gui/ImGUI_GL.cpp
	nex/opengl/gui/NanoGUI.hpp
	nex/opengl/gui/NanoGUI.cpp
    
    #nex/opengl/material
    nex/opengl/material/AbstractMaterialLoader.hpp
	nex/opengl/material/AbstractMaterialLoader.cpp
    nex/opengl/material/BlinnPhongMaterial.hpp
	nex/opengl/material/BlinnPhongMaterial.cpp
	nex/opengl/material/BlinnPhongMaterialLoader.hpp
	nex/opengl/material/BlinnPhongMaterialLoader.cpp
    nex/opengl/material/Material.hpp
	nex/opengl/material/PbrMaterial.hpp
	nex/opengl/material/PbrMaterial.cpp
	nex/opengl/material/PbrMaterialLoader.hpp
	nex/opengl/material/PbrMaterialLoader.cpp
    
    #nex/opengl/mesh
    nex/opengl/mesh/MeshGL.hpp
	nex/opengl/mesh/MeshGL.cpp
    nex/opengl/mesh/MeshFactoryGL.hpp
	nex/opengl/mesh/MeshFactoryGL.cpp
    nex/opengl/mesh/SampleMeshes.hpp
    nex/opengl/mesh/TestMeshes.hpp
	
    #nex/opengl/model
    nex/opengl/model/AssimpModelLoader.hpp
	nex/opengl/model/AssimpModelLoader.cpp
    nex/opengl/model/ModelGL.hpp
	nex/opengl/model/ModelGL.cpp
    nex/opengl/model/ModelManagerGL.hpp
	nex/opengl/model/ModelManagerGL.cpp
    nex/opengl/model/Vob.hpp
	nex/opengl/model/Vob.cpp
    
	#nex/opengl/post_processing
    nex/opengl/post_processing/AmbientOcclusion.cpp
	nex/opengl/post_processing/AmbientOcclusion.hpp
	nex/opengl/post_processing/HBAO_GL.hpp
	nex/opengl/post_processing/HBAO_GL.cpp
	nex/opengl/post_processing/SSAO_GL.hpp
	nex/opengl/post_processing/SSAO_GL.cpp
	#nex/opengl/post_processing/blur
	nex/opengl/post_processing/blur/GaussianBlurGL.hpp
	nex/opengl/post_processing/blur/GaussianBlurGL.cpp
	
    #nex/opengl/renderer
    nex/opengl/renderer/Renderer.hpp
    nex/opengl/renderer/Renderer.cpp
    nex/opengl/renderer/RendererOpenGL.hpp
	nex/opengl/renderer/RendererOpenGL.cpp
    
    #nex/opengl/scene
    nex/opengl/scene/SceneNode.cpp
    nex/opengl/scene/SceneNode.hpp
    
    #nex/opengl/shader
	nex/opengl/shader/DepthMapShaderGL.hpp
	nex/opengl/shader/DepthMapShaderGL.cpp
    nex/opengl/shader/NormalsShaderGL.hpp
	nex/opengl/shader/NormalsShaderGL.cpp
    nex/opengl/shader/PBRShaderGL.hpp
	nex/opengl/shader/PBRShaderGL.cpp
	nex/opengl/shader/PhongTexShaderGL.hpp
	nex/opengl/shader/PhongTexShaderGL.cpp
    nex/opengl/shader/ScreenShaderGL.hpp
	nex/opengl/shader/ScreenShaderGL.cpp
    nex/opengl/shader/ShaderGL.hpp
	nex/opengl/shader/ShaderGL.cpp
	nex/opengl/shader/ShadowShaderGL.hpp
	nex/opengl/shader/ShadowShaderGL.cpp
    nex/opengl/shader/ShaderManagerGL.hpp
	nex/opengl/shader/ShaderManagerGL.cpp
    nex/opengl/shader/SimpleColorShaderGL.hpp
	nex/opengl/shader/SimpleColorShaderGL.cpp
    nex/opengl/shader/SimpleExtrudeShaderGL.hpp
	nex/opengl/shader/SimpleExtrudeShaderGL.cpp
    nex/opengl/shader/SkyBoxShaderGL.hpp
	nex/opengl/shader/SkyBoxShaderGL.cpp
	#nex/opengl/shader/post_processing/blur
    nex/opengl/shader/post_processing/blur/GaussianBlurShaderGL.hpp
	nex/opengl/shader/post_processing/blur/GaussianBlurShaderGL.cpp
	
	#nex/opengl/shading_model
    nex/opengl/shading_model/PBR.cpp
    nex/opengl/shading_model/PBR.hpp
	nex/opengl/shading_model/PBR_DeferredGL.hpp
	nex/opengl/shading_model/PBR_DeferredGL.cpp
	nex/opengl/shading_model/ShadingModelFactoryGL.cpp
	nex/opengl/shading_model/ShadingModelFactoryGL.hpp
	nex/opengl/shading_model/ShadingModelFactoryGL.cpp
 
	#nex/opengl/shadowing
	nex/opengl/shadowing/CascadedShadowGL.cpp
	nex/opengl/shadowing/CascadedShadowGL.hpp
    
    #nex/opengl/texture
    nex/opengl/texture/ImageLoaderGL.hpp
	nex/opengl/texture/ImageLoaderGL.cpp
	nex/opengl/texture/SamplerGL.hpp
    nex/opengl/texture/SamplerGL.cpp
    nex/opengl/texture/Sprite.hpp
	nex/opengl/texture/Sprite.cpp
    nex/opengl/texture/TextureGL.hpp
    nex/opengl/texture/TextureGL.cpp
	nex/opengl/texture/TextureManagerGL.hpp
	nex/opengl/texture/TextureManagerGL.cpp
	
	
	#nex/opengl/window_system/
    #nex/opengl/window_system/glfw/
    nex/opengl/window_system/glfw/InputGLFW.hpp
	nex/opengl/window_system/glfw/InputGLFW.cpp
    nex/opengl/window_system/glfw/WindowGLFW.hpp
	nex/opengl/window_system/glfw/WindowGLFW.cpp
    nex/opengl/window_system/glfw/SubSystemProviderGLFW.hpp
	nex/opengl/window_system/glfw/SubSystemProviderGLFW.cpp
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