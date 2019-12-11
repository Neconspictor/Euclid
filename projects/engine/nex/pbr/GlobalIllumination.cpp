#include <nex/pbr/GlobalIllumination.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nex/resource/FileSystem.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <glm/glm.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>
#include <list>
#include <nex/scene/Scene.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Attachment.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/pbr/PbrPass.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/pbr/PbrDeferred.hpp>
#include <nex/pbr/PbrForward.hpp>
#include <nex/renderer/Renderer.hpp>
#include <nex/pbr/Cluster.hpp>
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/effects/EffectLibrary.hpp>
#include <nex/renderer/Drawer.hpp>
#include <nex/pbr/IrradianceSphereHullDrawPass.hpp>
#include <nex/shadow/ShadowMap.hpp>

const unsigned nex::GlobalIllumination::VOXEL_BASE_SIZE = 256;

class nex::GlobalIllumination::ProbeBakePass : public PbrGeometryShader 
{
public:

	ProbeBakePass() :
		PbrGeometryShader(ShaderProgram::create("pbr/probe/pbr_probe_capture_vs.glsl", "pbr/probe/pbr_probe_capture_fs.glsl", nullptr, nullptr, nullptr, generateDefines()),
			TRANSFORM_BUFFER_BINDINGPOINT)
	{
		mEyeLightDirection = { mProgram->getUniformLocation("dirLight.directionEye"), UniformType::VEC3 };
		mLightColor = { mProgram->getUniformLocation("dirLight.color"), UniformType::VEC3 };
		mLightPower = { mProgram->getUniformLocation("dirLight.power"), UniformType::FLOAT };
		mInverseView = { mProgram->getUniformLocation("inverseViewMatrix"), UniformType::MAT4 };
	}

	void setEyeLightDirection(const glm::vec3& dir) {
		mProgram->setVec3(mEyeLightDirection.location, dir);
	}

	void setLightColor(const glm::vec3& color) {
		mProgram->setVec3(mLightColor.location, color);
	}

	void setLightPower(float power) {
		mProgram->setFloat(mLightPower.location, power);
	}

	void setInverseViewMatrix(const glm::mat4& mat) {
		mProgram->setMat4(mInverseView.location, mat);
	}

