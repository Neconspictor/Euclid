if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()

set(
    PLATFORM_SOURCES 
    
    #platform
    include/platform/FileSystem.hpp
    include/platform/Input.hpp
    include/platform/Platform.hpp
    include/platform/Renderer.hpp
    include/platform/Window.hpp
    
    #platform/event
    include/platform/event/Channel.hpp
    include/platform/event/EventChannel.hpp
    include/platform/event/EventHandler.hpp
    include/platform/event/EventHandlerQueue.hpp
    include/platform/event/Task.hpp
    include/platform/event/TaskManager.hpp
    
    
    #platform/logging
    include/platform/logging/Logger.hpp
    include/platform/logging/LogLevel.hpp
    include/platform/logging/LogMessage.hpp
    include/platform/logging/LogSink.hpp
    include/platform/logging/LogSink.inl
    
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
    
    #platform/util
    include/platform/util/CallbackContainer.hpp
    include/platform/util/Signal.hpp
    include/platform/util/Util.hpp
    
    #platform/util/concurrent
    include/platform/util/concurrent/Active.hpp
    
    
    ###source files###
    src/platform/FileSystem.cpp
    src/platform/Input.cpp
    src/platform/Renderer.cpp
    src/platform/Window.cpp
    
    #platform/event
    src/platform/event/Task.cpp
    src/platform/event/TaskManager.cpp
    
    #platform/logging
    src/platform/logging/Logger.cpp
    src/platform/logging/LogLevel.cpp
    src/platform/logging/LogMessage.cpp
    src/platform/logging/LogSink.cpp
    
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
    
    #platform/util
    src/platform/util/Util.cpp
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
    ${CMAKE_SOURCE_DIR}/libs/gainput/include 
    ${CMAKE_SOURCE_DIR}/libs/glew/include 
    ${CMAKE_SOURCE_DIR}/libs/glfw/include 
    ${CMAKE_SOURCE_DIR}/libs/OIS/include 
    ${CMAKE_SOURCE_DIR}/libs/soil2/include
    ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/include
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_library ( ${PLATFORM_TARGET_NAME} ${PLATFORM_SOURCES})
#target_include_directories (${PLATFORM_TARGET_NAME} PUBLIC include)


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
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC opengl32)
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC ${glfw3_lib})
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC ${libglew32_lib})
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC ${soil2_lib})
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC glu32)

target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC debug ${gainputstatic_lib_debug})
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC optimized ${gainputstatic_lib_release})

target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC Xinput9_1_0)
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC ws2_32)

target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC debug ${ois_lib_debug})
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC optimized ${ois_lib_release})

target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC debug ${SDL2_lib_debug})
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC optimized ${SDL2_lib_release})
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC debug ${SDL2main_lib_debug})
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC optimized ${SDL2main_lib_release})

#boost libraries
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC debug ${libboost_chrono_debug})
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC optimized ${libboost_chrono_release})

target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC debug ${libboost_date_time_debug})
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC optimized ${libboost_date_time_release})

target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC debug ${libboost_system_debug})
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC optimized ${libboost_system_release})

target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC debug ${libboost_thread_debug})
target_link_libraries(${PLATFORM_TARGET_NAME} PUBLIC optimized ${libboost_thread_release})

set(PLATFORM_DEPENDENCIES_LIBS_DEBUG
    opengl32
    ${glfw3_lib}
    ${libglew32_lib}
    ${soil2_lib}
    ${gainputstatic_lib_debug}
    Xinput9_1_0
    ws2_32
    ${ois_lib_debug}
    ${SDL2_lib_debug}
    ${SDL2main_lib_debug}
    ${libboost_chrono_debug}
    ${libboost_date_time_debug}
    ${libboost_system_debug}
    ${libboost_thread_debug}
    
    PARENT_SCOPE
)

set(PLATFORM_DEPENDENCIES_LIBS_RELEASE
    opengl32
    ${glfw3_lib}
    ${libglew32_lib}
    ${soil2_lib}
    ${gainputstatic_lib_release}
    Xinput9_1_0
    ws2_32
    ${ois_lib_release}
    ${SDL2_lib_release}
    ${SDL2main_lib_release}
    ${libboost_chrono_release}
    ${libboost_date_time_release}
    ${libboost_system_release}
    ${libboost_thread_release}
    
    PARENT_SCOPE
)


add_custom_command(TARGET ${PLATFORM_TARGET_NAME} POST_BUILD
    #COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${ENGINE_TARGET_NAME}>" "${CMAKE_SOURCE_DIR}/bin/$<$<CONFIG:debug>:Debug>$<$<CONFIG:release>:Release>/${ENGINE_TARGET_NAME}$<$<CONFIG:debug>:d>.lib"
    
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${PLATFORM_TARGET_NAME}>" "${CMAKE_SOURCE_DIR}/bin/${PLATFORM_TARGET_NAME}$<$<CONFIG:debug>:d>.lib"
    #COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Release/SDL2.dll" "$<TARGET_FILE_DIR:Engine>/SDL2.dll"
)