set(
    ENGINE_SOURCES 
    
    ###header files###
    #camera
    include/camera/Camera.hpp
    include/camera/FPCamera.hpp
    include/camera/FPCameraBase.hpp
    include/camera/FPQuaternionCamera.hpp
    include/camera/TrackballCamera.hpp
    include/camera/TrackballQuatCamera.hpp
    
    #drawing
    include/drawing/ModelDrawer.hpp
    #drawing/opengl
    include/drawing/opengl/ModelDrawerGL.hpp
    
    #exception
    include/exception/MeshNotFoundException.hpp
    include/exception/ShaderInitException.hpp
    include/exception/ShaderNotFoundException.hpp
    
    #listener
    include/listener/ScrollListener.hpp
    
    #mesh
    include/mesh/Mesh.hpp
    include/mesh/SampleMeshes.hpp
    #mesh/opengl
    include/mesh/opengl/MeshGL.hpp
    include/mesh/opengl/MeshFactoryGL.hpp
    
    #material
    include/material/Material.hpp
    include/material/PhongMaterial.hpp
    
    #model
    include/model/Model.hpp
    include/model/ModelManager.hpp
    include/model/PhongModel.hpp
    include/model/Vob.hpp
    #model/opengl
    include/model/opengl/AssimpModelLoader.hpp
    include/model/opengl/ModelGL.hpp
    include/model/opengl/ModelManagerGL.hpp
    
    #renderer
    include/renderer/AbstractRendererFactory.hpp
    include/renderer/Renderer3D.hpp
    include/renderer/RendererOpenGL.hpp
    
    #shader
    include/shader/LampShader.hpp
    include/shader/PhongShader.hpp
    include/shader/PhongTextureShader.hpp
    include/shader/PlaygroundShader.hpp
    include/shader/ScreenShader.hpp
    include/shader/Shader.hpp
    include/shader/ShaderEnum.hpp
    include/shader/ShaderManager.hpp
    include/shader/SimpleColorShader.hpp
    include/shader/SimpleExtrudeShader.hpp
    include/shader/SimpleLightShader.hpp
    include/shader/SimpleReflectionShader.hpp
    include/shader/SkyBoxShader.hpp
    #shader/opengl
    include/shader/opengl/LampShaderGL.hpp
    include/shader/opengl/PhongShaderGL.hpp
    include/shader/opengl/PhongTexShaderGL.hpp
    include/shader/opengl/PlaygroundShaderGL.hpp
    include/shader/opengl/ScreenShaderGL.hpp
    include/shader/opengl/ShaderGL.hpp
    include/shader/opengl/ShaderManagerGL.hpp
    include/shader/opengl/SimpleColorShaderGL.hpp
    include/shader/opengl/SimpleExtrudeShaderGL.hpp
    include/shader/opengl/SimpleLightShaderGL.hpp
    include/shader/opengl/SimpleReflectionShaderGL.hpp
    include/shader/opengl/SkyBoxShaderGL.hpp
 
    
    #sprite
    include/sprite/Sprite.hpp
    #sprite/opengl
    include/sprite/opengl/SpriteGL.hpp
 
    #system
    include/system/Configuration.hpp
    include/system/Engine.hpp
    include/system/Video.hpp
    include/system/System.hpp
 
    
    #texture
    include/texture/CubeMap.hpp
    include/texture/Texture.hpp
    include/texture/TextureManager.hpp
    #texture/opengl
    include/texture/opengl/CubeMapGL.hpp
    include/texture/opengl/TextureGL.hpp
    include/texture/opengl/TextureManagerGL.hpp
    
    #util
    include/util/FPSCounter.hpp
    include/util/Globals.hpp
    include/util/Timer.hpp
    
    ###source files###
    #camera
    src/camera/Camera.cpp
    src/camera/FPCamera.cpp
    src/camera/FPCameraBase.cpp
    src/camera/FPQuaternionCamera.cpp
    src/camera/TrackballCamera.cpp
    src/camera/TrackballQuatCamera.cpp
    
    #drawing
    #drawing/opengl
    src/drawing/opengl/ModelDrawerGL.cpp
    
    #listener
    
    #mesh
    src/mesh/Mesh.cpp
    #mesh/opengl
    src/mesh/opengl/MeshGL.cpp
    src/mesh/opengl/MeshFactoryGL.cpp
    
    #material
    src/material/Material.cpp
    src/material/PhongMaterial.cpp
    
    #model
    src/model/Model.cpp
    src/model/PhongModel.cpp
    src/model/Vob.cpp
    #model/opengl
    src/model/opengl/AssimpModelLoader.cpp
    src/model/opengl/ModelGL.cpp
    src/model/opengl/ModelManagerGL.cpp
    
    #renderer
    src/renderer/RendererOpenGL.cpp
    
    #shader
    src/shader/ShaderEnum.cpp
    #shader/opengl
    src/shader/opengl/LampShaderGL.cpp
    src/shader/opengl/PhongShaderGL.cpp
    src/shader/opengl/PhongTexShaderGL.cpp
    src/shader/opengl/PlaygroundShaderGL.cpp
    src/shader/opengl/ScreenShaderGL.cpp
    src/shader/opengl/ShaderGL.cpp
    src/shader/opengl/ShaderManagerGL.cpp
    src/shader/opengl/SimpleColorShaderGL.cpp
    src/shader/opengl/SimpleExtrudeShaderGL.cpp
    src/shader/opengl/SimpleLightShaderGL.cpp
    src/shader/opengl/SimpleReflectionShaderGL.cpp
    src/shader/opengl/SkyBoxShaderGL.cpp
    
    #sprite
    src/sprite/Sprite.cpp
    #sprite/opengl
    src/sprite/opengl/SpriteGL.cpp
    
    #system
    src/system/Configuration.cpp
    src/system/Engine.cpp
    src/system/System.cpp
    src/system/Video.cpp
    
    #texture
    #texture/opengl
    src/texture/opengl/CubeMapGL.cpp
    src/texture/opengl/TextureGL.cpp
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
    ${CMAKE_SOURCE_DIR}/libs/soil2/include
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
    ${CMAKE_SOURCE_DIR}/libs/assimp-3.3.1/include
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_library (engine ${ENGINE_SOURCES})
target_include_directories (engine PUBLIC 
include/ 
    ${CMAKE_SOURCE_DIR}/libs/glm/include 
    ${CMAKE_SOURCE_DIR}/libs/soil2/include
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
    ${CMAKE_SOURCE_DIR}/libs/assimp-3.3.1/include
)


#set additional library directories
#link_directories(.)

#choose the right libraries by absolute paths!
#find_library(libglew32_lib NAMES libglew32 HINTS ${CMAKE_SOURCE_DIR}/libs/glew/x64/Release)
find_library(soil2_lib NAMES soil2 HINTS ${CMAKE_SOURCE_DIR}/libs/soil2/x64/Release)

#boost libraries
find_library(libboost_program_options_release NAMES libboost_program_options-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_program_options_debug NAMES libboost_program_options-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

#assimp library
find_library(assimp_release NAMES assimp-vc140-mt HINTS ${CMAKE_SOURCE_DIR}/libs/assimp-3.3.1/x64/Release)
find_library(assimp_debug NAMES assimp-vc140-mt HINTS ${CMAKE_SOURCE_DIR}/libs/assimp-3.3.1/x64/Debug)


# Properties->Linker->Input->Additional Dependencies
target_link_libraries(engine PUBLIC opengl32)
target_link_libraries(engine PUBLIC ${soil2_lib})
target_link_libraries(engine PUBLIC glu32)

target_link_libraries(engine PUBLIC debug ${SDL2_lib_debug})
target_link_libraries(engine PUBLIC optimized ${SDL2_lib_release})
target_link_libraries(engine PUBLIC debug ${SDL2main_lib_debug})
target_link_libraries(engine PUBLIC optimized ${SDL2main_lib_release})

target_link_libraries(engine PUBLIC platform)   

#boost libraries
target_link_libraries(engine PUBLIC debug ${libboost_program_options_debug})
target_link_libraries(engine PUBLIC optimized ${libboost_program_options_release})

#assimp library
target_link_libraries(engine PUBLIC debug ${assimp_debug})
target_link_libraries(engine PUBLIC optimized ${assimp_release})