	void updateConstants(const DirLight& light, const glm::mat4& projection, const glm::mat4& view) {
		setLightColor(light.color);
		setLightPower(light.power);

		glm::vec4 lightEyeDirection = view * glm::vec4(-light.directionWorld, 0);
		setEyeLightDirection(glm::vec3(lightEyeDirection));

		setInverseViewMatrix(inverse(view));
		setViewProjectionMatrices(projection, view, view, projection * view);
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

class nex::GlobalIllumination::VoxelizePass : public PbrGeometryShader
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

	VoxelizePass(bool doLighting) :
		PbrGeometryShader(ShaderProgram::create("GI/voxelize_vs.glsl", "GI/voxelize_fs.glsl", nullptr, nullptr, "GI/voxelize_gs.glsl", 
			generateDefines(doLighting)), TRANSFORM_BUFFER_BINDINGPOINT), mDoLighting(doLighting)
	{
		mWorldLightDirection = { mProgram->getUniformLocation("dirLight.directionWorld"), UniformType::VEC3 };
		mLightColor = { mProgram->getUniformLocation("dirLight.color"), UniformType::VEC3 };
		mLightPower = { mProgram->getUniformLocation("dirLight.power"), UniformType::FLOAT };
		mLightViewProjection = { mProgram->getUniformLocation("lightViewProjectionMatrix"), UniformType::MAT4 };
		mShadowMap = mProgram->createTextureUniform("shadowMap", UniformType::TEXTURE2D, SHADOW_DEPTH_MAP_BINDING_POINT);
	}

	bool isLightingApplied() const {
		return mDoLighting;
	}

	void useConstantBuffer(UniformBuffer* buffer) {
		buffer->bindToTarget(C_UNIFORM_BUFFER_BINDING_POINT);
	}

	void useVoxelBuffer(ShaderStorageBuffer* buffer) {
		buffer->bindToTarget(VOXEL_BUFFER_BINDING_POINT);
	}


	void setLightDirectionWS(const glm::vec3& dir) {
		mProgram->setVec3(mWorldLightDirection.location, -dir);
	}

	void setLightColor(const glm::vec3& color) {
		mProgram->setVec3(mLightColor.location, color);
	}

	void setLightPower(float power) {
		mProgram->setFloat(mLightPower.location, power);
	}

	void useLight(const DirLight& light) {
		setLightColor(light.color);
		setLightPower(light.power);
		setLightDirectionWS(light.directionWorld);
	}

	void useShadow(const ShadowMap* shadow) {
		mProgram->setTexture(shadow->getRenderResult(), Sampler::getPoint(), mShadowMap.bindingSlot);
		mProgram->setMat4(mLightViewProjection.location, shadow->getViewProjection());
	}



private:

	static std::vector<std::string> generateDefines(bool doLighting) {
		auto vec = std::vector<std::string>();

		// csm CascadeBuffer and TransformBuffer both use binding point 0 per default. Resolve this conflict.
		vec.push_back(std::string("#define PBR_COMMON_GEOMETRY_TRANSFORM_BUFFER_BINDING_POINT ") + std::to_string(TRANSFORM_BUFFER_BINDINGPOINT));
		vec.push_back(std::string("#define C_UNIFORM_BUFFER_BINDING_POINT ") + std::to_string(C_UNIFORM_BUFFER_BINDING_POINT));
		vec.push_back(std::string("#define VOXEL_BUFFER_BINDING_POINT ") + std::to_string(VOXEL_BUFFER_BINDING_POINT));
		vec.push_back(std::string("#define SHADOW_DEPTH_MAP_BINDING_POINT ") + std::to_string(SHADOW_DEPTH_MAP_BINDING_POINT));

		vec.push_back(std::string("#define VOXEL_LIGHTING_WHILE_VOXELIZING " + std::to_string(doLighting)));

		return vec;
	}


	static constexpr unsigned TRANSFORM_BUFFER_BINDINGPOINT = 0;
	static constexpr unsigned C_UNIFORM_BUFFER_BINDING_POINT = 0;
	static constexpr unsigned VOXEL_BUFFER_BINDING_POINT = 1;
	static constexpr unsigned SHADOW_DEPTH_MAP_BINDING_POINT = 5;


	Uniform mWorldLightDirection;
	Uniform mLightColor;
	Uniform mLightPower;
	UniformTex mShadowMap;
	Uniform mLightViewProjection;
	bool mDoLighting;
};

class nex::GlobalIllumination::VoxelVisualizePass : public Shader
{
public:

	VoxelVisualizePass() :
		Shader(ShaderProgram::create("GI/voxel_visualize_vs.glsl", "GI/voxel_visualize_fs.glsl", nullptr, nullptr, 
			"GI/voxel_visualize_gs.glsl", generateDefines()))
	{
		mViewProj = { mProgram->getUniformLocation("viewProj"), UniformType::MAT4 };
		mMipMap = { mProgram->getUniformLocation("mipMap"), UniformType::FLOAT };
		mVoxelImage = mProgram->createTextureUniform("voxelImage", UniformType::IMAGE3D, 0);

		mSampler.setMinFilter(TexFilter::Near_Mipmap_Near);
		mSampler.setMagFilter(TexFilter::Nearest);
		mSampler.setBorderColor(glm::vec4(0.0));
		mSampler.setWrapR(UVTechnique::ClampToBorder);
		mSampler.setWrapS(UVTechnique::ClampToBorder);
		mSampler.setWrapT(UVTechnique::ClampToBorder);
	}

	void setViewProjection(const glm::mat4& mat) {
		mProgram->setMat4(mViewProj.location, mat);
	}

