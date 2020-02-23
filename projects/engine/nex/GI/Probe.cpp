#include <nex/GI/Probe.hpp>
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

std::shared_ptr<Texture2D> ProbeFactory::mBrdfLookupTexture = nullptr;
std::shared_ptr<TypedOwningShaderProvider<Probe::ProbePass>> ProbeFactory::mProbeShaderProvider = nullptr;

std::unique_ptr<SphereMesh> ProbeFactory::mMesh = nullptr;
std::unique_ptr<Sampler> ProbeFactory::mSamplerIrradiance = nullptr;
std::unique_ptr<Sampler> ProbeFactory::mSamplerPrefiltered = nullptr;

std::unique_ptr<FileSystem> nex::ProbeFactory::mFileSystem;

const nex::TextureDesc nex::ProbeFactory::BRDF_DATA = {
			TexFilter::Linear,
			TexFilter::Linear,
			UVTechnique::ClampToEdge,
			UVTechnique::ClampToEdge,
			UVTechnique::ClampToEdge,
			InternalFormat::RG32F,
			false
};


const nex::TextureDesc nex::ProbeFactory::IRRADIANCE_DATA = {
			TexFilter::Linear,
			TexFilter::Linear,
			UVTechnique::ClampToEdge,
			UVTechnique::ClampToEdge,
			UVTechnique::ClampToEdge,
			InternalFormat::RGBA32F,
			false
};

const nex::TextureDesc nex::ProbeFactory::REFLECTION_DATA = {
			TexFilter::Linear_Mipmap_Linear,
			TexFilter::Linear,
			UVTechnique::ClampToEdge,
			UVTechnique::ClampToEdge,
			UVTechnique::ClampToEdge,
			InternalFormat::RGBA32F,
			true
};

const nex::TextureDesc nex::ProbeFactory::SOURCE_DATA = {
		TexFilter::Linear,
		TexFilter::Linear,
		UVTechnique::ClampToEdge,
		UVTechnique::ClampToEdge,
		UVTechnique::ClampToEdge,
		InternalFormat::RGB32F,
		false
};

nex::ProbeFactory::ProbeFactory(unsigned reflectionMapSize, unsigned irradianceArraySize, unsigned reflectionArraySize) : 
	mReflectionMapSize(reflectionMapSize), mIrradianceArraySize(irradianceArraySize), mIrradianceFreeSlots(irradianceArraySize),
	mReflectionArraySize(reflectionArraySize), mReflectionFreeSlots(reflectionArraySize)
{
	mIrradianceMaps = std::make_unique<CubeMapArray>(IRRADIANCE_SIZE, IRRADIANCE_SIZE, mIrradianceArraySize, IRRADIANCE_DATA, nullptr);
	mIrradianceSHMaps = std::make_unique<Texture1DArray>(Probe::SPHERHICAL_HARMONICS_WIDTH, mIrradianceArraySize, IRRADIANCE_DATA, nullptr);
	mReflectionMaps = std::make_unique<CubeMapArray>(mReflectionMapSize, mReflectionMapSize, mReflectionArraySize, REFLECTION_DATA, nullptr);
}

nex::Texture2D* nex::ProbeFactory::getBrdfLookupTexture()
{
	return mBrdfLookupTexture.get();
}

CubeMapArray * nex::ProbeFactory::getIrradianceMaps()
{
	return mIrradianceMaps.get();
}

nex::Texture1DArray* nex::ProbeFactory::getIrradianceSHMaps()
{
	return mIrradianceSHMaps.get();
}

CubeMapArray * nex::ProbeFactory::getReflectionMaps()
{
	return mReflectionMaps.get();
}

const std::filesystem::path & nex::ProbeFactory::getProbeRootDir() const
{
	return mFileSystem->getFirstIncludeDirectory();
}

