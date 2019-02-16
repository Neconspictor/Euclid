#include <nex/pbr/PBR.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/shader/ShaderManager.hpp>
#include <nex/shader/ShadowShader.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/RenderBackend.hpp>
#include "nex/shader/PBRShader.hpp"

using namespace glm;
using namespace nex;

PBR::PBR(Texture* backgroundHDR) :
	environmentMap(nullptr), skybox("misc/SkyBoxCube.obj", ShaderType::BlinnPhongTex){

	skybox.init();

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
	static auto* shaderManager = ShaderManager::get();

	SkyBoxShader* skyboxShader = reinterpret_cast<SkyBoxShader*>
		(shaderManager->getShader(ShaderType::SkyBox));

	mat4 skyBoxView = mat4(mat3(view));

	skyboxShader->bind();
	skyboxShader->setupRenderState();
	
	skyboxShader->setView(skyBoxView);
	skyboxShader->setProjection(projection);
	skyboxShader->setMVP(projection * skyBoxView);
	//skyboxShader->setSkyTexture(prefilterRenderTarget->getCubeMap());
	skyboxShader->setSkyTexture(environmentMap);

	StaticMeshDrawer::draw(skybox.getModel(), skyboxShader);

	//TODO optimize out
	skyboxShader->reverseRenderState();
}

void PBR::drawSceneToShadowMap(SceneNode * scene,
	const mat4 & lightViewMatrix, 
	const mat4 & lightProjMatrix)
{
	static auto* shaderManager = ShaderManager::get();
	const mat4& lightSpaceMatrix = lightProjMatrix * lightViewMatrix;
	ShadowShader* shader = (ShadowShader*)shaderManager->getShader(ShaderType::Shadow);

	shader->bind();
	shader->setLightSpaceMatrix(lightSpaceMatrix);

	// render shadows to a depth map
	StaticMeshDrawer::draw(scene, shader);
	//scene->draw(renderer, modelDrawer, lightProjMatrix, lightViewMatrix, ShaderType::Shadow);
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
	static auto* shaderManager = ShaderManager::get();
	 mat4 lightSpaceMatrix = lightProjMatrix * lightViewMatrix;

	 PBRShader* shader = reinterpret_cast<PBRShader*> (shaderManager->getShader(ShaderType::Pbr));

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
	
	StaticMeshDrawer::draw(scene, shader);
}

CubeMap * PBR::getConvolutedEnvironmentMap()
{
	return convolutedEnvironmentMap;
}

CubeMap* PBR::getEnvironmentMap()
{
	return environmentMap;
}

CubeMap * PBR::getPrefilteredEnvironmentMap()
{
	return prefilteredEnvMap;
}

Texture * PBR::getBrdfLookupTexture()
{
	return brdfLookupTexture;
}

StoreImage PBR::readBrdfLookupPixelData() const
{
	StoreImage store;
	StoreImage::create(&store, 1, 1);

	GenericImage& data = store.images[0][0];
	data.width = brdfLookupTexture->getWidth();
	data.height = brdfLookupTexture->getHeight();
	data.components = 2; // RG
	data.format = (unsigned)ColorSpace::RG;
	data.pixelSize = sizeof(float) * data.components;

	data.bufSize = data.width * data.height * data.pixelSize;
	data.pixels = new char[data.bufSize];

	// read the data back from the gpu
	brdfLookupTexture->readback(
		TextureTarget::TEXTURE2D, 
		0, ColorSpace::RG, 
		PixelDataType::FLOAT, 
		data.pixels.get());

	return store;
}

StoreImage PBR::readBackgroundPixelData() const
{
	StoreImage store;
	StoreImage::create(&store, 6, 1); // 6 sides, no mipmaps (only base level)

	for (unsigned i = 0; i < store.sideCount; ++i)
	{
		GenericImage& data = store.images[i][0];
		data.width = environmentMap->getSideWidth();
		data.height = environmentMap->getSideHeight();
		data.components = 3; // RGB
		data.format = (unsigned)ColorSpace::RGB;
		data.pixelSize = sizeof(float) * data.components; // internal format: RGB32F
		data.bufSize = data.width * data.height * data.pixelSize;
		data.pixels = new char[data.bufSize];

		// read the data back from the gpu
		environmentMap->readback(
			TextureTarget::CUBE_MAP,
			0, // base level
			ColorSpace::RGB,
			PixelDataType::FLOAT,
			data.pixels.get(),
			(CubeMapSide)i);
	}

	return store;
}

