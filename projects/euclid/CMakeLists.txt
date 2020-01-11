set(
    EUCLID_SOURCES 
    
    Globals.cpp
	Globals.hpp
	Main.cpp
	Euclid.cpp
	Euclid.hpp
    PBR_Deferred_Renderer.hpp
    PBR_Deferred_Renderer.cpp
    pch_euclid.hpp
    VideoConfig.hpp
	VideoConfig.cpp
	
	#gui
	gui/AppStyle.cpp
	gui/AppStyle.hpp
	gui/ConfigurationWindow.cpp
	gui/ConfigurationWindow.hpp
    gui/Controller.cpp
	gui/Controller.hpp
	gui/FontManager.cpp
	gui/FontManager.hpp
	gui/SceneGUI.cpp
	gui/SceneGUI.hpp
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${ENGINE_SOURCES})
assign_source_group(${EUCLID_SOURCES})

# Properties->C/C++->General->Additional Include Directories
#include_directories (
#    ./ 
#)

enable_precompiled_headers(pch_euclid.hpp EUCLID_SOURCES)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates Application.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_executable (Euclid ${EUCLID_SOURCES})

target_include_directories(Euclid PUBLIC ./)

#choose the right libraries by absolute paths!
target_link_libraries(Euclid PUBLIC engine_opengl)


#set (EASY_HOOK_DLL ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2/EasyHook64.dll)
#set (PROFILER_CORE_DLL ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2/ProfilerCore64.dll)
#set (BROFILER ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2/Brofiler.exe)

# Adds logic to INSTALL.vcproj to copy app.exe to destination directory
#install (TARGETS App
#		 RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin)
         

#install (FILES "${EASY_HOOK_DLL}" DESTINATION ${CMAKE_SOURCE_DIR}/bin)
#install (FILES "${PROFILER_CORE_DLL}" DESTINATION ${CMAKE_SOURCE_DIR}/bin)
#install (FILES "${BROFILER}" DESTINATION ${CMAKE_SOURCE_DIR}/bin)

add_postbuild_for_assimp_lib_for_target(Euclid)
