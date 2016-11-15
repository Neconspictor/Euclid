set(
    PLATFORM_SOURCES 
    
    #platform
    include/platform/FileSystem.hpp
    include/platform/Input.hpp
    include/platform/Platform.hpp
    include/platform/Renderer.hpp
    include/platform/Window.hpp
    
    #platform/event
    include/platform/event/EventChannel.hpp
    include/platform/event/EventHandler.hpp
    include/platform/event/EventHandlerQueue.hpp
    include/platform/event/GlobalEventChannel.hpp
    include/platform/event/Task.hpp
    include/platform/event/TaskManager.hpp
    
    
    #platform/logging
    include/platform/logging/GlobalLoggingServer.hpp
    include/platform/logging/LogEndpoint.hpp
    include/platform/logging/LogEndpoint.inl
    include/platform/logging/LoggingServer.hpp
    include/platform/logging/LoggingClient.hpp
    include/platform/logging/LogLevel.hpp
    include/platform/logging/LogMessage.hpp
    
    
    #platform/windows/
    include/platform/windows/PlatformWindows.hpp
    #platform/windows/input
    #platform/windows/input/SDL
    include/platform/windows/input/SDL/SDL.hpp
    include/platform/windows/input/SDL/SDLInitException.hpp
    include/platform/windows/input/SDL/SDLInputDevice.hpp
    #platform/windows/window
    include/platform/windows/window/WindowWin32.hpp
    
    #platform/util
    include/platform/util/CallbackContainer.hpp
    include/platform/util/Signal.hpp
    include/platform/util/StringUtils.hpp
    include/platform/util/TimeUtils.hpp
    include/platform/util/Util.hpp
    
    
    #platform/util/concurrent
    include/platform/util/concurrent/Active.hpp
    
    
    ###source files###
    src/platform/FileSystem.cpp
    src/platform/Input.cpp
	src/platform/Platform.cpp
    src/platform/Renderer.cpp
    src/platform/Window.cpp
    
    #platform/event
    src/platform/event/Task.cpp
    src/platform/event/TaskManager.cpp
    
    #platform/logging
    src/platform/logging/GlobalLoggingServer.cpp
    src/platform/logging/LogEndpoint.cpp
    src/platform/logging/LoggingServer.cpp
    src/platform/logging/LoggingClient.cpp
    src/platform/logging/LogLevel.cpp
    src/platform/logging/LogMessage.cpp
    
    #platform/windows
    src/platform/windows/PlatformWindows.cpp
    #platform/windows/input
    #platform/windows/input/SDL
    src/platform/windows/input/SDL/SDLInputDevice.cpp
    #platform/windows/window
    src/platform/windows/window/WindowWin32.cpp
    
    #platform/util
    src/platform/util/StringUtils.cpp
    #platform/util/concurrent
    src/platform/util/concurrent/Active.cpp
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${PLATFORM_SOURCES})
assign_source_group(${PLATFORM_SOURCES})

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
add_library (platform ${PLATFORM_SOURCES})
target_include_directories (platform PUBLIC 
    include
    ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2
)


#set additional library directories
#link_directories(.)

#choose the right libraries by absolute paths!
find_library(libglew32_lib NAMES libglew32 HINTS ${CMAKE_SOURCE_DIR}/libs/glew/x64/Release)


find_library(SDL2_lib_debug NAMES SDL2 HINTS ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Debug)
find_library(SDL2_lib_release NAMES SDL2 HINTS ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Release)
find_library(SDL2main_lib_debug NAMES SDL2main HINTS ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Debug)
find_library(SDL2main_lib_release NAMES SDL2main HINTS ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Release)

find_library(brofiler_lib NAMES ProfilerCore64 HINTS ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2)

# boost libraries libs\boost_1_62_0\x64\Debug
find_library(libboost_chrono_release NAMES libboost_chrono-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_chrono_debug NAMES libboost_chrono-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_date_time_release NAMES libboost_date_time-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_date_time_debug NAMES libboost_date_time-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_system_release NAMES libboost_system-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_system_debug NAMES libboost_system-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_thread_release NAMES libboost_thread-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_thread_debug NAMES libboost_thread-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

# Properties->Linker->Input->Additional Dependencies
target_link_libraries(platform PUBLIC opengl32)
target_link_libraries(platform PUBLIC ${libglew32_lib})
target_link_libraries(platform PUBLIC glu32)

target_link_libraries(platform PUBLIC debug ${SDL2_lib_debug})
target_link_libraries(platform PUBLIC optimized ${SDL2_lib_release})
target_link_libraries(platform PUBLIC debug ${SDL2main_lib_debug})
target_link_libraries(platform PUBLIC optimized ${SDL2main_lib_release})

#boost libraries
target_link_libraries(platform PUBLIC debug ${libboost_chrono_debug})
target_link_libraries(platform PUBLIC optimized ${libboost_chrono_release})

target_link_libraries(platform PUBLIC debug ${libboost_date_time_debug})
target_link_libraries(platform PUBLIC optimized ${libboost_date_time_release})

target_link_libraries(platform PUBLIC debug ${libboost_system_debug})
target_link_libraries(platform PUBLIC optimized ${libboost_system_release})

target_link_libraries(platform PUBLIC debug ${libboost_thread_debug})
target_link_libraries(platform PUBLIC optimized ${libboost_thread_release})

target_link_libraries(platform PUBLIC  ${brofiler_lib})