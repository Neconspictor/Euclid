#include <PBR.hpp>
#include <shader/SkyBoxShader.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace glm;

PBR::PBR() : environmentMap(nullptr), shader(nullptr), skybox("misc/SkyBoxCube.obj", Shaders::BlinnPhongTex){
}

PBR::~PBR(){

}

void PBR::init(Renderer3D * renderer, Texture* backgroundHDR)
{

	this->renderer = renderer;
	environmentMap = renderBackgroundToCube(backgroundHDR);
	//environmentMap = backgroundHDR;
	convolutedEnvironmentMap = convolute(environmentMap); //TODO
	//environmentMap = convolutedEnvironmentMap;

	shader = dynamic_cast<PBRShader*> (renderer->getShaderManager()->getConfig(Shaders::Pbr));
}

void PBR::drawSky(RenderTarget* renderTarget, const mat4& projection, const mat4& view)
{
	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	ShaderManager* shaderManager = renderer->getShaderManager();

	SkyBoxShader* skyboxShader = dynamic_cast<SkyBoxShader*>
		(shaderManager->getConfig(Shaders::SkyBox));
	skyboxShader->setSkyTexture(environmentMap);

	mat4 identity;
	mat4 skyBoxView = mat4(mat3(view));

	TransformData data = { &projection, &view, nullptr };
	data.model = &identity;
	data.view = &skyBoxView;

	modelDrawer->draw(&skybox, Shaders::SkyBox, data);
}

void PBR::drawSceneToShadowMap(SceneNode * scene, 
	float frameTimeElapsed, 
	DepthMap* shadowMap, 
	const DirectionalLight & light, 
	const mat4 & lightViewMatrix, 
	const mat4 & lightProjMatrix)
{
	const mat4& lightSpaceMatrix = lightProjMatrix * lightViewMatrix;
	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	scene->update(frameTimeElapsed);

	// render shadows to a depth map
	scene->draw(renderer, modelDrawer, lightProjMatrix, lightViewMatrix, Shaders::Shadow);
}

void PBR::drawScene(SceneNode * scene,
	RenderTarget* renderTarget,
	const vec3& cameraPosition, 
	float frameTimeElapsed,
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
	shader->setIrradianceMap(convolutedEnvironmentMap);
	shader->setLightColor(light.getColor());
	shader->setLightDirection(light.getLook());
	shader->setLightSpaceMatrix(lightSpaceMatrix);
	shader->setLightProjMatrix(lightProjMatrix);
	shader->setLightViewMatrix(lightViewMatrix);
	shader->setShadowMap(shadowMap);
	shader->setSkyBox(environmentMap);


	ModelDrawer* modelDrawer = renderer->getModelDrawer();

	scene->update(frameTimeElapsed);
	scene->draw(renderer, modelDrawer, projection, view);
}

CubeMap * PBR::getConvolutedEnvironmentMap()
{
	return convolutedEnvironmentMap;
}

CubeMap* PBR::getEnvironmentMap()
{
	return environmentMap;
}

CubeMap * PBR::renderBackgroundToCube(Texture * background)
{

	CubeRenderTarget* cubeRenderTarget = renderer->createCubeRenderTarget(2048, 2048);

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

	CubeMap* result = cubeRenderTarget->createCopy(); 
	renderer->destroyCubeRenderTarget(cubeRenderTarget);
	return result;
}

CubeMap * PBR::convolute(CubeMap * source)
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

	CubeMap* result = cubeRenderTarget->createCopy();
	renderer->destroyCubeRenderTarget(cubeRenderTarget);
	return result;
}
