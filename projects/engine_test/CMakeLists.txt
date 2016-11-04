set(
    ENGINE_TEST_SOURCES 
    
    ###header files###
    include/EngineTester.hpp
    ###source files###
    src/EngineTester.cpp
    src/EngineTestMain.cpp
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${ENGINE_SOURCES})
assign_source_group(${ENGINE_TEST_SOURCES})

# Properties->C/C++->General->Additional Include Directories
include_directories (
    include/ 
    ${CMAKE_SOURCE_DIR}/libs/glm/include  
    ${CMAKE_SOURCE_DIR}/libs/glew/include 
    ${CMAKE_SOURCE_DIR}/libs/glfw/include 
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
    ${CMAKE_SOURCE_DIR}/projects/engine/include
    ${CMAKE_SOURCE_DIR}/projects/platform/include
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_executable (Engine_Test ${ENGINE_TEST_SOURCES})

#choose the right libraries by absolute paths!
#target_link_libraries(Engine_Test PUBLIC engine)


# boost libraries
find_library(libboost_chrono_release NAMES libboost_chrono-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_chrono_debug NAMES libboost_chrono-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_date_time_release NAMES libboost_date_time-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_date_time_debug NAMES libboost_date_time-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_system_release NAMES libboost_system-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_system_debug NAMES libboost_system-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_thread_release NAMES libboost_thread-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_thread_debug NAMES libboost_thread-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_program_options_release NAMES libboost_program_options-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_program_options_debug NAMES libboost_program_options-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

#project dependencies
find_library(platform_lib_release NAMES platform HINTS ${CMAKE_SOURCE_DIR}/bin)
find_library(platform_lib_debug NAMES platformd HINTS ${CMAKE_SOURCE_DIR}/bin)

find_library(engine_lib_release NAMES engine HINTS ${CMAKE_SOURCE_DIR}/bin)
find_library(engine_lib_debug NAMES engined HINTS ${CMAKE_SOURCE_DIR}/bin)


#boost libraries
target_link_libraries(Engine_Test PUBLIC debug ${libboost_chrono_debug})
target_link_libraries(Engine_Test PUBLIC optimized ${libboost_chrono_release})

target_link_libraries(Engine_Test PUBLIC debug ${libboost_date_time_debug})
target_link_libraries(Engine_Test PUBLIC optimized ${libboost_date_time_release})

target_link_libraries(Engine_Test PUBLIC debug ${libboost_system_debug})
target_link_libraries(Engine_Test PUBLIC optimized ${libboost_system_release})

target_link_libraries(Engine_Test PUBLIC debug ${libboost_thread_debug})
target_link_libraries(Engine_Test PUBLIC optimized ${libboost_thread_release})

target_link_libraries(Engine_Test PUBLIC debug ${libboost_program_options_debug})
target_link_libraries(Engine_Test PUBLIC optimized ${libboost_program_options_release})


#project dependencies
target_link_libraries(Engine_Test PUBLIC debug ${platform_lib_debug})
target_link_libraries(Engine_Test PUBLIC optimized ${platform_lib_release})

target_link_libraries(Engine_Test PUBLIC debug ${engine_lib_debug})
target_link_libraries(Engine_Test PUBLIC optimized ${engine_lib_release})

target_link_libraries(Engine_Test PUBLIC debug ${PLATFORM_DEPENDENCIES_LIBS_DEBUG})
target_link_libraries(Engine_Test PUBLIC optimized ${PLATFORM_DEPENDENCIES_LIBS_RELEASE})

#if debug then ${DEBUG_EXE_PATH} elseif release then ${RELEASE_EXE_PATH}
#$<$<CONFIG:debug>:${DEBUG_EXE_PATH}>$<$<CONFIG:release>:${RELEASE_EXE_PATH}>

set (
    SDL_RELEASE_DLL
    ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Release/SDL2.dll
)

set (
    SDL_DEBUG_DLL
    ${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Debug/SDL2.dll
)

add_custom_command(TARGET Engine_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$<$<CONFIG:debug>:${SDL_DEBUG_DLL}>$<$<CONFIG:release>:${SDL_RELEASE_DLL}>" "$<TARGET_FILE_DIR:Engine_Test>/SDL2.dll"
    #COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Release/SDL2.dll" "$<TARGET_FILE_DIR:Engine>/SDL2.dll"
)

# Adds logic to INSTALL.vcproj to copy app.exe to destination directory
install (TARGETS Engine_Test
		 RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin)
         
install (FILES "${SDL_RELEASE_DLL}" DESTINATION ${CMAKE_SOURCE_DIR}/bin CONFIGURATIONS Release)
install (FILES "${SDL_DEBUG_DLL}" DESTINATION ${CMAKE_SOURCE_DIR}/bin CONFIGURATIONS Debug)