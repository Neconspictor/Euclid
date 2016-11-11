set(
    APP_SOURCES 
    
    ###header files###
    include/EngineTester.hpp
    include/Boost_OptionsTest.hpp
    ###source files###
    src/EngineTester.cpp
    src/EngineTestMain.cpp
    src/Boost_OptionsTest.cpp
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${ENGINE_SOURCES})
assign_source_group(${APP_SOURCES})

# Properties->C/C++->General->Additional Include Directories
include_directories (
    include/ 
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_executable (App ${APP_SOURCES})

#choose the right libraries by absolute paths!
target_link_libraries(App PUBLIC engine)

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

add_custom_command(TARGET App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$<$<CONFIG:debug>:${SDL_DEBUG_DLL}>$<$<CONFIG:release>:${SDL_RELEASE_DLL}>" "$<TARGET_FILE_DIR:App>/SDL2.dll"
    #COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/libs/SDL2.0.5/x64/Release/SDL2.dll" "$<TARGET_FILE_DIR:Engine>/SDL2.dll"
)

# Adds logic to INSTALL.vcproj to copy app.exe to destination directory
install (TARGETS App
		 RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin)
         
install (FILES "${SDL_RELEASE_DLL}" DESTINATION ${CMAKE_SOURCE_DIR}/bin CONFIGURATIONS Release)
install (FILES "${SDL_DEBUG_DLL}" DESTINATION ${CMAKE_SOURCE_DIR}/bin CONFIGURATIONS Debug)