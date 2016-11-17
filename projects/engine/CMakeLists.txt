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
    include/shader/ShaderManager.hpp
    include/shader/SimpleLightShader.hpp
    #shader/opengl
    include/shader/opengl/LampShaderGL.hpp
    include/shader/opengl/PlaygroundShaderGL.hpp
    include/shader/opengl/ShaderGL.hpp
    include/shader/opengl/SimpleLightShaderGL.hpp
    
    #system
    include/system/Configuration.hpp
    include/system/Engine.hpp
    include/system/Video.hpp
    include/system/System.hpp
    
    #texture
    include/texture/TextureManager.hpp
    #texture/opengl
    include/texture/opengl/TextureManagerGL.hpp
    
    #util
    include/util/FPSCounter.hpp
    include/util/GlobalPaths.hpp
    include/util/Timer.hpp
    
    ###source files###
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
    #shader/opengl
    src/shader/opengl/LampShaderGL.cpp
    src/shader/opengl/PlaygroundShaderGL.cpp
    src/shader/opengl/ShaderGL.cpp
    src/shader/opengl/SimpleLightShaderGL.cpp
    
    #system
    src/system/Configuration.cpp
    src/system/Engine.cpp
    src/system/System.cpp
    src/system/Video.cpp
    
    #texture
    #texture/opengl
    src/texture/opengl/TextureManagerGL.cpp
    
    #util
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${ENGINE_SOURCES})
assign_source_group(${ENGINE_SOURCES})

# Properties->C/C++->General->Additional Include Directories
include_directories (
    include/ 
    ${CMAKE_SOURCE_DIR}/libs/glm/include 
    ${CMAKE_SOURCE_DIR}/libs/glew/include 
    ${CMAKE_SOURCE_DIR}/libs/soil2/include
    ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/include
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_library (engine ${ENGINE_SOURCES})
target_include_directories (engine PUBLIC 
include/ 
    ${CMAKE_SOURCE_DIR}/libs/glm/include 
    ${CMAKE_SOURCE_DIR}/libs/glew/include 
    ${CMAKE_SOURCE_DIR}/libs/soil2/include
    ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/include
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
)


#set additional library directories
#link_directories(.)

#choose the right libraries by absolute paths!
find_library(libglew32_lib NAMES libglew32 HINTS ${CMAKE_SOURCE_DIR}/libs/glew/x64/Release)
find_library(soil2_lib NAMES soil2 HINTS ${CMAKE_SOURCE_DIR}/libs/soil2/x64/Release)

find_library(SDL2_lib_debug NAMES SDL2 HINTS ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Debug)
find_library(SDL2_lib_release NAMES SDL2 HINTS ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Release)
find_library(SDL2main_lib_debug NAMES SDL2main HINTS ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Debug)
find_library(SDL2main_lib_release NAMES SDL2main HINTS ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Release)

#boost libraries
find_library(libboost_program_options_release NAMES libboost_program_options-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_program_options_debug NAMES libboost_program_options-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

# Properties->Linker->Input->Additional Dependencies
target_link_libraries(engine PUBLIC opengl32)
target_link_libraries(engine PUBLIC ${libglew32_lib})
target_link_libraries(engine PUBLIC ${soil2_lib})
target_link_libraries(engine PUBLIC glu32)

#target_link_libraries(engine PUBLIC Xinput9_1_0)
#target_link_libraries(engine PUBLIC ws2_32)

target_link_libraries(engine PUBLIC debug ${SDL2_lib_debug})
target_link_libraries(engine PUBLIC optimized ${SDL2_lib_release})
target_link_libraries(engine PUBLIC debug ${SDL2main_lib_debug})
target_link_libraries(engine PUBLIC optimized ${SDL2main_lib_release})

target_link_libraries(engine PUBLIC platform)   

#boost libraries
target_link_libraries(engine PUBLIC debug ${libboost_program_options_debug})
target_link_libraries(engine PUBLIC optimized ${libboost_program_options_release})