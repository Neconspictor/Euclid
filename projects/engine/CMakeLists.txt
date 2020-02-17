set(
    ENGINE_SOURCES 
    
    pch_engine.hpp
    
	#nex
    
    #nex/anim
    nex/anim/AnimationManager.cpp
    nex/anim/AnimationManager.hpp
    nex/anim/AnimationType.hpp
    nex/anim/BoneAnimation.hpp
    nex/anim/BoneAnimation.cpp
    nex/anim/BoneAnimationLoader.hpp
    nex/anim/BoneAnimationLoader.cpp
    nex/anim/KeyFrame.hpp
    nex/anim/Rig.hpp
    nex/anim/Rig.cpp
    nex/anim/RigLoader.cpp
    nex/anim/RigLoader.hpp
    
    
    #nex/buffer
    nex/buffer/GpuBuffer.hpp
    nex/buffer/IndexBuffer.hpp
    nex/buffer/ShaderBuffer.hpp
    nex/buffer/VertexBuffer.hpp
    
    #nex/camera
    nex/camera/Camera.hpp
	nex/camera/Camera.cpp
    nex/camera/FPCamera.hpp
	nex/camera/FPCamera.cpp
	
	#nex/cluster
	nex/cluster/Cluster.cpp
    nex/cluster/Cluster.hpp
    
    #nex/common
    nex/common/BinaryReader.h
    nex/common/BinaryReader.cpp
    nex/common/Callback.hpp
    nex/common/Concurrent.hpp
    nex/common/ConcurrentQueue.hpp
    nex/common/debug_break.h
    nex/common/File.hpp
    nex/common/File.cpp
    nex/common/FrameUpdateable.hpp
    nex/common/Future.hpp
    nex/common/Future.cpp
    nex/common/Log.hpp
    nex/common/Log.cpp
	nex/common/Resizable.hpp
    
    #nex/config
    nex/config/Configuration.hpp
	nex/config/Configuration.cpp
    
    #nex/effects
    nex/effects/Blit.cpp
    nex/effects/Blit.hpp
    nex/effects/DepthMapPass.hpp
	nex/effects/DepthMapPass.cpp
    nex/effects/EffectLibrary.cpp
    nex/effects/EffectLibrary.hpp
    nex/effects/Flame.cpp
    nex/effects/Flame.hpp
    nex/effects/SpriteShader.hpp
	nex/effects/SpriteShader.cpp
    nex/effects/SimpleColorPass.hpp
    nex/effects/SimpleColorPass.cpp
    nex/effects/SkyBoxPass.hpp
	nex/effects/SkyBoxPass.cpp
	nex/effects/ViewSpaceZSpriteShader.cpp
	nex/effects/ViewSpaceZSpriteShader.hpp
    
    #nex/exception
    nex/exception/EnumFormatException.hpp
    nex/exception/OpenglException.hpp
    nex/exception/ResourceLoadException.hpp
    nex/exception/ShaderException.hpp
	
	#nex/GI
	nex/GI/DiffuseGiProbeCluster.cpp
	nex/GI/DiffuseGiProbeCluster.hpp
	nex/GI/GlobalIllumination.cpp
    nex/GI/GlobalIllumination.hpp
    nex/GI/IrradianceSphereHullDrawPass.cpp
    nex/GI/IrradianceSphereHullDrawPass.hpp
    nex/GI/Probe.cpp
    nex/GI/Probe.hpp
	nex/GI/ProbeBaker.cpp
	nex/GI/ProbeBaker.hpp
    nex/GI/ProbeGenerator.cpp
    nex/GI/ProbeGenerator.hpp
	nex/GI/ProbeManager.cpp
	nex/GI/ProbeManager.hpp
	nex/GI/ProbeSelector.cpp
	nex/GI/ProbeSelector.hpp
	nex/GI/VoxelConeTracer.cpp
	nex/GI/VoxelConeTracer.hpp
	
	
	#nex/gui	
	nex/gui/Controller.cpp
    nex/gui/Controller.hpp
	nex/gui/ControllerStateMachine.hpp
	nex/gui/ControllerStateMachine.cpp
	nex/gui/Dockspace.cpp
	nex/gui/Dockspace.hpp
	nex/gui/Drawable.cpp
	nex/gui/Drawable.hpp
    nex/gui/FileDialog.cpp
    nex/gui/FileDialog.hpp
    nex/gui/Gizmo.cpp
    nex/gui/Gizmo.hpp
    nex/gui/ImGUI.cpp
	nex/gui/ImGUI.hpp
	nex/gui/ImGUI_Extension.cpp
	nex/gui/ImGUI_Extension.hpp
	nex/gui/Menu.cpp
	nex/gui/Menu.hpp
    nex/gui/MenuWindow.cpp
    nex/gui/MenuWindow.hpp
	nex/gui/OceanGenerator.cpp
	nex/gui/OceanGenerator.hpp
	nex/gui/pagghiu_box_shadow.cpp
	nex/gui/ParticleSystemGenerator.cpp
    nex/gui/ParticleSystemGenerator.hpp
    nex/gui/Picker.cpp
    nex/gui/Picker.hpp
	nex/gui/ProbeGeneratorView.cpp
    nex/gui/ProbeGeneratorView.hpp
	nex/gui/RectangleShadow.cpp
	nex/gui/RectangleShadow.hpp
	nex/gui/SceneView.cpp
	nex/gui/SceneView.hpp
	nex/gui/Style.hpp
    nex/gui/TextureLoader.cpp
    nex/gui/TextureLoader.hpp
    nex/gui/TextureView.cpp
    nex/gui/TextureView.hpp
    nex/gui/TextureViewer.cpp
    nex/gui/TextureViewer.hpp
    nex/gui/VisualizationSphere.cpp
    nex/gui/VisualizationSphere.hpp
	
    
	#nex/gui/vob
	nex/gui/vob/OceanVobView.cpp
	nex/gui/vob/OceanVobView.hpp
	nex/gui/vob/ProbeVobView.cpp
	nex/gui/vob/ProbeVobView.hpp
	nex/gui/vob/RiggedVobView.cpp
	nex/gui/vob/RiggedVobView.hpp
	nex/gui/vob/VobEditor.cpp
    nex/gui/vob/VobEditor.hpp
    nex/gui/vob/VobLoader.cpp
    nex/gui/vob/VobLoader.hpp
	nex/gui/vob/VobView.cpp
	nex/gui/vob/VobView.hpp
	nex/gui/vob/VobViewMapper.cpp
	nex/gui/vob/VobViewMapper.hpp
	
    
    #nex/import
    nex/import/ImportScene.hpp
    nex/import/ImportScene.cpp
    
	#nex/light
	nex/light/Light.hpp
	nex/light/Light.cpp
    
    #nex/material
    nex/material/AbstractMaterialLoader.hpp
    nex/material/AbstractMaterialLoader.cpp
    nex/material/Material.hpp
    nex/material/Material.cpp
	nex/material/PbrMaterialLoader.hpp
	nex/material/PbrMaterialLoader.cpp
    
    #nex/math
    nex/math/algebra.cpp
    nex/math/algebra.hpp
    nex/math/BoundingBox.cpp
    nex/math/BoundingBox.hpp
    nex/math/Circle.cpp
    nex/math/Circle.hpp
    nex/math/Complex.cpp
    nex/math/Complex.hpp
    nex/math/Constant.hpp
    nex/math/Plane.cpp
    nex/math/Plane.hpp
    nex/math/Math.cpp
    nex/math/Math.hpp
    
    nex/math/Random.cpp
    nex/math/Random.hpp
    nex/math/Ray.cpp
    nex/math/Ray.hpp
    nex/math/Sphere.cpp
    nex/math/Sphere.hpp
    nex/math/Torus.cpp
    nex/math/Torus.hpp
	nex/math/TrafoSpace.cpp
	nex/math/TrafoSpace.hpp
    
    #nex/mesh
    nex/mesh/Mesh.hpp
    nex/mesh/Mesh.cpp
    nex/mesh/MeshGroup.hpp
	nex/mesh/MeshGroup.cpp
	nex/mesh/MeshFactory.hpp
	nex/mesh/MeshFactory.cpp
    nex/mesh/MeshLoader.hpp
	nex/mesh/MeshLoader.cpp
    nex/mesh/MeshManager.hpp
	nex/mesh/MeshManager.cpp
    nex/mesh/MeshStore.hpp
    nex/mesh/MeshStore.cpp
    nex/mesh/MeshTypes.hpp
    nex/mesh/SampleMeshes.hpp
    
    
    nex/mesh/UtilityMeshes.cpp
    nex/mesh/UtilityMeshes.hpp
    nex/mesh/VertexArray.hpp
    nex/mesh/VertexLayout.cpp
    nex/mesh/VertexLayout.hpp
    nex/mesh/VertexLayout.inl
    
    #nex/particle
    nex/particle/Particle.cpp
    nex/particle/Particle.hpp
    
    #nex/pbr
    nex/pbr/Pbr.cpp
    nex/pbr/Pbr.hpp
    nex/pbr/PbrDeferred.cpp
    nex/pbr/PbrDeferred.hpp
    nex/pbr/PbrForward.cpp
    nex/pbr/PbrForward.hpp
    nex/pbr/PbrPass.hpp
	nex/pbr/PbrPass.cpp
	
	
    #nex/platform/
    nex/platform/Input.hpp
	nex/platform/Input.cpp
    nex/platform/SubSystemProvider.hpp
    nex/platform/Window.hpp
	nex/platform/Window.cpp
	
	#nex/platform/glfw
	nex/platform/glfw/InputGLFW.cpp
	nex/platform/glfw/InputGLFW.hpp
	nex/platform/glfw/SubSystemProviderGLFW.cpp
	nex/platform/glfw/SubSystemProviderGLFW.hpp
	nex/platform/glfw/WindowGLFW.cpp
	nex/platform/glfw/WindowGLFW.hpp
    
	#nex/platform/windows
	nex/platform/windows/CrashHandlerWin32.hpp
	nex/platform/windows/StackWalker.cpp
	nex/platform/windows/StackWalker.h
	nex/platform/windows/WindowsPlatform.hpp
    
    #nex/post_processing
    nex/post_processing/DownSampler.cpp
    nex/post_processing/DownSampler.hpp
    nex/post_processing/FXAA.cpp
    nex/post_processing/FXAA.hpp
    nex/post_processing/HBAO.hpp
	nex/post_processing/HBAO.cpp
    nex/post_processing/AmbientOcclusion.cpp
	nex/post_processing/AmbientOcclusion.hpp
    nex/post_processing/PostProcessor.cpp
    nex/post_processing/PostProcessor.hpp
    
    ../../extern_sources/engine/extern/SMAA/AreaTex.h
    ../../extern_sources/engine/extern/SMAA/SearchTex.h
    nex/post_processing/SMAA.hpp
	nex/post_processing/SMAA.cpp
    nex/post_processing/SSR.cpp
    nex/post_processing/SSR.hpp
    nex/post_processing/TAA.cpp
    nex/post_processing/TAA.hpp
    
    #nex/post_processing/blur
	nex/post_processing/blur/GaussianBlur.hpp
	nex/post_processing/blur/GaussianBlur.cpp
    nex/post_processing/blur/GaussianBlurPass.hpp
	nex/post_processing/blur/GaussianBlurPass.cpp
    
    #nex/renderer
    nex/renderer/Drawer.hpp
	nex/renderer/Drawer.cpp
	nex/renderer/MaterialDataUpdater.cpp
	nex/renderer/MaterialDataUpdater.hpp
    nex/renderer/RenderBackend.hpp
    nex/renderer/RenderCommand.hpp
    nex/renderer/RenderCommandFactory.hpp
    nex/renderer/RenderCommandQueue.cpp
    nex/renderer/RenderCommandQueue.hpp
	nex/renderer/RenderContext.hpp
    nex/renderer/RenderEngine.cpp
    nex/renderer/RenderEngine.hpp
    nex/renderer/Renderer.cpp
    nex/renderer/Renderer.hpp
	nex/renderer/RenderTypes.cpp
    nex/renderer/RenderTypes.hpp
    
    #nex/resource
    nex/resource/FileSystem.hpp
	nex/resource/FileSystem.cpp
    nex/resource/Resource.cpp
    nex/resource/Resource.hpp
    nex/resource/ResourceLoader.cpp
    nex/resource/ResourceLoader.hpp
    
    #nex/scene
    nex/scene/Scene.cpp
    nex/scene/Scene.hpp
    nex/scene/Vob.cpp
    nex/scene/Vob.hpp
    
    
    #nex/shader
    nex/shader/Shader.cpp
    nex/shader/Shader.hpp
    nex/shader/ShaderProgram.cpp
    nex/shader/ShaderProgram.hpp
	nex/shader/ShaderProvider.hpp
    nex/shader/ShaderType.hpp
	
    
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
    nex/shadow/SceneNearFarComputePass.cpp
    nex/shadow/SceneNearFarComputePass.hpp
    nex/shadow/ShadowCommon.hpp
    nex/shadow/ShadowMap.cpp
    nex/shadow/ShadowMap.hpp
    
    #nex/sky
    nex/sky/AtmosphericScattering.cpp
    nex/sky/AtmosphericScattering.hpp
    
    #nex/terrain
    nex/terrain/HeightMap.hpp
    nex/terrain/HeightMap.cpp
    nex/terrain/TesselationTest.cpp
    nex/terrain/TesselationTest.hpp
    
    #nex/texture
    nex/texture/Attachment.hpp
    nex/texture/GBuffer.hpp
	nex/texture/GBuffer.cpp
    nex/texture/Image.hpp
	nex/texture/Image.cpp
    nex/texture/RenderTarget.cpp
    nex/texture/RenderTarget.hpp
    nex/texture/Sampler.hpp
    nex/texture/Sprite.cpp
    nex/texture/Sprite.hpp
    nex/texture/Texture.hpp  
    nex/texture/TextureSamplerData.hpp    
    nex/texture/TextureManager.hpp
	nex/texture/TextureManager.cpp
    
    
	#nex/util
    nex/util/Array.hpp
    nex/util/CallbackContainer.hpp
    nex/util/ExceptionHandling.cpp
	nex/util/ExceptionHandling.hpp
	nex/util/FPSCounter.hpp
	nex/util/Iterator.hpp
    nex/util/Macro.hpp
    nex/util/Memory.hpp
    nex/util/PointerUtils.hpp
    nex/util/Signal.hpp
    nex/util/StringUtils.hpp
	nex/util/StringUtils.cpp
	nex/util/Timer.hpp
    nex/util/TimeUtils.hpp

    
    #nex/util/concurrent
    nex/util/concurrent/Active.hpp
	nex/util/concurrent/Active.cpp
    
    #nex/water
    nex/water/Ocean.hpp
    nex/water/Ocean.cpp
    nex/water/PSSR.hpp
    nex/water/PSSR.cpp
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
    ../shaders/interface
    #${CMAKE_SOURCE_DIR}/libs/glm/include 
    #${CMAKE_SOURCE_DIR}/libs/boost_1_67_0/include
    #${CMAKE_SOURCE_DIR}/libs/assimp-3.3.1/include
)

find_package(ASSIMP REQUIRED)
#find_package(Boost 1.67 EXACT REQUIRED COMPONENTS program_options)
find_package(Boost 1.67 EXACT REQUIRED COMPONENTS atomic chrono date_time filesystem locale program_options system thread)
find_package(Boxer REQUIRED)
find_package(GLAD REQUIRED)
find_package(GLFW 3.2.1 EXACT REQUIRED)
find_package(GLI REQUIRED)
find_package(GLM REQUIRED)
find_package(IMGUI REQUIRED)
#find_package(MAGICK REQUIRED)
find_package(NFD REQUIRED)
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
		GLFW
        GLI
		GLM
		IMGUI
		#MAGICK
        NFD
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