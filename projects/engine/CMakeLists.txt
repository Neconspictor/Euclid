set(
    ENGINE_SOURCES 
    
    pch_engine.hpp
    
    nex/EffectLibrary.cpp
    nex/EffectLibrary.hpp
    
    nex/RenderBackend.hpp
    nex/Renderer.hpp
    nex/Renderer.cpp
    
    nex/SceneNode.cpp
    nex/SceneNode.hpp
	
	
	#nex
    nex/FileSystem.hpp
	nex/FileSystem.cpp
    nex/Input.hpp
	nex/Input.cpp
	nex/SubSystemProvider.hpp
    nex/Window.hpp
	nex/Window.cpp
    
    #nex/camera
    nex/camera/Camera.hpp
	nex/camera/Camera.cpp
    nex/camera/FPCamera.hpp
	nex/camera/FPCamera.cpp
    nex/camera/FPCameraBase.hpp
	nex/camera/FPCameraBase.cpp
    nex/camera/FPQuaternionCamera.hpp
	nex/camera/FPQuaternionCamera.cpp
    nex/camera/TrackballCamera.hpp
	nex/camera/TrackballCamera.cpp
    nex/camera/TrackballQuatCamera.hpp
	nex/camera/TrackballQuatCamera.cpp
    
    #nex/common
    nex/common/BinaryReader.h
    nex/common/BinaryReader.cpp
    nex/common/Callback.hpp
    nex/common/debug_break.h
    nex/common/Log.hpp
    nex/common/Log.cpp
    
    #nex/config
    nex/config/Configuration.hpp
	nex/config/Configuration.cpp
    
    #nex/drawing
    nex/drawing/StaticMeshDrawer.hpp
	nex/drawing/StaticMeshDrawer.cpp
    
    #nex/exception
    nex/exception/MeshNotFoundException.hpp
    nex/exception/ShaderInitException.hpp
    nex/exception/ShaderNotFoundException.hpp
    nex/exception/EnumFormatException.hpp
    nex/exception/OpenglException.hpp
    nex/exception/UnexpectedPlatformException.hpp
    
    #nex/event
    nex/event/EventChannel.hpp
    nex/event/EventHandler.hpp
    nex/event/EventHandlerQueue.hpp
    nex/event/GlobalEventChannel.hpp
    nex/event/Task.hpp
	nex/event/Task.cpp
    nex/event/TaskManager.hpp
	nex/event/TaskManager.cpp
    
	
	#nex/gui
	nex/gui/Controller.hpp
	nex/gui/ControllerStateMachine.hpp
	nex/gui/ControllerStateMachine.cpp
	nex/gui/Drawable.cpp
	nex/gui/Drawable.hpp
	nex/gui/Menu.cpp
	nex/gui/Menu.hpp
	nex/gui/Style.hpp
	nex/gui/imgui_tabs.cpp
	nex/gui/imgui_tabs.h
	nex/gui/Util.cpp
	nex/gui/Util.hpp
	nex/gui/ImGUI.hpp
    
	#nex/light
	nex/light/Light.hpp
	nex/light/Light.cpp
		
    #nex/listener
    nex/listener/ScrollListener.hpp
	
	#nex/memory
    nex/memory/Allocator.hpp
    nex/memory/FreeListAllocator.hpp
	nex/memory/FreeListAllocator.cpp
    nex/memory/LinearAllocator.hpp
	nex/memory/LinearAllocator.cpp
    nex/memory/MemoryManager.hpp
	nex/memory/MemoryManager.cpp
    nex/memory/ObjectPool.hpp
    nex/memory/PoolAllocator.hpp
	nex/memory/PoolAllocator.cpp
    nex/memory/ProxyAllocator.hpp
	nex/memory/ProxyAllocator.cpp
    nex/memory/StackAllocator.hpp
	nex/memory/StackAllocator.cpp
    
    #nex/mesh
    nex/mesh/IndexBuffer.hpp
    nex/mesh/MeshFactory.hpp
	nex/mesh/MeshFactory.cpp
    nex/mesh/MeshLoader.hpp
	nex/mesh/MeshLoader.cpp
    nex/mesh/SampleMeshes.hpp
    nex/mesh/Sphere.cpp
    nex/mesh/Sphere.hpp
    nex/mesh/StaticMesh.hpp
	nex/mesh/StaticMesh.cpp
    nex/mesh/StaticMeshManager.hpp
	nex/mesh/StaticMeshManager.cpp
    nex/mesh/SubMesh.hpp
    nex/mesh/SubMesh.cpp
    nex/mesh/VertexArray.hpp
    nex/mesh/VertexBuffer.hpp
    nex/mesh/VertexLayout.hpp
    nex/mesh/VertexLayout.inl
    nex/mesh/Vob.hpp
	nex/mesh/Vob.cpp
    
    
    #nex/material
    nex/material/AbstractMaterialLoader.hpp
	nex/material/AbstractMaterialLoader.cpp
	nex/material/BlinnPhongMaterialLoader.hpp
	nex/material/BlinnPhongMaterialLoader.cpp
    nex/material/Material.hpp
    nex/material/Material.cpp
	nex/material/PbrMaterialLoader.hpp
	nex/material/PbrMaterialLoader.cpp
    
    #nex/pbr
    nex/pbr/PBR.hpp
    nex/pbr/PBR.cpp
    nex/pbr/PBR_Deferred.hpp
    nex/pbr/PBR_Deferred.cpp
	
	#nex/platform/windows
	nex/platform/windows/CrashHandlerWin32.hpp
	nex/platform/windows/StackWalker.cpp
	nex/platform/windows/StackWalker.h
	nex/platform/windows/WindowsPlatform.hpp
    
    #nex/post_processing
    nex/post_processing/DownSampler.cpp
    nex/post_processing/DownSampler.hpp
    nex/post_processing/HBAO.hpp
	nex/post_processing/HBAO.cpp
    nex/post_processing/AmbientOcclusion.cpp
	nex/post_processing/AmbientOcclusion.hpp
    nex/post_processing/PostProcessor.cpp
    nex/post_processing/PostProcessor.hpp
	nex/post_processing/SSAO.hpp
	nex/post_processing/SSAO.cpp
    
    ../../extern_sources/engine/extern/SMAA/AreaTex.h
    ../../extern_sources/engine/extern/SMAA/SearchTex.h
    nex/post_processing/SMAA.hpp
	nex/post_processing/SMAA.cpp
    
    #nex/post_processing/blur
	nex/post_processing/blur/GaussianBlur.hpp
	nex/post_processing/blur/GaussianBlur.cpp
    
    
    #nex/shader
    nex/shader/DepthMapShader.hpp
	nex/shader/DepthMapShader.cpp
    nex/shader/PBRShader.hpp
	nex/shader/PBRShader.cpp
    nex/shader/ScreenShader.hpp
	nex/shader/ScreenShader.cpp
    nex/shader/Shader.cpp
    nex/shader/Shader.hpp
    nex/shader/ShaderBuffer.hpp
    nex/shader/ShadowShader.hpp
	nex/shader/ShadowShader.cpp
    nex/shader/SkyBoxShader.hpp
	nex/shader/SkyBoxShader.cpp
	#nex/shader/post_processing/blur
    nex/shader/post_processing/blur/GaussianBlurShader.hpp
	nex/shader/post_processing/blur/GaussianBlurShader.cpp
	
    
    #nex/shader_generator
    nex/shader_generator/CommentStateTracker.cpp
    nex/shader_generator/CommentStateTracker.hpp
    nex/shader_generator/ShaderSourceFileGenerator.cpp
    nex/shader_generator/ShaderSourceFileGenerator.hpp
    nex/shader_generator/SourceFileConsumer.cpp
    nex/shader_generator/SourceFileConsumer.hpp
    nex/shader_generator/SourceReader.cpp
    nex/shader_generator/SourceReader.hpp
    nex/shader_generator/StateTracker.cpp
    nex/shader_generator/StateTracker.hpp
    nex/shader_generator/StringStateTracker.cpp
    nex/shader_generator/StringStateTracker.hpp
    
    #nex/shadow
	nex/shadow/CascadedShadow.cpp
	nex/shadow/CascadedShadow.hpp
    
    #nex/sky
    nex/sky/AtmosphericScattering.cpp
    nex/sky/AtmosphericScattering.hpp
    
    #nex/texture
    nex/texture/Attachment.hpp
    nex/texture/GBuffer.hpp
	nex/texture/GBuffer.cpp
    nex/texture/Image.hpp
	nex/texture/Image.cpp
    nex/texture/RenderTarget.hpp
    nex/texture/Sampler.hpp
    nex/texture/Sprite.cpp
    nex/texture/Sprite.hpp
    nex/texture/Texture.hpp  
    nex/texture/TextureSamplerData.hpp    
    nex/texture/TextureManager.hpp
	nex/texture/TextureManager.cpp
    
	#nex/util
    nex/util/CallbackContainer.hpp
	nex/util/ExceptionHandling.hpp
	nex/util/FPSCounter.hpp
	nex/util/Math.hpp
    nex/util/Memory.hpp
    nex/util/PointerUtils.hpp
	nex/util/Projectional.hpp
	nex/util/Projectional.cpp
    nex/util/Signal.hpp
    nex/util/StringUtils.hpp
	nex/util/StringUtils.cpp
	nex/util/Timer.hpp
    nex/util/TimeUtils.hpp

    
    #nex/util/concurrent
    nex/util/concurrent/Active.hpp
	nex/util/concurrent/Active.cpp
)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
#source_group("" FILES ${ENGINE_SOURCES})
assign_source_group(${ENGINE_SOURCES})

