#include <nex/pbr/PbrProbe.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/RenderBackend.hpp>
#include "nex/shader/PBRShader.hpp"
#include "nex/EffectLibrary.hpp"
#include "nex/texture/Attachment.hpp"
#include "nex/drawing/StaticMeshDrawer.hpp"
#include "nex/light/Light.hpp"
#include <nex/texture/Sprite.hpp>

using namespace glm;
using namespace nex;

PbrProbe::PbrProbe(Texture* backgroundHDR) :
	environmentMap(nullptr), skybox("misc/SkyBoxCube.obj", MaterialType::None),
	mConvolutionPass(std::make_unique<PBR_ConvolutionShader>()),
	mPrefilterPass(std::make_unique<PBR_PrefilterShader>()),
	mBrdfPrecomputePass(std::make_unique<PBR_BrdfPrecomputeShader>())
{

	skybox.init();

	init(backgroundHDR);
	environmentMap.reset();
}

/*void PbrProbe::drawSky(const mat4& projection, const mat4& view)
{
	static auto* skyboxShader = RenderBackend::get()->getEffectLibrary()->getSkyBoxShader();

	mat4 skyBoxView = mat4(mat3(view));

	skyboxShader->bind();
	skyboxShader->setupRenderState();
	
	skyboxShader->setView(skyBoxView);
	skyboxShader->setProjection(projection);
	skyboxShader->setMVP(projection * skyBoxView);
	//skyboxShader->setSkyTexture(prefilterRenderTarget->getCubeMap());
	skyboxShader->setSkyTexture(getEnvironmentMap());

	StaticMeshDrawer::draw(skybox.getModel(), skyboxShader);

	//TODO optimize out
	skyboxShader->reverseRenderState();
}*/

CubeMap * PbrProbe::getConvolutedEnvironmentMap() const
{
	return convolutedEnvironmentMap.get();
}

CubeMap* PbrProbe::getEnvironmentMap() const
{
	return  environmentMap.get();
}

CubeMap * PbrProbe::getPrefilteredEnvironmentMap() const
{
	return  prefilteredEnvMap.get();
}

Texture2D * PbrProbe::getBrdfLookupTexture() const
{
	return brdfLookupTexture.get();
}

StoreImage PbrProbe::readBrdfLookupPixelData() const
{
	StoreImage store;
	StoreImage::create(&store, 1, 1, false);

	GenericImage& data = store.images[0][0];
	data.width = getBrdfLookupTexture()->getWidth();
	data.height = getBrdfLookupTexture()->getHeight();
	data.components = 2; // RG
	data.format = (unsigned)ColorSpace::RG;
	data.pixelSize = sizeof(float) * data.components;

	auto bufSize = data.width * data.height * data.pixelSize;
	data.pixels.resize(bufSize);

	// read the data back from the gpu
	brdfLookupTexture->readback(
		0, ColorSpace::RG, 
		PixelDataType::FLOAT, 
		data.pixels.data(),
		bufSize);

	return store;
}

StoreImage PbrProbe::readBackgroundPixelData() const
{
	StoreImage store;
	StoreImage::create(&store, 6, 1, true); // 6 sides, no mipmaps (only base level)

	// readback the cubemap
	const auto components = 3;
	const auto format = (unsigned)ColorSpace::RGB;
	const auto pixelDataSize = sizeof(float) * components; // RGB32F
	const auto width = getEnvironmentMap()->getSideWidth();
	const auto height = getEnvironmentMap()->getSideHeight();
	const auto sideSlice = width * height * pixelDataSize;
	std::vector<char> pixels(sideSlice * 6);
	environmentMap->readback(
		0, // base level
		ColorSpace::RGB,
		PixelDataType::FLOAT,
		pixels.data(),
		pixels.size());

	

	for (unsigned i = 0; i < store.images.size(); ++i)
	{
		GenericImage& data = store.images[i][0];
		data.width = width;
		data.height = height;
		data.components = components; // RGB
		data.format = format;
		data.pixelSize = pixelDataSize;
		auto bufSize = sideSlice;
		data.pixels.resize(bufSize);

		memcpy_s(data.pixels.data(), bufSize, pixels.data() + i * sideSlice, sideSlice);
	}

	return store;
}

