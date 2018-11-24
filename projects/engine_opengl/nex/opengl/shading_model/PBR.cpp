#include <nex/opengl/shading_model/PBR.hpp>
#include <nex/opengl/shader/SkyBoxShaderGL.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/opengl/shader/ShaderManagerGL.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include "nex/opengl/shader/ShadowShaderGL.hpp"
#include "nex/util/ExceptionHandling.hpp"

using namespace glm;

PBR::PBR(RendererOpenGL* renderer, TextureGL* backgroundHDR) :
	environmentMap(nullptr), renderer(renderer), skybox("misc/SkyBoxCube.obj", ShaderType::BlinnPhongTex){

	skybox.init(renderer->getModelManager());

	init(backgroundHDR);
}

PBR::~PBR(){

}

void PBR::drawSky(const mat4& projection, const mat4& view)
{
	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();
	ShaderManagerGL* shaderManager = renderer->getShaderManager();

	SkyBoxShaderGL* skyboxShader = reinterpret_cast<SkyBoxShaderGL*>
		(shaderManager->getShader(ShaderType::SkyBox));

	mat4 skyBoxView = mat4(mat3(view));

	skyboxShader->bind();
	skyboxShader->setupRenderState();
	
	skyboxShader->setView(skyBoxView);
	skyboxShader->setProjection(projection);
	skyboxShader->setMVP(projection * skyBoxView);
	//skyboxShader->setSkyTexture(prefilterRenderTarget->getCubeMap());
	skyboxShader->setSkyTexture(environmentMap->getCubeMap());

	modelDrawer->draw(skybox.getModel(), skyboxShader);

	//TODO optimize out
	skyboxShader->reverseRenderState();
}

void PBR::drawSceneToShadowMap(SceneNode * scene, 
	DepthMapGL* shadowMap,
	const DirectionalLight & light, 
	const mat4 & lightViewMatrix, 
	const mat4 & lightProjMatrix)
{
	const mat4& lightSpaceMatrix = lightProjMatrix * lightViewMatrix;
	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();
	ShadowShaderGL* shader = (ShadowShaderGL*) renderer->getShaderManager()->getShader(ShaderType::Shadow);

	shader->bind();
	shader->setLightSpaceMatrix(lightSpaceMatrix);

	// render shadows to a depth map
	modelDrawer->draw(scene, shader);
	//scene->draw(renderer, modelDrawer, lightProjMatrix, lightViewMatrix, ShaderType::Shadow);
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

	 PBRShaderGL* shader = reinterpret_cast<PBRShaderGL*> (renderer->getShaderManager()->getShader(ShaderType::Pbr));

	shader->bind();

	shader->setBrdfLookupTexture(brdfLookupTexture->getTexture());
	shader->setIrradianceMap(convolutedEnvironmentMap->getCubeMap());
	
	shader->setLightColor(light.getColor());
	shader->setLightDirection(light.getLook());
	shader->setLightSpaceMatrix(lightSpaceMatrix);
	shader->setLightProjMatrix(lightProjMatrix);
	shader->setLightViewMatrix(lightViewMatrix);

	shader->setPrefilterMap(prefilterRenderTarget->getCubeMap());
	shader->setShadowMap(shadowMap);

	//TODO validate whether this is needed
	//shader->setSkyBox(environmentMap->getCubeMap());

	shader->setCameraPosition(cameraPosition);

	shader->setViewMatrix(view);
	shader->setInverseViewMatrix(inverse(view));
	shader->setProjectionMatrix(projection);
	

	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();
	modelDrawer->draw(scene, shader);
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

GenericImageGL PBR::readBrdfLookupPixelData()
{
	GenericImageGL data;
	data.width = brdfLookupTexture->getWidth();
	data.height = brdfLookupTexture->getHeight();
	data.components = 4; // RGB
	data.format = GL_RGBA;
	data.pixelSize = 4 * data.components;

	data.bufSize = data.width * data.height * data.pixelSize;
	data.pixels = new char[data.bufSize];

	// read the data back from the gpu
	GLuint textureID = brdfLookupTexture->getTexture()->getTexture();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLCall(glActiveTexture(GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_2D, textureID));
	GLCall(glGetTexImage(GL_TEXTURE_2D, 0, data.format, GL_FLOAT, data.pixels));

	return data;
}

GenericImageGL PBR::readTestPixelData()
{
	GenericImageGL data;
	data.width = prefilterRenderTarget->getWidth();
	data.height = prefilterRenderTarget->getHeight();
	data.components = 4; // RGB
	data.format = GL_RGBA;
	data.pixelSize = 4 * data.components;

	data.bufSize = data.width * data.height * data.pixelSize;
	data.pixels = new char[data.bufSize];

	// read the data back from the gpu
	GLuint textureID = prefilterRenderTarget->getCubeMap()->getTexture();

	GLuint fbo;
	//glGenFramebuffers(1, &fbo);
	//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLCall(glActiveTexture(GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, textureID, 0);
	//glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, data.format, 0, 0, data.width, data.height, 0);
	GLCall(glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, data.format, GL_FLOAT, data.pixels));

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glDeleteFramebuffers(1, &fbo);

	return data;
}

