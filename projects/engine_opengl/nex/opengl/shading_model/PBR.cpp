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

PBR::~PBR()
{
	delete environmentMap;
	environmentMap = nullptr;
	delete brdfLookupTexture;
	brdfLookupTexture = nullptr;
	delete prefilteredEnvMap;
	prefilteredEnvMap = nullptr;

	delete convolutedEnvironmentMap;
	convolutedEnvironmentMap = nullptr;
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
	skyboxShader->setSkyTexture(environmentMap);

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

	shader->setBrdfLookupTexture(brdfLookupTexture);
	shader->setIrradianceMap(convolutedEnvironmentMap);
	
	shader->setLightColor(light.getColor());
	shader->setLightDirection(light.getLook());
	shader->setLightSpaceMatrix(lightSpaceMatrix);
	shader->setLightProjMatrix(lightProjMatrix);
	shader->setLightViewMatrix(lightViewMatrix);

	shader->setPrefilterMap(prefilteredEnvMap);
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
	return convolutedEnvironmentMap;
}

CubeMapGL* PBR::getEnvironmentMap()
{
	return environmentMap;
}

CubeMapGL * PBR::getPrefilteredEnvironmentMap()
{
	return prefilteredEnvMap;
}

TextureGL * PBR::getBrdfLookupTexture()
{
	return brdfLookupTexture;
}

StoreImageGL PBR::readBrdfLookupPixelData() const
{
	StoreImageGL store;
	StoreImageGL::create(&store, 1, 1);

	GenericImageGL& data = store.images[0][0];
	data.width = brdfLookupTexture->getWidth();
	data.height = brdfLookupTexture->getHeight();
	data.components = 2; // RG
	data.format = GL_RG;
	data.pixelSize = sizeof(float) * data.components;

	data.bufSize = data.width * data.height * data.pixelSize;
	data.pixels = new char[data.bufSize];

	// read the data back from the gpu
	renderer->readback(
		brdfLookupTexture, 
		TextureTarget::TEXTURE2D, 
		0, ColorSpace::RG, 
		PixelDataType::FLOAT, 
		data.pixels.get());

	return store;
}

StoreImageGL PBR::readBackgroundPixelData() const
{
	StoreImageGL store;
	StoreImageGL::create(&store, 6, 1); // 6 sides, no mipmaps (only base level)

	for (unsigned i = 0; i < store.sideCount; ++i)
	{
		GenericImageGL& data = store.images[i][0];
		data.width = environmentMap->getWidth();
		data.height = environmentMap->getHeight();
		data.components = 3; // RGB
		data.format = GL_RGB;
		data.pixelSize = sizeof(float) * data.components; // internal format: RGB32F
		data.bufSize = data.width * data.height * data.pixelSize;
		data.pixels = new char[data.bufSize];

		// read the data back from the gpu
		renderer->readback(
			environmentMap,
			static_cast<TextureTarget>(static_cast<unsigned>(TextureTarget::CUBE_POSITIVE_X) + i),
			0, // base level
			ColorSpace::RGB,
			PixelDataType::FLOAT,
			data.pixels.get());
	}

	return store;
}

StoreImageGL PBR::readConvolutedEnvMapPixelData()
{
	StoreImageGL store;
	StoreImageGL::create(&store, 6, 6); // 6 sides, 32/16/8/4/2/1 = 6 levels

	for (unsigned i = 0; i < store.sideCount; ++i)
	{
		for (unsigned level = 0; level < store.mipmapCounts[i]; ++level)
		{
			GenericImageGL& data = store.images[i][level];
			data.width = convolutedEnvironmentMap->getWidth();
			data.height = convolutedEnvironmentMap->getHeight();
			data.components = 3; // RGB
			data.format = GL_RGB;
			data.pixelSize = sizeof(float) * data.components; // internal format: RGB32F
			data.bufSize = data.width * data.height * data.pixelSize;
			data.pixels = new char[data.bufSize];

			// read the data back from the gpu
			renderer->readback(
				convolutedEnvironmentMap,
				static_cast<TextureTarget>(static_cast<unsigned>(TextureTarget::CUBE_POSITIVE_X) + i),
				level,
				ColorSpace::RGB,
				PixelDataType::FLOAT,
				data.pixels.get());
		}
	}

	return store;
}