StoreImage PbrProbe::readConvolutedEnvMapPixelData()
{
	StoreImage store;
	// note, that the convoluted environment map has no generated mip maps!
	StoreImage::create(&store, 6, 1, true); // 6 sides, 32/16/8/4/2/1 = 6 levels

	for (auto level = 0; level < store.mipmapCount; ++level)
	{
		// readback the mipmap level of the cubemap
		const auto components = 3;  // RGB
		const auto format = (unsigned)ColorSpace::RGB;
		const auto pixelDataSize = sizeof(float) * components; // internal format: RGB16F
		unsigned mipMapDivisor = std::pow(2, level);
		const auto width = getConvolutedEnvironmentMap()->getSideWidth() / mipMapDivisor;
		const auto height = getConvolutedEnvironmentMap()->getSideHeight() / mipMapDivisor;
		const auto sideSlice = width * height * pixelDataSize;
		std::vector<char> pixels(sideSlice * 6);

		convolutedEnvironmentMap->readback(
			level, // mipmap level
			ColorSpace::RGB,
			PixelDataType::FLOAT,
			pixels.data(),
			pixels.size());

		for (unsigned side = 0; side < store.images.size(); ++side)
		{
				GenericImage& data = store.images[side][level];
				data.width = width;
				data.height = height;
				data.components = components;
				data.format = format;
				data.pixelSize = pixelDataSize;
				auto bufSize = sideSlice;
				data.pixels.resize(bufSize);

				memcpy_s(data.pixels.data(), bufSize, pixels.data() + side * sideSlice, sideSlice);
		}
	}

	return store;
}

StoreImage PbrProbe::readPrefilteredEnvMapPixelData()
{
	StoreImage store;
	auto size = min<unsigned>(prefilteredEnvMap->getSideWidth(), prefilteredEnvMap->getSideHeight());

	// Note: we produced only 5 mip map levels instead of possible 9 (for 256 width/height)
	StoreImage::create(&store, 6, 5, true);

	for (auto level = 0; level < store.mipmapCount; ++level)
	{
		// readback the mipmap level of the cubemap
		const auto components = 3;  // RGB
		const auto format = (unsigned)ColorSpace::RGB;
		const auto pixelDataSize = sizeof(float) * components; // internal format: RGB16F
		unsigned mipMapDivisor = std::pow(2, level);
		const auto width = getPrefilteredEnvironmentMap()->getSideWidth() / mipMapDivisor;
		const auto height = getPrefilteredEnvironmentMap()->getSideHeight() / mipMapDivisor;
		const auto sideSlice = width * height * pixelDataSize;
		std::vector<char> pixels(sideSlice * 6);

		prefilteredEnvMap->readback(
			level, // mipmap level
			ColorSpace::RGB,
			PixelDataType::FLOAT,
			pixels.data(),
			pixels.size());

		for (unsigned side = 0; side < store.images.size(); ++side)
		{
			GenericImage& data = store.images[side][level];
			data.width = width;
			data.height = height;
			data.components = components;
			data.format = format;
			data.pixelSize = pixelDataSize;
			auto bufSize = sideSlice;
			data.pixels.resize(bufSize);

			memcpy_s(data.pixels.data(), bufSize, pixels.data() + side * sideSlice, sideSlice);
		}
	}

	return store;
}

std::shared_ptr<CubeMap> PbrProbe::renderBackgroundToCube(Texture* background)
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
	RenderAttachment depth;
	depth.target = TextureTarget::TEXTURE2D;
	depth.texture = std::make_unique<RenderBuffer>(2048, 2048, InternFormat::DEPTH24);
	depth.type = RenderAttachmentType::DEPTH;
	cubeRenderTarget->useDepthAttachment(depth);
	cubeRenderTarget->updateDepthAttachment();

	static auto* renderBackend = RenderBackend::get();
	static auto* effectLib = renderBackend->getEffectLibrary();
	static auto* shader = effectLib->getEquirectangularSkyBoxShader();

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

	
	


	/*TextureGL* gl = (TextureGL*)cubeRenderTarget->getTexture()->getImpl();
	const GLuint textureID = *gl->getTexture();
	std::stringstream ss;
	ss << "Before rendering background cube maps; cubemap texture id = " << textureID;*/


	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0x100000, GL_DEBUG_SEVERITY_NOTIFICATION, 0, "This is a test!");
	cubeRenderTarget->bind();
	renderBackend->setViewPort(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());
	//renderBackend->setScissor(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());


	for (unsigned int side = 0; side < 6; ++side) {
		shader->setView(views[side]);
		cubeRenderTarget->useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X));
		StaticMeshDrawer::draw(skybox.getModel(), shader);
	}


	auto& resultAttachment = cubeRenderTarget->getColorAttachments()[0];
	auto result = std::dynamic_pointer_cast<CubeMap>(resultAttachment.texture);

	// now create mipmaps for the cubemap fighting render artificats in the prefilter map
	result->generateMipMaps();

	//cubeRenderTarget->setRenderResult(nullptr);


	//CubeMap* result = cubeRenderTarget->createCopy(); 
	return result;
}

