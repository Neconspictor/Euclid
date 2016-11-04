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

#project dependencies
find_library(platform_lib_release NAMES platform HINTS ${CMAKE_SOURCE_DIR}/bin)
find_library(platform_lib_debug NAMES platformd HINTS ${CMAKE_SOURCE_DIR}/bin)

set (engine_lib_release  ${CMAKE_SOURCE_DIR}/bin/engine.lib)
set (engine_lib_debug  ${CMAKE_SOURCE_DIR}/bin/engined.lib)

#project dependencies
target_link_libraries(Engine_Test PUBLIC debug ${platform_lib_debug})
target_link_libraries(Engine_Test PUBLIC optimized ${platform_lib_release})

target_link_libraries(Engine_Test PUBLIC debug ${engine_lib_debug})
target_link_libraries(Engine_Test PUBLIC optimized ${engine_lib_release})

target_link_libraries(Engine_Test PUBLIC debug "${PLATFORM_DEPENDENCIES_LIBS_DEBUG}")
target_link_libraries(Engine_Test PUBLIC optimized "${PLATFORM_DEPENDENCIES_LIBS_RELEASE}")

target_link_libraries(Engine_Test PUBLIC debug "${ENGINE_DEPENDENCIES_LIBS_DEBUG}")
target_link_libraries(Engine_Test PUBLIC optimized "${ENGINE_DEPENDENCIES_LIBS_RELEASE}")

add_dependencies(Engine_Test platform engine)

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