StoreImageGL PBR::readPrefilteredEnvMapPixelData()
{
	StoreImageGL store;
	StoreImageGL::create(&store, 6, 9); // 6 sides, 256/128/64/32/16/8/4/2/1 = 9 mipmap textures

	for (unsigned i = 0; i < store.sideCount; ++i)
	{
		for (unsigned level =  0; level < store.mipmapCounts[i]; ++level)
		{
			GenericImageGL& data = store.images[i][level];
			data.width = prefilteredEnvMap->getWidth();
			data.height = prefilteredEnvMap->getHeight();
			data.components = 3; // RGB
			data.format = GL_RGB;
			data.pixelSize = sizeof(float) * data.components; // internal format: RGB32F
			data.bufSize = data.width * data.height * data.pixelSize;
			data.pixels = new char[data.bufSize];

			// read the data back from the gpu
			renderer->readback(
				prefilteredEnvMap,
				static_cast<TextureTarget>(static_cast<unsigned>(TextureTarget::CUBE_POSITIVE_X) + i),
				level,
				ColorSpace::RGB,
				PixelDataType::FLOAT,
				data.pixels.get());
		}
	}

	return store;
}

CubeMapGL * PBR::renderBackgroundToCube(TextureGL * background)
{
	TextureData textureData = {
		TextureFilter::Linear_Mipmap_Linear,
		TextureFilter::Linear,
		TextureUVTechnique::ClampToEdge,
		ColorSpace::RGB,
		PixelDataType::FLOAT,
		InternFormat::RGB32F,
		false
	};


	CubeRenderTargetGL cubeRenderTarget(2048, 2048, textureData);

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

	
	renderer->setViewPort(0, 0, cubeRenderTarget.getWidth(), cubeRenderTarget.getHeight());
	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();


	const GLuint textureID = cubeRenderTarget.getCubeMap()->getTexture();
	std::stringstream ss;
	ss << "Before rendering background cube maps; cubemap texture id = " << textureID;


	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0x100000, GL_DEBUG_SEVERITY_NOTIFICATION, 0, "This is a test!");

	for (unsigned int side = 0; side < 6; ++side) {
		shader->setView(views[side]);
		renderer->useCubeRenderTarget(&cubeRenderTarget, static_cast<CubeMapGL::Side>(side + CubeMapGL::POSITIVE_X));
		modelDrawer->draw(skybox.getModel(), shader);
	}


	CubeMapGL* result = cubeRenderTarget.getCubeMap();
	cubeRenderTarget.setCubeMap(nullptr); // ensures that it won't be deleted 

	// now create mipmaps for the cubemap fighting render artificats in the prefilter map
	result->generateMipMaps();

	//CubeMap* result = cubeRenderTarget->createCopy(); 
	return result;
}

CubeMapGL * PBR::convolute(CubeMapGL * source)
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

	for (int side = 0; side < 6; ++side) {
		shader->setView(views[side]);
		renderer->useCubeRenderTarget(cubeRenderTarget, static_cast<CubeMapGL::Side>(side + CubeMapGL::POSITIVE_X));
		modelDrawer->draw(skybox.getModel(), shader);
	}

	//CubeMap* result = cubeRenderTarget->createCopy();
	//renderer->destroyCubeRenderTarget(cubeRenderTarget);

	CubeMapGL* result = cubeRenderTarget->getCubeMap();
	cubeRenderTarget->setCubeMap(nullptr); // ensures that it won't be deleted 
	renderer->destroyCubeRenderTarget(cubeRenderTarget);

	return result;
}

CubeMapGL* PBR::prefilter(CubeMapGL * source)
{
	TextureData textureData = {
		TextureFilter::Linear_Mipmap_Linear,
		TextureFilter::Linear,
		TextureUVTechnique::ClampToEdge,
		ColorSpace::RGB,
		PixelDataType::FLOAT,
		InternFormat::RGB32F,
		true
	};

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
		for (unsigned int side = 0; side < 6; ++side) {
			shader->setView(views[side]);
			renderer->useCubeRenderTarget(prefilterRenderTarget, static_cast<CubeMapGL::Side>(side + CubeMapGL::POSITIVE_X), mipLevel);
			modelDrawer->draw(skybox.getModel(), shader);
		}
	}


	CubeMapGL* result = prefilterRenderTarget->getCubeMap();
	prefilterRenderTarget->setCubeMap(nullptr);
	renderer->destroyCubeRenderTarget(prefilterRenderTarget);

	return result;
}

TextureGL* PBR::createBRDFlookupTexture()
{
	TextureData data = {
		TextureFilter::Linear, 
		TextureFilter::Linear, 
		TextureUVTechnique::ClampToEdge, 
		ColorSpace::RG, 
		PixelDataType::FLOAT, 
		InternFormat::RG32F, 
		false};

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

	TextureGL* result = target->getTexture();
	target->setTexture(nullptr);
	renderer->destroyRenderTarget(target);

	return result;
}