std::shared_ptr<CubeMap> PbrProbe::convolute(CubeMap * source)
{
	static auto* renderBackend = RenderBackend::get();
	
	// uses RGB and 32bit per component (floats)

	const TextureData& data = {
				TextureFilter::Linear,
				TextureFilter::Linear,
				TextureUVTechnique::ClampToEdge,
				TextureUVTechnique::ClampToEdge,
				TextureUVTechnique::ClampToEdge,
				ColorSpace::RGB,
				PixelDataType::FLOAT,
				InternFormat::RGB32F,
				false };
	auto cubeRenderTarget = renderBackend->createCubeRenderTarget(32, 32, data);

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

	mConvolutionPass->bind();
	mConvolutionPass->setProjection(projection);
	mConvolutionPass->setEnvironmentMap(source);


	//view matrices;
	const mat4 views[] = {
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_X), //right; sign of up vector is not important
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_X), //left
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_Y), //top
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_Y), //bottom
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_Z), //back
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_Z) //front
	};

	cubeRenderTarget->bind();
	renderBackend->setViewPort(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());
	//renderBackend->setScissor(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());

	for (int side = 0; side < 6; ++side) {
		mConvolutionPass->setView(views[side]);
		cubeRenderTarget->useSide(static_cast<CubeMapSide>(side));
		StaticMeshDrawer::draw(skybox.getModel(), mConvolutionPass.get());
	}

	//CubeMap* result = cubeRenderTarget->createCopy();
	//renderer->destroyCubeRenderTarget(cubeRenderTarget);

	//CubeMap* result = (CubeMap*)cubeRenderTarget->setRenderResult(nullptr);
	return std::dynamic_pointer_cast<CubeMap>(cubeRenderTarget->getColorAttachments()[0].texture);
}

std::shared_ptr<CubeMap> PbrProbe::prefilter(CubeMap * source)
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

	auto prefilterRenderTarget = renderBackend->createCubeRenderTarget(128, 128, textureData);

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);


	mPrefilterPass->bind();
	mPrefilterPass->setProjection(projection);
	mPrefilterPass->setMapToPrefilter(source);

	//view matrices;
	const mat4 views[] = {
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_X), //right; sign of up vector is not important
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_X), //left
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_Y), //top
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_Y), //bottom
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_Z), //back
		CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_Z) //front
	};


	prefilterRenderTarget->bind();
	unsigned int maxMipLevels = 5;

	for (unsigned int mipLevel = 0; mipLevel < maxMipLevels; ++mipLevel) {

		// update the roughness value for the current mipmap level
		float roughness = (float)mipLevel / (float)(maxMipLevels - 1);
		mPrefilterPass->setRoughness(roughness);

		//resize render target according to mip level size
		prefilterRenderTarget->resizeForMipMap(mipLevel);

		// update the viewport size
		unsigned int width = prefilterRenderTarget->getWidthMipLevel(mipLevel);
		unsigned int height = prefilterRenderTarget->getHeightMipLevel(mipLevel);
		renderBackend->setViewPort(0, 0, width, height);
		//renderBackend->setScissor(0, 0, width, height);

		// render to the cubemap at the specified mip level
		for (unsigned int side = 0; side < 6; ++side) {
			mPrefilterPass->setView(views[side]);
			prefilterRenderTarget->useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X), mipLevel);
			StaticMeshDrawer::draw(skybox.getModel(), mPrefilterPass.get());
		}
	}


	//CubeMap* result = (CubeMap*)prefilterRenderTarget->setRenderResult(nullptr);
	auto result =  std::dynamic_pointer_cast<CubeMap>(prefilterRenderTarget->getColorAttachments()[0].texture);
	//result->generateMipMaps();

	return result;
}