	void setMipMap(int mipMap) {
		mProgram->setFloat(mMipMap.location, mipMap);
	}

	void useConstantBuffer(UniformBuffer* buffer) {
		buffer->bindToTarget(C_UNIFORM_BUFFER_BINDING_POINT);
	}

	void useVoxelBuffer(ShaderStorageBuffer* buffer) {
		buffer->bindToTarget(VOXEL_BUFFER_BINDING_POINT);
	}

	void useVoxelTexture(Texture3D* texture) {
		mProgram->setTexture(texture, &mSampler, 0);
		return;
		mProgram->setImageLayerOfTexture(mVoxelImage.location,
			texture, mVoxelImage.bindingSlot,
			TextureAccess::READ_WRITE,
			InternalFormat::RGBA32F,
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
	Sampler mSampler;
};


class nex::GlobalIllumination::VoxelFillComputeLightPass : public ComputeShader
{
public:

	VoxelFillComputeLightPass(unsigned localSizeX, bool doLighting) :
		ComputeShader(ShaderProgram::createComputeShader("GI/update_voxel_texture_cs.glsl", generateDefines(localSizeX, doLighting))),
		mDoLighting(doLighting)
	{
		mVoxelImage = mProgram->createTextureUniform("voxelImage", UniformType::IMAGE3D, VOXEL_IMAGE_BINDING_POINT);

		mWorldLightDirection = { mProgram->getUniformLocation("dirLight.directionWorld"), UniformType::VEC3 };
		mLightColor = { mProgram->getUniformLocation("dirLight.color"), UniformType::VEC3 };
		mLightPower = { mProgram->getUniformLocation("dirLight.power"), UniformType::FLOAT };
		mShadowMap = mProgram->createTextureUniform("shadowMap", UniformType::TEXTURE2D, SHADOW_DEPTH_MAP_BINDING_POINT);
		mLightViewProjection = { mProgram->getUniformLocation("lightViewProjectionMatrix"), UniformType::MAT4 };
	}

	bool isLightingApplied() const {
		return mDoLighting;
	}

	void useConstantBuffer(UniformBuffer* buffer) {
		buffer->bindToTarget(C_UNIFORM_BUFFER_BINDING_POINT);
	}

	void useVoxelBuffer(ShaderStorageBuffer* buffer) {
		buffer->bindToTarget(VOXEL_BUFFER_BINDING_POINT);
	}

	void setVoxelOutputImage(Texture3D* voxelImage) {
		mProgram->setImageLayerOfTexture(mVoxelImage.location,
			voxelImage, mVoxelImage.bindingSlot,
			TextureAccess::READ_WRITE,
			InternalFormat::RGBA32F,
			0,
			true,
			0);
	}

	void setLightDirectionWS(const glm::vec3& dir) {
		mProgram->setVec3(mWorldLightDirection.location, -dir);
	}

	void setLightColor(const glm::vec3& color) {
		mProgram->setVec3(mLightColor.location, color);
	}

	void setLightPower(float power) {
		mProgram->setFloat(mLightPower.location, power);
	}

	void useLight(const DirLight& light) {
		setLightColor(light.color);
		setLightPower(light.power);
		setLightDirectionWS(light.directionWorld);
	}

	void useShadow(const ShadowMap* shadow) {
		mProgram->setTexture(shadow->getRenderResult(), Sampler::getPoint(), mShadowMap.bindingSlot);
		mProgram->setMat4(mLightViewProjection.location, shadow->getViewProjection());
	}

private:

	static std::vector<std::string> generateDefines(unsigned localSizeX, bool doLighting) {
		auto vec = std::vector<std::string>();

		vec.push_back(std::string("#define C_UNIFORM_BUFFER_BINDING_POINT ") + std::to_string(C_UNIFORM_BUFFER_BINDING_POINT));
		vec.push_back(std::string("#define VOXEL_BUFFER_BINDING_POINT ") + std::to_string(VOXEL_BUFFER_BINDING_POINT));
		vec.push_back(std::string("#define LOCAL_SIZE_X ") + std::to_string(localSizeX));
		vec.push_back(std::string("#define SHADOW_DEPTH_MAP_BINDING_POINT ") + std::to_string(SHADOW_DEPTH_MAP_BINDING_POINT));
		vec.push_back(std::string("#define VOXEL_LIGHTING_WHILE_VOXELIZING " + std::to_string(!doLighting)));

		return vec;
	}

	static constexpr unsigned C_UNIFORM_BUFFER_BINDING_POINT = 0;
	static constexpr unsigned VOXEL_BUFFER_BINDING_POINT = 0;
	static constexpr unsigned VOXEL_IMAGE_BINDING_POINT = 0;
	static constexpr unsigned SHADOW_DEPTH_MAP_BINDING_POINT = 1;

	UniformTex mVoxelImage;

	Uniform mWorldLightDirection;
	Uniform mLightColor;
	Uniform mLightPower;
	UniformTex mShadowMap;
	Uniform mLightViewProjection;
	bool mDoLighting;
};


class nex::GlobalIllumination::MipMapTexture3DPass : public ComputeShader
{
public:

	MipMapTexture3DPass() :
		ComputeShader(ShaderProgram::createComputeShader("GI/mipmap_texture3d_box_filter_cs.glsl"))
	{
		mSourceImage = mProgram->createTextureUniform("source", UniformType::IMAGE3D, SOURCE_BINDING_POINT);
		mDestImage = mProgram->createTextureUniform("dest", UniformType::IMAGE3D, DEST_BINDING_POINT);
	}

	void setInputImage(Texture3D* texture, unsigned mipMap) {
		mProgram->setImageLayerOfTexture(mSourceImage.location,
			texture, mSourceImage.bindingSlot,
			TextureAccess::READ_ONLY,
			InternalFormat::RGBA32F,
			mipMap,
			true,
			0);
	}

	void setOutputImage(Texture3D* texture, unsigned mipMap) {
		mProgram->setImageLayerOfTexture(mDestImage.location,
			texture, mDestImage.bindingSlot,
			TextureAccess::WRITE_ONLY,
			InternalFormat::RGBA32F,
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


nex::GlobalIllumination::GlobalIllumination(const std::string& compiledProbeDirectory, unsigned prefilteredSize, unsigned depth, bool deferredVoxelizationLighting) :
mFactory(prefilteredSize, depth),
mEnvironmentLights(0, 0, nullptr, ShaderBuffer::UsageHint::DYNAMIC_COPY),
mProbeBakePass(std::make_unique<ProbeBakePass>()), 
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
	auto deferredGeometryPass = std::make_unique<PbrDeferredGeometryShader>(ShaderProgram::create(
		"pbr/pbr_deferred_geometry_pass_vs.glsl",
		"pbr/pbr_deferred_geometry_pass_fs.glsl"));

	auto deferredGeometryBonesPass = std::make_unique<PbrDeferredGeometryBonesShader>(ShaderProgram::create(
		"pbr/pbr_deferred_geometry_pass_vs.glsl",
		"pbr/pbr_deferred_geometry_pass_fs.glsl",
		nullptr,
		nullptr,
		nullptr,
		{"#define BONE_ANIMATION 1", "#define BONE_ANIMATION_TRAFOS_BINDING_POINT 1"}));


	PbrDeferred::LightingPassFactory deferredFactory = [](CascadedShadow* c, GlobalIllumination* g) {
		return std::make_unique<PbrDeferredLightingPass>(
			"screen_space_vs.glsl",
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
		std::move(deferredGeometryBonesPass),
		std::move(deferredFactory),
		nullptr,
		nullptr,
		nullptr);

	mForward = std::make_unique<PbrForward>(std::move(forwardFactory),
		nullptr,
		nullptr,
		nullptr);

	mIrradianceDepthPass = std::make_unique<TransformShader>(ShaderProgram::create("pbr/probe/irradiance_depth_pass_vs.glsl", 
		"pbr/probe/irradiance_depth_pass_fs.glsl"));

	mSphere = std::make_unique<MeshGroup>();
	AABB box = {glm::vec3(-10, -10, -10), glm::vec3(10, 10, 10)};
	auto sphere = std::make_unique<SphereMesh>(16, 16);
	//auto sphere = std::make_unique<MeshAABB>(box, Topology::TRIANGLES);
	//sphere->finalize();
	auto material = std::make_unique<Material>(RenderBackend::get()->getEffectLibrary()->getIrradianceSphereHullDrawShader());

	mSphere->addMapping(sphere.get(), material.get());
	mSphere->add(std::move(sphere));
	mSphere->addMaterial(std::move(material));

	// Note: we have to generate mipmaps since memory can only be allocated during texture creation.
	auto data = TextureDesc::createRenderTargetRGBAHDR(InternalFormat::RGBA32F, true);
	data.minFilter = TexFilter::Linear_Mipmap_Linear; //Linear_Mipmap_Linear TODO: which filtering is better?
	data.magFilter = TexFilter::Linear; //Linear
	data.wrapR = data.wrapS = data.wrapT = UVTechnique::ClampToBorder;
	data.borderColor = glm::vec4(0.0);
	data.pixelDataType = PixelDataType::FLOAT;
	mVoxelTexture = std::make_unique<Texture3D>(VOXEL_BASE_SIZE, VOXEL_BASE_SIZE, VOXEL_BASE_SIZE, data, nullptr);

	// use super-sampling during voxelization (improves coverage and thus reduces holes in voxelization)
	// For rough approximations, 4x should be enough, but for static voxelization we use higher values for better quality.


	auto resolution = min (VOXEL_BASE_SIZE, 8192);

	//8192 is too much data for >256
	if (VOXEL_BASE_SIZE > 256)
		resolution = min(resolution, 4096);
	
	mVoxelizationRT = std::make_unique<RenderTarget>(resolution, resolution);

	deferVoxelizationLighting(deferredVoxelizationLighting);
}

nex::GlobalIllumination::~GlobalIllumination() = default;

void nex::GlobalIllumination::bakeProbes(const Scene & scene, Renderer* renderer)
{
	const int size = 1024;

	PerspectiveCamera camera(1.0f, glm::radians(90.0f), 0.1f, 100.0f);
	//OrthographicCamera camera(1.0f, 1.0f, 0.1f, 100.0f);
	
	camera.update();

	TextureDesc data;
	data.colorspace = ColorSpace::RGB;
	data.internalFormat = InternalFormat::RGB32F;
	data.pixelDataType = PixelDataType::FLOAT;
	data.minFilter = TexFilter::Linear;
	data.magFilter = TexFilter::Linear;
	data.generateMipMaps = false;

	auto renderTarget = std::make_unique<nex::CubeRenderTarget>(size, size, data);

	RenderAttachment depth;
	depth.target = TextureTarget::TEXTURE2D;
	data = TextureDesc();
	data.minFilter = TexFilter::Linear;
	data.magFilter = TexFilter::Linear;
	data.generateMipMaps = false;
	data.colorspace = ColorSpace::DEPTH_STENCIL;
	data.internalFormat = InternalFormat::DEPTH24_STENCIL8;
	depth.texture = std::make_unique<RenderBuffer>(size, size, data);

	depth.type = RenderAttachmentType::DEPTH_STENCIL;

	renderTarget->useDepthAttachment(std::move(depth));
	renderTarget->updateDepthAttachment();


	TextureDesc dataDepth;
	dataDepth.colorspace = ColorSpace::RGBA;
	dataDepth.internalFormat = InternalFormat::RGBA32F;
	dataDepth.pixelDataType = PixelDataType::FLOAT;
	dataDepth.minFilter = TexFilter::Linear_Mipmap_Linear;
	dataDepth.magFilter = TexFilter::Linear;
	dataDepth.generateMipMaps = false;

	auto renderTargetDepth = std::make_unique<nex::CubeRenderTarget>(PbrProbe::SOURCE_CUBE_SIZE, PbrProbe::SOURCE_CUBE_SIZE, dataDepth);

	RenderAttachment depthDepth;
	depthDepth.target = TextureTarget::TEXTURE2D;
	dataDepth = TextureDesc();
	dataDepth.minFilter = TexFilter::Linear;
	dataDepth.magFilter = TexFilter::Linear;
	dataDepth.generateMipMaps = false;
	dataDepth.colorspace = ColorSpace::DEPTH_STENCIL;
	dataDepth.internalFormat = InternalFormat::DEPTH24_STENCIL8;
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
	backgroundHDRData.internalFormat = InternalFormat::RGB32F;
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
	const int size = 1024;

	PerspectiveCamera camera(1.0f, glm::radians(90.0f), 0.1f, 100.0f);
	//OrthographicCamera camera(1.0f, 1.0f, 0.1f, 100.0f);

	camera.update();

	TextureDesc data;
	data.colorspace = ColorSpace::RGB;
	data.internalFormat = InternalFormat::RGB32F;
	data.pixelDataType = PixelDataType::FLOAT;
	data.minFilter = TexFilter::Linear;
	data.magFilter = TexFilter::Linear;
	data.generateMipMaps = false;

	auto renderTarget = std::make_unique<nex::CubeRenderTarget>(size, size, data);

	RenderAttachment depth;
	depth.target = TextureTarget::TEXTURE2D;
	data = TextureDesc();
	data.minFilter = TexFilter::Linear;
	data.magFilter = TexFilter::Linear;
	data.generateMipMaps = false;
	data.colorspace = ColorSpace::DEPTH_STENCIL;
	data.internalFormat = InternalFormat::DEPTH24_STENCIL8;
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

	auto* probVobPtr = new ProbeVob(nullptr, probe->getMeshGroup()->getBatches(), probe.get());
	auto vob = std::unique_ptr<ProbeVob>(probVobPtr);

	mProbes.emplace_back(std::move(probe));
	mProbeVobs.emplace_back(std::move(vob));

	return mProbeVobs.back().get();
}

void nex::GlobalIllumination::deferVoxelizationLighting(bool deferLighting)
{
	mVoxelizePass = std::make_unique<VoxelizePass>(!deferLighting);
	mVoxelFillComputeLightPass = std::make_unique<VoxelFillComputeLightPass>(VOXEL_BASE_SIZE, deferLighting);
	mDeferLighting = deferLighting;
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

bool nex::GlobalIllumination::isVoxelLightingDeferred() const
{
	return mDeferLighting;
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
		
		const auto& trafo = vob->getWorldTrafo();
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

void nex::GlobalIllumination::voxelize(const nex::RenderCommandQueue::ConstBufferCollection& collection, const AABB& sceneBoundingBox, 
	const DirLight* light, const ShadowMap* shadows)
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


	//auto* renderTarget = RenderBackend::get()->getDefaultRenderTarget();
	//renderTarget->bind();
	//renderTarget->enableDrawToColorAttachments(false);
	auto viewPort = RenderBackend::get()->getViewport();
	mVoxelizationRT->bind();
	RenderBackend::get()->setViewPort(0,0, mVoxelizationRT->getWidth(), mVoxelizationRT->getHeight());
	

	// TODO: The voxelization pass is only suitable for pbr materials.
	// Refactor for supporting other types of materials (e.g. particle systems etc.)
	mVoxelizePass->bind();
	mVoxelizePass->useConstantBuffer(&mVoxelConstantBuffer);
	mVoxelizePass->useVoxelBuffer(&mVoxelBuffer);

	if (mVoxelizePass->isLightingApplied()) {
		mVoxelizePass->useLight(*light);
		mVoxelizePass->useShadow(shadows);
	}

	RenderState state;

	for (auto* commands : collection) {

		for (auto& command : *commands) {

			if (command.worldTrafo == nullptr || command.prevWorldTrafo == nullptr)
				continue;

			mVoxelizePass->setModelMatrix(*command.worldTrafo, *command.prevWorldTrafo);
			mVoxelizePass->uploadTransformMatrices();
			auto state = command.batch->getState();
			state.doCullFaces = false; // Is needed, since we project manually the triangles. Culling would be terribly wrong.
			
			for (auto& pair : command.batch->getEntries()) {
				auto* material = pair.second;
				if (dynamic_cast<const PbrMaterial*>(material)) {
					Drawer::draw(mVoxelizePass.get(), pair.first, material, &state);
				}
				
			}
			
		}
	}

	RenderBackend::get()->setViewPort(viewPort.x, viewPort.y, viewPort.width, viewPort.height);
}

void nex::GlobalIllumination::updateVoxelTexture(const DirLight* light, const ShadowMap* shadows)
{

	//auto viewPort = RenderBackend::get()->getViewport();
	//mVoxelizationRT->bind();
	//RenderBackend::get()->setViewPort(0, 0, mVoxelizationRT->getWidth(), mVoxelizationRT->getHeight());

	mVoxelFillComputeLightPass->bind();
	mVoxelFillComputeLightPass->setVoxelOutputImage(mVoxelTexture.get());
	mVoxelFillComputeLightPass->useConstantBuffer(&mVoxelConstantBuffer);
	mVoxelFillComputeLightPass->useVoxelBuffer(&mVoxelBuffer);

	if (mVoxelFillComputeLightPass->isLightingApplied()) {
		mVoxelFillComputeLightPass->useLight(*light);
		mVoxelFillComputeLightPass->useShadow(shadows);
	}
	mVoxelFillComputeLightPass->dispatch(VOXEL_BASE_SIZE, VOXEL_BASE_SIZE, VOXEL_BASE_SIZE);

	RenderBackend::get()->syncMemoryWithGPU(MemorySync_TextureFetch | MemorySync_TextureUpdate | MemorySync_ShaderImageAccess);

	mMipMapTexture3DPass->bind();

	const int lastMipMapIndex = Texture::getMipMapCount(mVoxelTexture->getWidth()) - 1;

	for (int i = 0; i < lastMipMapIndex; ++i) {
		const auto mipMapSize = mVoxelTexture->getWidth() / std::pow(2, i);
		mMipMapTexture3DPass->setInputImage(mVoxelTexture.get(), i);
		mMipMapTexture3DPass->setOutputImage(mVoxelTexture.get(), i + 1);
		mMipMapTexture3DPass->dispatch(mipMapSize, mipMapSize, mipMapSize);
	}
	RenderBackend::get()->wait();
	//RenderBackend::get()->setViewPort(viewPort.x, viewPort.y, viewPort.width, viewPort.height);
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
	Drawer::draw(mapping->first, mapping->second);*/




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
	std::list<const Vob*> queue;


	scene.acquireLock();
	for (const auto* vob : scene.getActiveVobsUnsafe())
	{
		bool hasBoneAnimations = vob->getType() == VobType::Skinned;

		//skip rigged vobs
		if (hasBoneAnimations) continue;

		queue.push_back(vob);

		

		while (!queue.empty())
		{
			auto* vob = queue.front();
			queue.pop_front();

			for (auto* child : vob->getChildren()) {
				queue.push_back(child);
			}

			auto* batches = vob->getBatches();
			if (!batches) continue;

			for (const auto& batch : *batches) {
				command.batch = &batch;
				command.worldTrafo = &vob->getWorldTrafo();
				command.prevWorldTrafo = &vob->getPrevWorldTrafo();
				command.boundingBox = &vob->getBoundingBox();
				command.isBoneAnimated = false;
				command.bones = nullptr;

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

	Constants constants;
	constants.camera = &camera;
	constants.sun = &light;
	constants.windowWidth = renderTarget.getWidth();
	constants.windowHeight = renderTarget.getHeight();
	constants.time = 0.0f;

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
		stencilTest->setCompareFunc(CompFunc::ALWAYS, 1, 0xFF);
		stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);

		renderTarget.bind();
		renderTarget.useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X)); // side +
		renderBackend->setViewPort(0, 0, renderTarget.getWidth(), renderTarget.getHeight());
		renderTarget.clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
		

		renderer->render(queue, constants, false, &renderTarget);

		RenderBackend::get()->getDepthBuffer()->setState(DepthBuffer::State());
		stencilTest->enableStencilTest(false);
		stencilTest->setCompareFunc(CompFunc::ALWAYS, 1, 0xFF);
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

				Drawer::draw(command.mesh, nullptr, &pass, nullptr);
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

		Constants constants;
		constants.camera = &camera;
		constants.sun = &light;

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
			stencilTest->setCompareFunc(CompFunc::ALWAYS, 1, 0xFF);
			stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);

			renderTarget.bind();
			renderTarget.useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X)); // side +
			renderBackend->setViewPort(0, 0, renderTarget.getWidth(), renderTarget.getHeight());
			renderTarget.clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);

			auto collection = queue.getCommands(RenderCommandQueue::Deferrable | RenderCommandQueue::Forward | RenderCommandQueue::Transparent);

			mIrradianceDepthPass->bind();

			RenderState defaultState;

			mIrradianceDepthPass->updateConstants(constants);

			for (auto* commandQueue : collection) {
				for (const auto& command : *commandQueue)
				{
					mIrradianceDepthPass->setModelMatrix(*command.worldTrafo, *command.prevWorldTrafo);
					mIrradianceDepthPass->uploadTransformMatrices();
					for (auto& pair : command.batch->getEntries()) {
						Drawer::draw(mIrradianceDepthPass.get(), pair.first, nullptr, &defaultState);
					}
					
				}
			}

			RenderBackend::get()->getDepthBuffer()->setState(DepthBuffer::State());
			stencilTest->enableStencilTest(false);
			stencilTest->setCompareFunc(CompFunc::ALWAYS, 1, 0xFF);
			stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);
		}


