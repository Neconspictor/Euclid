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
    
    nex/opengl/CacheGL.hpp
    nex/opengl/CacheGL.cpp
    nex/opengl/opengl.hpp
    nex/opengl/opengl.cpp
    nex/opengl/RenderBackend.cpp
    nex/opengl/RenderBackendGL.hpp
    
    #nex/opengl/buffer
    nex/opengl/buffer/GpuBufferGL.cpp
    nex/opengl/buffer/GpuBufferGL.hpp
    nex/opengl/buffer/IndexBuffer.cpp
	nex/opengl/buffer/ShaderBufferGL.cpp
    nex/opengl/buffer/VertexBuffer.cpp
	
	#nex/opengl/gui
	nex/opengl/gui/ImGUI_GL.hpp
	nex/opengl/gui/ImGUI_GL.cpp
    
    #nex/opengl/mesh
    nex/opengl/mesh/VertexArray.cpp
    nex/opengl/mesh/VertexArrayGL.hpp
    
    #nex/opengl/shader
    nex/opengl/shader/PassGL.cpp
    nex/opengl/shader/ShaderGL.hpp
	nex/opengl/shader/ShaderGL.cpp
    
    #nex/opengl/texture
    nex/opengl/texture/RenderTargetGL.cpp
    nex/opengl/texture/RenderTargetGL.hpp
	nex/opengl/texture/SamplerGL.hpp
    nex/opengl/texture/SamplerGL.cpp
    nex/opengl/texture/TextureGL.hpp
    nex/opengl/texture/TextureGL.cpp
	
	
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