#include <nex/opengl/shading_model/PBR.hpp>
#include <nex/opengl/shader/SkyBoxShaderGL.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/opengl/shader/ShaderManagerGL.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/opengl/model/ModelManagerGL.hpp>

using namespace glm;

PBR::PBR(RendererOpenGL* renderer, TextureGL* backgroundHDR) :
	environmentMap(nullptr), shader(nullptr), renderer(renderer), skybox("misc/SkyBoxCube.obj", ShaderType::BlinnPhongTex){

	skybox.init(renderer->getModelManager());

	init(backgroundHDR);
}

PBR::~PBR(){

}

void PBR::drawSky(const mat4& projection, const mat4& view)
{
	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();
	ShaderManagerGL* shaderManager = renderer->getShaderManager();

	SkyBoxShaderGL* skyboxShader = dynamic_cast<SkyBoxShaderGL*>
		(shaderManager->getConfig(ShaderType::SkyBox));

	skyboxShader->setSkyTexture(environmentMap->getCubeMap());
	//skyboxShader->setSkyTexture(prefilterRenderTarget->getCubeMap());

	mat4 identity;
	mat4 skyBoxView = mat4(mat3(view));

	TransformData data = { &projection, &view, nullptr };
	data.model = &identity;
	data.view = &skyBoxView;

	modelDrawer->draw(&skybox, ShaderType::SkyBox, data);
}

void PBR::drawSceneToShadowMap(SceneNode * scene, 
	DepthMapGL* shadowMap,
	const DirectionalLight & light, 
	const mat4 & lightViewMatrix, 
	const mat4 & lightProjMatrix)
{
	const mat4& lightSpaceMatrix = lightProjMatrix * lightViewMatrix;
	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();

	// render shadows to a depth map
	scene->draw(renderer, modelDrawer, lightProjMatrix, lightViewMatrix, ShaderType::Shadow);
}

void PBR::drawScene(SceneNode * scene,
	const vec3& cameraPosition, 
	TextureGL* shadowMap,
	const DirectionalLight& light, 
	const mat4& lightViewMatrix, 
	const mat4& lightProjMatrix,
	const mat4& view,
	const mat4& projection)
{

	 mat4 lightSpaceMatrix = lightProjMatrix * lightViewMatrix;

	shader = dynamic_cast<PBRShaderGL*> (renderer->getShaderManager()->getConfig(ShaderType::Pbr));

	shader->setCameraPosition(cameraPosition);
	shader->setIrradianceMap(convolutedEnvironmentMap->getCubeMap());
	shader->setPrefilterMap(prefilterRenderTarget->getCubeMap());
	shader->setBrdfLookupTexture(brdfLookupTexture->getTexture());
	shader->setLightColor(light.getColor());
	shader->setLightDirection(light.getLook());
	shader->setLightSpaceMatrix(lightSpaceMatrix);
	shader->setLightProjMatrix(lightProjMatrix);
	shader->setLightViewMatrix(lightViewMatrix);
	shader->setShadowMap(shadowMap);
	shader->setSkyBox(environmentMap->getCubeMap());


	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();
	scene->draw(renderer, modelDrawer, projection, view, ShaderType::Pbr);
}

CubeMapGL * PBR::getConvolutedEnvironmentMap()
{
	return convolutedEnvironmentMap->getCubeMap();
}

CubeMapGL* PBR::getEnvironmentMap()
{
	return environmentMap->getCubeMap();
}

CubeMapGL * PBR::getPrefilteredEnvironmentMap()
{
	return prefilterRenderTarget->getCubeMap();
}

TextureGL * PBR::getBrdfLookupTexture()
{
	return brdfLookupTexture->getTexture();
}

