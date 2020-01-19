#include <nex/GI/PbrProbe.hpp>
#include <nex/effects/SkyBoxpass.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/texture/Texture.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/pbr/PbrPass.hpp>
#include "nex/effects/EffectLibrary.hpp"
#include "nex/texture/Attachment.hpp"
#include "nex/renderer/Drawer.hpp"
#include "nex/light/Light.hpp"
#include <nex/texture/Sprite.hpp>
#include "nex/mesh/SampleMeshes.hpp"
#include <nex/material/Material.hpp>
#include <nex/mesh/MeshManager.hpp>
#include "nex/resource/FileSystem.hpp"
#include <nex/mesh/UtilityMeshes.hpp>
#include "nex/mesh/MeshFactory.hpp"
#include <nex/resource/ResourceLoader.hpp>
#include <nex/resource/Resource.hpp>
#include <functional>

using namespace glm;
using namespace nex;

std::shared_ptr<Texture2D> PbrProbeFactory::mBrdfLookupTexture = nullptr;
std::shared_ptr<TypedOwningShaderProvider<PbrProbe::ProbePass>> PbrProbeFactory::mProbeShaderProvider = nullptr;

std::unique_ptr<SphereMesh> PbrProbeFactory::mMesh = nullptr;
std::unique_ptr<Sampler> PbrProbeFactory::mSamplerIrradiance = nullptr;
std::unique_ptr<Sampler> PbrProbeFactory::mSamplerPrefiltered = nullptr;

std::unique_ptr<FileSystem> nex::PbrProbeFactory::mFileSystem;

const nex::TextureDesc nex::PbrProbeFactory::BRDF_DATA = {
			TexFilter::Linear,
			TexFilter::Linear,
			UVTechnique::ClampToEdge,
			UVTechnique::ClampToEdge,
			UVTechnique::ClampToEdge,
			ColorSpace::RG,
			PixelDataType::FLOAT,
			InternalFormat::RG32F,
			false
};


const nex::TextureDesc nex::PbrProbeFactory::IRRADIANCE_DATA = {
			TexFilter::Linear,
			TexFilter::Linear,
			UVTechnique::ClampToEdge,
			UVTechnique::ClampToEdge,
			UVTechnique::ClampToEdge,
			ColorSpace::RGBA,
			PixelDataType::FLOAT,
			InternalFormat::RGBA32F,
			false
};

const nex::TextureDesc nex::PbrProbeFactory::REFLECTION_DATA = {
			TexFilter::Linear_Mipmap_Linear,
			TexFilter::Linear,
			UVTechnique::ClampToEdge,
			UVTechnique::ClampToEdge,
			UVTechnique::ClampToEdge,
			ColorSpace::RGBA,
			PixelDataType::FLOAT,
			InternalFormat::RGBA32F,
			true
};

const nex::TextureDesc nex::PbrProbeFactory::SOURCE_DATA = {
		TexFilter::Linear,
		TexFilter::Linear,
		UVTechnique::ClampToEdge,
		UVTechnique::ClampToEdge,
		UVTechnique::ClampToEdge,
		ColorSpace::RGB,
		PixelDataType::FLOAT,
		InternalFormat::RGB32F,
		false
};

nex::PbrProbeFactory::PbrProbeFactory(unsigned reflectionMapSize, unsigned probeArraySize) : 
	mReflectionMapSize(reflectionMapSize), mProbeArraySize(probeArraySize), mFreeSlots(probeArraySize)
{
	mIrradianceMaps = std::make_unique<CubeMapArray>(IRRADIANCE_SIZE, IRRADIANCE_SIZE, probeArraySize, IRRADIANCE_DATA, nullptr);
	mReflectionMaps = std::make_unique<CubeMapArray>(mReflectionMapSize, mReflectionMapSize, probeArraySize, REFLECTION_DATA, nullptr);




}

nex::Texture2D* nex::PbrProbeFactory::getBrdfLookupTexture()
{
	return mBrdfLookupTexture.get();
}

CubeMapArray * nex::PbrProbeFactory::getIrradianceMaps()
{
	return mIrradianceMaps.get();
}

CubeMapArray * nex::PbrProbeFactory::getReflectionMaps()
{
	return mReflectionMaps.get();
}

const std::filesystem::path & nex::PbrProbeFactory::getProbeRootDir() const
{
	return mFileSystem->getFirstIncludeDirectory();
}

