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

find_library(test_lib NAMES ${PLATFORM_TARGET_NAME} HINTS ${CMAKE_SOURCE_DIR}/bin)

find_library(libboost_chrono_release NAMES libboost_chrono-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_chrono_debug NAMES libboost_chrono-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_date_time_release NAMES libboost_date_time-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_date_time_debug NAMES libboost_date_time-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_system_release NAMES libboost_system-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_system_debug NAMES libboost_system-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

find_library(libboost_thread_release NAMES libboost_thread-vc140-mt-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)
find_library(libboost_thread_debug NAMES libboost_thread-vc140-mt-gd-1_62 HINTS ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/x64)

#project dependencies
find_library(platform_lib_release NAMES platform HINTS ${CMAKE_SOURCE_DIR}/bin)
find_library(platform_lib_debug NAMES platformd HINTS ${CMAKE_SOURCE_DIR}/bin)




#project dependencies
target_link_libraries(Platform_Test PUBLIC ${test_lib})
target_link_libraries(Platform_Test PUBLIC debug ${platform_lib_debug})
target_link_libraries(Platform_Test PUBLIC optimized ${platform_lib_release})