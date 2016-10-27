set(
    ENGINE_SOURCES 
    
    ###header files###
    #camera
    include/camera/Camera.hpp
    
    #listener
    include/listener/ScrollListener.hpp
    
    #mesh
    include/mesh/TestMeshes.hpp
    
    #model
    include/model/BasicModel.hpp
    include/model/Model.hpp
    include/model/ModelFactory.hpp
    
    #renderer
    include/renderer/AbstractRendererFactory.hpp
    include/renderer/RendererOpenGL.hpp
    
    #shader
    include/shader/LampShader.hpp
    include/shader/PlaygroundShader.hpp
    include/shader/Shader.hpp
    include/shader/SimpleLightShader.hpp
    
    #texture
    include/texture/TextureManager.hpp
    
    ###source files###
    src/Main.cpp
    
    #camera
    src/camera/Camera.cpp
    
    #listener
    
    #mesh
    #src/mesh/TestMeshes.cpp
    
    #model
    src/model/BasicModel.cpp
    src/model/Model.cpp
    src/model/ModelFactory.cpp
    
    #renderer
    src/renderer/RendererOpenGL.cpp
    
    #shader
    src/shader/LampShader.cpp
    src/shader/PlaygroundShader.cpp
    src/shader/Shader.cpp
    src/shader/SimpleLightShader.cpp
    
    #texture
    src/texture/TextureManager.cpp
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${ENGINE_SOURCES})
assign_source_group(${ENGINE_SOURCES})

# Properties->C/C++->General->Additional Include Directories
include_directories (
    include/ 
    ${CMAKE_SOURCE_DIR}/libs/glm/include 
    ${CMAKE_SOURCE_DIR}/libs/gainput/include 
    ${CMAKE_SOURCE_DIR}/libs/glew/include 
    ${CMAKE_SOURCE_DIR}/libs/glfw/include 
    ${CMAKE_SOURCE_DIR}/libs/OIS/include 
    ${CMAKE_SOURCE_DIR}/libs/soil2/include
    ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/include
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_executable (Engine ${ENGINE_SOURCES})


#set additional library directories
#link_directories(.)

#choose the right libraries by absolute paths!
find_library(glfw3_lib NAMES glfw3 HINTS ${CMAKE_SOURCE_DIR}/libs/glfw/x64/Release)
find_library(libglew32_lib NAMES libglew32 HINTS ${CMAKE_SOURCE_DIR}/libs/glew/x64/Release)
find_library(soil2_lib NAMES soil2 HINTS ${CMAKE_SOURCE_DIR}/libs/soil2/x64/Release)

find_library(gainputstatic_lib_debug NAMES gainputstatic HINTS ${CMAKE_SOURCE_DIR}/libs/gainput/x64/Debug)
find_library(gainputstatic_lib_release NAMES gainputstatic HINTS ${CMAKE_SOURCE_DIR}/libs/gainput/x64/Release)

find_library(ois_lib_debug NAMES ois_d HINTS ${CMAKE_SOURCE_DIR}/libs/OIS/x64/Debug)
find_library(ois_lib_release NAMES ois HINTS ${CMAKE_SOURCE_DIR}/libs/OIS/x64/Release)

find_library(SDL2_lib_debug NAMES SDL2 HINTS ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Debug)
find_library(SDL2_lib_release NAMES SDL2 HINTS ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Release)
find_library(SDL2main_lib_debug NAMES SDL2main HINTS ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Debug)
find_library(SDL2main_lib_release NAMES SDL2main HINTS ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Release)


# Properties->Linker->Input->Additional Dependencies
target_link_libraries(Engine PUBLIC opengl32)
target_link_libraries(Engine PUBLIC ${glfw3_lib})
target_link_libraries(Engine PUBLIC ${libglew32_lib})
target_link_libraries(Engine PUBLIC ${soil2_lib})
target_link_libraries(Engine PUBLIC glu32)

target_link_libraries(Engine PUBLIC debug ${gainputstatic_lib_debug})
target_link_libraries(Engine PUBLIC optimized ${gainputstatic_lib_release})

target_link_libraries(Engine PUBLIC Xinput9_1_0)
target_link_libraries(Engine PUBLIC ws2_32)

target_link_libraries(Engine PUBLIC debug ${ois_lib_debug})
target_link_libraries(Engine PUBLIC optimized ${ois_lib_release})

target_link_libraries(Engine PUBLIC debug ${SDL2_lib_debug})
target_link_libraries(Engine PUBLIC optimized ${SDL2_lib_release})
target_link_libraries(Engine PUBLIC debug ${SDL2main_lib_debug})
target_link_libraries(Engine PUBLIC optimized ${SDL2main_lib_release})
target_link_libraries(Engine PUBLIC platform)

#if debug then ${DEBUG_EXE_PATH} elseif release then ${RELEASE_EXE_PATH}
#$<$<CONFIG:debug>:${DEBUG_EXE_PATH}>$<$<CONFIG:release>:${RELEASE_EXE_PATH}>

set (
    SDL_RELEASE_DLL
    ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Release/SDL2.dll
)

set (
    SDL_DEBUG_DLL
    ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Debug/SDL2.dll
)

add_custom_command(TARGET Engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$<$<CONFIG:debug>:${SDL_DEBUG_DLL}>$<$<CONFIG:release>:${SDL_RELEASE_DLL}>" "$<TARGET_FILE_DIR:Engine>/SDL2.dll"
    #COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Release/SDL2.dll" "$<TARGET_FILE_DIR:Engine>/SDL2.dll"
)

# Adds logic to INSTALL.vcproj to copy app.exe to destination directory
install (TARGETS Engine
		 RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin)
         
install (FILES "${SDL_RELEASE_DLL}" DESTINATION ${CMAKE_SOURCE_DIR}/bin CONFIGURATIONS Release)
install (FILES "${SDL_DEBUG_DLL}" DESTINATION ${CMAKE_SOURCE_DIR}/bin CONFIGURATIONS Debug)      