void nex::PbrProbeFactory::init(const std::filesystem::path & probeCompiledDirectory, std::string probeFileExtension)
{
	std::vector<std::filesystem::path> includes = { probeCompiledDirectory };
	mFileSystem = std::make_unique<FileSystem>(std::move(includes), probeCompiledDirectory, probeFileExtension);

	auto probeRoot = mFileSystem->getFirstIncludeDirectory();

	Rectangle backup = RenderBackend::get()->getViewport();

	mProbeShaderProvider = std::make_shared<TypedOwningShaderProvider<PbrProbe::ProbePass>>(
		std::make_unique<PbrProbe::ProbePass>()
		);

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
		FileSystem::load(brdfMapPath, readImage);

		mBrdfLookupTexture.reset((Texture2D*)Texture::createFromImage(readImage, BRDF_DATA));
	}
	else
	{
		mBrdfLookupTexture = createBRDFlookupTexture(&brdfPrecomputePass);
		const StoreImage brdfLUTImage = StoreImage::create(mBrdfLookupTexture.get());
		FileSystem::store(brdfMapPath, brdfLUTImage);
	}

	mSamplerIrradiance = std::make_unique<Sampler>();

	SamplerDesc desc;
	//desc.minLOD = 0;
	//desc.maxLOD = 7;
	desc.minFilter = TexFilter::Linear_Mipmap_Linear;
	mSamplerPrefiltered = std::make_unique<Sampler>(desc);

	RenderBackend::get()->setViewPort(backup.x, backup.y, backup.width, backup.height);
}

class PbrProbe::ProbePass : public TransformShader
{
public:

	ProbePass() : TransformShader(ShaderProgram::create("pbr/pbr_probeVisualization_vs.glsl", "pbr/pbr_probeVisualization_fs.glsl"))
	{
		mIrradianceMaps = { mProgram->getUniformLocation("irradianceMaps"), UniformType::CUBE_MAP_ARRAY };
		mReflectionMaps = { mProgram->getUniformLocation("reflectionMaps"), UniformType::CUBE_MAP_ARRAY };
		mArrayIndex = { mProgram->getUniformLocation("arrayIndex"), UniformType::FLOAT };

		SamplerDesc desc;
		//desc.minLOD = 0;
		//desc.maxLOD = 7;
		desc.minFilter = TexFilter::Linear_Mipmap_Linear;
		mReflectionSampler.setState(desc);
	}

	void setIrradianceMaps(CubeMapArray* map) {
		mProgram->setTexture(map, &mSampler, 0);
	}
	void setReflectionMaps(CubeMapArray* map) {
		mProgram->setTexture(map, &mReflectionSampler, 1);
	}

	void setArrayIndex(float index) {
		mProgram->setFloat(mArrayIndex.location, index);
	}

	void updateMaterial(const Material& m) override {
		const ProbeMaterial* material;
		try {
			material = &dynamic_cast<const ProbeMaterial&>(m);
		}
		catch (std::bad_cast & e) {
			throw_with_trace(e);
		}


		setIrradianceMaps(material->mIrradianceMaps);
		setReflectionMaps(material->mReflectionMaps);
		setArrayIndex(material->mArrayIndex);

	}

	Uniform mArrayIndex;
	Uniform mIrradianceMaps;
	Uniform mReflectionMaps;
	Sampler mSampler;
	Sampler mReflectionSampler;
};

nex::PbrProbe::ProbeMaterial::ProbeMaterial(ProbeShaderProvider provider) : Material(std::move(provider))
{
	assert(mShaderProvider != nullptr);
	mRenderState.doDepthTest = true;
	mRenderState.doCullFaces = true;
	mRenderState.doShadowCast = false;
	mRenderState.doShadowReceive = false;
	//mRenderState.cullSide = PolygonSide::FRONT;
}

void nex::PbrProbe::ProbeMaterial::setIrradianceMaps(CubeMapArray* irradianceMaps)
{
	mIrradianceMaps = irradianceMaps;
}

void nex::PbrProbe::ProbeMaterial::setReflectionMaps(CubeMapArray* reflectionMaps)
{
	mReflectionMaps = reflectionMaps;
}

void nex::PbrProbe::ProbeMaterial::setArrayIndex(float index)
{
	mArrayIndex = index;
}


void nex::PbrProbeFactory::initProbeBackground(ProbeVob& probeVob, Texture * backgroundHDR, unsigned storeID, bool useCache, bool storeRenderedResult)
{
	auto source = createSource(backgroundHDR, storeID, mFileSystem->getFirstIncludeDirectory(), useCache, storeRenderedResult);
	initProbe(probeVob, source.get(), storeID, useCache, storeRenderedResult);
	return;
}

void nex::PbrProbeFactory::initProbe(ProbeVob& probeVob, CubeMap * environmentMap, unsigned storeID, bool useCache, bool storeRenderedResult)
{
	auto* probe = probeVob.getProbe();
	const bool alreadyInitialized = probe->isInitialized();

	if (!alreadyInitialized && mFreeSlots == 0) {
		throw std::runtime_error(" nex::PbrProbeFactory::initProbe: No free slots!");
	}

	const auto arrayIndex = alreadyInitialized ? probe->getArrayIndex() : mProbeArraySize - mFreeSlots;


	thread_local auto* renderBackend = RenderBackend::get();
	Rectangle backup = renderBackend->getViewport();
	renderBackend->getRasterizer()->enableScissorTest(false);

	const auto& probeRoot = mFileSystem->getFirstIncludeDirectory();

	initReflection(environmentMap, storeID, arrayIndex, probeRoot, mReflectionMapSize, useCache, storeRenderedResult);

	if (true) {
		TextureDesc data;
		data.colorspace = ColorSpace::RGBA;
		data.internalFormat = InternalFormat::RGBA32F;
		data.pixelDataType = PixelDataType::FLOAT;

		auto shOutput = std::make_unique<Texture2D>(9, 1, data, nullptr);

		convoluteSphericalHarmonics(environmentMap, shOutput.get(), 0);

		//glm::vec4 readBackData[9*1 + 5];
		//const auto readBackDataSize = sizeof(readBackData);
		//shOutput->readback(0, ColorSpace::RGBA, PixelDataType::FLOAT, readBackData, readBackDataSize);

		initIrradianceSH(shOutput.get(), storeID, arrayIndex, probeRoot, useCache, storeRenderedResult);
	}
	else {
		initIrradiance(environmentMap, storeID, arrayIndex, probeRoot, useCache, storeRenderedResult);
	}

	renderBackend->setViewPort(backup.x, backup.y, backup.width, backup.height);

	auto material = std::make_unique<nex::PbrProbe::ProbeMaterial>(mProbeShaderProvider);

	material->setReflectionMaps(mReflectionMaps.get());
	material->setIrradianceMaps(mIrradianceMaps.get());
	material->setArrayIndex(arrayIndex);

	probe->init(storeID, arrayIndex, std::move(material), mMesh.get());
	probeVob.setBatches(probe->getMeshGroup()->getBatches());

	if (!alreadyInitialized)
		--mFreeSlots;
}

void nex::PbrProbeFactory::initProbe(ProbeVob & probeVob, unsigned storeID, bool useCache, bool storeRenderedResult)
{
	static std::function<std::shared_ptr<CubeMap>()> renderFunc = []()->std::shared_ptr<CubeMap> {
		throw_with_trace(std::runtime_error("Expected environment map to exist!"));
		return nullptr;
	};

	StoreImage readImage = loadCubeMap(mFileSystem->getFirstIncludeDirectory(),
		STORE_FILE_BASE_SOURCE,
		storeID,
		useCache,
		storeRenderedResult,
		renderFunc);

	auto environmentMap = std::shared_ptr<CubeMap>((CubeMap*)Texture::createFromImage(readImage, SOURCE_DATA));
	initProbe(probeVob, environmentMap.get(), storeID, useCache, storeRenderedResult);
}

bool nex::PbrProbeFactory::isProbeStored(const PbrProbe& probe) const
{
	const auto storeID = probe.getStoreID();
	if (storeID == PbrProbeFactory::INVALID_STOREID) return false;

	auto fileName = STORE_FILE_BASE_SOURCE + std::to_string(storeID) + std::string(PbrProbeFactory::STORE_FILE_EXTENSION);
	auto resolvedPath = mFileSystem->resolvePath(fileName);
	return std::filesystem::exists(resolvedPath);
}

PbrProbe::PbrProbe(const glm::vec3& position, unsigned storeID) :
	mMaterial(nullptr),
	mMeshGroup(std::make_unique<MeshGroup>()),
	mArrayIndex(PbrProbeFactory::INVALID_ARRAY_INDEX),
	mStoreID(storeID),
	mInit(false),
	mPosition(position),
	mInfluenceRadius(10.0f),
	mInfluenceType(InfluenceType::SPHERE)
{
	setPosition(mPosition);
}

PbrProbe::~PbrProbe() = default;

unsigned nex::PbrProbe::getArrayIndex() const
{
	return mArrayIndex;
}

MeshGroup* nex::PbrProbe::getMeshGroup()
{
	return mMeshGroup.get();
}

const AABB& nex::PbrProbe::getInfluenceBox() const
{
	return mInfluenceBox;
}

float nex::PbrProbe::getInfluenceRadius() const
{
	return mInfluenceRadius;
}

nex::PbrProbe::InfluenceType nex::PbrProbe::getInfluenceType() const
{
	return mInfluenceType;
}

const nex::PbrProbe::Handles * nex::PbrProbe::getHandles() const
{
	return &mHandles;
}

const glm::vec3 & nex::PbrProbe::getPosition() const
{
	return mPosition;
}

unsigned nex::PbrProbe::getStoreID() const
{
	return mStoreID;
}

std::shared_ptr<CubeMap> PbrProbeFactory::renderBackgroundToCube(Texture* background)
{
	auto cubeRenderTarget = std::make_unique<CubeRenderTarget>(SOURCE_CUBE_SIZE, SOURCE_CUBE_SIZE, SOURCE_DATA);
	RenderAttachment depth;
	depth.target = TextureTarget::TEXTURE2D;
	TextureDesc data;
	data.colorspace = ColorSpace::DEPTH;
	data.internalFormat = InternalFormat::DEPTH24;
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
		Drawer::draw(skyBox.get(), shader);
	}


	auto& resultAttachment = cubeRenderTarget->getColorAttachments()[0];
	auto result = std::dynamic_pointer_cast<CubeMap>(resultAttachment.texture);

	// now create mipmaps for the cubemap fighting render artificats in the prefilter map
	//cubeRenderTarget->enableDrawToColorAttachments(false);

	
	//cubeRenderTarget.reset();
	//result->generateMipMaps();
	//RenderBackend::get()->wait();
	return result;
}

std::shared_ptr<CubeMap> PbrProbeFactory::convolute(CubeMap * source)
{
	thread_local auto* renderBackend = RenderBackend::get();

	auto cubeRenderTarget = renderBackend->createCubeRenderTarget(IRRADIANCE_SIZE, IRRADIANCE_SIZE, 
		IRRADIANCE_DATA);

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

	thread_local auto mConvolutionPass(std::make_unique<PbrConvolutionPass>());

	mConvolutionPass->bind();
	mConvolutionPass->setProjection(projection);
	mConvolutionPass->setEnvironmentMap(source);

	cubeRenderTarget->bind();
	renderBackend->setViewPort(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());

	thread_local auto skyBox = createSkyBox();

	const auto& views = CubeMap::getViewLookAts();

	for (unsigned int side = 0; side < views.size(); ++side) {
		mConvolutionPass->setView(views[side]);
		cubeRenderTarget->useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X));
		Drawer::draw(skyBox.get(), mConvolutionPass.get());
	}

	return std::dynamic_pointer_cast<CubeMap>(cubeRenderTarget->getColorAttachments()[0].texture);
}

std::shared_ptr<CubeMap> nex::PbrProbeFactory::createIrradianceSH(Texture2D* shCoefficients)
{
	thread_local auto* renderBackend = RenderBackend::get();

	auto cubeRenderTarget = renderBackend->createCubeRenderTarget(IRRADIANCE_SIZE, IRRADIANCE_SIZE,
		IRRADIANCE_DATA);

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

	thread_local auto mIrradianceShPass(std::make_unique<PbrIrradianceShPass>());

	mIrradianceShPass->bind();
	mIrradianceShPass->setProjection(projection);
	mIrradianceShPass->setCoefficientMap(shCoefficients);

	cubeRenderTarget->bind();
	renderBackend->setViewPort(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());

	thread_local auto skyBox = createSkyBox();

	const auto& views = CubeMap::getViewLookAts();

	for (unsigned int side = 0; side < views.size(); ++side) {
		mIrradianceShPass->setView(views[side]);
		cubeRenderTarget->useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X));
		Drawer::draw(skyBox.get(), mIrradianceShPass.get());
	}

	return std::dynamic_pointer_cast<CubeMap>(cubeRenderTarget->getColorAttachments()[0].texture);
}

std::shared_ptr<CubeMap> PbrProbeFactory::prefilter(CubeMap * source, unsigned prefilteredSize)
{
	thread_local auto* renderBackend = RenderBackend::get();
	thread_local auto skyBox = createSkyBox();


	auto prefilterRenderTarget = renderBackend->createCubeRenderTarget(prefilteredSize, prefilteredSize, REFLECTION_DATA);

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);


	thread_local auto mPrefilterPass(std::make_unique<PbrPrefilterPass>());
	mPrefilterPass->bind();
	mPrefilterPass->setProjection(projection);
	mPrefilterPass->setMapToPrefilter(source);

	//view matrices;
	const auto& views = CubeMap::getViewLookAts();

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
		for (unsigned int side = 0; side < views.size(); ++side) {
			mPrefilterPass->setView(views[side]);
			prefilterRenderTarget->useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X), mipLevel);
			Drawer::draw(skyBox.get(), mPrefilterPass.get());
		}
	}


	//CubeMap* result = (CubeMap*)prefilterRenderTarget->setRenderResult(nullptr);
	auto result = std::dynamic_pointer_cast<CubeMap>(prefilterRenderTarget->getColorAttachments()[0].texture);
	//result->generateMipMaps();

	return result;
}

void nex::PbrProbeFactory::convoluteSphericalHarmonics(CubeMap* source, Texture2D* output, unsigned rowIndex)
{
	thread_local auto shComputePass(std::make_unique<SHComputePass>());
	shComputePass->bind();
	shComputePass->compute(output, 0, source, rowIndex, 1);
	RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess | MemorySync_TextureUpdate);
}


std::unique_ptr<MeshGroup> nex::PbrProbeFactory::createSkyBox()
{
	int vertexCount = (int)sizeof(sample_meshes::skyBoxVertices);
	int indexCount = (int)sizeof(sample_meshes::skyBoxIndices);

	AABB boundingBox;
	boundingBox.min = glm::vec3(0.0f);
	boundingBox.max = glm::vec3(0.0f);

	std::unique_ptr<Mesh> mesh = MeshFactory::createPosition((const VertexPosition*)sample_meshes::skyBoxVertices, vertexCount,
		sample_meshes::skyBoxIndices, (int)indexCount, std::move(boundingBox));

	auto model = std::make_unique<MeshGroup>();
	model->add(std::move(mesh), std::make_unique<Material>(nullptr));
	model->calcBatches();
	model->finalize();

	return model;
}

std::shared_ptr<Texture2D> PbrProbeFactory::createBRDFlookupTexture(Shader* brdfPrecompute)
{
	thread_local auto* renderBackend = RenderBackend::get();

	auto target = std::make_unique<RenderTarget2D>(BRDF_SIZE, BRDF_SIZE, BRDF_DATA);

	target->bind();
	renderBackend->setViewPort(0, 0, BRDF_SIZE, BRDF_SIZE);
	target->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);

	brdfPrecompute->bind();
	const auto& state = RenderState::getNoDepthTest();
	Drawer::drawFullscreenTriangle(state, brdfPrecompute);

	auto result = std::dynamic_pointer_cast<Texture2D>(target->getColorAttachments()[0].texture);

	target->getColorAttachments()[0].texture = nullptr;
	target->updateColorAttachment(0);
	return result;
}

StoreImage nex::PbrProbeFactory::loadCubeMap(const std::filesystem::path & probeRoot, const std::string & baseName, unsigned storeID, bool useCache, bool storeRenderedResult, const std::function<std::shared_ptr<CubeMap>()>& renderFunc)
{
	const std::filesystem::path storeFile = probeRoot / (baseName + std::to_string(storeID) + std::string(STORE_FILE_EXTENSION));
	bool validStoreID = storeID != INVALID_STOREID;


	StoreImage readImage;

	if (std::filesystem::exists(storeFile) && useCache && validStoreID)
	{
		FileSystem::load(storeFile, readImage);
	}
	else
	{
		auto texture = renderFunc();
		readImage = StoreImage::create(texture.get());

		if (storeRenderedResult && validStoreID) {
			FileSystem::store(storeFile, readImage);
		}
	}

	return readImage;
}