CubeRenderTargetGL * PBR::renderBackgroundToCube(TextureGL * background)
{
	TextureData textureData = {false, false, Linear_Mipmap_Linear, Linear, ClampToEdge, RGB, true, BITS_32};

	CubeRenderTargetGL* cubeRenderTarget = renderer->createCubeRenderTarget(2048, 2048, textureData);

	ShaderManagerGL* shaderManager = renderer->getShaderManager();

	EquirectangularSkyBoxShaderGL* equirectangularSkyBoxShader = dynamic_cast<EquirectangularSkyBoxShaderGL*>
		(shaderManager->getConfig(ShaderType::SkyBoxEquirectangular));

	equirectangularSkyBoxShader->setSkyTexture(background);

	TransformData data;
	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
	data.projection = &projection;
	mat4 model = glm::mat4();
	data.model = &model;


	//view matrices;
	const mat4 views[] = {
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_X), //right; sign of up vector is not important
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_X), //left
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_Y), //top
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_Y), //bottom
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_Z), //back
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_Z) //front
	};

	
	renderer->setViewPort(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());
	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();

	for (int side = CubeMapGL::POSITIVE_X; side < 6; ++side) {
		data.view = &views[side];
		renderer->useCubeRenderTarget(cubeRenderTarget, (CubeMapGL::Side)side);
		modelDrawer->draw(&skybox, ShaderType::SkyBoxEquirectangular, data);
	}

	// now create mipmaps for the cubemap fighting render artificats in the prefilter map
	cubeRenderTarget->getCubeMap()->generateMipMaps();

	//CubeMap* result = cubeRenderTarget->createCopy(); 
	//renderer->destroyCubeRenderTarget(cubeRenderTarget);
	return cubeRenderTarget;
}

CubeRenderTargetGL * PBR::convolute(CubeMapGL * source)
{
	CubeRenderTargetGL* cubeRenderTarget = renderer->createCubeRenderTarget(32, 32);

	ShaderManagerGL* shaderManager = renderer->getShaderManager();

	PBR_ConvolutionShaderGL* convolutionShader = dynamic_cast<PBR_ConvolutionShaderGL*>
		(shaderManager->getConfig(ShaderType::Pbr_Convolution));

	convolutionShader->setEnvironmentMap(source);

	TransformData data;
	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
	data.projection = &projection;
	mat4 model = glm::mat4();
	data.model = &model;


	//view matrices;
	const mat4 views[] = {
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_X), //right; sign of up vector is not important
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_X), //left
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_Y), //top
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_Y), //bottom
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_Z), //back
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_Z) //front
	};

	renderer->setViewPort(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());
	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();

	for (int side = CubeMapGL::POSITIVE_X; side < 6; ++side) {
		data.view = &views[side];
		renderer->useCubeRenderTarget(cubeRenderTarget, (CubeMapGL::Side)side);
		modelDrawer->draw(&skybox, ShaderType::Pbr_Convolution, data);
	}

	//CubeMap* result = cubeRenderTarget->createCopy();
	//renderer->destroyCubeRenderTarget(cubeRenderTarget);
	return cubeRenderTarget;
}

