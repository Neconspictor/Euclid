#include <nex/pbr/GlobalIllumination.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nex/resource/FileSystem.hpp>
#include <nex/Scene.hpp>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <glm/glm.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>
#include <list>
#include <nex/Scene.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Attachment.hpp>
#include <nex/shader/Pass.hpp>
#include <nex/pbr/PbrPass.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/pbr/PbrDeferred.hpp>
#include <nex/pbr/PbrForward.hpp>
#include <nex/renderer/Renderer.hpp>
#include <nex/pbr/Cluster.hpp>
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/EffectLibrary.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/pbr/IrradianceSphereHullDrawPass.hpp>

const unsigned nex::GlobalIllumination::VOXEL_BASE_SIZE = 128;

class nex::GlobalIllumination::ProbeBakePass : public PbrGeometryPass 
{
public:

	ProbeBakePass() :
		PbrGeometryPass(Shader::create("pbr/probe/pbr_probe_capture_vs.glsl", "pbr/probe/pbr_probe_capture_fs.glsl", nullptr, nullptr, nullptr, generateDefines()),
			TRANSFORM_BUFFER_BINDINGPOINT)
	{
		mEyeLightDirection = { mShader->getUniformLocation("dirLight.directionEye"), UniformType::VEC3 };
		mLightColor = { mShader->getUniformLocation("dirLight.color"), UniformType::VEC3 };
		mLightPower = { mShader->getUniformLocation("dirLight.power"), UniformType::FLOAT };
		mInverseView = { mShader->getUniformLocation("inverseViewMatrix"), UniformType::MAT4 };
	}

	void setEyeLightDirection(const glm::vec3& dir) {
		mShader->setVec3(mEyeLightDirection.location, dir);
	}

	void setLightColor(const glm::vec3& color) {
		mShader->setVec3(mLightColor.location, color);
	}

	void setLightPower(float power) {
		mShader->setFloat(mLightPower.location, power);
	}

	void setInverseViewMatrix(const glm::mat4& mat) {
		mShader->setMat4(mInverseView.location, mat);
	}

	void updateConstants(const DirLight& light, const glm::mat4& projection, const glm::mat4& view) {
		setLightColor(light.color);
		setLightPower(light.power);

		glm::vec4 lightEyeDirection = view * glm::vec4(-light.directionWorld, 0);
		setEyeLightDirection(glm::vec3(lightEyeDirection));

		setInverseViewMatrix(inverse(view));
		setViewProjectionMatrices(projection, view, view);
	}

private:

	static std::vector<std::string> generateDefines() {
		auto vec = std::vector<std::string>();

		// csm CascadeBuffer and TransformBuffer both use binding point 0 per default. Resolve this conflict.
		vec.push_back(std::string("#define PBR_COMMON_GEOMETRY_TRANSFORM_BUFFER_BINDING_POINT ") + std::to_string(TRANSFORM_BUFFER_BINDINGPOINT));
		vec.push_back(std::string("#define CSM_CASCADE_BUFFER_BINDING_POINT ") + std::to_string(CASCADE_BUFFER_BINDINGPOINT));

		return vec;
	}


	static constexpr unsigned TRANSFORM_BUFFER_BINDINGPOINT = 0;
	static constexpr unsigned CASCADE_BUFFER_BINDINGPOINT = 1;

	Uniform mEyeLightDirection;
	Uniform mLightColor;
	Uniform mLightPower;
	Uniform mInverseView;

};

class nex::GlobalIllumination::VoxelizePass : public PbrGeometryPass
{
public:

	struct VoxelType {
		glm::uint colorMask;
		glm::uint normalMask;
	};

	struct Constants {
		float       g_xFrame_VoxelRadianceDataSize;				// voxel half-extent in world space units
		float       g_xFrame_VoxelRadianceDataSize_rcp;			// 1.0 / voxel-half extent +++
		glm::uint	g_xFrame_VoxelRadianceDataRes;				// voxel grid resolution +++
		float		g_xFrame_VoxelRadianceDataRes_rcp;			// 1.0 / voxel grid resolution +++

		glm::uint	g_xFrame_VoxelRadianceDataMIPs;				// voxel grid mipmap count
		glm::uint	g_xFrame_VoxelRadianceNumCones;				// number of diffuse cones to trace
		float		g_xFrame_VoxelRadianceNumCones_rcp;			// 1.0 / number of diffuse cones to trace
		float		g_xFrame_VoxelRadianceRayStepSize;			// raymarch step size in voxel space units

		glm::vec4	g_xFrame_VoxelRadianceDataCenter;			// center of the voxel grid in world space units +++
		glm::uint	g_xFrame_VoxelRadianceReflectionsEnabled;	// are voxel gi reflections enabled or not
	};

	VoxelizePass() :
		PbrGeometryPass(Shader::create("GI/voxelize_vs.glsl", "GI/voxelize_fs.glsl", nullptr, nullptr, "GI/voxelize_gs.glsl", generateDefines()),
			TRANSFORM_BUFFER_BINDINGPOINT)
	{
		mWorldLightDirection = { mShader->getUniformLocation("dirLight.directionWorld"), UniformType::VEC3 };
		mLightColor = { mShader->getUniformLocation("dirLight.color"), UniformType::VEC3 };
		mLightPower = { mShader->getUniformLocation("dirLight.power"), UniformType::FLOAT };
		mCascadedDephMap = mShader->createTextureUniform("cascadedDepthMap", UniformType::TEXTURE2D_ARRAY, CSM_CASCADE_DEPTH_MAP_BINDING_POINT);

		mSampler.setMinFilter(TextureFilter::NearestNeighbor);
		mSampler.setMagFilter(TextureFilter::NearestNeighbor);
	}

	void setLightDirectionWS(const glm::vec3& dir) {
		mShader->setVec3(mWorldLightDirection.location, -dir);
	}

	void setLightColor(const glm::vec3& color) {
		mShader->setVec3(mLightColor.location, color);
	}

	void setLightPower(float power) {
		mShader->setFloat(mLightPower.location, power);
	}