std::shared_ptr<CubeMap> PbrProbeFactory::createSource(Texture* backgroundHDR, 
	unsigned storeID,
	const std::filesystem::path& probeRoot, 
	bool useCache, 
	bool storeRenderedResult)
{
	StoreImage readImage = loadCubeMap(probeRoot, 
		STORE_FILE_BASE_SOURCE,
		storeID,
		useCache, 
		storeRenderedResult, 
		std::bind(&PbrProbeFactory::renderBackgroundToCube, this, backgroundHDR));

	return std::shared_ptr<CubeMap>((CubeMap*)Texture::createFromImage(readImage, SOURCE_DATA));
}

void nex::PbrProbeFactory::initReflection(CubeMap* source, 
	unsigned storeID,
	unsigned arrayIndex,
	const std::filesystem::path& probeRoot, 
	unsigned reflectionMapSize,
	bool useCache, 
	bool storeRenderedResult)
{
	StoreImage readImage = loadCubeMap(probeRoot,
		STORE_FILE_BASE_REFLECTION,
		storeID,
		useCache,
		storeRenderedResult,
		std::bind(&PbrProbeFactory::prefilter, this, source, reflectionMapSize));

	StoreImage::fill(getReflectionMaps(), readImage, arrayIndex);
}

void PbrProbeFactory::initIrradiance(CubeMap* source,
	unsigned storeID,
	unsigned arrayIndex,
	const std::filesystem::path& probeRoot, 
	bool useCache, 
	bool storeRenderedResult)
{
	StoreImage readImage = loadCubeMap(probeRoot,
		STORE_FILE_BASE_IRRADIANCE,
		storeID,
		useCache,
		storeRenderedResult,
		std::bind(&PbrProbeFactory::convolute, this, source));

	StoreImage::fill(getIrradianceMaps(), readImage, arrayIndex);
}

void nex::PbrProbeFactory::initIrradianceSH(Texture2D* shCoefficients,
	unsigned storeID,
	unsigned arrayIndex,
	const std::filesystem::path& probeRoot, 
	bool useCache, 
	bool storeRenderedResult)
{
	StoreImage readImage = loadCubeMap(probeRoot,
		STORE_FILE_BASE_IRRADIANCE_SH,
		storeID,
		useCache,
		storeRenderedResult,
		std::bind(&PbrProbeFactory::createIrradianceSH, this, shCoefficients));

	StoreImage::fill(getIrradianceMaps(), readImage, arrayIndex);
}

void PbrProbe::init(
	unsigned storeID, 
	unsigned arrayIndex,
	std::unique_ptr<ProbeMaterial> probeMaterial, 
	Mesh* mesh)
{
	mStoreID = storeID;
	mArrayIndex = arrayIndex;
	mMaterial = std::move(probeMaterial);

	mMeshGroup = std::make_unique<MeshGroup>();
	mMeshGroup->addMapping(mesh, mMaterial.get());
	mMeshGroup->calcBatches();
	mInit = true;
}

bool nex::PbrProbe::isInitialized() const
{
	return mInit;
}

void nex::PbrProbe::setInfluenceBox(const glm::vec3& halfWidth)
{
	mInfluenceBox = AABB2(mPosition, halfWidth);
}

void nex::PbrProbe::setInfluenceRadius(float radius)
{
	mInfluenceRadius = radius;
}

void nex::PbrProbe::setInfluenceType(InfluenceType type)
{
	mInfluenceType = type;
}

void nex::PbrProbe::setPosition(const glm::vec3 & position)
{
	mPosition = position;

	AABB2 box2(mInfluenceBox);
	box2.center = mPosition;
	mInfluenceBox = box2;	
}

ProbeVob::ProbeVob(Vob* parent, PbrProbe* probe) : Vob(parent), mProbe(probe)
{
	assert(mProbe != nullptr);
	mName = "Pbr probe vob";
	mTypeName = "Pbr probe vob";
	nex::ProbeVob::setPositionLocalToParent(glm::vec3(mProbe->getPosition()));
}

PbrProbe* ProbeVob::getProbe()
{
	return mProbe;
}

void nex::ProbeVob::setPositionLocalToParent(const glm::vec3 & position)
{
	nex::Vob::setPositionLocalToParent(position);
	mProbe->setPosition(position);
}