CubeRenderTargetGL * PBR::renderBackgroundToCube(TextureGL * background)
{
	TextureData textureData = {false, false, Linear_Mipmap_Linear, Linear, ClampToEdge, RGB, true, BITS_32};

	CubeRenderTargetGL* cubeRenderTarget = renderer->createCubeRenderTarget(2048, 2048, textureData);

	ShaderManagerGL* shaderManager = renderer->getShaderManager();

	EquirectangularSkyBoxShaderGL* shader = reinterpret_cast<EquirectangularSkyBoxShaderGL*>
		(shaderManager->getShader(ShaderType::SkyBoxEquirectangular));

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

	shader->bind();
	shader->setProjection(projection);
	shader->setSkyTexture(background);

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
		shader->setView(views[side]);
		renderer->useCubeRenderTarget(cubeRenderTarget, (CubeMapGL::Side)side);
		modelDrawer->draw(skybox.getModel(), shader);
	}

	// now create mipmaps for the cubemap fighting render artificats in the prefilter map
	cubeRenderTarget->getCubeMap()->generateMipMaps();

	//CubeMap* result = cubeRenderTarget->createCopy(); 
	//renderer->destroyCubeRenderTarget(cubeRenderTarget);
	return cubeRenderTarget;
}

CubeRenderTargetGL * PBR::convolute(CubeMapGL * source)
{
	// uses RGB and 32bit per component (floats)
	CubeRenderTargetGL* cubeRenderTarget = renderer->createCubeRenderTarget(32, 32);

	ShaderManagerGL* shaderManager = renderer->getShaderManager();

	PBR_ConvolutionShaderGL* shader = reinterpret_cast<PBR_ConvolutionShaderGL*>
		(shaderManager->getShader(ShaderType::Pbr_Convolution));

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

	shader->bind();
	shader->setProjection(projection);
	shader->setEnvironmentMap(source);


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
		shader->setView(views[side]);
		renderer->useCubeRenderTarget(cubeRenderTarget, (CubeMapGL::Side)side);
		modelDrawer->draw(skybox.getModel(), shader);
	}

	//CubeMap* result = cubeRenderTarget->createCopy();
	//renderer->destroyCubeRenderTarget(cubeRenderTarget);
	return cubeRenderTarget;
}

CubeRenderTargetGL* PBR::prefilter(CubeMapGL * source)
{
	TextureData textureData = { false, true, Linear_Mipmap_Linear, Linear, ClampToEdge, RGB, true, BITS_32 };

	CubeRenderTargetGL* prefilterRenderTarget = renderer->createCubeRenderTarget(256, 256, textureData);

	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();
	ShaderManagerGL* shaderManager = renderer->getShaderManager();

	PBR_PrefilterShaderGL* shader = dynamic_cast<PBR_PrefilterShaderGL*>
		(shaderManager->getShader(ShaderType::Pbr_Prefilter));

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);


	shader->bind();
	shader->setProjection(projection);
	shader->setMapToPrefilter(source);

	//view matrices;
	const mat4 views[] = {
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_X), //right; sign of up vector is not important
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_X), //left
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_Y), //top
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_Y), //bottom
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_Z), //back
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_Z) //front
	};

	unsigned int maxMipLevels = 5;

	for (unsigned int mipLevel = 0; mipLevel < maxMipLevels; ++mipLevel) {

		// update the roughness value for the current mipmap level
		float roughness = (float)mipLevel / (float)(maxMipLevels - 1);
		shader->setRoughness(roughness);

		//resize render target according to mip level size
		prefilterRenderTarget->resizeForMipMap(mipLevel);

		// update the viewport size
		unsigned int width = prefilterRenderTarget->getWidthMipLevel(mipLevel);
		unsigned int height = prefilterRenderTarget->getHeightMipLevel(mipLevel);
		renderer->setViewPort(0, 0, width, height);

		// render to the cubemap at the specified mip level
		for (int side = CubeMapGL::POSITIVE_X; side < 6; ++side) {
			shader->setView(views[side]);
			renderer->useCubeRenderTarget(prefilterRenderTarget, (CubeMapGL::Side)side, mipLevel);
			modelDrawer->draw(skybox.getModel(), shader);
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

	ShaderManagerGL* shaderManager = renderer->getShaderManager();

	PBR_BrdfPrecomputeShaderGL* brdfPrecomputeShader = reinterpret_cast<PBR_BrdfPrecomputeShaderGL*>
		(shaderManager->getShader(ShaderType::Pbr_BrdfPrecompute));

	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();

	renderer->useBaseRenderTarget(target);
	renderer->beginScene();

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

	brdfPrecomputeShader->bind();
	modelDrawer->draw(&sprite, brdfPrecomputeShader);

	return target;
}

void PBR::init(TextureGL* backgroundHDR)
{

	Viewport backup = renderer->getViewport();

	environmentMap = renderBackgroundToCube(backgroundHDR);
	prefilterRenderTarget = prefilter(environmentMap->getCubeMap());
	convolutedEnvironmentMap = convolute(environmentMap->getCubeMap());
	

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


	// read backs
	/*GenericImageGL brdfLUTImage = readBrdfLookupPixelData();


	glm::vec4* pixels = (glm::vec4*)brdfLUTImage.pixels;

	std::ofstream out("brdfLUT.txt");

	for (unsigned x = 0; x < brdfLUTImage.width; ++x)
	{
		for (unsigned y = 0; y < brdfLUTImage.height; ++y)
		{
			int index = x * brdfLUTImage.width + y;
			out << pixels[index].x << " " << pixels[index].y << " " << pixels[index].z << " " << pixels[index].w << "\n";
		}
	}

	out.close();


	// release memory
	delete brdfLUTImage.pixels;*/
}