	void useShadow(CascadedShadow* shadow) {
		mShader->setTexture(shadow->getDepthTextureArray(), &mSampler, mCascadedDephMap.bindingSlot);
		shadow->getCascadeBuffer()->bindToTarget(CASCADE_BUFFER_BINDINGPOINT);
	}

	void updateLight(const DirLight& light) {
		setLightColor(light.color);
		setLightPower(light.power);
		setLightDirectionWS(light.directionWorld);
	}

	void useConstantBuffer(UniformBuffer* buffer) {
		buffer->bindToTarget(C_UNIFORM_BUFFER_BINDING_POINT);
	}

	void useVoxelBuffer(ShaderStorageBuffer* buffer) {
		buffer->bindToTarget(VOXEL_BUFFER_BINDING_POINT);
	}



private:

	static std::vector<std::string> generateDefines() {
		auto vec = std::vector<std::string>();

		// csm CascadeBuffer and TransformBuffer both use binding point 0 per default. Resolve this conflict.
		vec.push_back(std::string("#define PBR_COMMON_GEOMETRY_TRANSFORM_BUFFER_BINDING_POINT ") + std::to_string(TRANSFORM_BUFFER_BINDINGPOINT));
		vec.push_back(std::string("#define C_UNIFORM_BUFFER_BINDING_POINT ") + std::to_string(C_UNIFORM_BUFFER_BINDING_POINT));
		vec.push_back(std::string("#define VOXEL_BUFFER_BINDING_POINT ") + std::to_string(VOXEL_BUFFER_BINDING_POINT));
		vec.push_back(std::string("#define CSM_CASCADE_BUFFER_BINDING_POINT ") + std::to_string(CASCADE_BUFFER_BINDINGPOINT));

		return vec;
	}


	static constexpr unsigned TRANSFORM_BUFFER_BINDINGPOINT = 0;
	static constexpr unsigned C_UNIFORM_BUFFER_BINDING_POINT = 0;
	static constexpr unsigned VOXEL_BUFFER_BINDING_POINT = 1;
	static constexpr unsigned CASCADE_BUFFER_BINDINGPOINT = 2;
	static constexpr unsigned CSM_CASCADE_DEPTH_MAP_BINDING_POINT = 5;

	Uniform mWorldLightDirection;
	Uniform mLightColor;
	Uniform mLightPower;
	Uniform mInverseView;
	UniformTex mCascadedDephMap;
};

class nex::GlobalIllumination::VoxelVisualizePass : public Pass
{
public:

	VoxelVisualizePass() :
		Pass(Shader::create("GI/voxel_visualize_vs.glsl", "GI/voxel_visualize_fs.glsl", nullptr, nullptr, 
			"GI/voxel_visualize_gs.glsl", generateDefines()))
	{
		mViewProj = { mShader->getUniformLocation("viewProj"), UniformType::MAT4 };
		mMipMap = { mShader->getUniformLocation("mipMap"), UniformType::FLOAT };
		mVoxelImage = mShader->createTextureUniform("voxelImage", UniformType::IMAGE3D, 0);
		mSampler.setMinFilter(TextureFilter::Near_Mipmap_Near);
		mSampler.setMagFilter(TextureFilter::NearestNeighbor);
		mSampler.setBorderColor(glm::vec4(0.0));
		mSampler.setWrapR(TextureUVTechnique::ClampToBorder);
		mSampler.setWrapS(TextureUVTechnique::ClampToBorder);
		mSampler.setWrapT(TextureUVTechnique::ClampToBorder);
	}

	void setViewProjection(const glm::mat4& mat) {
		mShader->setMat4(mViewProj.location, mat);
	}

	void setMipMap(int mipMap) {
		mShader->setFloat(mMipMap.location, mipMap);
	}

	void useConstantBuffer(UniformBuffer* buffer) {
		buffer->bindToTarget(C_UNIFORM_BUFFER_BINDING_POINT);
	}

	void useVoxelBuffer(ShaderStorageBuffer* buffer) {
		buffer->bindToTarget(VOXEL_BUFFER_BINDING_POINT);
	}

	void useVoxelTexture(Texture3D* texture) {
		mShader->setTexture(texture, &mSampler, 0);
		return;
		mShader->setImageLayerOfTexture(mVoxelImage.location,
			texture, mVoxelImage.bindingSlot,
			TextureAccess::READ_WRITE,
			InternFormat::RGBA32F,
			0,
			true,
			0);
	}

private:

	static std::vector<std::string> generateDefines() {
		auto vec = std::vector<std::string>();

		vec.push_back(std::string("#define C_UNIFORM_BUFFER_BINDING_POINT ") + std::to_string(C_UNIFORM_BUFFER_BINDING_POINT));
		vec.push_back(std::string("#define VOXEL_BUFFER_BINDING_POINT ") + std::to_string(VOXEL_BUFFER_BINDING_POINT));

		return vec;
	}

	static constexpr unsigned C_UNIFORM_BUFFER_BINDING_POINT = 0;
	static constexpr unsigned VOXEL_BUFFER_BINDING_POINT = 0;

	Uniform mViewProj;
	Uniform mMipMap;
	UniformTex mVoxelImage;
};


class nex::GlobalIllumination::VoxelFillComputeLightPass : public ComputePass
{
public:

	VoxelFillComputeLightPass(unsigned localSizeX) :
		ComputePass(Shader::createComputeShader("GI/copy_to_texture_calc_light_cs.glsl", generateDefines(localSizeX)))
	{
		mVoxelImage = mShader->createTextureUniform("voxelImage", UniformType::IMAGE3D, VOXEL_IMAGE_BINDING_POINT);
	}

	void useConstantBuffer(UniformBuffer* buffer) {
		buffer->bindToTarget(C_UNIFORM_BUFFER_BINDING_POINT);
	}

	void useVoxelBuffer(ShaderStorageBuffer* buffer) {
		buffer->bindToTarget(VOXEL_BUFFER_BINDING_POINT);
	}

	void setVoxelOutputImage(Texture3D* voxelImage) {
		mShader->setImageLayerOfTexture(mVoxelImage.location,
			voxelImage, mVoxelImage.bindingSlot,
			TextureAccess::READ_WRITE,
			InternFormat::RGBA32F,
			0,
			true,
			0);
	}

private:

	static std::vector<std::string> generateDefines(unsigned localSizeX) {
		auto vec = std::vector<std::string>();

		vec.push_back(std::string("#define C_UNIFORM_BUFFER_BINDING_POINT ") + std::to_string(C_UNIFORM_BUFFER_BINDING_POINT));
		vec.push_back(std::string("#define VOXEL_BUFFER_BINDING_POINT ") + std::to_string(VOXEL_BUFFER_BINDING_POINT));
		vec.push_back(std::string("#define LOCAL_SIZE_X ") + std::to_string(localSizeX));

		return vec;
	}

	static constexpr unsigned C_UNIFORM_BUFFER_BINDING_POINT = 0;
	static constexpr unsigned VOXEL_BUFFER_BINDING_POINT = 0;
	static constexpr unsigned VOXEL_IMAGE_BINDING_POINT = 0;

	UniformTex mVoxelImage;
};


class nex::GlobalIllumination::MipMapTexture3DPass : public ComputePass
{
public:

	MipMapTexture3DPass() :
		ComputePass(Shader::createComputeShader("GI/mipmap_texture3d_box_filter_cs.glsl"))
	{
		mSourceImage = mShader->createTextureUniform("source", UniformType::IMAGE3D, SOURCE_BINDING_POINT);
		mDestImage = mShader->createTextureUniform("dest", UniformType::IMAGE3D, DEST_BINDING_POINT);
	}

	void setInputImage(Texture3D* texture, unsigned mipMap) {
		mShader->setImageLayerOfTexture(mSourceImage.location,
			texture, mSourceImage.bindingSlot,
			TextureAccess::READ_ONLY,
			InternFormat::RGBA32F,
			mipMap,
			true,
			0);
	}

	void setOutputImage(Texture3D* texture, unsigned mipMap) {
		mShader->setImageLayerOfTexture(mDestImage.location,
			texture, mDestImage.bindingSlot,
			TextureAccess::WRITE_ONLY,
			InternFormat::RGBA32F,
			mipMap,
			true,
			0);
	}

private:

	static constexpr unsigned SOURCE_BINDING_POINT = 0;
	static constexpr unsigned DEST_BINDING_POINT = 1;

	UniformTex mSourceImage;
	UniformTex mDestImage;
};


nex::GlobalIllumination::GlobalIllumination(const std::string& compiledProbeDirectory, unsigned prefilteredSize, unsigned depth) :
mFactory(prefilteredSize, depth),
mEnvironmentLights(0, 0, nullptr, ShaderBuffer::UsageHint::DYNAMIC_COPY),
mProbeBakePass(std::make_unique<ProbeBakePass>()), 
mVoxelizePass(std::make_unique<VoxelizePass>()),
mVoxelFillComputeLightPass(std::make_unique<VoxelFillComputeLightPass>(VOXEL_BASE_SIZE)),
mMipMapTexture3DPass(std::make_unique<MipMapTexture3DPass>()),
mVoxelVisualizePass(std::make_unique<VoxelVisualizePass>()),
mAmbientLightPower(1.0f),
mNextStoreID(0),
mProbeCluster(std::make_unique<ProbeCluster>()),
mVoxelBuffer(0, sizeof(VoxelizePass::VoxelType) * VOXEL_BASE_SIZE * VOXEL_BASE_SIZE * VOXEL_BASE_SIZE, nullptr, ShaderBuffer::UsageHint::DYNAMIC_COPY),
mVoxelConstantBuffer(0, sizeof(VoxelizePass::Constants), nullptr, GpuBuffer::UsageHint::DYNAMIC_DRAW),
mVisualize(false),
mVoxelVisualizeMipMap(0),
mUseConeTracing(true)
{
	auto deferredGeometryPass = std::make_unique<PbrDeferredGeometryPass>(Shader::create(
		"pbr/pbr_deferred_geometry_pass_vs.glsl",
		"pbr/pbr_deferred_geometry_pass_fs.glsl"));


	PbrDeferred::LightingPassFactory deferredFactory = [](CascadedShadow* c, GlobalIllumination* g) {
		return std::make_unique<PbrDeferredLightingPass>(
			"pbr/probe/pbr_probe_deferred_capture_vs.glsl",
			"pbr/probe/pbr_probe_deferred_capture_fs.glsl",
			g,
			c);
	};

	PbrForward::LightingPassFactory forwardFactory = [](CascadedShadow* c, GlobalIllumination* g) {
		return std::make_unique<PbrForwardPass>(
			"pbr/probe/pbr_probe_capture_vs.glsl",
			"pbr/probe/pbr_probe_capture_fs.glsl",
			g,
			c);
	};

	mDeferred = std::make_unique<PbrDeferred>(std::move(deferredGeometryPass),
		std::move(deferredFactory),
		nullptr,
		nullptr,
		nullptr);

	mForward = std::make_unique<PbrForward>(std::move(forwardFactory),
		nullptr,
		nullptr,
		nullptr);

	mIrradianceDepthPass = std::make_unique<TransformPass>(Shader::create("pbr/probe/irradiance_depth_pass_vs.glsl", 
		"pbr/probe/irradiance_depth_pass_fs.glsl"));

	mSphere = std::make_unique<StaticMeshContainer>();
	AABB box = {glm::vec3(-10, -10, -10), glm::vec3(10, 10, 10)};
	auto sphere = std::make_unique<SphereMesh>(16, 16);
	//auto sphere = std::make_unique<MeshAABB>(box, Topology::TRIANGLES);
	//sphere->finalize();
	auto material = std::make_unique<Material>(RenderBackend::get()->getEffectLibrary()->getIrradianceSphereHullDrawTechnique());

	mSphere->addMapping(sphere.get(), material.get());
	mSphere->add(std::move(sphere));
	mSphere->addMaterial(std::move(material));

	// Note: we have to generate mipmaps since memory can only be allocated during texture creation.
	auto data = TextureDesc::createRenderTargetRGBAHDR(InternFormat::RGBA32F, true);
	data.minFilter = TextureFilter::Linear_Mipmap_Linear; //Linear_Mipmap_Linear TODO: which filtering is better?
	data.magFilter = TextureFilter::Linear; //Linear
	data.wrapR = data.wrapS = data.wrapT = TextureUVTechnique::ClampToBorder;
	data.borderColor = glm::vec4(0.0);
	data.pixelDataType = PixelDataType::FLOAT;
	mVoxelTexture = std::make_unique<Texture3D>(VOXEL_BASE_SIZE, VOXEL_BASE_SIZE, VOXEL_BASE_SIZE, data, nullptr);
}

