
if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()

set(ENGINE_TARGET_NAME engine) 
# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer

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
    
    #system
    include/system/Configuration.hpp
    include/system/Engine.hpp
    include/system/System.hpp
    
    #texture
    include/texture/TextureManager.hpp
    
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
    src/shader/LampShader.cpp
    src/shader/PlaygroundShader.cpp
    src/shader/Shader.cpp
    src/shader/SimpleLightShader.cpp
    
    #system
    src/system/Configuration.cpp
    src/system/Engine.cpp
    src/system/System.cpp
    
    #texture
    src/texture/TextureManager.cpp
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${ENGINE_SOURCES})
assign_source_group(${ENGINE_SOURCES})

add_library (${ENGINE_TARGET_NAME} ${ENGINE_SOURCES})
set_property(TARGET ${ENGINE_TARGET_NAME} PROPERTY DBG_POSTFIX d)


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
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
)

target_include_directories (${ENGINE_TARGET_NAME} PUBLIC 
include/ 
    ${CMAKE_SOURCE_DIR}/libs/glm/include 
    ${CMAKE_SOURCE_DIR}/libs/gainput/include 
    ${CMAKE_SOURCE_DIR}/libs/glew/include 
    ${CMAKE_SOURCE_DIR}/libs/glfw/include 
    ${CMAKE_SOURCE_DIR}/libs/OIS/include 
    ${CMAKE_SOURCE_DIR}/libs/soil2/include
    ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/include
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
    ${CMAKE_SOURCE_DIR}/projects/platform/include
)


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

#boost libraries
find_library(libboost_program_options_release NAMES libboost_program_options-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_program_options_debug NAMES libboost_program_options-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_chrono_release NAMES libboost_chrono-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_chrono_debug NAMES libboost_chrono-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_date_time_release NAMES libboost_date_time-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_date_time_debug NAMES libboost_date_time-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_system_release NAMES libboost_system-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_system_debug NAMES libboost_system-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_thread_release NAMES libboost_thread-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_thread_debug NAMES libboost_thread-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(test_lib NAMES ${PLATFORM_TARGET_NAME} HINTS ${CMAKE_SOURCE_DIR}/bin)

#project dependencies
find_library(platform_lib_release NAMES platform HINTS ${CMAKE_SOURCE_DIR}/bin)
find_library(platform_lib_debug NAMES platformd HINTS ${CMAKE_SOURCE_DIR}/bin)

# Properties->Linker->Input->Additional Dependencies
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC ${test_lib})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC opengl32)
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC ${glfw3_lib})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC ${libglew32_lib})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC ${soil2_lib})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC glu32)

target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC debug ${gainputstatic_lib_debug})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC optimized ${gainputstatic_lib_release})

target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC Xinput9_1_0)
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC ws2_32)

target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC debug ${ois_lib_debug})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC optimized ${ois_lib_release})

target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC debug ${SDL2_lib_debug})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC optimized ${SDL2_lib_release})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC debug ${SDL2main_lib_debug})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC optimized ${SDL2main_lib_release})

#target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC platform)

#boost libraries
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC debug ${libboost_program_options_debug})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC optimized ${libboost_program_options_release})

target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC debug ${libboost_chrono_debug})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC optimized ${libboost_chrono_release})

target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC debug ${libboost_date_time_debug})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC optimized ${libboost_date_time_release})

target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC debug ${libboost_system_debug})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC optimized ${libboost_system_release})

target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC debug ${libboost_thread_debug})
target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC optimized ${libboost_thread_release})

target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC debug ${PLATFORM_DEPENDENCIES_LIBS_DEBUG})

#project dependencies
#target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC debug ${platform_lib_debug})
#target_link_libraries(${ENGINE_TARGET_NAME} PUBLIC optimized ${platform_lib_release})


add_custom_command(TARGET ${ENGINE_TARGET_NAME} POST_BUILD
    #COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${ENGINE_TARGET_NAME}>" "${CMAKE_SOURCE_DIR}/bin/$<$<CONFIG:debug>:Debug>$<$<CONFIG:release>:Release>/${ENGINE_TARGET_NAME}$<$<CONFIG:debug>:d>.lib"
    
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${ENGINE_TARGET_NAME}>" "${CMAKE_SOURCE_DIR}/bin/${ENGINE_TARGET_NAME}$<$<CONFIG:debug>:d>.lib"
    #COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Release/SDL2.dll" "$<TARGET_FILE_DIR:Engine>/SDL2.dll"
)

set_property(TARGET ${ENGINE_TARGET_NAME} PROPERTY COMPILE_DEPENDS "")