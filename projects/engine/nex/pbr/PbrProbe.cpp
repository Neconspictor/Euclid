#include <nex/pbr/PbrProbe.hpp>
#include <nex/effects/SkyBoxpass.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/texture/Texture.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include "nex/pbr/PbrPass.hpp"
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

std::shared_ptr<Texture2D> PbrProbe::mBrdfLookupTexture = nullptr;
std::shared_ptr<TypedOwningShaderProvider<PbrProbe::ProbePass>> PbrProbe::mProbeShaderProvider = nullptr;

std::unique_ptr<SphereMesh> PbrProbe::mMesh = nullptr;
std::unique_ptr<Sampler> PbrProbe::mSamplerIrradiance = nullptr;
std::unique_ptr<Sampler> PbrProbe::mSamplerPrefiltered = nullptr;

std::unique_ptr<FileSystem> nex::PbrProbeFactory::mFileSystem;

const nex::TextureDesc nex::PbrProbe::BRDF_DATA = {
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


const nex::TextureDesc nex::PbrProbe::IRRADIANCE_DATA = {
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

const nex::TextureDesc nex::PbrProbe::PREFILTERED_DATA = {
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

const nex::TextureDesc nex::PbrProbe::SOURCE_DATA = {
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

nex::PbrProbeFactory::PbrProbeFactory(unsigned prefilteredSide, unsigned mapSize) : 
	mPrefilteredSide(prefilteredSide), mMapSize(mapSize), mFreeSlots(mapSize)
{
	mIrradianceMaps = std::make_unique<CubeMapArray>(PbrProbe::IRRADIANCE_SIZE, PbrProbe::IRRADIANCE_SIZE, mapSize, PbrProbe::IRRADIANCE_DATA, nullptr);
	mPrefilteredMaps = std::make_unique<CubeMapArray>(mPrefilteredSide, mPrefilteredSide, mapSize, PbrProbe::PREFILTERED_DATA, nullptr);
}

CubeMapArray * nex::PbrProbeFactory::getIrradianceMaps()
{
	return mIrradianceMaps.get();
}

CubeMapArray * nex::PbrProbeFactory::getPrefilteredMaps()
{
	return mPrefilteredMaps.get();
}

const std::filesystem::path & nex::PbrProbeFactory::getProbeRootDir() const
{
	return mFileSystem->getFirstIncludeDirectory();
}

void nex::PbrProbeFactory::init(const std::filesystem::path & probeCompiledDirectory, std::string probeFileExtension)
{
	std::vector<std::filesystem::path> includes = { probeCompiledDirectory };
	mFileSystem = std::make_unique<FileSystem>(std::move(includes), probeCompiledDirectory, probeFileExtension);
	PbrProbe::initGlobals(mFileSystem->getFirstIncludeDirectory());
}

class PbrProbe::ProbePass : public TransformShader
{
public:

	ProbePass() : TransformShader(ShaderProgram::create("pbr/pbr_probeVisualization_vs.glsl", "pbr/pbr_probeVisualization_fs.glsl"))
	{
		mIrradianceMaps = { mProgram->getUniformLocation("irradianceMaps"), UniformType::CUBE_MAP_ARRAY };
		mPrefilterMaps = { mProgram->getUniformLocation("prefilteredMaps"), UniformType::CUBE_MAP_ARRAY };
		mArrayIndex = { mProgram->getUniformLocation("arrayIndex"), UniformType::FLOAT };

		SamplerDesc desc;
		//desc.minLOD = 0;
		//desc.maxLOD = 7;
		desc.minFilter = TexFilter::Linear_Mipmap_Linear;
		mPrefilteredSampler.setState(desc);
	}

	void setIrradianceMaps(CubeMapArray* map) {
		mProgram->setTexture(map, &mSampler, 0);
	}
	void setPrefilteredMaps(CubeMapArray* map) {
		mProgram->setTexture(map, &mPrefilteredSampler, 1);
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


		setIrradianceMaps(material->mFactory->getIrradianceMaps());
		setPrefilteredMaps(material->mFactory->getPrefilteredMaps());
		setArrayIndex(material->mArrayIndex);

	}

	Uniform mArrayIndex;
	Uniform mIrradianceMaps;
	Uniform mPrefilterMaps;
	Sampler mSampler;
	Sampler mPrefilteredSampler;
};

nex::PbrProbe::ProbeMaterial::ProbeMaterial(ProbeShaderProvider provider) : Material(std::move(provider))
{
	assert(mShaderProvider != nullptr);
	mRenderState.doCullFaces = true;
	mRenderState.doShadowCast = false;
	mRenderState.doShadowReceive = false;
	//mRenderState.cullSide = PolygonSide::FRONT;
}

void nex::PbrProbe::ProbeMaterial::setProbeFactory(PbrProbeFactory * factory)
{
	mFactory = factory;
}

void nex::PbrProbe::ProbeMaterial::setArrayIndex(float index)
{
	mArrayIndex = index;
}


void nex::PbrProbeFactory::initProbeBackground(PbrProbe& probe, Texture * backgroundHDR, unsigned storeID, bool useCache, bool storeRenderedResult)
{
	const bool alreadyInitialized = probe.isInitialized();

	if (!alreadyInitialized && mFreeSlots == 0) {
		throw std::runtime_error(" nex::PbrProbeFactory::initProbe: No free slots!");
	}


	const auto arrayIndex = alreadyInitialized ? probe.getArrayIndex() : mMapSize - mFreeSlots;

	probe.init(backgroundHDR,
		mPrefilteredSide,
		storeID,
		this,
		arrayIndex,
		mFileSystem->getFirstIncludeDirectory(),
		useCache,
		storeRenderedResult);

	if (!alreadyInitialized)
		--mFreeSlots;
}

void nex::PbrProbeFactory::initProbe(PbrProbe& probe, CubeMap * environmentMap, unsigned storeID, bool useCache, bool storeRenderedResult)
{
	const bool alreadyInitialized = probe.isInitialized();

	if (!alreadyInitialized && mFreeSlots == 0) {
		throw std::runtime_error(" nex::PbrProbeFactory::initProbe: No free slots!");
	}

	const auto arrayIndex = alreadyInitialized ? probe.getArrayIndex() : mMapSize - mFreeSlots;

	probe.init(environmentMap,
		mPrefilteredSide,
		storeID,
		this,
		arrayIndex,
		mFileSystem->getFirstIncludeDirectory(),
		useCache,
		storeRenderedResult);

	if (!alreadyInitialized)
		--mFreeSlots;
}

void nex::PbrProbeFactory::initProbe(PbrProbe & probe, unsigned storeID, bool useCache, bool storeRenderedResult)
{
	const bool alreadyInitialized = probe.isInitialized();

	if (!alreadyInitialized && mFreeSlots == 0) {
		throw std::runtime_error(" nex::PbrProbeFactory::initProbe: No free slots!");
	}


	const auto arrayIndex = alreadyInitialized ? probe.getArrayIndex() : mMapSize - mFreeSlots;

	probe.init(mPrefilteredSide,
		storeID,
		this,
		arrayIndex,
		mFileSystem->getFirstIncludeDirectory(),
		useCache,
		storeRenderedResult);

	if (!alreadyInitialized)
		--mFreeSlots;
}

PbrProbe::PbrProbe(const glm::vec3& position, unsigned storeID) :
	mMaterial(std::make_unique<ProbeMaterial>(mProbeShaderProvider)),
	mMeshGroup(std::make_unique<MeshGroup>()),
	mFactory(nullptr),
	mArrayIndex(INVALID_ARRAY_INDEX),
	mStoreID(storeID),
	mInit(false),
	mPosition(position),
	mInfluenceRadius(10.0f),
	mInfluenceType(InfluenceType::SPHERE)
{
	mMeshGroup->addMapping(getSphere(), mMaterial.get());
	mMeshGroup->calcBatches();

	setPosition(mPosition);
}

PbrProbe::~PbrProbe() = default;

void PbrProbe::initGlobals(const std::filesystem::path& probeRoot)
{
	Rectangle backup = RenderBackend::get()->getViewport();

	mProbeShaderProvider = std::make_shared<TypedOwningShaderProvider<ProbePass>>(
		std::make_unique<ProbePass>()
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

Mesh* PbrProbe::getSphere()
{
	return mMesh.get();
}

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

CubeMapArray * PbrProbe::getIrradianceMaps() const
{
	return mFactory->getIrradianceMaps();
}

const nex::PbrProbe::Handles * nex::PbrProbe::getHandles() const
{
	return &mHandles;
}

CubeMapArray * PbrProbe::getPrefilteredMaps() const
{
	return mFactory->getPrefilteredMaps();
}

Texture2D * PbrProbe::getBrdfLookupTexture()
{
	return mBrdfLookupTexture.get();
}

const glm::vec3 & nex::PbrProbe::getPosition() const
{
	return mPosition;
}

unsigned nex::PbrProbe::getStoreID() const
{
	return mStoreID;
}

std::shared_ptr<CubeMap> PbrProbe::renderBackgroundToCube(Texture* background)
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
	result->generateMipMaps();
	return result;
}

std::shared_ptr<CubeMap> PbrProbe::convolute(CubeMap * source)
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

std::shared_ptr<CubeMap> nex::PbrProbe::createIrradianceSH(Texture2D* shCoefficients)
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

std::shared_ptr<CubeMap> PbrProbe::prefilter(CubeMap * source, unsigned prefilteredSize)
{
	thread_local auto* renderBackend = RenderBackend::get();
	thread_local auto skyBox = createSkyBox();


	auto prefilterRenderTarget = renderBackend->createCubeRenderTarget(prefilteredSize, prefilteredSize, PREFILTERED_DATA);

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

void nex::PbrProbe::convoluteSphericalHarmonics(CubeMap* source, Texture2D* output, unsigned rowIndex)
{
	thread_local auto shComputePass(std::make_unique<SHComputePass>());
	shComputePass->bind();
	shComputePass->compute(output, 0, source, rowIndex, 1);
	RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess | MemorySync_TextureUpdate);
}


std::unique_ptr<MeshGroup> nex::PbrProbe::createSkyBox()
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

std::shared_ptr<Texture2D> PbrProbe::createBRDFlookupTexture(Shader* brdfPrecompute)
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

StoreImage nex::PbrProbe::loadCubeMap(const std::filesystem::path & probeRoot, const std::string & baseName, unsigned storeID, bool useCache, bool storeRenderedResult, const std::function<std::shared_ptr<CubeMap>()>& renderFunc)
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

std::shared_ptr<CubeMap> PbrProbe::createSource(Texture* backgroundHDR, const std::filesystem::path& probeRoot, bool useCache, bool storeRenderedResult)
{
	StoreImage readImage = loadCubeMap(probeRoot, 
		"pbr_environmentMap_", 
		mStoreID, 
		useCache, 
		storeRenderedResult, 
		std::bind(&PbrProbe::renderBackgroundToCube, this, backgroundHDR));

	return std::shared_ptr<CubeMap>((CubeMap*)Texture::createFromImage(readImage, SOURCE_DATA));
}

void PbrProbe::initPrefiltered(CubeMap* source,  unsigned prefilteredSize, const std::filesystem::path& probeRoot, bool useCache, bool storeRenderedResult)
{
	StoreImage readImage = loadCubeMap(probeRoot,
		"pbr_prefilteredEnvMap_",
		mStoreID,
		useCache,
		storeRenderedResult,
		std::bind(&PbrProbe::prefilter, this, source, prefilteredSize));

	StoreImage::fill(mFactory->getPrefilteredMaps(), readImage, mArrayIndex);
}

void PbrProbe::initIrradiance(CubeMap* source, const std::filesystem::path& probeRoot, bool useCache, bool storeRenderedResult)
{
	StoreImage readImage = loadCubeMap(probeRoot,
		"pbr_convolutedEnvMap_",
		mStoreID,
		useCache,
		storeRenderedResult,
		std::bind(&PbrProbe::convolute, this, source));

	StoreImage::fill(mFactory->getIrradianceMaps(), readImage, mArrayIndex);
}

void nex::PbrProbe::initIrradianceSH(Texture2D* shCoefficients, const std::filesystem::path& probeRoot, bool useCache, bool storeRenderedResult)
{
	StoreImage readImage = loadCubeMap(probeRoot,
		"pbr_irradianceMap_sh_",
		mStoreID,
		useCache,
		storeRenderedResult,
		std::bind(&PbrProbe::createIrradianceSH, this, shCoefficients));

	StoreImage::fill(mFactory->getIrradianceMaps(), readImage, mArrayIndex);
}

void PbrProbe::init(Texture* backgroundHDR,
				unsigned prefilteredSize, unsigned storeID, 
				PbrProbeFactory* factory, unsigned arrayIndex,
				const std::filesystem::path& probeRoot, bool useCache, bool storeRenderedResult)
{
	if (factory == nullptr)
		throw_with_trace(std::invalid_argument("PbrProbe::init: probe factory is null!"));


	thread_local auto* renderBackend = RenderBackend::get();
	nex::Rectangle backup = renderBackend->getViewport();
	renderBackend->getRasterizer()->enableScissorTest(false);

	mStoreID = storeID;
	mArrayIndex = arrayIndex;
	mFactory = factory;

	auto source = createSource(backgroundHDR, probeRoot, useCache, storeRenderedResult);
	init(source.get(), prefilteredSize, storeID, factory, arrayIndex, probeRoot, useCache, storeRenderedResult);
	renderBackend->setViewPort(backup.x, backup.y, backup.width, backup.height);
}

void nex::PbrProbe::init(CubeMap * environment,
	unsigned prefilteredSize, unsigned storeID, 
	PbrProbeFactory * factory, unsigned arrayIndex, 
	const std::filesystem::path & probeRoot, bool useCache, bool storeRenderedResult)
{
	if (factory == nullptr)
		throw_with_trace(std::invalid_argument("PbrProbe::init: probe factory is null!"));

	thread_local auto* renderBackend = RenderBackend::get();
	Rectangle backup = renderBackend->getViewport();
	renderBackend->getRasterizer()->enableScissorTest(false);

	mStoreID = storeID;
	mArrayIndex = arrayIndex;
	mFactory = factory;

	initPrefiltered(environment, prefilteredSize, probeRoot, useCache, storeRenderedResult);

	if (true) {
		TextureDesc data;
		data.colorspace = ColorSpace::RGBA;
		data.internalFormat = InternalFormat::RGBA32F;
		data.pixelDataType = PixelDataType::FLOAT;

		auto shOutput = std::make_unique<Texture2D>(9, 1, data, nullptr);

		convoluteSphericalHarmonics(environment, shOutput.get(), 0);

		//glm::vec4 readBackData[9*1 + 5];
		//const auto readBackDataSize = sizeof(readBackData);
		//shOutput->readback(0, ColorSpace::RGBA, PixelDataType::FLOAT, readBackData, readBackDataSize);

		initIrradianceSH(shOutput.get(), probeRoot, useCache, storeRenderedResult);
	}
	else {
		initIrradiance(environment, probeRoot, useCache, storeRenderedResult);
	}

	mMaterial->setProbeFactory(mFactory);
	mMaterial->setArrayIndex(mArrayIndex);

	renderBackend->setViewPort(backup.x, backup.y, backup.width, backup.height);

	mInit = true;
}

void nex::PbrProbe::init(unsigned prefilteredSize, unsigned storeID, PbrProbeFactory * factory, unsigned arrayIndex, const std::filesystem::path & probeRoot, bool useCache, bool storeRenderedResult)
{
	thread_local auto* renderBackend = RenderBackend::get();
	Rectangle backup = renderBackend->getViewport();
	renderBackend->getRasterizer()->enableScissorTest(false);

	mStoreID = storeID;
	mArrayIndex = arrayIndex;
	mFactory = factory;

	std::function<std::shared_ptr<CubeMap>()> renderFunc = []()->std::shared_ptr<CubeMap> {
		throw_with_trace(std::runtime_error("Expected environment map to exist!"));
		return nullptr;
	};

	StoreImage readImage = loadCubeMap(probeRoot,
		"pbr_environmentMap_",
		mStoreID,
		useCache,
		storeRenderedResult,
		renderFunc);

	auto environmentMap = std::shared_ptr<CubeMap>((CubeMap*)Texture::createFromImage(readImage, SOURCE_DATA));

	init(environmentMap.get(), prefilteredSize, storeID, factory, arrayIndex, probeRoot, useCache, storeRenderedResult);


	renderBackend->setViewPort(backup.x, backup.y, backup.width, backup.height);
}

bool nex::PbrProbe::isInitialized() const
{
	return mInit;
}

bool nex::PbrProbe::isSourceStored(const std::filesystem::path& probeRoot) const
{
	if (mStoreID == INVALID_STOREID) return false;

	auto baseName = "pbr_environmentMap_";

	const std::filesystem::path storeFile = probeRoot / (baseName + std::to_string(mStoreID) + std::string(STORE_FILE_EXTENSION));

	return std::filesystem::exists(storeFile);
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
	mTypeName = "Pbr probe vob";
	nex::ProbeVob::setPosition(glm::vec3(mProbe->getPosition()));
}

PbrProbe* ProbeVob::getProbe()
{
	return mProbe;
}

void nex::ProbeVob::setPosition(const glm::vec3 & position)
{
	nex::Vob::setPosition(position);
	mProbe->setPosition(position);
}