nex::GlobalIllumination::~GlobalIllumination() = default;

void nex::GlobalIllumination::bakeProbes(const Scene & scene, Renderer* renderer)
{
	const size_t size = 1024;

	PerspectiveCamera camera(1.0f, glm::radians(90.0f), 0.1f, 100.0f);
	//OrthographicCamera camera(1.0f, 1.0f, 0.1f, 100.0f);
	
	camera.update();

	TextureDesc data;
	data.colorspace = ColorSpace::RGB;
	data.internalFormat = InternFormat::RGB32F;
	data.pixelDataType = PixelDataType::FLOAT;
	data.minFilter = TextureFilter::Linear;
	data.magFilter = TextureFilter::Linear;
	data.generateMipMaps = false;

	auto renderTarget = std::make_unique<nex::CubeRenderTarget>(size, size, data);

	RenderAttachment depth;
	depth.target = TextureTarget::TEXTURE2D;
	data = TextureDesc();
	data.minFilter = TextureFilter::Linear;
	data.magFilter = TextureFilter::Linear;
	data.generateMipMaps = false;
	data.colorspace = ColorSpace::DEPTH_STENCIL;
	data.internalFormat = InternFormat::DEPTH24_STENCIL8;
	depth.texture = std::make_unique<RenderBuffer>(size, size, data);

	depth.type = RenderAttachmentType::DEPTH_STENCIL;

	renderTarget->useDepthAttachment(std::move(depth));
	renderTarget->updateDepthAttachment();


	TextureDesc dataDepth;
	dataDepth.colorspace = ColorSpace::RGBA;
	dataDepth.internalFormat = InternFormat::RGBA32F;
	dataDepth.pixelDataType = PixelDataType::FLOAT;
	dataDepth.minFilter = TextureFilter::Linear_Mipmap_Linear;
	dataDepth.magFilter = TextureFilter::Linear;
	dataDepth.generateMipMaps = false;

	auto renderTargetDepth = std::make_unique<nex::CubeRenderTarget>(PbrProbe::SOURCE_CUBE_SIZE, PbrProbe::SOURCE_CUBE_SIZE, dataDepth);

	RenderAttachment depthDepth;
	depthDepth.target = TextureTarget::TEXTURE2D;
	dataDepth = TextureDesc();
	dataDepth.minFilter = TextureFilter::Linear;
	dataDepth.magFilter = TextureFilter::Linear;
	dataDepth.generateMipMaps = false;
	dataDepth.colorspace = ColorSpace::DEPTH_STENCIL;
	dataDepth.internalFormat = InternFormat::DEPTH24_STENCIL8;
	depthDepth.texture = std::make_unique<RenderBuffer>(PbrProbe::SOURCE_CUBE_SIZE, PbrProbe::SOURCE_CUBE_SIZE, dataDepth);

	depthDepth.type = RenderAttachmentType::DEPTH;

	renderTargetDepth->useDepthAttachment(std::move(depthDepth));
	renderTargetDepth->updateDepthAttachment();



	DirLight light;
	light.color = glm::vec3(1.0f, 1.0f, 1.0f);
	light.power = 3.0f;
	light.directionWorld = { -1,-1,-1 };


	mDeferred->setDirLight(&light);
	mForward->setDirLight(&light);

	auto* pbrTechnique = renderer->getPbrTechnique();
	pbrTechnique->overrideForward(mForward.get());
	pbrTechnique->overrideDeferred(mDeferred.get());

	PbrProbe backgroundProbe(glm::vec3(0, 0, 0), 2);
	TextureDesc backgroundHDRData;
	backgroundHDRData.pixelDataType = PixelDataType::FLOAT;
	backgroundHDRData.internalFormat = InternFormat::RGB32F;
	auto* backgroundHDR = TextureManager::get()->getImage("hdr/HDR_Free_City_Night_Lights_Ref.hdr", backgroundHDRData, true);
	mFactory.initProbeBackground(backgroundProbe, backgroundHDR, 2, false, false);

	for (auto* probeVob : scene.getActiveProbeVobsUnsafe()) { //const auto& spatial : mProbeSpatials

		auto& probe = *probeVob->getProbe();
		if (probe.isInitialized()) continue;

		const auto& position = probe.getPosition();

		const auto storeID = probe.getStoreID();

		//if (storeID == 0) continue;

		if (probe.isSourceStored(mFactory.getProbeRootDir())) {
			mFactory.initProbe(probe, storeID, true, true);
		}
		else {
			RenderCommandQueue commandQueue;
			commandQueue.useSphereCulling(position, camera.getFarDistance());
			collectBakeCommands(commandQueue, scene, false);
			commandQueue.getProbeCommands().clear();
			commandQueue.getToolCommands().clear();
			commandQueue.sort();

			renderer->updateRenderTargets(size, size);
			auto cubeMap = renderToCubeMap(commandQueue, renderer, *renderTarget, camera, position, light);

			renderer->updateRenderTargets(PbrProbe::SOURCE_CUBE_SIZE, PbrProbe::SOURCE_CUBE_SIZE);
			//auto cubeMapDepth = renderToDepthCubeMap(commandQueue, renderer, *renderTargetDepth, camera, position, light);
			mFactory.initProbe(probe, cubeMap.get(), storeID, false, false);
		}

		probeVob->mDebugName = "pbr probe " + std::to_string(probe.getArrayIndex()) + ", " + std::to_string(probe.getStoreID());
	}

	pbrTechnique->overrideForward(nullptr);
	pbrTechnique->overrideDeferred(nullptr);
}