enable_precompiled_headers(pch_engine.hpp ENGINE_SOURCES)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_library (engine ${ENGINE_SOURCES})
target_include_directories (engine PUBLIC 
	./
    ../../extern_sources/engine/
    #${CMAKE_SOURCE_DIR}/libs/glm/include 
    #${CMAKE_SOURCE_DIR}/libs/boost_1_67_0/include
    #${CMAKE_SOURCE_DIR}/libs/assimp-3.3.1/include
)

find_package(ASSIMP REQUIRED)
#find_package(Boost 1.67 EXACT REQUIRED COMPONENTS program_options)
find_package(Boost 1.67 EXACT REQUIRED COMPONENTS atomic chrono date_time filesystem locale program_options system thread)
find_package(Boxer REQUIRED)
find_package(GLAD REQUIRED)
find_package(GLI REQUIRED)
find_package(GLM REQUIRED)
find_package(IMGUI REQUIRED)
find_package(STB REQUIRED)

target_link_libraries(engine

        PUBLIC

        ASSIMP
		Boost::atomic
        Boost::boost
        Boost::chrono
        Boost::date_time
        Boost::filesystem
        Boost::locale
        Boost::program_options
        Boost::system
        Boost::thread
		Boxer
        GLI
		GLM
		IMGUI
		STB
)

# Add boost random package to MinGW as it has no real std::random_device support! 
if(MINGW)
	find_package(Boost 1.67 EXACT REQUIRED COMPONENTS random)

	target_link_libraries(engine

        PUBLIC

		Boost::random
)
endif()