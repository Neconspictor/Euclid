cmake_minimum_required(VERSION 3.12)


# Maps to a solution file (OGL_Playground.sln). The solution will 
# have all targets (exe, lib, dll) as projects (.vcproj)
project (NecEngine CXX)
set(CMAKE_CXX_STANDARD 17)


# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set compiler flags and options. 
# Here it is setting the Visual Studio warning level to 4
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")

# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
#message ("cxx Flags: " ${CMAKE_CXX_FLAGS})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
message(STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")

include(cmake/general.cmake)

# set the default boost library search folder only if the user didn't specified it explicitly
if (NOT BOOST_LIBRARYDIR)
    get_library_path("boost-1_67_0" BOOST_LIBRARYDIR )
endif()

if (NOT BOOST_INCLUDEDIR)
    SET(BOOST_INCLUDEDIR "${NEX_BASE_LIBRARY_FOLDER}/boost-1_67_0")
endif()
set(Boost_USE_STATIC_LIBS TRUE)

find_package(Boost 1.67 EXACT REQUIRED)

# let boost specific variables appear also in the non advanced section
mark_as_advanced(CLEAR Boost_DIR Boost_ROOT Boost_INCLUDE_DIR Boost_LIBRARYDIR)


# Sub-directories where more CMakeLists.txt exist
add_subdirectory(projects)