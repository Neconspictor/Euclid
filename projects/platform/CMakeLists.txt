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
    
    #platform/exception
    include/platform/exception/EnumFormatException.hpp;
    include/platform/exception/OpenglException.hpp;
    include/platform/exception/UnexpectedPlatformException.hpp;
    
    
    #platform/logging
    include/platform/logging/GlobalLoggingServer.hpp
    include/platform/logging/LogEndpoint.hpp
    include/platform/logging/LogEndpoint.inl
    include/platform/logging/LoggingServer.hpp
    include/platform/logging/LoggingClient.hpp
    include/platform/logging/LogLevel.hpp
    include/platform/logging/LogMessage.hpp
    
    #platform/memory
    include/platform/memory/Allocator.hpp
    include/platform/memory/FreeListAllocator.hpp
    include/platform/memory/LinearAllocator.hpp
    include/platform/memory/MemoryManager.hpp
    include/platform/memory/ObjectPool.hpp
    include/platform/memory/PoolAllocator.hpp
    include/platform/memory/ProxyAllocator.hpp
    include/platform/memory/StackAllocator.hpp
    
    
    #platform/window_system/
    #platform/window_system/glfw/
    include/platform/window_system/glfw/WindowGLFW.hpp
    include/platform/window_system/glfw/WindowSystemGLFW.hpp
    
    #platform/windows/
    include/platform/windows/PlatformWindows.hpp
    
    #platform/util
    include/platform/util/CallbackContainer.hpp
    include/platform/util/PointerUtils.hpp
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
    
    #platform/memory
    src/platform/memory/FreeListAllocator.cpp
    src/platform/memory/LinearAllocator.cpp
    src/platform/memory/MemoryManager.cpp
    src/platform/memory/PoolAllocator.cpp
    src/platform/memory/ProxyAllocator.cpp
    src/platform/memory/StackAllocator.cpp
    
    #platform/window_system/
    #platform/window_system/glfw/
    src/platform/window_system/glfw/WindowGLFW.cpp
    src/platform/window_system/glfw/WindowSystemGLFW.cpp
    
    #platform/windows
    src/platform/windows/PlatformWindows.cpp
    
    #platform/util
    src/platform/util/StringUtils.cpp
    #platform/util/concurrent
    src/platform/util/concurrent/Active.cpp
    
    #external resources
     ${CMAKE_SOURCE_DIR}/libs/glad/src/glad.c
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${PLATFORM_SOURCES})
assign_source_group(${PLATFORM_SOURCES})

# Properties->C/C++->General->Additional Include Directories
include_directories (
    include/ 
    ${CMAKE_SOURCE_DIR}/libs/glm/include 
    ${CMAKE_SOURCE_DIR}/libs/soil2/include
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
    ${CMAKE_SOURCE_DIR}/libs/glfw/include
    ${CMAKE_SOURCE_DIR}/libs/glad/include
    ${CMAKE_SOURCE_DIR}/libs/utf8-cpp/source
    #${CMAKE_SOURCE_DIR}/libs/nana/include
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_library (platform ${PLATFORM_SOURCES})
target_include_directories (platform PUBLIC 
    include
    ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2
    ${CMAKE_SOURCE_DIR}/libs/glad/include
    ${CMAKE_SOURCE_DIR}/libs/glfw/include
    #${CMAKE_SOURCE_DIR}/libs/nana/include
)


#set additional library directories
#link_directories(.)

#choose the right libraries by absolute paths!
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

find_library(boost_locale_release NAMES boost_locale-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(boost_locale_debug NAMES boost_locale-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)


#glfw
find_library(glfw3_release NAMES glfw3 HINTS ${CMAKE_SOURCE_DIR}/libs/glfw/x64/Release)
find_library(glfw3_debug NAMES glfw3 HINTS ${CMAKE_SOURCE_DIR}/libs/glfw/x64/Debug)

#nana
#find_library(nana_debug NAMES nana_v140_Debug_x64 HINTS ${CMAKE_SOURCE_DIR}/libs/nana/lib/x64)
#find_library(nana_release NAMES nana_v140_Release_x64 HINTS ${CMAKE_SOURCE_DIR}/libs/nana/lib/x64)


# Properties->Linker->Input->Additional Dependencies
target_link_libraries(platform PUBLIC opengl32)
target_link_libraries(platform PUBLIC glu32)


#boost libraries
target_link_libraries(platform PUBLIC debug ${libboost_chrono_debug})
target_link_libraries(platform PUBLIC optimized ${libboost_chrono_release})

target_link_libraries(platform PUBLIC debug ${libboost_date_time_debug})
target_link_libraries(platform PUBLIC optimized ${libboost_date_time_release})

target_link_libraries(platform PUBLIC debug ${libboost_system_debug})
target_link_libraries(platform PUBLIC optimized ${libboost_system_release})

target_link_libraries(platform PUBLIC debug ${libboost_thread_debug})
target_link_libraries(platform PUBLIC optimized ${libboost_thread_release})

target_link_libraries(platform PUBLIC debug ${boost_locale_debug})
target_link_libraries(platform PUBLIC optimized ${boost_locale_release})

# Brofiler
target_link_libraries(platform PUBLIC  ${brofiler_lib})

#GLFW
target_link_libraries(platform PUBLIC debug ${glfw3_release})
target_link_libraries(platform PUBLIC optimized ${glfw3_release})

#nana
#target_link_libraries(platform PUBLIC debug ${nana_debug})
#target_link_libraries(platform PUBLIC optimized ${nana_release})