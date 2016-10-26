cmake_minimum_required (VERSION 3.6.2)

#utility functions
function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

# Maps to a solution file (OGL_Playground.sln). The solution will 
# have all targets (exe, lib, dll) as projects (.vcproj)
project (OGL_Playground)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set compiler flags and options. 
# Here it is setting the Visual Studio warning level to 4
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")

# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
message ("cxx Flags: " ${CMAKE_CXX_FLAGS})

#preprocessor definitions; in order to get defines working as preprocessor definitions, -D has to be put in front
ADD_DEFINITIONS(-DGLEW_STATIC)


# Collect sources into the variable MATH_SOURCES with 
# having to explicitly list each header and source file
#file (GLOB MY_SOURCES
#	  "src/*.cpp"
#	  "src/platform/*.cpp"
#	  "src/platform/mockup/*.cpp"
#	  "src/platform/mockup/window/*.cpp"
#	  "src/platform/windows/*.cpp"
#	  "src/platform/windows/input/*.cpp"
#	  "src/platform/windows/window/*.cpp")

set(
    MY_SOURCES 
    
    ###header files###
    #camera
    include/camera/Camera.hpp
    
    #input
    include/input/Input.hpp
    
    #listener
    include/listener/ScrollListener.hpp
    
    #mesh
    include/mesh/TestMeshes.hpp
    
    #model
    include/model/BasicModel.hpp
    include/model/Model.hpp
    include/model/ModelFactory.hpp
    
    #platform
    include/platform/Platform.hpp
    #platform/mockup
    include/platform/mockup/PlatformMockup.hpp
    #platform/mockup/window
    include/platform/mockup/window/WindowMockup.hpp
    #platform/windows/
    include/platform/windows/PlatformWindows.hpp
    #platform/windows/input
    #platform/windows/input/gainput
    include/platform/windows/input/gainput/HoldResetGesture.hpp
    include/platform/windows/input/gainput/WindowsInputDevice.hpp
    #platform/windows/input/OIS
    include/platform/windows/input/OIS/OISWindowsID.hpp
    #platform/windows/input/SDL
    include/platform/windows/input/SDL/SDL.hpp
    include/platform/windows/input/SDL/SDLInitException.hpp
    include/platform/windows/input/SDL/SDLInputDevice.hpp
    #platform/windows/window
    include/platform/windows/window/WindowWin32.hpp
    
    #renderer
    include/renderer/AbstractRendererFactory.hpp
    include/renderer/Renderer.hpp
    include/renderer/RendererOpenGL.hpp
    
    #shader
    include/shader/LampShader.hpp
    include/shader/PlaygroundShader.hpp
    include/shader/Shader.hpp
    include/shader/SimpleLightShader.hpp
    
    #texture
    include/texture/TextureManager.hpp

    #util
    include/util/CallbackContainer.hpp
    include/util/Signal.hpp
    include/util/Util.hpp
    
    #window
    include/window/Window.hpp
    
    
    ###source files###
    src/Main.cpp
    
    #camera
    src/camera/Camera.cpp
    
    #input
    src/input/Input.cpp
    
    #listener
    
    #mesh
    #src/mesh/TestMeshes.cpp
    
    #model
    src/model/BasicModel.cpp
    src/model/Model.cpp
    src/model/ModelFactory.cpp
    
    #platform
    #platform/mockup
    src/platform/mockup/PlatformMockup.cpp
    #platform/mockup/window
    src/platform/mockup/window/WindowMockup.cpp
    #platform/windows
    src/platform/windows/PlatformWindows.cpp
    #platform/windows/input
    #platform/windows/input/gainput
    src/platform/windows/input/gainput/HoldResetGesture.cpp
    src/platform/windows/input/gainput/WindowsInputDevice.cpp
    #platform/windows/input/OIS
    src/platform/windows/input/OIS/OISWindowsID.cpp
    #platform/windows/input/SDL
    src/platform/windows/input/SDL/SDLInputDevice.cpp
    #platform/windows/window
    src/platform/windows/window/WindowWin32.cpp
    
    #renderer
    src/renderer/RendererOpenGL.cpp
    
    #shader
    src/shader/LampShader.cpp
    src/shader/PlaygroundShader.cpp
    src/shader/Shader.cpp
    src/shader/SimpleLightShader.cpp
    
    #texture
    src/texture/TextureManager.cpp
    
    #util
    
    #window
    src/window/Window.cpp
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${MY_SOURCES})
assign_source_group(${MY_SOURCES})

# Properties->C/C++->General->Additional Include Directories
include_directories (
    include/ 
    ./libs/glm/include 
    ./libs/gainput/include 
    ./libs/glew/include 
    ./libs/glfw/include 
    ./libs/OIS/include 
    ./libs/soil2/include
    ./libs/SDL2.0.5/include
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_executable (OGL_Playground ${MY_SOURCES})


#set additional library directories
link_directories(.)

#choose the right libraries by absolute paths!
find_library(glfw3_lib NAMES glfw3 HINTS ./libs/glfw/x64/Release)
find_library(libglew32_lib NAMES libglew32 HINTS ./libs/glew/x64/Release)
find_library(soil2_lib NAMES soil2 HINTS ./libs/soil2/x64/Release)

find_library(gainputstatic_lib_debug NAMES gainputstatic HINTS ./libs/gainput/x64/Debug)
find_library(gainputstatic_lib_release NAMES gainputstatic HINTS ./libs/gainput/x64/Release)

find_library(ois_lib_debug NAMES ois_d HINTS ./libs/OIS/x64/Debug)
find_library(ois_lib_release NAMES ois HINTS ./libs/OIS/x64/Release)

find_library(SDL2_lib NAMES SDL2 HINTS ./libs/SDL2.0.5/x64/Release)
find_library(SDL2main_lib NAMES SDL2main HINTS ./libs/SDL2.0.5/x64/Release)


# Properties->Linker->Input->Additional Dependencies
target_link_libraries(OGL_Playground PUBLIC opengl32)
target_link_libraries(OGL_Playground PUBLIC ${glfw3_lib})
target_link_libraries(OGL_Playground PUBLIC ${libglew32_lib})
target_link_libraries(OGL_Playground PUBLIC ${soil2_lib})
target_link_libraries(OGL_Playground PUBLIC glu32)
target_link_libraries(OGL_Playground PUBLIC debug ${gainputstatic_lib_debug})
target_link_libraries(OGL_Playground PUBLIC optimized ${gainputstatic_lib_release})
target_link_libraries(OGL_Playground PUBLIC Xinput9_1_0)
target_link_libraries(OGL_Playground PUBLIC ws2_32)
target_link_libraries(OGL_Playground PUBLIC debug ${ois_lib_debug})
target_link_libraries(OGL_Playground PUBLIC optimized ${ois_lib_release})
target_link_libraries(OGL_Playground PUBLIC ${SDL2_lib})
target_link_libraries(OGL_Playground PUBLIC ${SDL2main_lib})


# Adds logic to INSTALL.vcproj to copy app.exe to destination directory
install (TARGETS OGL_Playground
		 RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin)