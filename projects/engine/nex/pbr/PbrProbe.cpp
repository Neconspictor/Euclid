#include <nex/pbr/PbrProbe.hpp>
#include <nex/shader/SkyBoxpass.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/texture/Texture.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include "nex/pbr/PbrPass.hpp"
#include "nex/EffectLibrary.hpp"
#include "nex/texture/Attachment.hpp"
#include "nex/drawing/StaticMeshDrawer.hpp"
#include "nex/light/Light.hpp"
#include <nex/texture/Sprite.hpp>
#include "nex/mesh/SampleMeshes.hpp"
#include <nex/material/Material.hpp>
#include "nex/mesh/StaticMeshManager.hpp"
#include "nex/resource/FileSystem.hpp"
#include "nex/shader/Technique.hpp"
#include "nex/mesh/Sphere.hpp"
#include "nex/mesh/MeshFactory.hpp"
#include <nex/resource/ResourceLoader.hpp>
#include <nex/resource/Resource.hpp>

using namespace glm;
using namespace nex;

std::shared_ptr<Texture2D> PbrProbe::mBrdfLookupTexture = nullptr;
std::unique_ptr<PbrProbe::ProbeTechnique> PbrProbe::mTechnique = nullptr;
std::unique_ptr<SphereMesh> PbrProbe::mMesh = nullptr;
std::unique_ptr<Sampler> PbrProbe::mSamplerIrradiance = nullptr;
std::unique_ptr<Sampler> PbrProbe::mSamplerPrefiltered = nullptr;

std::unique_ptr<FileSystem> nex::PbrProbeFactory::mFileSystem;

const nex::TextureData nex::PbrProbe::BRDF_DATA = {
			TextureFilter::Linear,
			TextureFilter::Linear,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			ColorSpace::RG,
			PixelDataType::FLOAT,
			InternFormat::RG32F,
			false
};


const nex::TextureData nex::PbrProbe::IRRADIANCE_DATA = {
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

const nex::TextureData nex::PbrProbe::PREFILTERED_DATA = {
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

const nex::TextureData nex::PbrProbe::SOURCE_DATA = {
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

nex::PbrProbeFactory::PbrProbeFactory(unsigned prefilteredSide, unsigned mapSize) : 
	mPrefilteredSide(prefilteredSide), mMapSize(mapSize), mFreeSlots(mapSize)
{
	mIrraddianceMaps = std::make_unique<CubeMapArray>(PbrProbe::IRRADIANCE_SIZE, PbrProbe::IRRADIANCE_SIZE, mapSize, PbrProbe::IRRADIANCE_DATA, nullptr);
	mPrefilteredMaps = std::make_unique<CubeMapArray>(mPrefilteredSide, mPrefilteredSide, mapSize, PbrProbe::PREFILTERED_DATA, nullptr);
}

void nex::PbrProbeFactory::init(const std::filesystem::path & probeCompiledDirectory, std::string probeFileExtension)
{
	std::vector<std::filesystem::path> includes = { probeCompiledDirectory };
	mFileSystem = std::make_unique<FileSystem>(std::move(includes), probeCompiledDirectory, probeFileExtension);
	PbrProbe::initGlobals(mFileSystem->getFirstIncludeDirectory());
}

class nex::PbrProbe::ProbeTechnique : public nex::Technique {
public:

	class ProbePass : public TransformPass
	{
	public:

		ProbePass() : TransformPass(Shader::create("pbr/pbr_probeVisualization_vs.glsl", "pbr/pbr_probeVisualization_fs.glsl"))
		{
			mIrradianceMap = { mShader->getUniformLocation("irradianceMap"), UniformType::CUBE_MAP };
			mPrefilterMap = { mShader->getUniformLocation("prefilterMap"), UniformType::CUBE_MAP };

			SamplerDesc desc;
			//desc.minLOD = 0;
			//desc.maxLOD = 7;
			desc.minFilter = TextureFilter::Linear_Mipmap_Linear;
			mPrefilterSampler.setState(desc);
		}

		void setIrradianceMap(CubeMap* map) {
			mShader->setTexture(map, &mSampler, 0);
		}
		void setPrefilteredMap(CubeMap* map) {
			mShader->setTexture(map, &mPrefilterSampler, 1);
		}

		Uniform mIrradianceMap;
		Uniform mPrefilterMap;
		Sampler mSampler;
		Sampler mPrefilterSampler;
	};


	ProbeTechnique() : Technique(nullptr)
	{
		setSelected(&mProbePass);
	}

	ProbePass mProbePass;
};

class nex::PbrProbe::ProbeMaterial : public nex::Material {
public:
	ProbeMaterial(ProbeTechnique* technique) : Material(technique), mProbeTechnique(technique)
	{
		assert(technique != nullptr);
	}

	void setIrradianceMap(CubeMap* map)
	{
		irradianceMap = map;
	}

	void setPrefilterMap(CubeMap* map)
	{
		prefilteredMap = map;
	}

	void upload() override {
		mProbeTechnique->mProbePass.setIrradianceMap(irradianceMap);
		mProbeTechnique->mProbePass.setPrefilteredMap(prefilteredMap);
	}

	ProbeTechnique* mProbeTechnique;
	CubeMap* irradianceMap;
	CubeMap* prefilteredMap;
};

void nex::PbrProbeFactory::initProbe(PbrProbe * probe, Texture * backgroundHDR, unsigned storeID)
{
	probe->init(backgroundHDR,
		mPrefilteredSide,
		storeID,
		mFileSystem->getFirstIncludeDirectory());
}

PbrProbe::PbrProbe() :
	environmentMap(nullptr),
	mMaterial(std::make_unique<ProbeMaterial>(mTechnique.get()))
{
}

PbrProbe::~PbrProbe() = default;

void nex::PbrProbe::createHandles()
{
	mHandles.convoluted = convolutedEnvironmentMap->getHandleWithSampler(*mSamplerIrradiance);
	mHandles.prefiltered = prefilteredEnvMap->getHandleWithSampler(*mSamplerPrefiltered);
}

void nex::PbrProbe::activateHandles()
{
	convolutedEnvironmentMap->residentHandle(mHandles.convoluted);
	prefilteredEnvMap->residentHandle(mHandles.prefiltered);
}

void nex::PbrProbe::deactivateHandles()
{
	convolutedEnvironmentMap->makeHandleNonResident(mHandles.convoluted);
	prefilteredEnvMap->makeHandleNonResident(mHandles.prefiltered);
}

void PbrProbe::initGlobals(const std::filesystem::path& probeRoot)
{

	Viewport backup = RenderBackend::get()->getViewport();

	mTechnique = std::make_unique<ProbeTechnique>();
	mMesh = std::make_unique<SphereMesh>(16, 16);

	PbrBrdfPrecomputePass brdfPrecomputePass;
	const std::filesystem::path brdfMapPath = probeRoot / ("brdfLUT.probe");

	// setup sprite for brdf integration lookup texture
	const vec2 dim = { 1.0, 1.0 };
	const vec2 pos = { 0.5f * (1.0f - dim.x), 0.5f * (1.0f - dim.y) };

	Sprite brdfSprite;
	brdfSprite.setPosition(pos);
	brdfSprite.setWidth(dim.x);
	brdfSprite.setHeight(dim.y);

	// we don't need a texture
	// we use the sprite only as a polygon model
	brdfSprite.setTexture(nullptr);

	if (std::filesystem::exists(brdfMapPath))
	{
		StoreImage readImage;
		try
		{
			FileSystem::load(brdfMapPath, readImage);
		}
		catch (const std::exception&e)
		{
		}

		TextureData data = BRDF_DATA;

		data.minLOD = 0;
		data.maxLOD = readImage.mipmapCount - 1;
		data.lodBaseLevel = 0;
		data.lodMaxLevel = readImage.mipmapCount - 1;

		mBrdfLookupTexture.reset((Texture2D*)Texture::createFromImage(readImage, data));
		const StoreImage brdfLUTImage = readBrdfLookupPixelData();
	}
	else
	{
		mBrdfLookupTexture = createBRDFlookupTexture(&brdfPrecomputePass);
		const StoreImage brdfLUTImage = readBrdfLookupPixelData();
		FileSystem::store(brdfMapPath, brdfLUTImage);
	}

	mSamplerIrradiance = std::make_unique<Sampler>();

	SamplerDesc desc;
	//desc.minLOD = 0;
	//desc.maxLOD = 7;
	desc.minFilter = TextureFilter::Linear_Mipmap_Linear;
	mSamplerPrefiltered = std::make_unique<Sampler>(desc);

	RenderBackend::get()->setViewPort(backup.x, backup.y, backup.width, backup.height);

}

Mesh* PbrProbe::getSphere()
{
	return mMesh.get();
}

Material* PbrProbe::getMaterial()
{
	return mMaterial.get();
}

CubeMap * PbrProbe::getConvolutedEnvironmentMap() const
{
	return convolutedEnvironmentMap.get();
}

CubeMap* PbrProbe::getEnvironmentMap() const
{
	return  environmentMap.get();
}

const nex::PbrProbe::Handles * nex::PbrProbe::getHandles() const
{
	return &mHandles;
}

CubeMap * PbrProbe::getPrefilteredEnvironmentMap() const
{
	return  prefilteredEnvMap.get();
}

Texture2D * PbrProbe::getBrdfLookupTexture()
{
	return mBrdfLookupTexture.get();
}

StoreImage PbrProbe::readBrdfLookupPixelData()
{
	StoreImage store;
	StoreImage::create(&store, 1, 1, TextureTarget::TEXTURE2D);

	GenericImage& data = store.images[0][0];
	data.width = mBrdfLookupTexture->getWidth();
	data.height = mBrdfLookupTexture->getHeight();
	data.channels = getComponents(BRDF_DATA.colorspace);
	data.format = (unsigned)BRDF_DATA.colorspace;
	data.pixelSize = sizeof(float) * data.channels;

	auto bufSize = data.width * data.height * data.pixelSize;
	data.pixels = std::vector<char>(bufSize);;

	// read the data back from the gpu
	mBrdfLookupTexture->readback(
		0, BRDF_DATA.colorspace,
		BRDF_DATA.pixelDataType,
		data.pixels.getPixels(),
		bufSize);

	return store;
}

StoreImage PbrProbe::readBackgroundPixelData() const
{
	StoreImage store;
	StoreImage::create(&store, 6, 1, TextureTarget::CUBE_MAP); // 6 sides, no mipmaps (only base level)

	// readback the cubemap
	const auto components = getComponents(SOURCE_DATA.colorspace);
	const auto format = (unsigned)SOURCE_DATA.colorspace;
	const auto pixelDataSize = sizeof(float) * components; // RGB32F
	const auto width = getEnvironmentMap()->getWidth();
	const auto height = getEnvironmentMap()->getHeight();
	const auto sideSlice = width * height * pixelDataSize;
	std::vector<char> pixels(sideSlice * 6);
	environmentMap->readback(
		0, // base level
		SOURCE_DATA.colorspace,
		SOURCE_DATA.pixelDataType,
		pixels.data(),
		pixels.size());



	for (unsigned i = 0; i < store.images.size(); ++i)
	{
		GenericImage& data = store.images[i][0];
		data.width = width;
		data.height = height;
		data.channels = components; // RGB
		data.format = format;
		data.pixelSize = pixelDataSize;
		auto bufSize = sideSlice;
		data.pixels = std::vector<char>(bufSize);

		memcpy_s(data.pixels.getPixels(), bufSize, pixels.data() + i * sideSlice, sideSlice);
	}

	return store;
}

StoreImage PbrProbe::readConvolutedEnvMapPixelData()
{
	StoreImage store;
	// note, that the convoluted environment map has no generated mip maps!
	StoreImage::create(&store, 6, 1, TextureTarget::CUBE_MAP); // 6 sides, 32/16/8/4/2/1 = 6 levels

	for (auto level = 0; level < store.mipmapCount; ++level)
	{
		// readback the mipmap level of the cubemap
		const auto components = getComponents(IRRADIANCE_DATA.colorspace);  // RGB
		const auto format = (unsigned)IRRADIANCE_DATA.colorspace;
		const auto pixelDataSize = sizeof(float) * components; // internal format: RGB16F
		unsigned mipMapDivisor = std::pow(2, level);
		const auto width = getConvolutedEnvironmentMap()->getWidth() / mipMapDivisor;
		const auto height = getConvolutedEnvironmentMap()->getHeight() / mipMapDivisor;
		const auto sideSlice = width * height * pixelDataSize;
		std::vector<char> pixels(sideSlice * 6);

		convolutedEnvironmentMap->readback(
			level, // mipmap level
			IRRADIANCE_DATA.colorspace,
			IRRADIANCE_DATA.pixelDataType,
			pixels.data(),
			pixels.size());

		for (unsigned side = 0; side < store.images.size(); ++side)
		{
			GenericImage& data = store.images[side][level];
			data.width = width;
			data.height = height;
			data.channels = components;
			data.format = format;
			data.pixelSize = pixelDataSize;
			auto bufSize = sideSlice;
			data.pixels = std::vector<char>(bufSize);

			memcpy_s(data.pixels.getPixels(), bufSize, pixels.data() + side * sideSlice, sideSlice);
		}
	}

	return store;
}

StoreImage PbrProbe::readPrefilteredEnvMapPixelData()
{
	StoreImage store;
	const auto mipMapLevelZero = min<unsigned>(prefilteredEnvMap->getWidth(), prefilteredEnvMap->getHeight());
	const auto mipMapCount = prefilteredEnvMap->getMipMapCount(mipMapLevelZero);

	StoreImage::create(&store, 6, mipMapCount, TextureTarget::CUBE_MAP);

	for (auto level = 0; level < store.mipmapCount; ++level)
	{
		// readback the mipmap level of the cubemap
		const auto components = getComponents(PREFILTERED_DATA.colorspace);  // RGB
		const auto format = (unsigned)PREFILTERED_DATA.colorspace;
		const auto pixelDataSize = sizeof(float) * components; // internal format: RGB16F
		unsigned mipMapDivisor = std::pow(2, level);
		const auto width = getPrefilteredEnvironmentMap()->getWidth() / mipMapDivisor;
		const auto height = getPrefilteredEnvironmentMap()->getHeight() / mipMapDivisor;
		const auto sideSlice = width * height * pixelDataSize;
		std::vector<char> pixels(sideSlice * 6);

		prefilteredEnvMap->readback(
			level, // mipmap level
			PREFILTERED_DATA.colorspace,
			PREFILTERED_DATA.pixelDataType,
			pixels.data(),
			pixels.size());

		for (unsigned side = 0; side < store.images.size(); ++side)
		{
			GenericImage& data = store.images[side][level];
			data.width = width;
			data.height = height;
			data.channels = components;
			data.format = format;
			data.pixelSize = pixelDataSize;
			auto bufSize = sideSlice;
			data.pixels = std::vector<char>(bufSize);

			memcpy_s(data.pixels.getPixels(), bufSize, pixels.data() + side * sideSlice, sideSlice);
		}
	}

	return store;
}

std::shared_ptr<CubeMap> PbrProbe::renderBackgroundToCube(Texture* background)
{
	auto cubeRenderTarget = std::make_unique<CubeRenderTarget>(SOURCE_CUBE_SIZE, SOURCE_CUBE_SIZE, SOURCE_DATA);
	RenderAttachment depth;
	depth.target = TextureTarget::TEXTURE2D;
	TextureData data;
	data.colorspace = ColorSpace::DEPTH;
	data.internalFormat = InternFormat::DEPTH24;
	depth.texture = std::make_unique<RenderBuffer>(SOURCE_CUBE_SIZE, SOURCE_CUBE_SIZE, data);
	depth.type = RenderAttachmentType::DEPTH;
	cubeRenderTarget->useDepthAttachment(depth);
	cubeRenderTarget->updateDepthAttachment();

	thread_local auto* renderBackend = RenderBackend::get();
	thread_local auto* effectLib = renderBackend->getEffectLibrary();
	thread_local auto* shader = effectLib->getEquirectangularSkyBoxShader();

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

	shader->bind();
	shader->setProjection(projection);
	shader->setSkyTexture(background);

	//view matrices;
	const auto& views = CubeMap::getViewLookAts();

	cubeRenderTarget->bind();
	renderBackend->setViewPort(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());

	auto skyBox = createSkyBox();

	for (unsigned int side = 0; side < views.size(); ++side) {
		shader->setView(views[side]);
		cubeRenderTarget->useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X));
		StaticMeshDrawer::draw(skyBox.get(), shader);
	}


	auto& resultAttachment = cubeRenderTarget->getColorAttachments()[0];
	auto result = std::dynamic_pointer_cast<CubeMap>(resultAttachment.texture);

	// now create mipmaps for the cubemap fighting render artificats in the prefilter map
	result->generateMipMaps();
	return result;
}

std::shared_ptr<CubeMap> PbrProbe::convolute(CubeMap * source)
{
	thread_local auto* renderBackend = RenderBackend::get();

	auto cubeRenderTarget = renderBackend->createCubeRenderTarget(IRRADIANCE_SIZE, IRRADIANCE_SIZE, 
		IRRADIANCE_DATA);

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

	auto mConvolutionPass(std::make_unique<PbrConvolutionPass>());
	mConvolutionPass->bind();
	mConvolutionPass->setProjection(projection);
	mConvolutionPass->setEnvironmentMap(source);

	cubeRenderTarget->bind();
	renderBackend->setViewPort(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());

	auto skyBox = createSkyBox();

	const auto& views = CubeMap::getViewLookAts();

	for (int side = 0; side < views.size(); ++side) {
		mConvolutionPass->setView(views[side]);
		cubeRenderTarget->useSide(static_cast<CubeMapSide>(side));
		StaticMeshDrawer::draw(skyBox.get(), mConvolutionPass.get());
	}

	return std::dynamic_pointer_cast<CubeMap>(cubeRenderTarget->getColorAttachments()[0].texture);
}

std::shared_ptr<CubeMap> PbrProbe::prefilter(CubeMap * source, unsigned prefilteredSize)
{
	auto* renderBackend = RenderBackend::get();
	auto skyBox = createSkyBox();


	auto prefilterRenderTarget = renderBackend->createCubeRenderTarget(prefilteredSize, prefilteredSize, PREFILTERED_DATA);

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);


	auto mPrefilterPass(std::make_unique<PbrPrefilterPass>());
	mPrefilterPass->bind();
	mPrefilterPass->setProjection(projection);
	mPrefilterPass->setMapToPrefilter(source);

	//view matrices;
	const mat4 views[] = {
		CubeMap::getViewLookAtMatrix(CubeMapSide::POSITIVE_X), //right; sign of up vector is not important
		CubeMap::getViewLookAtMatrix(CubeMapSide::NEGATIVE_X), //left
		CubeMap::getViewLookAtMatrix(CubeMapSide::POSITIVE_Y), //top
		CubeMap::getViewLookAtMatrix(CubeMapSide::NEGATIVE_Y), //bottom
		CubeMap::getViewLookAtMatrix(CubeMapSide::POSITIVE_Z), //back
		CubeMap::getViewLookAtMatrix(CubeMapSide::NEGATIVE_Z) //front
	};

	prefilterRenderTarget->bind();
	const auto mipMapLevelZero = min<unsigned>(prefilterRenderTarget->getWidth(), prefilterRenderTarget->getHeight());
	const auto mipMapCount = Texture::getMipMapCount(mipMapLevelZero);

	for (unsigned int mipLevel = 0; mipLevel < mipMapCount; ++mipLevel) {

		// update the roughness value for the current mipmap level
		float roughness = (float)mipLevel / (float)(mipMapCount - 1);
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
			StaticMeshDrawer::draw(skyBox.get(), mPrefilterPass.get());
		}
	}


	//CubeMap* result = (CubeMap*)prefilterRenderTarget->setRenderResult(nullptr);
	auto result = std::dynamic_pointer_cast<CubeMap>(prefilterRenderTarget->getColorAttachments()[0].texture);
	//result->generateMipMaps();

	return result;
}


std::unique_ptr<StaticMeshContainer> nex::PbrProbe::createSkyBox()
{
	int vertexCount = (int)sizeof(sample_meshes::skyBoxVertices);
	int indexCount = (int)sizeof(sample_meshes::skyBoxIndices);

	AABB boundingBox;
	boundingBox.min = glm::vec3(0.0f);
	boundingBox.max = glm::vec3(0.0f);

	std::unique_ptr<Mesh> mesh = MeshFactory::createPosition((const VertexPosition*)sample_meshes::skyBoxVertices, vertexCount,
		sample_meshes::skyBoxIndices, (int)indexCount, std::move(boundingBox));

	auto model = std::make_unique<StaticMeshContainer>();
	model->add(std::move(mesh), std::make_unique<Material>(nullptr));
	model->finalize();

	return model;
}

std::shared_ptr<Texture2D> PbrProbe::createBRDFlookupTexture(Pass* brdfPrecompute)
{
	thread_local auto* renderBackend = RenderBackend::get();

	auto target = std::make_unique<RenderTarget2D>(BRDF_SIZE, BRDF_SIZE, BRDF_DATA);

	target->bind();
	renderBackend->setViewPort(0, 0, BRDF_SIZE, BRDF_SIZE);
	target->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);

	brdfPrecompute->bind();
	RenderState state = RenderState::createNoDepthTest();
	StaticMeshDrawer::drawFullscreenTriangle(state, brdfPrecompute);

	auto result = std::dynamic_pointer_cast<Texture2D>(target->getColorAttachments()[0].texture);

	target->getColorAttachments()[0].texture = nullptr;
	target->updateColorAttachment(0);
	return result;
}

void PbrProbe::initBackground(Texture* backgroundHDR, const std::filesystem::path& probeRoot)
{
	thread_local auto* renderBackend = RenderBackend::get();

	Viewport backup = renderBackend->getViewport();
	renderBackend->getRasterizer()->enableScissorTest(false);

	// if environment map has been compiled already and load it from file 

	const std::filesystem::path environmentMapPath = probeRoot / ("pbr_environmentMap_" + std::to_string(mStoreID) + ".probe");

	if (std::filesystem::exists(environmentMapPath))
	{
		StoreImage readImage;
		try
		{
			FileSystem::load(environmentMapPath, readImage);
		}
		catch (const std::exception&e)
		{
		}

		TextureData data = SOURCE_DATA;
		data.minLOD = 0;
		data.maxLOD = readImage.mipmapCount - 1;
		data.lodBaseLevel = 0;
		data.lodMaxLevel = readImage.mipmapCount - 1;

		environmentMap.reset((CubeMap*)Texture::createFromImage(readImage, data));
	}
	else
	{
		environmentMap = renderBackgroundToCube(backgroundHDR);
		const StoreImage enviromentMapImage = readBackgroundPixelData();
		std::cout << "environmentMapPath = " << environmentMapPath << std::endl;
		FileSystem::store(environmentMapPath, enviromentMapImage);
	}
}

void PbrProbe::initPrefiltered(Texture* backgroundHDR, unsigned prefilteredSize, const std::filesystem::path& probeRoot)
{
	thread_local auto* renderBackend = RenderBackend::get();

	Viewport backup = renderBackend->getViewport();
	renderBackend->getRasterizer()->enableScissorTest(false);

	// if environment map has been compiled already and load it from file 

	const std::filesystem::path prefilteredMapPath = probeRoot / ("pbr_prefilteredEnvMap_" + std::to_string(mStoreID) + ".probe");

	if (std::filesystem::exists(prefilteredMapPath))
	{
		StoreImage readImage;
		try
		{
			FileSystem::load(prefilteredMapPath, readImage);
		}
		catch (const std::exception&e)
		{
		}


		TextureData data = PREFILTERED_DATA;

		data.generateMipMaps = false; // We set the mipmaps manually from store
		data.minLOD = 0;
		data.maxLOD = readImage.mipmapCount - 1;
		data.lodBaseLevel = 0;
		data.lodMaxLevel = readImage.mipmapCount - 1;

		prefilteredEnvMap.reset((CubeMap*)Texture::createFromImage(readImage, data));
	}
	else
	{
		prefilteredEnvMap = prefilter(getEnvironmentMap(), prefilteredSize);
		const StoreImage prefilteredMapImage = readPrefilteredEnvMapPixelData();
		FileSystem::store(prefilteredMapPath, prefilteredMapImage);
	}
}

