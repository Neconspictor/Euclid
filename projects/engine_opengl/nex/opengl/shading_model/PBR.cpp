#include <nex/opengl/shading_model/PBR.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/shader/ShaderManager.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>

using namespace glm;

PBR::PBR(RendererOpenGL* renderer, Texture* backgroundHDR) :
	environmentMap(nullptr), shader(nullptr), renderer(renderer), skybox("misc/SkyBoxCube.obj", Shaders::BlinnPhongTex){

	skybox.init(renderer->getModelManager());

	init(backgroundHDR);
}

PBR::~PBR(){

}

void PBR::drawSky(const mat4& projection, const mat4& view)
{
	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	ShaderManager* shaderManager = renderer->getShaderManager();

	SkyBoxShader* skyboxShader = dynamic_cast<SkyBoxShader*>
		(shaderManager->getConfig(Shaders::SkyBox));

	skyboxShader->setSkyTexture(environmentMap->getCubeMap());
	//skyboxShader->setSkyTexture(prefilterRenderTarget->getCubeMap());

	mat4 identity;
	mat4 skyBoxView = mat4(mat3(view));

	TransformData data = { &projection, &view, nullptr };
	data.model = &identity;
	data.view = &skyBoxView;

	modelDrawer->draw(&skybox, Shaders::SkyBox, data);
}

void PBR::drawSceneToShadowMap(SceneNode * scene, 
	DepthMap* shadowMap, 
	const DirectionalLight & light, 
	const mat4 & lightViewMatrix, 
	const mat4 & lightProjMatrix)
{
	const mat4& lightSpaceMatrix = lightProjMatrix * lightViewMatrix;
	ModelDrawer* modelDrawer = renderer->getModelDrawer();

	// render shadows to a depth map
	scene->draw(renderer, modelDrawer, lightProjMatrix, lightViewMatrix, Shaders::Shadow);
}

void PBR::drawScene(SceneNode * scene,
	const vec3& cameraPosition, 
	Texture* shadowMap,
	const DirectionalLight& light, 
	const mat4& lightViewMatrix, 
	const mat4& lightProjMatrix,
	const mat4& view,
	const mat4& projection)
{

	 mat4 lightSpaceMatrix = lightProjMatrix * lightViewMatrix;

	shader = dynamic_cast<PBRShader*> (renderer->getShaderManager()->getConfig(Shaders::Pbr));

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


	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	scene->draw(renderer, modelDrawer, projection, view, Shaders::Pbr);
}

CubeMap * PBR::getConvolutedEnvironmentMap()
{
	return convolutedEnvironmentMap->getCubeMap();
}

CubeMap* PBR::getEnvironmentMap()
{
	return environmentMap->getCubeMap();
}

CubeMap * PBR::getPrefilteredEnvironmentMap()
{
	return prefilterRenderTarget->getCubeMap();
}

Texture * PBR::getBrdfLookupTexture()
{
	return brdfLookupTexture->getTexture();
}

CubeRenderTarget * PBR::renderBackgroundToCube(Texture * background)
{
	TextureData textureData = {false, false, Linear_Mipmap_Linear, Linear, ClampToEdge, RGB, true, BITS_32};

	CubeRenderTarget* cubeRenderTarget = renderer->createCubeRenderTarget(2048, 2048, textureData);

	ShaderManager* shaderManager = renderer->getShaderManager();

	EquirectangularSkyBoxShader* equirectangularSkyBoxShader = dynamic_cast<EquirectangularSkyBoxShader*>
		(shaderManager->getConfig(Shaders::SkyBoxEquirectangular));

	equirectangularSkyBoxShader->setSkyTexture(background);

	TransformData data;
	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
	data.projection = &projection;
	mat4 model = glm::mat4();
	data.model = &model;


	//view matrices;
	const mat4 views[] = {
		CubeMap::getViewLookAtMatrixRH(CubeMap::POSITIVE_X), //right; sign of up vector is not important
		CubeMap::getViewLookAtMatrixRH(CubeMap::NEGATIVE_X), //left
		CubeMap::getViewLookAtMatrixRH(CubeMap::POSITIVE_Y), //top
		CubeMap::getViewLookAtMatrixRH(CubeMap::NEGATIVE_Y), //bottom
		CubeMap::getViewLookAtMatrixRH(CubeMap::POSITIVE_Z), //back
		CubeMap::getViewLookAtMatrixRH(CubeMap::NEGATIVE_Z) //front
	};

	
	renderer->setViewPort(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());
	ModelDrawer* modelDrawer = renderer->getModelDrawer();

	for (int side = CubeMap::POSITIVE_X; side < 6; ++side) {
		data.view = &views[side];
		renderer->useCubeRenderTarget(cubeRenderTarget, (CubeMap::Side)side);
		modelDrawer->draw(&skybox, Shaders::SkyBoxEquirectangular, data);
	}

	// now create mipmaps for the cubemap fighting render artificats in the prefilter map
	cubeRenderTarget->getCubeMap()->generateMipMaps();

	//CubeMap* result = cubeRenderTarget->createCopy(); 
	//renderer->destroyCubeRenderTarget(cubeRenderTarget);
	return cubeRenderTarget;
}