std::shared_ptr<Texture2D> PbrProbe::createBRDFlookupTexture()
{
	TextureData data = {
		TextureFilter::Linear, 
		TextureFilter::Linear, 
		TextureUVTechnique::ClampToEdge, 
		TextureUVTechnique::ClampToEdge,
		TextureUVTechnique::ClampToEdge,
		ColorSpace::RG, 
		PixelDataType::FLOAT, 
		InternFormat::RG16F,
		false};

	TextureData depthData = TextureData::createDepth(CompareFunction::LESS_EQUAL,
		ColorSpace::DEPTH_STENCIL,
		PixelDataType::UNSIGNED_INT_24_8,
		InternFormat::DEPTH24_STENCIL8);

	static auto* renderBackend = RenderBackend::get();

	//auto target = renderBackend->create2DRenderTarget(1024, 1024, data, depthData, 1);
	auto target = std::make_unique<RenderTarget2D>(512, 512, data);

	target->bind();
	renderBackend->setViewPort(0, 0, 512, 512);
	//renderBackend->setScissor(0, 0, 512, 512);
	target->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);

	mBrdfPrecomputePass->bind();
	StaticMeshDrawer::draw(Sprite::getScreenSprite(), mBrdfPrecomputePass.get());

	//Texture2D* result = (Texture2D*)target->setRenderResult(nullptr);
	auto result = std::dynamic_pointer_cast<Texture2D>(target->getColorAttachments()[0].texture);

	target->getColorAttachments()[0].texture = nullptr;
	target->updateColorAttachment(0);
	return result;
}

void PbrProbe::init(Texture* backgroundHDR)
{
	static auto* renderBackend = RenderBackend::get();

	Viewport backup = renderBackend->getViewport();
	renderBackend->getRasterizer()->enableScissorTest(false);

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
		environmentMap.reset((CubeMap*)Texture::createFromImage(readImage, data, true));
	} else
	{
		environmentMap = renderBackgroundToCube(backgroundHDR);
		StoreImage enviromentMapImage = readBackgroundPixelData();
		StoreImage::write(enviromentMapImage, "pbr_environmentMap.NeXImage");
	}


	if (std::filesystem::exists("pbr_prefilteredEnvMap.NeXImage") && true)
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
			InternFormat::RGB16F,
			true
		};
		prefilteredEnvMap.reset((CubeMap*)Texture::createFromImage(readImage, data, true));
	}
	else
	{
		prefilteredEnvMap = prefilter(getEnvironmentMap());
		StoreImage enviromentMapImage = readPrefilteredEnvMapPixelData();
		StoreImage::write(enviromentMapImage, "pbr_prefilteredEnvMap.NeXImage");
	}

	



	// if environment map has been compiled already and load it from file 
	if (std::filesystem::exists("pbr_convolutedEnvMap.NeXImage") && true)
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
			InternFormat::RGB16F,
			false };

		convolutedEnvironmentMap.reset((CubeMap*)Texture::createFromImage(readImage, data, true));
	}
	else
	{
		convolutedEnvironmentMap = convolute(getEnvironmentMap());
		StoreImage enviromentMapImage = readConvolutedEnvMapPixelData();
		StoreImage::write(enviromentMapImage, "pbr_convolutedEnvMap.NeXImage");
	}

	renderBackend->setViewPort(backup.x, backup.y, backup.width, backup.height);
	//renderBackend->setScissor(backup.x, backup.y, backup.width, backup.height);


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
		InternFormat::RG16F,
		false };

		brdfLookupTexture.reset((Texture2D*)Texture::createFromImage(readImage, data, false));
	}
	else
	{
		brdfLookupTexture = createBRDFlookupTexture();
		StoreImage brdfLUTImage = readBrdfLookupPixelData();
		StoreImage::write(brdfLUTImage, "brdfLUT.NeXImage");
	}
}