void nex::GlobalIllumination::bakeProbe(ProbeVob* probeVob, const Scene& scene, Renderer* renderer)
{
	const size_t size = 1024;

	PerspectiveCamera camera(1.0f, glm::radians(90.0f), 0.1f, 100.0f);
	//OrthographicCamera camera(1.0f, 1.0f, 0.1f, 100.0f);

	camera.update();

	TextureDesc data;
	data.colorspace = ColorSpace::RGB;
	data.internalFormat = InternFormat::RGB32F;
	data.pixelDataType = PixelDataType::FLOAT;
	data.minFilter = TextureFilter::Linear;
	data.magFilter = TextureFilter::Linear;
	data.generateMipMaps = false;

	auto renderTarget = std::make_unique<nex::CubeRenderTarget>(size, size, data);

	RenderAttachment depth;
	depth.target = TextureTarget::TEXTURE2D;
	data = TextureDesc();
	data.minFilter = TextureFilter::Linear;
	data.magFilter = TextureFilter::Linear;
	data.generateMipMaps = false;
	data.colorspace = ColorSpace::DEPTH_STENCIL;
	data.internalFormat = InternFormat::DEPTH24_STENCIL8;
	depth.texture = std::make_unique<RenderBuffer>(size, size, data);

	depth.type = RenderAttachmentType::DEPTH_STENCIL;

	renderTarget->useDepthAttachment(std::move(depth));
	renderTarget->updateDepthAttachment();

	DirLight light;
	light.color = glm::vec3(1.0f, 1.0f, 1.0f);
	light.power = 3.0f;
	light.directionWorld = { -1,-1,-1 };


	mDeferred->setDirLight(&light);
	mForward->setDirLight(&light);

	auto* pbrTechnique = renderer->getPbrTechnique();
	pbrTechnique->overrideForward(mForward.get());
	pbrTechnique->overrideDeferred(mDeferred.get());

	auto width = renderer->getWidth();
	auto height = renderer->getHeight();
	renderer->updateRenderTargets(size, size);

	auto& probe = *probeVob->getProbe();
	const auto& position = probe.getPosition();

	if (probe.isSourceStored(mFactory.getProbeRootDir())) {
		mFactory.initProbe(probe, probe.getStoreID(), true, true);
	}
	else {
		RenderCommandQueue commandQueue;
		commandQueue.useSphereCulling(position, camera.getFarDistance());
		collectBakeCommands(commandQueue, scene, false);
		commandQueue.getProbeCommands().clear();
		commandQueue.getToolCommands().clear();
		commandQueue.sort();

		auto cubeMap = renderToCubeMap(commandQueue, renderer, *renderTarget, camera, position, light);
		mFactory.initProbe(probe, cubeMap.get(), probe.getStoreID(), false, false);
	}

	probeVob->mDebugName = "pbr probe " + std::to_string(probe.getArrayIndex()) + ", " + std::to_string(probe.getStoreID());

	pbrTechnique->overrideForward(nullptr);
	pbrTechnique->overrideDeferred(nullptr);


	renderer->updateRenderTargets(width, height);
}

const std::vector<std::unique_ptr<nex::PbrProbe>>& nex::GlobalIllumination::getProbes() const
{
	return mProbes;
}

nex::ProbeCluster* nex::GlobalIllumination::getProbeCluster()
{
	return mProbeCluster.get();
}

const nex::ProbeCluster* nex::GlobalIllumination::getProbeCluster() const
{
	return mProbeCluster.get();
}

bool nex::GlobalIllumination::getVisualize() const
{
	return mVisualize;
}

nex::ProbeVob* nex::GlobalIllumination::addUninitProbeUnsafe(const glm::vec3& position, unsigned storeID)
{
	advanceNextStoreID(storeID);

	auto probe = std::make_unique<PbrProbe>(position, storeID);

	auto* meshRootNode = StaticMesh::createNodeHierarchy(
		{ std::pair<Mesh*, Material*>(PbrProbe::getSphere(), probe->getMaterial()) });


	auto* probVobPtr = new ProbeVob(meshRootNode, probe.get());
	auto vob = std::unique_ptr<ProbeVob>(probVobPtr);

	mProbes.emplace_back(std::move(probe));
	mProbeVobs.emplace_back(std::move(vob));

	return mProbeVobs.back().get();
}

nex::PbrProbe * nex::GlobalIllumination::getActiveProbe()
{
	return mActive;
}

float nex::GlobalIllumination::getAmbientPower() const
{
	return mAmbientLightPower;
}

nex::CubeMapArray * nex::GlobalIllumination::getIrradianceMaps()
{
	return mFactory.getIrradianceMaps();
}

unsigned nex::GlobalIllumination::getNextStoreID() const
{
	return mNextStoreID;
}

nex::CubeMapArray * nex::GlobalIllumination::getPrefilteredMaps()
{
	return mFactory.getPrefilteredMaps();
}

nex::PbrProbeFactory* nex::GlobalIllumination::getFactory()
{
	return &mFactory;
}

nex::ShaderStorageBuffer* nex::GlobalIllumination::getEnvironmentLightShaderBuffer()
{
	return &mEnvironmentLights;
}

nex::UniformBuffer* nex::GlobalIllumination::getVoxelConstants()
{
	return &mVoxelConstantBuffer;
}

nex::Texture3D* nex::GlobalIllumination::getVoxelTexture()
{
	return mVoxelTexture.get();
}

bool nex::GlobalIllumination::isConeTracingUsed() const
{
	return mUseConeTracing;
}

void nex::GlobalIllumination::setActiveProbe(PbrProbe * probe)
{
	mActive = probe;
}

void nex::GlobalIllumination::setAmbientPower(float ambientPower)
{
	mAmbientLightPower = ambientPower;
}

void nex::GlobalIllumination::setUseConetracing(bool use)
{
	mUseConeTracing = use;
}

void nex::GlobalIllumination::setVisualize(bool visualize, int mipMapLevel)
{
	mVisualize = visualize;
	mVoxelVisualizeMipMap = mipMapLevel;
}

