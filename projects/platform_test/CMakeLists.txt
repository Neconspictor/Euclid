set(
    PLATFORM_TEST_SOURCES 
    
    ###header files###
    include/LoggingTester.hpp
    include/TaskManagementTester.hpp
    include/PlatformTestMain.hpp
    
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
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_executable (Platform_Test ${PLATFORM_TEST_SOURCES})

#choose the right libraries by absolute paths!
target_link_libraries(Platform_Test PUBLIC platform)