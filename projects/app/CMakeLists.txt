set(
    APP_SOURCES 
    
	Main.cpp
    PBR_MainLoopTask.hpp
	PBR_MainLoopTask.cpp
    MainLoopTask.hpp
	MainLoopTask.cpp
    pch_app.hpp
	
	#pbr_deferred
	pbr_deferred/PBR_Deferred_MainLoopTask.hpp
	pbr_deferred/PBR_Deferred_MainLoopTask.cpp
	
	#ui_mode
	ui_mode/UI_Mode.hpp
	ui_mode/UI_ModeStateMachine.hpp
	ui_mode/UI_ModeStateMachine.cpp
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${ENGINE_SOURCES})
assign_source_group(${APP_SOURCES})

# Properties->C/C++->General->Additional Include Directories
include_directories (
    ./ 
    ${CMAKE_SOURCE_DIR}/libs/boost_1_62_0/include
)

enable_precompiled_headers(pch_app.hpp APP_SOURCES)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_executable (App ${APP_SOURCES})

#choose the right libraries by absolute paths!
target_link_libraries(App PUBLIC engine_opengl)


set (EASY_HOOK_DLL ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2/EasyHook64.dll)
set (PROFILER_CORE_DLL ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2/ProfilerCore64.dll)
set (BROFILER ${CMAKE_SOURCE_DIR}/tools/Brofiler-1.1.2/Brofiler.exe)

# Adds logic to INSTALL.vcproj to copy app.exe to destination directory
install (TARGETS App
		 RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin)
         

install (FILES "${EASY_HOOK_DLL}" DESTINATION ${CMAKE_SOURCE_DIR}/bin)
install (FILES "${PROFILER_CORE_DLL}" DESTINATION ${CMAKE_SOURCE_DIR}/bin)
install (FILES "${BROFILER}" DESTINATION ${CMAKE_SOURCE_DIR}/bin)