void nex::GlobalIllumination::update(const nex::Scene::ProbeRange & activeProbes)
{
	const auto byteSize = activeProbes.size() * sizeof(EnvironmentLight);

	if (mEnvironmentLights.getSize() < byteSize)
		mEnvironmentLights.resize(byteSize,nullptr, GpuBuffer::UsageHint::DYNAMIC_COPY);

	std::vector<EnvironmentLight> lights(activeProbes.size());

	size_t counter = 0;
	for (auto* vob : activeProbes) {
		auto& light = lights[counter];
		auto* probe = vob->getProbe();
		++counter;
		
		const auto& trafo = vob->getMeshRootNode()->getWorldTrafo();
		light.enabled = true;
		light.position = glm::vec4(trafo[3][0], trafo[3][1], trafo[3][2], 1.0f);
		light.sphereRange = probe->getInfluenceRadius();
		const auto& box = probe->getInfluenceBox();
		light.minWorld = glm::vec4(box.min, 0.0);
		light.maxWorld = glm::vec4(box.max, 0.0);
		light.usesBoundingBox = probe->getInfluenceType() == PbrProbe::InfluenceType::BOX;
		light.arrayIndex = probe->getArrayIndex();
	}

	mEnvironmentLights.update(byteSize, lights.data());
}

void nex::GlobalIllumination::renderVoxels(const glm::mat4& projection, const glm::mat4& view)
{
	mVoxelVisualizePass->bind();
	mVoxelVisualizePass->setViewProjection(projection * view);
	mVoxelVisualizePass->setMipMap(mVoxelVisualizeMipMap);
	mVoxelVisualizePass->useConstantBuffer(&mVoxelConstantBuffer);
	mVoxelVisualizePass->useVoxelBuffer(&mVoxelBuffer);
	mVoxelVisualizePass->useVoxelTexture(mVoxelTexture.get());

	VertexBuffer::unbindAny(); //Make sure we don't use 'zero' data
	RenderBackend::get()->drawArray(RenderState(), nex::Topology::POINTS, 0, 
								VOXEL_BASE_SIZE * VOXEL_BASE_SIZE * VOXEL_BASE_SIZE);
}

void nex::GlobalIllumination::voxelize(const nex::RenderCommandQueue::ConstBufferCollection& collection, 
	const AABB& sceneBoundingBox, const DirLight& light, CascadedShadow* shadows)
{
	VoxelizePass::Constants constants;
	auto diff = sceneBoundingBox.max - sceneBoundingBox.min;
	auto voxelExtent = std::max<float>({ diff.x / (float)VOXEL_BASE_SIZE, diff.y / (float)VOXEL_BASE_SIZE, diff.z / (float)VOXEL_BASE_SIZE });
	
	constants.g_xFrame_VoxelRadianceDataSize = voxelExtent / 2.0f;
	//constants.g_xFrame_VoxelRadianceDataSize = 0.125;
	constants.g_xFrame_VoxelRadianceDataSize_rcp = 1.0f / constants.g_xFrame_VoxelRadianceDataSize;
	constants.g_xFrame_VoxelRadianceDataRes = VOXEL_BASE_SIZE;
	constants.g_xFrame_VoxelRadianceDataRes_rcp = 1.0f / (float)constants.g_xFrame_VoxelRadianceDataRes;
	constants.g_xFrame_VoxelRadianceDataCenter = glm::vec4((sceneBoundingBox.max + sceneBoundingBox.min) / 2.0f, 1.0);
	constants.g_xFrame_VoxelRadianceNumCones = 1;
	constants.g_xFrame_VoxelRadianceNumCones_rcp = 1.0f / float(constants.g_xFrame_VoxelRadianceNumCones);
	constants.g_xFrame_VoxelRadianceDataMIPs = Texture::getMipMapCount(mVoxelTexture->getWidth());
	constants.g_xFrame_VoxelRadianceRayStepSize = 0.7f;
	mVoxelConstantBuffer.update(sizeof(VoxelizePass::Constants), &constants);


	auto* renderTarget = RenderBackend::get()->getDefaultRenderTarget();
	renderTarget->bind();
	renderTarget->enableDrawToColorAttachments(false);
	auto viewPort = RenderBackend::get()->getViewport();
	constexpr float scale = 2;
	RenderBackend::get()->setViewPort(0,0, VOXEL_BASE_SIZE * scale, VOXEL_BASE_SIZE * scale);
	

	mVoxelizePass->bind();
	mVoxelizePass->updateLight(light);
	mVoxelizePass->useConstantBuffer(&mVoxelConstantBuffer);
	mVoxelizePass->useVoxelBuffer(&mVoxelBuffer);
	mVoxelizePass->useShadow(shadows);
	

	RenderState state;

	for (auto* commands : collection) {

		for (auto& command : *commands) {
			mVoxelizePass->setModelMatrix(command.worldTrafo, command.prevWorldTrafo);
			mVoxelizePass->uploadTransformMatrices();
			auto state = command.material->getRenderState();
			state.doDepthTest = false;
			state.doDepthWrite = false;
			state.doCullFaces = false;
			StaticMeshDrawer::draw(command.mesh, command.material, &state);
		}
	}

	renderTarget->enableDrawToColorAttachments(true);

	RenderBackend::get()->setViewPort(viewPort.x, viewPort.y, viewPort.width, viewPort.height);

	/*mVoxelBuffer.syncWithGPU();
	auto* data = (VoxelizePass::VoxelType*)mVoxelBuffer.map(GpuBuffer::Access::READ_ONLY);

	mVoxelBuffer.unmap();

	auto* constantsBufferMemory = (VoxelizePass::Constants*)mVoxelConstantBuffer.map(GpuBuffer::Access::READ_ONLY);

	mVoxelConstantBuffer.unmap();*/

	mVoxelFillComputeLightPass->bind();
	mVoxelFillComputeLightPass->setVoxelOutputImage(mVoxelTexture.get());
	mVoxelFillComputeLightPass->useConstantBuffer(&mVoxelConstantBuffer);
	mVoxelFillComputeLightPass->useVoxelBuffer(&mVoxelBuffer);
	mVoxelFillComputeLightPass->dispatch(VOXEL_BASE_SIZE, VOXEL_BASE_SIZE, VOXEL_BASE_SIZE);

	
	mMipMapTexture3DPass->bind();

	const int lastMipMapIndex = Texture::getMipMapCount(mVoxelTexture->getWidth()) - 1;

	for (int i = 0; i < lastMipMapIndex; ++i) {
		const auto mipMapSize = mVoxelTexture->getWidth() / std::pow(2, i);
		mMipMapTexture3DPass->setInputImage(mVoxelTexture.get(), i);
		mMipMapTexture3DPass->setOutputImage(mVoxelTexture.get(), i+1); 
		mMipMapTexture3DPass->dispatch(mipMapSize, mipMapSize, mipMapSize);
	}

	//mVoxelTexture->generateMipMaps();

	/*std::vector<glm::vec4> dest(VOXEL_BASE_SIZE * VOXEL_BASE_SIZE * VOXEL_BASE_SIZE);
	mVoxelTexture->readback(3, ColorSpace::RGBA, PixelDataType::FLOAT, (void*)dest.data(), dest.size() * sizeof(glm::vec4));
	auto* data = dest.data();*/

}

