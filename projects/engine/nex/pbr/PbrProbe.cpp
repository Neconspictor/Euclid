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
	if (mFreeSlots == 0) {
		throw std::runtime_error(" nex::PbrProbeFactory::initProbe: No free slots!");
	}

	const auto arrayIndex = mMapSize - mFreeSlots;
	--mFreeSlots;

	probe->init(backgroundHDR,
		mPrefilteredSide,
		storeID,
		mIrradianceMaps.get(),
		mPrefilteredMaps.get(),
		arrayIndex,
		mFileSystem->getFirstIncludeDirectory());
}

PbrProbe::PbrProbe() :
	mMaterial(std::make_unique<ProbeMaterial>(mTechnique.get())),
	mIrradianceMaps(nullptr),
	mPrefilteredMaps(nullptr)
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
	desc.minFilter = TextureFilter::Linear_Mipmap_Linear;
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

unsigned nex::PbrProbe::getLayerFaceIndex() const
{
	return mArrayIndex * 6;
}

Material* PbrProbe::getMaterial()
{
	return mMaterial.get();
}

CubeMap * PbrProbe::getConvolutedEnvironmentMap() const
{
	return convolutedEnvironmentMap.get();
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

std::shared_ptr<CubeMap> PbrProbe::createSource(Texture* backgroundHDR, const std::filesystem::path& probeRoot)
{
	thread_local auto* renderBackend = RenderBackend::get();

	Viewport backup = renderBackend->getViewport();
	renderBackend->getRasterizer()->enableScissorTest(false);

	// if environment map has been compiled already and load it from file 

	const std::filesystem::path environmentMapPath = probeRoot / ("pbr_environmentMap_" + std::to_string(mStoreID) + ".probe");
	std::shared_ptr<CubeMap> environmentMap;

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

		environmentMap.reset((CubeMap*)Texture::createFromImage(readImage, data));
	}
	else
	{
		environmentMap = renderBackgroundToCube(backgroundHDR);
		const StoreImage enviromentMapImage = StoreImage::create(environmentMap.get());
		std::cout << "environmentMapPath = " << environmentMapPath << std::endl;
		FileSystem::store(environmentMapPath, enviromentMapImage);
	}

	return environmentMap;
}

void PbrProbe::initPrefiltered(CubeMap* source, unsigned prefilteredSize, const std::filesystem::path& probeRoot)
{
	thread_local auto* renderBackend = RenderBackend::get();

	Viewport backup = renderBackend->getViewport();
	renderBackend->getRasterizer()->enableScissorTest(false);

	// if environment map has been compiled already and load it from file 

	const std::filesystem::path prefilteredMapPath = probeRoot / ("pbr_prefilteredEnvMap_" + std::to_string(mStoreID) + ".probe");

	StoreImage readImage;

	if (std::filesystem::exists(prefilteredMapPath))
	{
		
		FileSystem::load(prefilteredMapPath, readImage);

		TextureData data = PREFILTERED_DATA;
		data.generateMipMaps = false; // We set the mipmaps manually from store

		prefilteredEnvMap.reset((CubeMap*)Texture::createFromImage(readImage, data));
	}
	else
	{
		prefilteredEnvMap = prefilter(source, prefilteredSize);
		readImage = StoreImage::create(prefilteredEnvMap.get());
		FileSystem::store(prefilteredMapPath, readImage);
	}

	for (unsigned mipmap = 0; mipmap < readImage.mipmapCount; ++mipmap) {
		for (unsigned i = 0; i < readImage.images.size(); ++i) {

			const auto& image = readImage.images[i][mipmap];

			mPrefilteredMaps->fill(0, 0, getLayerFaceIndex() + i,
				image.width, image.height, 1, mipmap, image.pixels.getPixels());
		}
	}
}

void PbrProbe::initIrradiance(CubeMap* source, const std::filesystem::path& probeRoot)
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

		convolutedEnvironmentMap.reset((CubeMap*)Texture::createFromImage(readImage, data));
	}
	else
	{
		convolutedEnvironmentMap = convolute(source);
		const StoreImage convolutedMapImage = StoreImage::create(convolutedEnvironmentMap.get());
		FileSystem::store(convolutedMapPath, convolutedMapImage);
	}
}

void PbrProbe::init(Texture* backgroundHDR,
				unsigned prefilteredSize, unsigned storeID, 
				CubeMapArray* irradianceMaps, CubeMapArray* prefilteredMaps, unsigned arrayIndex,
				const std::filesystem::path& probeRoot)
{
	if (irradianceMaps == nullptr || prefilteredMaps == nullptr)
		throw std::invalid_argument("PbrProbe::init: irradianceMaps or prefilteredMaps is null!");


	thread_local auto* renderBackend = RenderBackend::get();
	Viewport backup = renderBackend->getViewport();

	mStoreID = storeID;
	mArrayIndex = arrayIndex;
	mIrradianceMaps = irradianceMaps;
	mPrefilteredMaps = prefilteredMaps;

	auto source = createSource(backgroundHDR, probeRoot);
	initPrefiltered(source.get(), prefilteredSize, probeRoot);
	initIrradiance(source.get(), probeRoot);

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