void nex::ProbeFactory::init(const std::filesystem::path & probeCompiledDirectory, std::string probeFileExtension)
{
	std::vector<std::filesystem::path> includes = { probeCompiledDirectory };
	mFileSystem = std::make_unique<FileSystem>(std::move(includes), probeCompiledDirectory, probeFileExtension);

	auto probeRoot = mFileSystem->getFirstIncludeDirectory();

	Rectangle backup = RenderBackend::get()->getViewport();

	mProbeShaderProvider = std::make_shared<TypedOwningShaderProvider<Probe::ProbePass>>(
		std::make_unique<Probe::ProbePass>()
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
		const StoreImage brdfLUTImage = StoreImage::create(mBrdfLookupTexture.get(), PixelDataType::FLOAT);
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

class Probe::ProbePass : public TransformShader
{
public:

	ProbePass() : TransformShader(ShaderProgram::create("pbr/probe/pbr_probeVisualization_vs.glsl", "pbr/probe/pbr_probeVisualization_fs.glsl"))
	{
		mProbes = { mProgram->getUniformLocation("probes"), UniformType::CUBE_MAP_ARRAY };
		mArrayIndex = { mProgram->getUniformLocation("arrayIndex"), UniformType::FLOAT };

		SamplerDesc desc;
		//desc.minLOD = 0;
		//desc.maxLOD = 7;
		desc.minFilter = TexFilter::Linear_Mipmap_Linear;
		mReflectionSampler.setState(desc);
	}

	void setProbesTexture(CubeMapArray* map) {
		auto* sampler = &mReflectionSampler;
		if (map->hasNonBaseLevelMipMaps()) {
			sampler = &mReflectionSampler;
		}

		mProgram->setTexture(map, sampler, 0);
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

		setProbesTexture(material->mProbesTexture);
		setArrayIndex(material->mArrayIndex);
	}

	Uniform mArrayIndex;
	Uniform mProbes;
	Sampler mSampler;
	Sampler mReflectionSampler;
};

nex::Probe::ProbeMaterial::ProbeMaterial(ProbeShaderProvider provider) : Material(std::move(provider))
{
	assert(mShaderProvider != nullptr);
	mRenderState.doDepthTest = true;
	mRenderState.doCullFaces = true;
	mRenderState.doShadowCast = false;
	mRenderState.doShadowReceive = false;
	//mRenderState.cullSide = PolygonSide::FRONT;
}

void nex::Probe::ProbeMaterial::setProbesTexture(CubeMapArray* probesTexture)
{
	mProbesTexture = probesTexture;
}

void nex::Probe::ProbeMaterial::setArrayIndex(float index)
{
	mArrayIndex = index;
}

void nex::ProbeFactory::initProbe(ProbeVob& probeVob, const CubeMap * environmentMap, bool useCache, bool storeRenderedResult)
{
	auto* probe = probeVob.getProbe();
	const auto storeID = probe->getStoreID();
	const bool alreadyInitialized = probe->isInitialized();
	const auto type = probe->getType();

	const auto isIrradiance = type == Probe::Type::Irradiance;

	if (!alreadyInitialized && 
		(isIrradiance && (mIrradianceFreeSlots == 0) || !isIrradiance && (mReflectionFreeSlots == 0) )) {
		throw std::runtime_error(" nex::ProbeFactory::initProbe: No free slots!");
	}

	const auto nextIndex = isIrradiance ?
		mIrradianceArraySize - mIrradianceFreeSlots 
		: mReflectionArraySize - mReflectionFreeSlots;

	const auto arrayIndex = alreadyInitialized ? probe->getArrayIndex() : nextIndex;


	thread_local auto* renderBackend = RenderBackend::get();
	Rectangle backup = renderBackend->getViewport();
	renderBackend->getRasterizer()->enableScissorTest(false);

	const auto& probeRoot = mFileSystem->getFirstIncludeDirectory();

	if (!isIrradiance) {
		initReflection(environmentMap, storeID, arrayIndex, probeRoot, mReflectionMapSize, useCache, storeRenderedResult);
	}
	else {
		convoluteSphericalHarmonics(environmentMap, mIrradianceSHMaps.get(), arrayIndex);

		//glm::vec4 readBackData[9*1 + 5];
		//const auto readBackDataSize = sizeof(readBackData);
		//shOutput->readback(0, ColorSpace::RGBA, PixelDataType::FLOAT, readBackData, readBackDataSize);

		initIrradianceSH(probe, mIrradianceSHMaps.get(), storeID, arrayIndex, probeRoot, useCache, storeRenderedResult);

		//initIrradiance(environmentMap, storeID, arrayIndex, probeRoot, useCache, storeRenderedResult);
	}

	renderBackend->setViewPort(backup.x, backup.y, backup.width, backup.height);

	auto material = std::make_unique<nex::Probe::ProbeMaterial>(mProbeShaderProvider);


	if (isIrradiance) {
		material->setProbesTexture(mIrradianceMaps.get());
	}
	else {
		material->setProbesTexture(mReflectionMaps.get());
	}

	material->setArrayIndex(arrayIndex);

	probe->init(storeID, arrayIndex);

	auto group = std::make_unique<MeshGroup>();
	group->addMapping(mMesh.get(), material.get());
	group->calcBatches();
	probeVob.setMeshGroup(std::move(group));

	if (!alreadyInitialized) {
		if (isIrradiance) --mIrradianceFreeSlots;
		else --mReflectionFreeSlots;
	}
}

void nex::ProbeFactory::initProbe(ProbeVob & probeVob, bool useCache, bool storeRenderedResult)
{
	static std::function<std::shared_ptr<CubeMap>()> renderFunc = []()->std::shared_ptr<CubeMap> {
		throw_with_trace(std::runtime_error("Expected environment map to exist!"));
		return nullptr;
	};

	auto* probe = probeVob.getProbe();
	const auto storeID = probe->getStoreID();
	const auto& source = probe->getSource();

	std::optional<std::shared_ptr<CubeMap>> optionalStore;
	const CubeMap* environmentMap = nullptr;

	if (source) {
		const auto* sourceTex = source.value();
		if (const CubeMap* cubeMapSource = dynamic_cast<const CubeMap*>(sourceTex)) {
			environmentMap = cubeMapSource;
		}
		else {
			optionalStore = createCubeMap(sourceTex, storeID, useCache, storeRenderedResult);
		}
	}
	else {
		StoreImage readImage = loadCubeMap(mFileSystem->getFirstIncludeDirectory(),
			STORE_FILE_BASE_SOURCE,
			storeID,
			useCache,
			storeRenderedResult,
			renderFunc);

		optionalStore = std::shared_ptr<CubeMap>((CubeMap*)Texture::createFromImage(readImage, SOURCE_DATA));
	}

	if (optionalStore) {
		environmentMap = optionalStore.value().get();
	}
	
	initProbe(probeVob, environmentMap, useCache, storeRenderedResult);
}

bool nex::ProbeFactory::isProbeStored(const Probe& probe) const
{
	const auto storeID = probe.getStoreID();
	if (storeID == ProbeFactory::INVALID_STOREID) return false;

	auto fileName = STORE_FILE_BASE_SOURCE + std::to_string(storeID) + std::string(ProbeFactory::STORE_FILE_EXTENSION);
	auto resolvedPath = mFileSystem->resolvePath(fileName, "./",true);
	return std::filesystem::exists(resolvedPath);
}

Probe::Probe(Type type, const glm::vec3& position, std::optional<Texture*> source, unsigned storeID) :
	mArrayIndex(ProbeFactory::INVALID_ARRAY_INDEX),
	mStoreID(storeID),
	mInit(false),
	mPosition(position),
	mInfluenceRadius(10.0f),
	mInfluenceType(InfluenceType::SPHERE),
	mType(type),
	mSource(std::move(source))
{
	setPosition(mPosition);
}

Probe::~Probe() = default;

unsigned nex::Probe::getArrayIndex() const
{
	return mArrayIndex;
}

const AABB& nex::Probe::getInfluenceBox() const
{
	return mInfluenceBox;
}

float nex::Probe::getInfluenceRadius() const
{
	return mInfluenceRadius;
}

nex::Probe::InfluenceType nex::Probe::getInfluenceType() const
{
	return mInfluenceType;
}

const glm::vec3 & nex::Probe::getPosition() const
{
	return mPosition;
}

const std::vector<glm::vec4>& nex::Probe::getSHCoefficients() const
{
	return mSHCoefficients;
}

std::vector<glm::vec4>& nex::Probe::getSHCoefficients()
{
	return mSHCoefficients;
}

const std::optional<Texture*>& nex::Probe::getSource() const
{
	return mSource;
}

unsigned nex::Probe::getStoreID() const
{
	return mStoreID;
}

nex::Probe::Type nex::Probe::getType() const
{
	return mType;
}

std::shared_ptr<CubeMap> ProbeFactory::renderEquirectangularToCube(const Texture* background)
{
	auto cubeRenderTarget = std::make_unique<CubeRenderTarget>(SOURCE_CUBE_SIZE, SOURCE_CUBE_SIZE, SOURCE_DATA);
	RenderAttachment depth;
	depth.target = TextureTarget::TEXTURE2D;
	TextureDesc data;
	data.internalFormat = InternalFormat::DEPTH24;
	depth.texture = std::make_unique<RenderBuffer>(SOURCE_CUBE_SIZE, SOURCE_CUBE_SIZE, 1, data);
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

std::shared_ptr<CubeMap> ProbeFactory::convolute(const CubeMap * source)
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

std::shared_ptr<CubeMap> nex::ProbeFactory::createIrradianceSH(const Texture1DArray* shCoefficients, unsigned arrayIndex)
{
	thread_local auto* renderBackend = RenderBackend::get();

	auto cubeRenderTarget = renderBackend->createCubeRenderTarget(IRRADIANCE_SIZE, IRRADIANCE_SIZE,
		IRRADIANCE_DATA);

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

	thread_local auto mIrradianceShPass(std::make_unique<PbrIrradianceShPass>());

	mIrradianceShPass->bind();
	mIrradianceShPass->setProjection(projection);
	mIrradianceShPass->setCoefficientMap(shCoefficients);
	mIrradianceShPass->setArrayIndex(arrayIndex);

	cubeRenderTarget->bind();
	renderBackend->setViewPort(0, 0, cubeRenderTarget->getWidth(), cubeRenderTarget->getHeight());

	thread_local auto skyBox = createSkyBox();

	const auto& views = CubeMap::getViewLookAts();

	for (unsigned int side = 0; side < views.size(); ++side) {
		mIrradianceShPass->setView(views[side]);
		cubeRenderTarget->useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X));
		Drawer::draw(skyBox.get(), mIrradianceShPass.get());
	}

	renderBackend->wait();

	return std::dynamic_pointer_cast<CubeMap>(cubeRenderTarget->getColorAttachments()[0].texture);
}

std::shared_ptr<CubeMap> ProbeFactory::prefilter(const CubeMap * source, unsigned prefilteredSize)
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
	const auto mipMapCount = Texture::calcMipMapCount(mipMapLevelZero);

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

void nex::ProbeFactory::convoluteSphericalHarmonics(const CubeMap* source, Texture1DArray* output, unsigned rowIndex)
{
	thread_local auto shComputePass(std::make_unique<SHComputePass>());
	shComputePass->bind();
	shComputePass->compute(output, 0, source, rowIndex, 1);
	RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess | MemorySync_TextureUpdate);
}


std::unique_ptr<MeshGroup> nex::ProbeFactory::createSkyBox()
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

std::shared_ptr<Texture2D> ProbeFactory::createBRDFlookupTexture(Shader* brdfPrecompute)
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

StoreImage nex::ProbeFactory::loadCubeMap(const std::filesystem::path & probeRoot, const std::string & baseName, unsigned storeID, bool useCache, bool storeRenderedResult, const std::function<std::shared_ptr<CubeMap>()>& renderFunc)
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
		readImage = StoreImage::create(texture.get(), PixelDataType::FLOAT);

		if (storeRenderedResult && validStoreID) {
			FileSystem::store(storeFile, readImage);
		}
	}

	return readImage;
}

std::shared_ptr<CubeMap> ProbeFactory::createCubeMap(const Texture* backgroundHDR,
	unsigned storeID,
	bool useCache, 
	bool storeRenderedResult)
{
	StoreImage readImage = loadCubeMap(mFileSystem->getFirstIncludeDirectory(), 
		STORE_FILE_BASE_SOURCE,
		storeID,
		useCache, 
		storeRenderedResult, 
		[=]() {return ProbeFactory::renderEquirectangularToCube(backgroundHDR); });

	return std::shared_ptr<CubeMap>((CubeMap*)Texture::createFromImage(readImage, SOURCE_DATA));
}

void nex::ProbeFactory::initReflection(const CubeMap* source, 
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
		[=]() {return prefilter(source, reflectionMapSize); });

	StoreImage::fill(getReflectionMaps(), readImage, arrayIndex);
}

void ProbeFactory::initIrradiance(const CubeMap* source,
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
		[=]() {return convolute(source); });

	StoreImage::fill(getIrradianceMaps(), readImage, arrayIndex);
}

void nex::ProbeFactory::initIrradianceSH(
	Probe* probe,
	const Texture1DArray* shCoefficients,
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
		[=]() {return createIrradianceSH(shCoefficients, arrayIndex); });

	StoreImage::fill(getIrradianceMaps(), readImage, arrayIndex);

	auto& coeeficients = probe->getSHCoefficients();
	coeeficients.resize(Probe::SPHERHICAL_HARMONICS_WIDTH);

	TextureTransferDesc desc;
	desc.imageDesc.width = coeeficients.size();
	desc.imageDesc.height = 1;
	desc.imageDesc.depth = 1;
	desc.imageDesc.rowByteAlignmnet = 1;
	desc.imageDesc.colorspace = ColorSpace::RGBA; // A is not used, just for faster processing
	desc.data = coeeficients.data();
	desc.imageDesc.pixelDataType = PixelDataType::FLOAT;
	desc.dataByteSize = coeeficients.size() * sizeof(glm::vec4);
	desc.yOffset = arrayIndex;
	shCoefficients->readback(desc);
}