CubeRenderTarget * PBR::convolute(CubeMap * source)
{
	CubeRenderTarget* cubeRenderTarget = renderer->createCubeRenderTarget(32, 32);

	ShaderManager* shaderManager = renderer->getShaderManager();

	PBR_ConvolutionShader* convolutionShader = dynamic_cast<PBR_ConvolutionShader*>
		(shaderManager->getConfig(Shaders::Pbr_Convolution));

	convolutionShader->setEnvironmentMap(source);

	TransformData data;
	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
	data.projection = &projection;
	mat4 model = glm::mat4();
	data.model = &model;


	//view matrices;
	const mat4 views[] = {
		CubeMap::getViewLookAtMatrixRH(CubeMap::POSITIVE_X), //right; sign of up vector is not important
		CubeMap::getViewLookAtMatrixRH(CubeMap::NEGATIVE_X), //left
		CubeMap::getViewLookAtMatrixRH(CubeMap::POSITIVE_Y), //top
		CubeMap::getViewLookAtMatrixRH(CubeMap::NEGATIVE_Y), //bottom
		CubeMap::getViewLookAtMatrixRH(CubeMap::POSITIVE_Z), //back
		CubeMap::getViewLookAtMatrixRH(CubeMap::NEGATIVE_Z) //front
	};

	renderer->setViewPort(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());
	ModelDrawer* modelDrawer = renderer->getModelDrawer();

	for (int side = CubeMap::POSITIVE_X; side < 6; ++side) {
		data.view = &views[side];
		renderer->useCubeRenderTarget(cubeRenderTarget, (CubeMap::Side)side);
		modelDrawer->draw(&skybox, Shaders::Pbr_Convolution, data);
	}

	//CubeMap* result = cubeRenderTarget->createCopy();
	//renderer->destroyCubeRenderTarget(cubeRenderTarget);
	return cubeRenderTarget;
}

CubeRenderTarget* PBR::prefilter(CubeMap * source)
{
	TextureData textureData = { false, true, Linear_Mipmap_Linear, Linear, ClampToEdge, RGB, true, BITS_32 };

	CubeRenderTarget* prefilterRenderTarget = renderer->createCubeRenderTarget(256, 256, textureData);

	ShaderManager* shaderManager = renderer->getShaderManager();

	PBR_PrefilterShader* prefilterShader = dynamic_cast<PBR_PrefilterShader*>
		(shaderManager->getConfig(Shaders::Pbr_Prefilter));

	prefilterShader->setMapToPrefilter(source);

	TransformData data;
	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
	data.projection = &projection;
	mat4 model = glm::mat4();
	data.model = &model;


	//view matrices;
	const mat4 views[] = {
		CubeMap::getViewLookAtMatrixRH(CubeMap::POSITIVE_X), //right; sign of up vector is not important
		CubeMap::getViewLookAtMatrixRH(CubeMap::NEGATIVE_X), //left
		CubeMap::getViewLookAtMatrixRH(CubeMap::POSITIVE_Y), //top
		CubeMap::getViewLookAtMatrixRH(CubeMap::NEGATIVE_Y), //bottom
		CubeMap::getViewLookAtMatrixRH(CubeMap::POSITIVE_Z), //back
		CubeMap::getViewLookAtMatrixRH(CubeMap::NEGATIVE_Z) //front
	};

	ModelDrawer* modelDrawer = renderer->getModelDrawer();

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
		for (int side = CubeMap::POSITIVE_X; side < 6; ++side) {
			data.view = &views[side];
			renderer->useCubeRenderTarget(prefilterRenderTarget, (CubeMap::Side)side, mipLevel);
			modelDrawer->draw(&skybox, Shaders::Pbr_Prefilter, data);
		}

	}

	//CubeMap* result = prefilterRenderTarget->createCopy();
	//renderer->destroyCubeRenderTarget(prefilterRenderTarget);

	//TODO extract the cubemap from the render target (if possible?)
	return prefilterRenderTarget;
}

RenderTarget * PBR::createBRDFlookupTexture()
{
	TextureData data = { false, false, Linear, Linear, ClampToEdge, RG, true, BITS_32 };
	RenderTarget* target = renderer->create2DRenderTarget(1024, 1024, data);


	ModelManager* modelManager = renderer->getModelManager();

	Model* spriteModel = modelManager->getSprite();// getModel(ModelManager::SPRITE_MODEL_NAME, Shaders::Unknown);

	ShaderManager* shaderManager = renderer->getShaderManager();

	PBR_BrdfPrecomputeShader* brdfPrecomputeShader = dynamic_cast<PBR_BrdfPrecomputeShader*>
		(shaderManager->getConfig(Shaders::Pbr_BrdfPrecompute));

	ModelDrawer* modelDrawer = renderer->getModelDrawer();

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

	modelDrawer->draw(&sprite, Shaders::Pbr_BrdfPrecompute);
	//renderer->endScene();

	return target;
}

void PBR::init(Texture* backgroundHDR)
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


	shader = dynamic_cast<PBRShader*> (renderer->getShaderManager()->getConfig(Shaders::Pbr));
}