StoreImage PBR::readConvolutedEnvMapPixelData()
{
	StoreImage store;
	StoreImage::create(&store, 6, 6); // 6 sides, 32/16/8/4/2/1 = 6 levels

	for (unsigned i = 0; i < store.sideCount; ++i)
	{
		for (unsigned level = 0; level < store.mipmapCounts[i]; ++level)
		{
			GenericImage& data = store.images[i][level];
			data.width = convolutedEnvironmentMap->getSideWidth();
			data.height = convolutedEnvironmentMap->getSideHeight();
			data.components = 3; // RGB
			data.format = (unsigned)ColorSpace::RGB;
			data.pixelSize = sizeof(float) * data.components; // internal format: RGB32F
			data.bufSize = data.width * data.height * data.pixelSize;
			data.pixels = new char[data.bufSize];

			// read the data back from the gpu
			convolutedEnvironmentMap->readback(
				TextureTarget::CUBE_MAP,
				level,
				ColorSpace::RGB,
				PixelDataType::FLOAT,
				data.pixels.get(),
				(CubeMapSide)i);
		}
	}

	return store;
}

StoreImage PBR::readPrefilteredEnvMapPixelData()
{
	StoreImage store;
	StoreImage::create(&store, 6, 9); // 6 sides, 256/128/64/32/16/8/4/2/1 = 9 mipmap textures

	for (unsigned i = 0; i < store.sideCount; ++i)
	{
		for (unsigned level =  0; level < store.mipmapCounts[i]; ++level)
		{
			GenericImage& data = store.images[i][level];
			data.width = prefilteredEnvMap->getSideWidth();
			data.height = prefilteredEnvMap->getSideHeight();
			data.components = 3; // RGB
			data.format = (unsigned)ColorSpace::RGB;
			data.pixelSize = sizeof(float) * data.components; // internal format: RGB32F
			data.bufSize = data.width * data.height * data.pixelSize;
			data.pixels = new char[data.bufSize];

			// read the data back from the gpu
			prefilteredEnvMap->readback(
				TextureTarget::CUBE_MAP,
				level,
				ColorSpace::RGB,
				PixelDataType::FLOAT,
				data.pixels.get(),
				(CubeMapSide)i);
		}
	}

	return store;
}

CubeMap * PBR::renderBackgroundToCube(Texture * background)
{
	TextureData textureData = {
		TextureFilter::Linear_Mipmap_Linear,
		TextureFilter::Linear,
		TextureUVTechnique::ClampToEdge,
		TextureUVTechnique::ClampToEdge,
		TextureUVTechnique::ClampToEdge,
		ColorSpace::RGB,
		PixelDataType::FLOAT,
		InternFormat::RGB32F,
		false
	};


	auto cubeRenderTarget = std::make_unique<CubeRenderTarget>(2048, 2048, textureData);

	static auto* shaderManager = ShaderManager::get();
	static auto* renderBackend = RenderBackend::get();

	EquirectangularSkyBoxShader* shader = reinterpret_cast<EquirectangularSkyBoxShader*>
		(shaderManager->getShader(ShaderType::SkyBoxEquirectangular));

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

	shader->bind();
	shader->setProjection(projection);
	shader->setSkyTexture(background);

	//view matrices;
	const mat4 views[] = {
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_X), //right; sign of up vector is not important
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_X), //left
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_Y), //top
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_Y), //bottom
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_Z), //back
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_Z) //front
	};

	
	renderBackend->setViewPort(0, 0, cubeRenderTarget->getSideWidth(), cubeRenderTarget->getSideHeight());


	/*TextureGL* gl = (TextureGL*)cubeRenderTarget->getTexture()->getImpl();
	const GLuint textureID = *gl->getTexture();
	std::stringstream ss;
	ss << "Before rendering background cube maps; cubemap texture id = " << textureID;*/


	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0x100000, GL_DEBUG_SEVERITY_NOTIFICATION, 0, "This is a test!");

	for (unsigned int side = 0; side < 6; ++side) {
		shader->setView(views[side]);
		cubeRenderTarget->useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X));
		StaticMeshDrawer::draw(skybox.getModel(), shader);
	}


	CubeMap* result = (CubeMap*)cubeRenderTarget->getRenderResult();

	// now create mipmaps for the cubemap fighting render artificats in the prefilter map
	result->generateMipMaps();

	cubeRenderTarget->setRenderResult(nullptr);

	//CubeMap* result = cubeRenderTarget->createCopy(); 
	return result;
}