void PbrProbe::initIrradiance(Texture* backgroundHDR, const std::filesystem::path& probeRoot)
{
	thread_local auto* renderBackend = RenderBackend::get();

	Viewport backup = renderBackend->getViewport();
	renderBackend->getRasterizer()->enableScissorTest(false);

	// if environment map has been compiled already and load it from file 
	const std::filesystem::path convolutedMapPath = probeRoot / ("pbr_convolutedEnvMap_" + std::to_string(mStoreID) + ".probe");


	// if environment map has been compiled already and load it from file 
	if (std::filesystem::exists(convolutedMapPath))
	{
		StoreImage readImage;
		try
		{
			FileSystem::load(convolutedMapPath, readImage);
		}
		catch (const std::exception&e)
		{
		}


		TextureData data = IRRADIANCE_DATA;

		data.minLOD = 0;
		data.maxLOD = readImage.mipmapCount - 1;
		data.lodBaseLevel = 0;
		data.lodMaxLevel = readImage.mipmapCount - 1;

		convolutedEnvironmentMap.reset((CubeMap*)Texture::createFromImage(readImage, data));
	}
	else
	{
		convolutedEnvironmentMap = convolute(getEnvironmentMap());
		const StoreImage convolutedMapImage = readConvolutedEnvMapPixelData();
		FileSystem::store(convolutedMapPath, convolutedMapImage);
	}
}

void PbrProbe::init(Texture* backgroundHDR,unsigned prefilteredSize, unsigned storeID, const std::filesystem::path& probeRoot)
{
	thread_local auto* renderBackend = RenderBackend::get();
	Viewport backup = renderBackend->getViewport();

	mStoreID = storeID;

	initBackground(backgroundHDR, probeRoot);
	initPrefiltered(backgroundHDR, prefilteredSize, probeRoot);
	initIrradiance(backgroundHDR, probeRoot);

	renderBackend->getRasterizer()->enableScissorTest(false);

	mMaterial->setIrradianceMap(convolutedEnvironmentMap.get());
	mMaterial->setPrefilterMap(prefilteredEnvMap.get());

	renderBackend->setViewPort(backup.x, backup.y, backup.width, backup.height);
}

ProbeVob::ProbeVob(SceneNode* meshRootNode, PbrProbe* probe) : Vob(meshRootNode), mProbe(probe)
{
	assert(mProbe != nullptr);
}

PbrProbe* ProbeVob::getProbe()
{
	return mProbe;
}