set(
    ENGINE_SOURCES 
    
    ###header files###
    pch_engine.hpp
		
    #antialiasing
    antialiasing/SMAA.hpp
    
    #camera
    camera/Camera.hpp
	camera/Camera.cpp
    camera/FPCamera.hpp
	camera/FPCamera.cpp
    camera/FPCameraBase.hpp
	camera/FPCameraBase.cpp
    camera/FPQuaternionCamera.hpp
	camera/FPQuaternionCamera.cpp
    camera/TrackballCamera.hpp
	camera/TrackballCamera.cpp
    camera/TrackballQuatCamera.hpp
	camera/TrackballQuatCamera.cpp
    
    #drawing
    drawing/ModelDrawer.hpp
    
    #exception
    exception/MeshNotFoundException.hpp
    exception/ShaderInitException.hpp
    exception/ShaderNotFoundException.hpp
	
	#gui
	gui/Menu.cpp
	gui/Menu.hpp
	gui/SceneGUI.cpp
	gui/SceneGUI.hpp
	gui/Style.hpp
	gui/UI_Mode.hpp
	gui/UI_ModeStateMachine.hpp
	gui/UI_ModeStateMachine.cpp
	gui/Util.cpp
	gui/Util.hpp
	gui/Drawable.hpp
    
	#light
	light/Light.hpp
	light/Light.cpp
		
    #listener
    listener/ScrollListener.hpp
    
    #mesh
    mesh/Mesh.hpp
	mesh/Mesh.cpp
    mesh/SampleMeshes.hpp
    
    #material
    material/AbstractMaterialLoader.hpp
	material/AbstractMaterialLoader.cpp
    material/BlinnPhongMaterial.hpp
	material/BlinnPhongMaterial.cpp
	material/BlinnPhongMaterialLoader.hpp
	material/BlinnPhongMaterialLoader.cpp
    material/Material.hpp
	material/PbrMaterial.hpp
	material/PbrMaterial.cpp
	material/PbrMaterialLoader.hpp
	material/PbrMaterialLoader.cpp

    
    #model
    model/Model.hpp
	model/Model.cpp
    model/ModelManager.hpp
	model/ModelManager.cpp
    model/Vob.hpp
	model/Vob.cpp

	
    #post_processing
	post_processing/HBAO.hpp
	post_processing/HBAO.cpp
	post_processing/SSAO.hpp
	post_processing/SSAO.cpp
	#post_processing/blur
	post_processing/blur/GaussianBlur.hpp
	post_processing/blur/GaussianBlur.cpp
	
    #renderer
    renderer/AbstractRendererFactory.hpp
    renderer/Renderer3D.hpp
    
    #shader
	shader/DepthMapShader.hpp
    shader/NormalsShader.hpp
    shader/PBRShader.hpp	
	shader/PhongTextureShader.hpp
    shader/ScreenShader.hpp
    shader/Shader.hpp
	shader/Shader.cpp
    shader/ShaderManager.hpp
	shader/ShadowShader.hpp
    shader/SimpleColorShader.hpp
    shader/SimpleExtrudeShader.hpp
    shader/SkyBoxShader.hpp
	#shader/post_processing/blur
	shader/post_processing/blur/GaussianBlurShader.hpp
	
	#shading_model
	shading_model/PBR.hpp
	shading_model/PBR.cpp
	shading_model/PBR_Deferred.hpp
	shading_model/PBR_Deferred.cpp
	shading_model/ShadingModelFactory.hpp
	shading_model/ShadingModelFactory.cpp
	
	
    #scene
    scene/SceneNode.hpp
	scene/SceneNode.cpp
    
    #shadowing
    shadowing/ShadowMapper.hpp
 
    
    #sprite
    sprite/Sprite.hpp
	sprite/Sprite.cpp
 
    #system
    system/Configuration.hpp
	system/Configuration.cpp
    system/Engine.hpp
	system/Engine.cpp
    system/System.hpp
	system/System.cpp
	system/Video.hpp
	system/Video.cpp
 
    
    #texture
    texture/Texture.hpp
	texture/Texture.cpp
    texture/TextureManager.hpp
    
    #util
    util/FPSCounter.hpp
    util/Globals.hpp
	util/Math.hpp
	util/Projectional.hpp
	util/Projectional.cpp
    util/Timer.hpp
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
    #${CMAKE_SOURCE_DIR}/libs/glm/include 
    #${CMAKE_SOURCE_DIR}/libs/boost_1_67_0/include
    #${CMAKE_SOURCE_DIR}/libs/assimp-3.3.1/include
)

find_package(ASSIMP REQUIRED)
find_package(Boost 1.67 EXACT REQUIRED COMPONENTS program_options)
find_package(GLM REQUIRED)
find_package(STB REQUIRED)

target_link_libraries(engine

        PUBLIC

        ASSIMP
        GLM
		Boost::boost
        Boost::program_options
		
		STB
		
		platform
)