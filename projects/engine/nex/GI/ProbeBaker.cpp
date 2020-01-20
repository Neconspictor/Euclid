#include <nex/GI/ProbeBaker.hpp>
#include <nex/renderer/Renderer.hpp>
#include <nex/renderer/RenderTypes.hpp>
#include <nex/texture/Attachment.hpp>
#include <nex/pbr/PbrDeferred.hpp>
#include <nex/pbr/PbrForward.hpp>
#include <nex/material/Material.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/effects/EffectLibrary.hpp>
#include <nex/GI/IrradianceSphereHullDrawPass.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/GI/Probe.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>
#include <list>
#include <nex/scene/Vob.hpp>

class nex::ProbeBaker::ProbeBakePass : public PbrGeometryShader
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



nex::ProbeBaker::ProbeBaker()
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
		{ "#define BONE_ANIMATION 1", "#define BONE_ANIMATION_TRAFOS_BINDING_POINT 1" }));


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
	AABB box = { glm::vec3(-10, -10, -10), glm::vec3(10, 10, 10) };
	auto sphere = std::make_unique<SphereMesh>(16, 16);
	//auto sphere = std::make_unique<MeshAABB>(box, Topology::TRIANGLES);
	//sphere->finalize();

	auto* shader = RenderBackend::get()->getEffectLibrary()->getIrradianceSphereHullDrawShader();
	auto material = std::make_unique<Material>(std::make_shared<ShaderProvider>(shader));

	mSphere->addMapping(sphere.get(), material.get());
	mSphere->add(std::move(sphere));
	mSphere->addMaterial(std::move(material));
}

nex::ProbeBaker::~ProbeBaker() = default;

void nex::ProbeBaker::bakeProbes(Scene& scene, const DirLight& light, ProbeFactory& factory, Renderer* renderer)
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

	auto renderTargetDepth = std::make_unique<nex::CubeRenderTarget>(ProbeFactory::SOURCE_CUBE_SIZE, ProbeFactory::SOURCE_CUBE_SIZE, dataDepth);

	RenderAttachment depthDepth;
	depthDepth.target = TextureTarget::TEXTURE2D;
	dataDepth = TextureDesc();
	dataDepth.minFilter = TexFilter::Linear;
	dataDepth.magFilter = TexFilter::Linear;
	dataDepth.generateMipMaps = false;
	dataDepth.colorspace = ColorSpace::DEPTH_STENCIL;
	dataDepth.internalFormat = InternalFormat::DEPTH24_STENCIL8;
	depthDepth.texture = std::make_unique<RenderBuffer>(ProbeFactory::SOURCE_CUBE_SIZE, ProbeFactory::SOURCE_CUBE_SIZE, dataDepth);

	depthDepth.type = RenderAttachmentType::DEPTH;

	renderTargetDepth->useDepthAttachment(std::move(depthDepth));
	renderTargetDepth->updateDepthAttachment();

	mDeferred->setDirLight(&light);
	mForward->setDirLight(&light);

	auto* pbrTechnique = renderer->getPbrTechnique();
	pbrTechnique->overrideForward(mForward.get());
	pbrTechnique->overrideDeferred(mDeferred.get());

	auto lock = scene.acquireLock();

	for (auto* probeVob : scene.getActiveProbeVobsUnsafe()) { //const auto& spatial : mProbeSpatials

		auto& probe = *probeVob->getProbe();
		if (probe.isInitialized()) continue;

		const auto& position = probe.getPosition();

		const auto storeID = probe.getStoreID();

		//if (storeID == 0) continue;

		if (factory.isProbeStored(probe)) {
			factory.initProbe(*probeVob, storeID, true, true);
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

			renderer->updateRenderTargets(ProbeFactory::SOURCE_CUBE_SIZE, ProbeFactory::SOURCE_CUBE_SIZE);
			//auto cubeMapDepth = renderToDepthCubeMap(commandQueue, renderer, *renderTargetDepth, camera, position, light);
			factory.initProbe(*probeVob, cubeMap.get(), storeID, false, false);
		}

		probeVob->getName() = "pbr probe " + std::to_string(probe.getArrayIndex()) + ", " + std::to_string(probe.getStoreID());
	}

	pbrTechnique->overrideForward(nullptr);
	pbrTechnique->overrideDeferred(nullptr);
}

void nex::ProbeBaker::bakeProbe(ProbeVob* probeVob, 
	const Scene& scene, 
	const DirLight& light, 
	ProbeFactory& factory, 
	Renderer* renderer)
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

	if (factory.isProbeStored(probe)) {
		factory.initProbe(*probeVob, probe.getStoreID(), true, true);
	}
	else {
		RenderCommandQueue commandQueue;
		commandQueue.useSphereCulling(position, camera.getFarDistance());
		collectBakeCommands(commandQueue, scene, false);
		commandQueue.getProbeCommands().clear();
		commandQueue.getToolCommands().clear();
		commandQueue.sort();

		auto cubeMap = renderToCubeMap(commandQueue, renderer, *renderTarget, camera, position, light);
		factory.initProbe(*probeVob, cubeMap.get(), probe.getStoreID(), false, false);
	}

	probeVob->getName() = "pbr probe " + std::to_string(probe.getArrayIndex()) + ", " + std::to_string(probe.getStoreID());

	pbrTechnique->overrideForward(nullptr);
	pbrTechnique->overrideDeferred(nullptr);


	renderer->updateRenderTargets(width, height);
}

void nex::ProbeBaker::collectBakeCommands(nex::RenderCommandQueue& commandQueue, const Scene& scene, bool doCulling)
{
	RenderCommand command;
	std::list<const Vob*> queue;


	scene.acquireLock();
	for (const auto* vob : scene.getActiveVobsUnsafe())
	{
		//skip rigged vobs
		if (dynamic_cast<const RiggedVob*>(vob)) continue;

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
				command.worldTrafo = &vob->getTrafoMeshToWorld();
				command.prevWorldTrafo = &vob->getTrafoPrevMeshToWorld();
				command.boundingBox = &vob->getBoundingBoxWorld();
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

std::shared_ptr<nex::CubeMap> nex::ProbeBaker::renderToCubeMap(
	const nex::RenderCommandQueue& queue,
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


	const size_t size = 1024;
	PerspectiveCamera camera(1.0f, glm::radians(90.0f), 0.1f, 100.0f);
	//OrthographicCamera camera(1.0f, 1.0f, 0.1f, 100.0f);

	camera.update();
	camera.setPosition(worldPosition, true);
	camera.update();

	RenderContext constants;
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

std::shared_ptr<nex::CubeMap> nex::ProbeBaker::renderToDepthCubeMap(const nex::RenderCommandQueue& queue,
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

	RenderContext constants;
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