		auto& resultAttachment = renderTarget.getColorAttachments()[0];
		auto result = std::dynamic_pointer_cast<CubeMap>(resultAttachment.texture);
		result->generateMipMaps();

		return result;
	}

	nex::gui::GlobalIlluminationView::GlobalIlluminationView(std::string title, 
		MainMenuBar* menuBar, Menu* menu, GlobalIllumination* globalIllumination,
		const DirLight* light,
		const ShadowMap* shadow,
		const RenderCommandQueue* queue,
		const Scene* scene) :
		MenuWindow(std::move(title), menuBar, menu),
		mGlobalIllumination(globalIllumination),
		mLight(light),
		mShadow(shadow),
		mQueue(queue),
		mScene(scene),
		mShadowConfig(shadow)

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

		if (ImGui::Button("Revoxelize")) {
			auto collection = mQueue->getCommands(RenderCommandQueue::Deferrable | RenderCommandQueue::Forward
				| RenderCommandQueue::Transparent);
			mGlobalIllumination->voxelize(collection, mScene->getSceneBoundingBox(), mLight, mShadow);
			mGlobalIllumination->updateVoxelTexture(mLight, mShadow);
		}

		if (mGlobalIllumination->isVoxelLightingDeferred()) {
			if (ImGui::Button("Update light")) {
				mGlobalIllumination->updateVoxelTexture(mLight, mShadow);
			}
		}

		ImGui::Text("GI Shadow map:");
		mShadowConfig.drawGUI();
		
	}