void Probe::init(
	unsigned storeID, 
	unsigned arrayIndex)
{
	mStoreID = storeID;
	mArrayIndex = arrayIndex;
	mInit = true;
}

bool nex::Probe::isInitialized() const
{
	return mInit;
}

void nex::Probe::setInfluenceBox(const glm::vec3& halfWidth)
{
	mInfluenceBox = AABB2(mPosition, halfWidth);
}

void nex::Probe::setInfluenceRadius(float radius)
{
	mInfluenceRadius = radius;
}

void nex::Probe::setInfluenceType(InfluenceType type)
{
	mInfluenceType = type;
}

void nex::Probe::setPosition(const glm::vec3 & position)
{
	mPosition = position;

	AABB2 box2(mInfluenceBox);
	box2.center = mPosition;
	mInfluenceBox = box2;	
}

ProbeVob::ProbeVob(Vob* parent, Probe* probe) : Vob(parent), mProbe(probe)
{
	assert(mProbe != nullptr);
	usePerObjectMaterialData(false);

	mName = "Pbr probe vob";
	mTypeName = "Pbr probe vob";
	nex::ProbeVob::setPositionLocalToParent(glm::vec3(mProbe->getPosition()));
}

Probe* ProbeVob::getProbe()
{
	return mProbe;
}

const Probe* ProbeVob::getProbe() const
{
	return mProbe;
}

void nex::ProbeVob::setPositionLocalToParent(const glm::vec3 & position)
{
	nex::Vob::setPositionLocalToParent(position);
	mProbe->setPosition(position);
}