void nex::GlobalIllumination::drawTest(const glm::mat4& projection, const glm::mat4& view, Texture* depth)
{
	/*auto& mapping = mSphere->getMappings().begin();
	auto* pass = (IrradianceSphereHullDrawPass*) mapping->second->getTechnique()->getActiveSubMeshPass();
	pass->bind();

	auto model = glm::mat4();
	const auto positionWS = glm::vec3(0.0f, 0.0f, 30.0f);
	const auto radius = 15.0f;
	model = glm::translate(model, positionWS);
	model = glm::scale(model, glm::vec3(radius));
	
	pass->setColor(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
	pass->setPositionWS(positionWS);
	pass->setProbeRadius(radius);

	const auto& v = RenderBackend::get()->getViewport();
	glm::vec2 viewPort = glm::vec2(v.width, v.height);

	const auto zn = 0.1f;
	const auto zf = 150.0f;
	glm::vec3 clipInfo = glm::vec3(zn * zf, zn - zf, zf);
	

	pass->setViewPort(viewPort);
	pass->setClipInfo(clipInfo);
	pass->setDepth(depth);
	pass->setInverseProjMatrix(inverse(projection));

	pass->setModelMatrix(model, model);
	pass->setViewProjectionMatrices(projection, view, view);
	pass->uploadTransformMatrices();

	auto& renderState = mapping->second->getRenderState();
	renderState.cullSide = PolygonSide::FRONT;
	renderState.doCullFaces = true;
	renderState.doBlend = true;
	renderState.doDepthWrite = false;
	renderState.doDepthTest = true;
	renderState.depthCompare = CompareFunction::GREATER_EQUAL;
	renderState.blendDesc.operation = BlendOperation::ADD;
	renderState.blendDesc.source = BlendFunc::ONE;
	renderState.blendDesc.destination = BlendFunc::ONE;
	StaticMeshDrawer::draw(mapping->first, mapping->second);*/




}

void nex::GlobalIllumination::advanceNextStoreID(unsigned id)
{
	if (id == PbrProbe::INVALID_STOREID) return;

	mNextStoreID = max(mNextStoreID, id);

	if (mNextStoreID == id) ++mNextStoreID;
}

void nex::GlobalIllumination::collectBakeCommands(nex::RenderCommandQueue & commandQueue, const Scene& scene, bool doCulling)
{
	RenderCommand command;
	std::list<SceneNode*> queue;


	scene.acquireLock();
	for (const auto& root : scene.getActiveVobsUnsafe())
	{
		queue.push_back(root->getMeshRootNode());

		while (!queue.empty())
		{
			auto* node = queue.front();
			queue.pop_front();

			auto range = node->getChildren();

			for (auto* node : range)
			{
				queue.push_back(node);
			}

			auto* mesh = node->getMesh();
			if (mesh != nullptr)
			{
				command.mesh = mesh;
				command.material = node->getMaterial();
				command.worldTrafo = node->getWorldTrafo();
				command.prevWorldTrafo = node->getPrevWorldTrafo();
				command.boundingBox = node->getMeshBoundingBoxWorld();
				commandQueue.push(command, doCulling);
			}
		}
	}

	//commandQueue.getForwardCommands().clear();
	//commandQueue.getShadowCommands().clear();
	commandQueue.getToolCommands().clear();
	//commandQueue.getDeferrablePbrCommands().clear();
}

