set(
    COMPUTE_TEST_SOURCES 
    
    Globals.cpp
	Globals.hpp
	Main.cpp
	NeXEngine.cpp
	NeXEngine.hpp
    pch_compute_test.hpp
    VideoConfig.hpp
	VideoConfig.cpp
	
	#gui
	gui/AppStyle.cpp
	gui/AppStyle.hpp
	gui/ConfigurationWindow.cpp
	gui/ConfigurationWindow.hpp
	gui/SceneGUI.cpp
	gui/SceneGUI.hpp
	gui/Controller.cpp
	gui/Controller.hpp
	
	#renderer
	renderer/ComputeTest_Renderer.cpp
    renderer/ComputeTest_Renderer.hpp
	
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${ENGINE_SOURCES})
assign_source_group(${COMPUTE_TEST_SOURCES})

# Properties->C/C++->General->Additional Include Directories
#include_directories (
#    ./ 
#)

enable_precompiled_headers(pch_compute_test.hpp COMPUTE_TEST_SOURCES)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_executable (ComputeTest ${COMPUTE_TEST_SOURCES})

target_include_directories(ComputeTest PUBLIC ./)

#choose the right libraries by absolute paths!
target_link_libraries(ComputeTest PUBLIC engine_opengl)


#set (EASY_HOOK_DLL ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2/EasyHook64.dll)
#set (PROFILER_CORE_DLL ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2/ProfilerCore64.dll)
#set (BROFILER ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2/Brofiler.exe)

# Adds logic to INSTALL.vcproj to copy app.exe to destination directory
#install (TARGETS App
#		 RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin)
         

#install (FILES "${EASY_HOOK_DLL}" DESTINATION ${CMAKE_SOURCE_DIR}/bin)
#install (FILES "${PROFILER_CORE_DLL}" DESTINATION ${CMAKE_SOURCE_DIR}/bin)
#install (FILES "${BROFILER}" DESTINATION ${CMAKE_SOURCE_DIR}/bin)

add_postbuild_for_assimp_lib_for_target(ComputeTest)