CubeMap * PBR::convolute(CubeMap * source)
{
	static auto* renderBackend = RenderBackend::get();
	static auto* shaderManager = ShaderManager::get();
	
	// uses RGB and 32bit per component (floats)
	CubeRenderTarget* cubeRenderTarget = renderBackend->createCubeRenderTarget(32, 32);


	PBR_ConvolutionShader* shader = reinterpret_cast<PBR_ConvolutionShader*>
		(shaderManager->getShader(ShaderType::Pbr_Convolution));

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

	shader->bind();
	shader->setProjection(projection);
	shader->setEnvironmentMap(source);


	//view matrices;
	const mat4 views[] = {
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_X), //right; sign of up vector is not important
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_X), //left
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_Y), //top
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_Y), //bottom
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_Z), //back
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_Z) //front
	};

	renderBackend->setViewPort(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());

	for (int side = 0; side < 6; ++side) {
		shader->setView(views[side]);
		cubeRenderTarget->useSide(static_cast<CubeMapSide>(side));
		StaticMeshDrawer::draw(skybox.getModel(), shader);
	}

	//CubeMap* result = cubeRenderTarget->createCopy();
	//renderer->destroyCubeRenderTarget(cubeRenderTarget);

	CubeMap* result = (CubeMap*)cubeRenderTarget->setRenderResult(nullptr);
	renderBackend->destroyCubeRenderTarget(cubeRenderTarget);

	return result;
}

CubeMap* PBR::prefilter(CubeMap * source)
{
	TextureData textureData = {
		TextureFilter::Linear_Mipmap_Linear,
		TextureFilter::Linear,
		TextureUVTechnique::ClampToEdge,
		TextureUVTechnique::ClampToEdge,
		TextureUVTechnique::ClampToEdge,
		ColorSpace::RGB,
		PixelDataType::FLOAT,
		InternFormat::RGB32F,
		true
	};

	static auto* renderBackend = RenderBackend::get();
	static auto* shaderManager = ShaderManager::get();

	CubeRenderTarget* prefilterRenderTarget = renderBackend->createCubeRenderTarget(256, 256, textureData);

	PBR_PrefilterShader* shader = dynamic_cast<PBR_PrefilterShader*>
		(shaderManager->getShader(ShaderType::Pbr_Prefilter));

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);


	shader->bind();
	shader->setProjection(projection);
	shader->setMapToPrefilter(source);

	//view matrices;
	const mat4 views[] = {
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_X), //right; sign of up vector is not important
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_X), //left
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_Y), //top
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_Y), //bottom
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_Z), //back
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_Z) //front
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
		renderBackend->setViewPort(0, 0, width, height);

		// render to the cubemap at the specified mip level
		for (unsigned int side = 0; side < 6; ++side) {
			shader->setView(views[side]);
			prefilterRenderTarget->useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X), mipLevel);
			StaticMeshDrawer::draw(skybox.getModel(), shader);
		}
	}


	CubeMap* result = (CubeMap*)prefilterRenderTarget->setRenderResult(nullptr);
	renderBackend->destroyCubeRenderTarget(prefilterRenderTarget);

	return result;
}

