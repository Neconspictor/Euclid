set(
    PLATFORM_SOURCES 
    
    #platform
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform/Platform.hpp
    include/platform/Input.hpp
    include/platform/Renderer.hpp
    include/platform/Window.hpp
    
    #platform/mockup
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform/mockup/PlatformMockup.hpp
    #platform/mockup/window
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform/mockup/window/WindowMockup.hpp
    #platform/windows/
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform/windows/PlatformWindows.hpp
    #platform/windows/input
    #platform/windows/input/gainput
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform/windows/input/gainput/HoldResetGesture.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform/windows/input/gainput/WindowsInputDevice.hpp
    #platform/windows/input/OIS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform/windows/input/OIS/OISWindowsID.hpp
    #platform/windows/input/SDL
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform/windows/input/SDL/SDL.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform/windows/input/SDL/SDLInitException.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform/windows/input/SDL/SDLInputDevice.hpp
    #platform/windows/window
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform/windows/window/WindowWin32.hpp
    
    #util
    include/platform/util/CallbackContainer.hpp
    include/platform/util/Signal.hpp
    include/platform/util/Util.hpp
    
    
    ###source files###
    
    src/platform/Input.cpp
    src/platform/Renderer.cpp
    src/platform/Window.cpp
    
    #platform
    #platform/mockup
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/mockup/PlatformMockup.cpp
    #platform/mockup/window
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/mockup/window/WindowMockup.cpp
    #platform/windows
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/windows/PlatformWindows.cpp
    #platform/windows/input
    #platform/windows/input/gainput
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/windows/input/gainput/HoldResetGesture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/windows/input/gainput/WindowsInputDevice.cpp
    #platform/windows/input/OIS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/windows/input/OIS/OISWindowsID.cpp
    #platform/windows/input/SDL
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/windows/input/SDL/SDLInputDevice.cpp
    #platform/windows/window
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/windows/window/WindowWin32.cpp
    
    #util
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${PLATFORM_SOURCES})
assign_source_group(${PLATFORM_SOURCES})

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
add_library (platform ${PLATFORM_SOURCES})
target_include_directories (platform PUBLIC include)


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
target_link_libraries(platform PUBLIC opengl32)
target_link_libraries(platform PUBLIC ${glfw3_lib})
target_link_libraries(platform PUBLIC ${libglew32_lib})
target_link_libraries(platform PUBLIC ${soil2_lib})
target_link_libraries(platform PUBLIC glu32)

target_link_libraries(platform PUBLIC debug ${gainputstatic_lib_debug})
target_link_libraries(platform PUBLIC optimized ${gainputstatic_lib_release})

target_link_libraries(platform PUBLIC Xinput9_1_0)
target_link_libraries(platform PUBLIC ws2_32)

target_link_libraries(platform PUBLIC debug ${ois_lib_debug})
target_link_libraries(platform PUBLIC optimized ${ois_lib_release})

target_link_libraries(platform PUBLIC debug ${SDL2_lib_debug})
target_link_libraries(platform PUBLIC optimized ${SDL2_lib_release})
target_link_libraries(platform PUBLIC debug ${SDL2main_lib_debug})
target_link_libraries(platform PUBLIC optimized ${SDL2main_lib_release})