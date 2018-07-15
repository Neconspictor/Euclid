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

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_library (platform ${PLATFORM_SOURCES})

target_include_directories (platform PUBLIC 
    ./
#    #${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2
#    ${CMAKE_SOURCE_DIR}/libs/gli/include
#    ${CMAKE_SOURCE_DIR}/libs/DDS/include
)


#set additional library directories
#link_directories(.)

#choose the right libraries by absolute paths!
#find_library(brofiler_lib NAMES ProfilerCore64 HINTS ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2)

# Brofiler
#target_link_libraries(platform PUBLIC  ${brofiler_lib})


find_package(Boost 1.67 EXACT REQUIRED COMPONENTS atomic chrono date_time filesystem locale program_options system thread)
find_package(GLAD REQUIRED)
find_package(GLM REQUIRED)
find_package(IMGUI REQUIRED)
find_package(STB REQUIRED)

target_link_libraries(platform

        PUBLIC

        GLM
        
		Boost::atomic
        Boost::boost
        Boost::chrono
        Boost::date_time
        Boost::filesystem
        Boost::locale
        Boost::program_options
        Boost::system
        Boost::thread
		
		IMGUI
		
		STB
)