CubeRenderTargetGL* PBR::prefilter(CubeMapGL * source)
{
	TextureData textureData = { false, true, Linear_Mipmap_Linear, Linear, ClampToEdge, RGB, true, BITS_32 };

	CubeRenderTargetGL* prefilterRenderTarget = renderer->createCubeRenderTarget(256, 256, textureData);

	ShaderManagerGL* shaderManager = renderer->getShaderManager();

	PBR_PrefilterShaderGL* prefilterShader = dynamic_cast<PBR_PrefilterShaderGL*>
		(shaderManager->getConfig(ShaderType::Pbr_Prefilter));

	prefilterShader->setMapToPrefilter(source);

	TransformData data;
	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
	data.projection = &projection;
	mat4 model = glm::mat4();
	data.model = &model;


	//view matrices;
	const mat4 views[] = {
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_X), //right; sign of up vector is not important
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_X), //left
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_Y), //top
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_Y), //bottom
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_Z), //back
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_Z) //front
	};

	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();

	unsigned int maxMipLevels = 5;

	for (unsigned int mipLevel = 0; mipLevel < maxMipLevels; ++mipLevel) {

		// update the roughness value for the current mipmap level
		float roughness = (float)mipLevel / (float)(maxMipLevels - 1);
		prefilterShader->setRoughness(roughness);

		//resize render target according to mip level size
		prefilterRenderTarget->resizeForMipMap(mipLevel);

		// update the viewport size
		unsigned int width = prefilterRenderTarget->getWidthMipLevel(mipLevel);
		unsigned int height = prefilterRenderTarget->getHeightMipLevel(mipLevel);
		renderer->setViewPort(0, 0, width, height);

		// render to the cubemap at the specified mip level
		for (int side = CubeMapGL::POSITIVE_X; side < 6; ++side) {
			data.view = &views[side];
			renderer->useCubeRenderTarget(prefilterRenderTarget, (CubeMapGL::Side)side, mipLevel);
			modelDrawer->draw(&skybox, ShaderType::Pbr_Prefilter, data);
		}

	}

	//CubeMap* result = prefilterRenderTarget->createCopy();
	//renderer->destroyCubeRenderTarget(prefilterRenderTarget);

	//TODO extract the cubemap from the render target (if possible?)
	return prefilterRenderTarget;
}

RenderTargetGL * PBR::createBRDFlookupTexture()
{
	TextureData data = { false, false, Linear, Linear, ClampToEdge, RG, true, BITS_32 };
	RenderTargetGL* target = renderer->create2DRenderTarget(1024, 1024, data);


	ModelManagerGL* modelManager = renderer->getModelManager();

	ModelGL* spriteModel = modelManager->getSprite();// getModel(ModelManager::SPRITE_MODEL_NAME, Shaders::Unknown);

	ShaderManagerGL* shaderManager = renderer->getShaderManager();

	PBR_BrdfPrecomputeShaderGL* brdfPrecomputeShader = dynamic_cast<PBR_BrdfPrecomputeShaderGL*>
		(shaderManager->getConfig(ShaderType::Pbr_BrdfPrecompute));

	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();

	renderer->useBaseRenderTarget(target);
	renderer->beginScene();
	//renderer->beginScene();

	Sprite sprite;
	// setup sprite for brdf integration lookup texture
	vec2 dim = { 1.0, 1.0 };
	vec2 pos = { 0, 0 };

	// center
	pos.x = 0.5f * (1.0f - dim.x);
	pos.y = 0.5f * (1.0f - dim.y);

	sprite.setPosition(pos);
	sprite.setWidth(dim.x);
	sprite.setHeight(dim.y);

	modelDrawer->draw(&sprite, ShaderType::Pbr_BrdfPrecompute);
	//renderer->endScene();

	return target;
}

void PBR::init(TextureGL* backgroundHDR)
{

	Viewport backup = renderer->getViewport();

	environmentMap = renderBackgroundToCube(backgroundHDR);
	convolutedEnvironmentMap = convolute(environmentMap->getCubeMap());
	prefilterRenderTarget = prefilter(environmentMap->getCubeMap());

	renderer->setViewPort(backup.x, backup.y, backup.width, backup.height);


	// setup sprite for brdf integration lookup texture
	vec2 dim = { 1.0, 1.0 };
	vec2 pos = { 0, 0 };

	// center
	pos.x = 0.5f * (1.0f - dim.x);
	pos.y = 0.5f * (1.0f - dim.y);

	brdfSprite.setPosition(pos);
	brdfSprite.setWidth(dim.x);
	brdfSprite.setHeight(dim.y);

	// we don't need a texture
	// we use the sprite only as a polygon model
	brdfSprite.setTexture(nullptr);

	brdfLookupTexture = createBRDFlookupTexture();


	shader = dynamic_cast<PBRShaderGL*> (renderer->getShaderManager()->getConfig(ShaderType::Pbr));
}