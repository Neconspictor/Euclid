set(
    PLATFORM_TEST_SOURCES 
    
    ###header files###
    include/LoggingTester.hpp
    include/TaskManagementTester.hpp
    
    ###source files###
    src/LoggingTester.cpp
    src/TaskManagementTester.cpp
    src/PlatformTestMain.cpp
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${ENGINE_SOURCES})
assign_source_group(${PLATFORM_TEST_SOURCES})

# Properties->C/C++->General->Additional Include Directories
include_directories (
    include/ 
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
    ${CMAKE_SOURCE_DIR}/projects/platform/include
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_executable (Platform_Test ${PLATFORM_TEST_SOURCES})

#choose the right libraries by absolute paths!
#target_link_libraries(Platform_Test PUBLIC platform)

#project dependencies
find_library(platform_lib_release NAMES platform HINTS ${CMAKE_SOURCE_DIR}/bin)
find_library(platform_lib_debug NAMES platformd HINTS ${CMAKE_SOURCE_DIR}/bin)


#project dependencies
target_link_libraries(Platform_Test PUBLIC debug ${platform_lib_debug})
target_link_libraries(Platform_Test PUBLIC optimized ${platform_lib_release})

target_link_libraries(Platform_Test PUBLIC debug "${PLATFORM_DEPENDENCIES_LIBS_DEBUG}")
target_link_libraries(Platform_Test PUBLIC optimized "${PLATFORM_DEPENDENCIES_LIBS_RELEASE}")