std::shared_ptr<nex::CubeMap> nex::GlobalIllumination::renderToCubeMap(
	const nex::RenderCommandQueue & queue,
	Renderer* renderer,
	CubeRenderTarget & renderTarget,
	nex::Camera& camera2,
	const glm::vec3 & worldPosition,
	const DirLight& light)
{
	thread_local auto* renderBackend = RenderBackend::get();

	//view matrices;
	const auto& views = CubeMap::getViewLookAts();
	const auto viewModel = glm::translate(glm::mat4(1.0f), worldPosition);

	//pass.bind();

	const glm::vec3 ups[6]{
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f)
	};

	static const glm::vec3 dir[6]{
		glm::vec3(1.0f, 0.0f, 0.0f), // right
		glm::vec3(-1.0f, 0.0f, 0.0f), // left
		glm::vec3(0.0f, 1.0f, 0.0f), // up
		glm::vec3(0.0f, -1.0f, 0.0f), // bottom
		glm::vec3(0.0f, 0.0f, -1.0f), // front
		glm::vec3(0.0f, 0.0f, 1.0f) // back
	};


	const size_t size = 1024;
	PerspectiveCamera camera(1.0f, glm::radians(90.0f), 0.1f, 100.0f);
	//OrthographicCamera camera(1.0f, 1.0f, 0.1f, 100.0f);

	camera.update();
	camera.setPosition(worldPosition, true);
	camera.update();

	for (unsigned side = 0; side < views.size(); ++side) {
		
		//if (side == 0 || side == 1) continue;

		//const auto view = glm::lookAt(worldPosition, worldPosition + dir[5 - side], ups[5 - side]);
		camera.setUp(ups[side]);
		camera.setLook(dir[side]);
		camera.update();
		camera.update();
		//camera.setView(view, true);
		

		//RenderBackend::get()->getDepthBuffer()->enableDepthTest(true);
		RenderBackend::get()->getDepthBuffer()->setState(DepthBuffer::State());
		auto* stencilTest = RenderBackend::get()->getStencilTest();
		stencilTest->enableStencilTest(false);
		stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
		stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);

		renderTarget.bind();
		renderTarget.useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X)); // side +
		renderBackend->setViewPort(0, 0, renderTarget.getWidth(), renderTarget.getHeight());
		renderTarget.clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
		

		renderer->render(queue, camera, light, renderTarget.getWidth(), renderTarget.getHeight(), false, &renderTarget);

		RenderBackend::get()->getDepthBuffer()->setState(DepthBuffer::State());
		stencilTest->enableStencilTest(false);
		stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
		stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);

		/*for (const auto& buffer : buffers) {
			for (const auto& command : *buffer)
			{
				pass.setModelMatrix(command.worldTrafo, command.prevWorldTrafo);
				pass.uploadTransformMatrices();

				auto* pbrMaterial = (PbrMaterial*)command.material;
				auto* shaderInterface = pass.getShaderInterface();
				shaderInterface->setAlbedoMap(pbrMaterial->getAlbedoMap());
				shaderInterface->setAoMap(pbrMaterial->getAoMap());
				shaderInterface->setMetalMap(pbrMaterial->getMetallicMap());
				shaderInterface->setNormalMap(pbrMaterial->getNormalMap());
				shaderInterface->setRoughnessMap(pbrMaterial->getRoughnessMap());

				StaticMeshDrawer::draw(command.mesh, nullptr, &pass, nullptr);
			}

		}*/
	}


	auto& resultAttachment = renderTarget.getColorAttachments()[0];
	auto result = std::dynamic_pointer_cast<CubeMap>(resultAttachment.texture);

	// now create mipmaps for the cubemap fighting render artificats in the prefilter map
	//result->generateMipMaps();
	return result;
}

	std::shared_ptr<nex::CubeMap> nex::GlobalIllumination::renderToDepthCubeMap(const nex::RenderCommandQueue& queue, 
		Renderer* renderer, 
		CubeRenderTarget& renderTarget, 
		nex::Camera& camera2, 
		const glm::vec3& worldPosition, 
		const DirLight& light)
	{
		thread_local auto* renderBackend = RenderBackend::get();

		//view matrices;
		const auto& views = CubeMap::getViewLookAts();
		const auto viewModel = glm::translate(glm::mat4(1.0f), worldPosition);

		//pass.bind();

		const glm::vec3 ups[6]{
			glm::vec3(0.0f, -1.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec3(0.0f, -1.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f)
		};

		static const glm::vec3 dir[6]{
			glm::vec3(1.0f, 0.0f, 0.0f), // right
			glm::vec3(-1.0f, 0.0f, 0.0f), // left
			glm::vec3(0.0f, 1.0f, 0.0f), // up
			glm::vec3(0.0f, -1.0f, 0.0f), // bottom
			glm::vec3(0.0f, 0.0f, -1.0f), // front
			glm::vec3(0.0f, 0.0f, 1.0f) // back
		};

		PerspectiveCamera camera(1.0f, glm::radians(90.0f), 0.1f, 100.0f);

		camera.update();
		camera.setPosition(worldPosition, true);
		camera.update();

		for (unsigned side = 0; side < views.size(); ++side) {

			//if (side == 0 || side == 1) continue;

			//const auto view = glm::lookAt(worldPosition, worldPosition + dir[5 - side], ups[5 - side]);
			camera.setUp(ups[side]);
			camera.setLook(dir[side]);
			camera.update();
			camera.update();
			//camera.setView(view, true);


			//RenderBackend::get()->getDepthBuffer()->enableDepthTest(true);
			RenderBackend::get()->getDepthBuffer()->setState(DepthBuffer::State());
			auto* stencilTest = RenderBackend::get()->getStencilTest();
			stencilTest->enableStencilTest(false);
			stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
			stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);

			renderTarget.bind();
			renderTarget.useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X)); // side +
			renderBackend->setViewPort(0, 0, renderTarget.getWidth(), renderTarget.getHeight());
			renderTarget.clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);

			auto collection = queue.getCommands(RenderCommandQueue::Deferrable | RenderCommandQueue::Forward | RenderCommandQueue::Transparent);

			mIrradianceDepthPass->bind();

			RenderState defaultState;

			mIrradianceDepthPass->setViewProjectionMatrices(camera.getProjectionMatrix(), camera.getView(), camera.getView());

			for (auto* commandQueue : collection) {
				for (const auto& command : *commandQueue)
				{
					mIrradianceDepthPass->setModelMatrix(command.worldTrafo, command.prevWorldTrafo);
					mIrradianceDepthPass->uploadTransformMatrices();
					StaticMeshDrawer::draw(command.mesh, nullptr, &defaultState);
				}
			}

			RenderBackend::get()->getDepthBuffer()->setState(DepthBuffer::State());
			stencilTest->enableStencilTest(false);
			stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
			stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);
		}


		auto& resultAttachment = renderTarget.getColorAttachments()[0];
		auto result = std::dynamic_pointer_cast<CubeMap>(resultAttachment.texture);
		result->generateMipMaps();

		return result;
	}

	nex::gui::GlobalIlluminationView::GlobalIlluminationView(std::string title, 
		MainMenuBar* menuBar, Menu* menu, GlobalIllumination* globalIllumination) :
		MenuWindow(std::move(title), menuBar, menu),
		mGlobalIllumination(globalIllumination)

	{

	}

	void nex::gui::GlobalIlluminationView::drawSelf()
	{
		bool visualize = mGlobalIllumination->getVisualize();

		static int mipMap = 0;
		if (ImGui::DragInt("Voxelization mip map level", &mipMap)) {
			mGlobalIllumination->setVisualize(visualize, mipMap);
		}

		if (ImGui::Checkbox("Visualize scene voxelization", &visualize)) {
			mGlobalIllumination->setVisualize(visualize, mipMap);
		}
	}
