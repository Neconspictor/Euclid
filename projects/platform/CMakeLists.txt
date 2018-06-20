set(
    PLATFORM_SOURCES 
		
    pch_platform.hpp
    
    #platform
    platform/FileSystem.hpp
	platform/FileSystem.cpp
    platform/Input.hpp
	platform/Input.cpp
    platform/PlatformProvider.hpp
    platform/Renderer.hpp
	platform/Renderer.cpp
    platform/Window.hpp
	platform/Window.cpp
    
    #platform/event
    platform/event/EventChannel.hpp
    platform/event/EventHandler.hpp
    platform/event/EventHandlerQueue.hpp
    platform/event/GlobalEventChannel.hpp
    platform/event/Task.hpp
	platform/event/Task.cpp
    platform/event/TaskManager.hpp
	platform/event/TaskManager.cpp
	
	#platform/extern
	#platform/extern/imgui
	platform/extern/imgui/imconfig.h
	platform/extern/imgui/imgui.cpp
	platform/extern/imgui/imgui.h
	platform/extern/imgui/imgui_demo.cpp
	platform/extern/imgui/imgui_draw.cpp
	platform/extern/imgui/imgui_internal.h
	platform/extern/imgui/stb_rect_pack.h
	platform/extern/imgui/stb_textedit.h
	platform/extern/imgui/stb_truetype.h
    
    #platform/exception
    platform/exception/EnumFormatException.hpp
    platform/exception/OpenglException.hpp
    platform/exception/UnexpectedPlatformException.hpp
	
	#platform/gui
	platform/gui/ImGUI.hpp
    
    
    #platform/logging
    platform/logging/GlobalLoggingServer.hpp
	platform/logging/GlobalLoggingServer.cpp
    platform/logging/LogEndpoint.hpp
    platform/logging/LogEndpoint.inl
	platform/logging/LogEndpoint.cpp
    platform/logging/LoggingServer.hpp
	platform/logging/LoggingServer.cpp
    platform/logging/LoggingClient.hpp
	platform/logging/LoggingClient.cpp
    platform/logging/LogLevel.hpp
	platform/logging/LogLevel.cpp
    platform/logging/LogMessage.hpp
	platform/logging/LogMessage.cpp
    
    #platform/memory
    platform/memory/Allocator.hpp
    platform/memory/FreeListAllocator.hpp
	platform/memory/FreeListAllocator.cpp
    platform/memory/LinearAllocator.hpp
	platform/memory/LinearAllocator.cpp
    platform/memory/MemoryManager.hpp
	platform/memory/MemoryManager.cpp
    platform/memory/ObjectPool.hpp
    platform/memory/PoolAllocator.hpp
	platform/memory/PoolAllocator.cpp
    platform/memory/ProxyAllocator.hpp
	platform/memory/ProxyAllocator.cpp
    platform/memory/StackAllocator.hpp
	platform/memory/StackAllocator.cpp
    
    #platform/util
    platform/util/CallbackContainer.hpp
	platform/util/MathUtils.hpp
    platform/util/PointerUtils.hpp
    platform/util/Signal.hpp
    platform/util/StringUtils.hpp
	platform/util/StringUtils.cpp
    platform/util/TimeUtils.hpp
    platform/util/Util.hpp
    
    
    #platform/util/concurrent
    platform/util/concurrent/Active.hpp
	platform/util/concurrent/Active.cpp
)

#set (
#		PLATFORM_EXTERNAL_SOURCES
#		${CMAKE_SOURCE_DIR}/libs/glad/src/glad.c
#)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${PLATFORM_SOURCES})
assign_source_group(${PLATFORM_SOURCES})

enable_precompiled_headers(pch_platform.hpp PLATFORM_SOURCES)

# Properties->C/C++->General->Additional Include Directories
include_directories (
    ./ 
    ${CMAKE_SOURCE_DIR}/libs/glm/include 
    ${CMAKE_SOURCE_DIR}/libs/soil2/include
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
    #${CMAKE_SOURCE_DIR}/libs/glfw/include
    #${CMAKE_SOURCE_DIR}/libs/glad/include
    #${CMAKE_SOURCE_DIR}/libs/nanogui/ext/glad/include
    ${CMAKE_SOURCE_DIR}/libs/utf8-cpp/source
    #${CMAKE_SOURCE_DIR}/libs/nanogui/include
    #${CMAKE_SOURCE_DIR}/libs/nanogui/ext/eigen
    #${CMAKE_SOURCE_DIR}/libs/nanogui/ext/nanovg/src
    ${CMAKE_SOURCE_DIR}/libs/gli/include
    #${CMAKE_SOURCE_DIR}/libs/nana/include
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_library (platform ${PLATFORM_SOURCES})
target_include_directories (platform PUBLIC 
    ./
    ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2
    #${CMAKE_SOURCE_DIR}/libs/glad/include
    #${CMAKE_SOURCE_DIR}/libs/nanogui/ext/glad/include
    #${CMAKE_SOURCE_DIR}/libs/glfw/include
    #${CMAKE_SOURCE_DIR}/libs/nanogui/include
    #${CMAKE_SOURCE_DIR}/libs/nanogui/ext/eigen
    #${CMAKE_SOURCE_DIR}/libs/nanogui/ext/nanovg/src
    ${CMAKE_SOURCE_DIR}/libs/gli/include
    ${CMAKE_SOURCE_DIR}/libs/DDS/include
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

find_library(libboost_filesystem_release NAMES libboost_filesystem-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_filesystem_debug NAMES libboost_filesystem-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)



#glfw
#find_library(glfw3_release NAMES glfw3 HINTS ${CMAKE_SOURCE_DIR}/libs/glfw/x64/Release)
#find_library(glfw3_debug NAMES glfw3 HINTS ${CMAKE_SOURCE_DIR}/libs/glfw/x64/Debug)

#nanogui
#find_library(nanogui_release NAMES nanogui HINTS ${CMAKE_SOURCE_DIR}/libs/nanogui/x64/Release)
#find_library(nanogui_debug NAMES nanogui HINTS ${CMAKE_SOURCE_DIR}/libs/nanogui/x64/Debug)


#nana
#find_library(nana_debug NAMES nana_v140_Debug_x64 HINTS ${CMAKE_SOURCE_DIR}/libs/nana/lib/x64)
#find_library(nana_release NAMES nana_v140_Release_x64 HINTS ${CMAKE_SOURCE_DIR}/libs/nana/lib/x64)


# Properties->Linker->Input->Additional Dependencies
#target_link_libraries(platform PUBLIC opengl32)
#target_link_libraries(platform PUBLIC glu32)


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

target_link_libraries(platform PUBLIC debug ${libboost_filesystem_debug})
target_link_libraries(platform PUBLIC optimized ${libboost_filesystem_release})

# Brofiler
target_link_libraries(platform PUBLIC  ${brofiler_lib})

#nana
#target_link_libraries(platform PUBLIC debug ${nana_debug})
#target_link_libraries(platform PUBLIC optimized ${nana_release})