void PBR::init(TextureGL* backgroundHDR)
{

	Viewport backup = renderer->getViewport();

	// if environment map has been compiled already and load it from file 
	if (std::filesystem::exists("pbr_environmentMap.NeXImage"))
	{
		StoreImageGL readImage;
		StoreImageGL::load(&readImage, "pbr_environmentMap.NeXImage");

		TextureData data = {
			TextureFilter::Linear,
			TextureFilter::Linear,
			TextureUVTechnique::ClampToEdge,
			ColorSpace::RGB,
			PixelDataType::FLOAT,
			InternFormat::RGB32F,
			false
		};
		environmentMap = dynamic_cast<CubeMapGL*>(TextureGL::createFromImage(readImage, data, true));
	} else
	{
		environmentMap = renderBackgroundToCube(backgroundHDR);
		StoreImageGL enviromentMapImage = readBackgroundPixelData();
		StoreImageGL::write(enviromentMapImage, "pbr_environmentMap.NeXImage");
	}


	if (std::filesystem::exists("pbr_prefilteredEnvMap.NeXImage"))
	{
		StoreImageGL readImage;
		StoreImageGL::load(&readImage, "pbr_prefilteredEnvMap.NeXImage");

		TextureData data = {
			TextureFilter::Linear_Mipmap_Linear,
			TextureFilter::Linear,
			TextureUVTechnique::ClampToEdge,
			ColorSpace::RGB,
			PixelDataType::FLOAT,
			InternFormat::RGB32F,
			true
		};
		prefilteredEnvMap = dynamic_cast<CubeMapGL*>(TextureGL::createFromImage(readImage, data, true));
	}
	else
	{
		prefilteredEnvMap = prefilter(environmentMap);
		StoreImageGL enviromentMapImage = readPrefilteredEnvMapPixelData();
		StoreImageGL::write(enviromentMapImage, "pbr_prefilteredEnvMap.NeXImage");
	}

	



	// if environment map has been compiled already and load it from file 
	if (std::filesystem::exists("pbr_convolutedEnvMap.NeXImage"))
	{
		StoreImageGL readImage;
		StoreImageGL::load(&readImage, "pbr_convolutedEnvMap.NeXImage");

		const TextureData data = {
			TextureFilter::Linear,
			TextureFilter::Linear,
			TextureUVTechnique::ClampToEdge,
			ColorSpace::RGB,
			PixelDataType::FLOAT,
			InternFormat::RGB32F,
			true };

		convolutedEnvironmentMap = dynamic_cast<CubeMapGL*>(TextureGL::createFromImage(readImage, data, true));
	}
	else
	{
		convolutedEnvironmentMap = convolute(environmentMap);
		StoreImageGL enviromentMapImage = readConvolutedEnvMapPixelData();
		StoreImageGL::write(enviromentMapImage, "pbr_convolutedEnvMap.NeXImage");
	}

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

	

	if (std::filesystem::exists("brdfLUT.NeXImage"))
	{
		StoreImageGL readImage;
		StoreImageGL::load(&readImage, "brdfLUT.NeXImage");

		TextureData data = {
		TextureFilter::Linear,
		TextureFilter::Linear,
		TextureUVTechnique::ClampToEdge,
		ColorSpace::RG,
		PixelDataType::FLOAT,
		InternFormat::RG32F,
		false };

		brdfLookupTexture = TextureGL::createFromImage(readImage, data, false);
	}
	else
	{
		brdfLookupTexture = createBRDFlookupTexture();
		StoreImageGL brdfLUTImage = readBrdfLookupPixelData();
		StoreImageGL::write(brdfLUTImage, "brdfLUT.NeXImage");
	}


	// read backs
	/*GenericImageGL brdfLUTImage;
	TextureManagerGL::get()->readImage(&brdfLUTImage, "brdfLUT.NeXImage");
	TextureManagerGL::get()->writeHDR(brdfLUTImage, "readBacks/brdfLUT.hdr");
	TextureManagerGL::get()->writeImage(brdfLUTImage, "brdfLUT.NeXImage");*/

	//TextureManagerGL::get()->readGLITest("brdfLUT.hdr");
	{
		//StoreImageGL brdfLUTImage = readBrdfLookupPixelData();
		//StoreImageGL::write(brdfLUTImage, "brdfLUT.NeXImage");

		{
			//StoreImageGL readBrdfLUTImage;
			//StoreImageGL::load(&readBrdfLUTImage, "brdfLUT.NeXImage");
		}
	}

	{
		//StoreImageGL enviromentMapImage = readBackgroundPixelData();
		//StoreImageGL::write(enviromentMapImage, "pbr_environmentMap.NeXImage");

		{
			//StoreImageGL readImage;
			//StoreImageGL::load(&readImage, "pbr_environmentMap.NeXImage");
		}
	}

}