Texture2D* PBR::createBRDFlookupTexture()
{
	TextureData data = {
		TextureFilter::Linear, 
		TextureFilter::Linear, 
		TextureUVTechnique::ClampToEdge, 
		TextureUVTechnique::ClampToEdge,
		TextureUVTechnique::ClampToEdge,
		ColorSpace::RG, 
		PixelDataType::FLOAT, 
		InternFormat::RG32F, 
		false};

	static auto* renderBackend = RenderBackend::get();
	static auto* shaderManager = ShaderManager::get();

	RenderTarget2D* target = renderBackend->create2DRenderTarget(1024, 1024, data);

	PBR_BrdfPrecomputeShader* brdfPrecomputeShader = reinterpret_cast<PBR_BrdfPrecomputeShader*>
		(shaderManager->getShader(ShaderType::Pbr_BrdfPrecompute));

	target->bind();
	target->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);

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
	StaticMeshDrawer::draw(&sprite, brdfPrecomputeShader);

	Texture2D* result = (Texture2D*)target->setRenderResult(nullptr);
	renderBackend->destroyRenderTarget(target);

	return result;
}

void PBR::init(Texture* backgroundHDR)
{
	static auto* renderBackend = RenderBackend::get();

	Viewport backup = renderBackend->getViewport();

	// if environment map has been compiled already and load it from file 
	if (std::filesystem::exists("pbr_environmentMap.NeXImage"))
	{
		StoreImage readImage;
		StoreImage::load(&readImage, "pbr_environmentMap.NeXImage");

		TextureData data = {
			TextureFilter::Linear,
			TextureFilter::Linear,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			ColorSpace::RGB,
			PixelDataType::FLOAT,
			InternFormat::RGB32F,
			false
		};
		environmentMap = (CubeMap*)(Texture::createFromImage(readImage, data, true));
	} else
	{
		environmentMap = renderBackgroundToCube(backgroundHDR);
		StoreImage enviromentMapImage = readBackgroundPixelData();
		StoreImage::write(enviromentMapImage, "pbr_environmentMap.NeXImage");
	}


	if (std::filesystem::exists("pbr_prefilteredEnvMap.NeXImage"))
	{
		StoreImage readImage;
		StoreImage::load(&readImage, "pbr_prefilteredEnvMap.NeXImage");

		TextureData data = {
			TextureFilter::Linear_Mipmap_Linear,
			TextureFilter::Linear,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			ColorSpace::RGB,
			PixelDataType::FLOAT,
			InternFormat::RGB32F,
			true
		};
		prefilteredEnvMap = (nex::CubeMap*)(Texture::createFromImage(readImage, data, true));
	}
	else
	{
		prefilteredEnvMap = prefilter(environmentMap);
		StoreImage enviromentMapImage = readPrefilteredEnvMapPixelData();
		StoreImage::write(enviromentMapImage, "pbr_prefilteredEnvMap.NeXImage");
	}

	



	// if environment map has been compiled already and load it from file 
	if (std::filesystem::exists("pbr_convolutedEnvMap.NeXImage"))
	{
		StoreImage readImage;
		StoreImage::load(&readImage, "pbr_convolutedEnvMap.NeXImage");

		const TextureData data = {
			TextureFilter::Linear,
			TextureFilter::Linear,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			ColorSpace::RGB,
			PixelDataType::FLOAT,
			InternFormat::RGB32F,
			true };

		convolutedEnvironmentMap = (nex::CubeMap*)(Texture::createFromImage(readImage, data, true));
	}
	else
	{
		convolutedEnvironmentMap = convolute(environmentMap);
		StoreImage enviromentMapImage = readConvolutedEnvMapPixelData();
		StoreImage::write(enviromentMapImage, "pbr_convolutedEnvMap.NeXImage");
	}

	renderBackend->setViewPort(backup.x, backup.y, backup.width, backup.height);


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
		StoreImage readImage;
		StoreImage::load(&readImage, "brdfLUT.NeXImage");

		TextureData data = {
		TextureFilter::Linear,
		TextureFilter::Linear,
		TextureUVTechnique::ClampToEdge,
		TextureUVTechnique::ClampToEdge,
		TextureUVTechnique::ClampToEdge,
		ColorSpace::RG,
		PixelDataType::FLOAT,
		InternFormat::RG32F,
		false };

		brdfLookupTexture = (Texture2D*)Texture::createFromImage(readImage, data, false);
	}
	else
	{
		brdfLookupTexture = createBRDFlookupTexture();
		StoreImage brdfLUTImage = readBrdfLookupPixelData();
		StoreImage::write(brdfLUTImage, "brdfLUT.NeXImage");
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