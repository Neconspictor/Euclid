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

	void updateConstants(const DirectionalLight& light, const glm::mat4& projection, const glm::mat4& view) {
		setLightColor(light.getColor());
		setLightPower(light.getLightPower());

		glm::vec4 lightEyeDirection = view * glm::vec4(light.getDirection(), 0);
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


nex::GlobalIllumination::GlobalIllumination(const std::string& compiledProbeDirectory, unsigned prefilteredSize, unsigned depth) :
mFactory(prefilteredSize, depth), mProbesBuffer(1, sizeof(ProbeData), ShaderBuffer::UsageHint::DYNAMIC_COPY),
mProbeBakePass(std::make_unique<ProbeBakePass>()), mAmbientLightPower(1.0f),
mProbeScene(std::make_unique<Scene>())
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
}

nex::GlobalIllumination::~GlobalIllumination() = default;

void nex::GlobalIllumination::bakeProbes(const Scene & scene, Renderer* renderer)
{
	const size_t size = 1024;

	PerspectiveCamera camera(size, size, glm::radians(90.0f), 0.1f, 100.0f);
	//OrthographicCamera camera(1.0f, 1.0f, 0.1f, 100.0f);
	
	camera.update();

	TextureData data;
	data.colorspace = ColorSpace::RGB;
	data.internalFormat = InternFormat::RGB32F;
	data.pixelDataType = PixelDataType::FLOAT;
	data.minFilter = TextureFilter::Linear;
	data.magFilter = TextureFilter::Linear;
	data.generateMipMaps = false;

	auto renderTarget = std::make_unique<nex::CubeRenderTarget>(size, size, data);

	RenderAttachment depth;
	depth.target = TextureTarget::TEXTURE2D;
	data = TextureData();
	data.minFilter = TextureFilter::Linear;
	data.magFilter = TextureFilter::Linear;
	data.generateMipMaps = false;
	data.colorspace = ColorSpace::DEPTH_STENCIL;
	data.internalFormat = InternFormat::DEPTH24_STENCIL8;
	depth.texture = std::make_unique<RenderBuffer>(size, size, data);
	depth.type = RenderAttachmentType::DEPTH_STENCIL;

	renderTarget->useDepthAttachment(std::move(depth));
	renderTarget->updateDepthAttachment();

	DirectionalLight light;
	light.setColor(glm::vec3(1.0f, 1.0f, 1.0f));
	light.setPower(3.0f);
	light.setDirection({ -1,-1,-1 });


	mDeferred->setDirLight(&light);
	mForward->setDirLight(&light);

	auto* pbrTechnique = renderer->getPbrTechnique();
	pbrTechnique->overrideForward(mForward.get());
	pbrTechnique->overrideDeferred(mDeferred.get());

	renderer->updateRenderTargets(size, size);

	for (auto& probeVob : scene.getActiveProbeVobsUnsafe()) { //const auto& spatial : mProbeSpatials

		auto& probe = *probeVob->getProbe();
		if (probe.isInitialized()) continue;

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

		//auto readImage = StoreImage::create(cubeMap.get());
		//StoreImage::fill(mFactory.getIrradianceMaps(), readImage, probe->getArrayIndex());
	}

	pbrTechnique->overrideForward(nullptr);
	pbrTechnique->overrideDeferred(nullptr);
}

const std::vector<std::unique_ptr<nex::PbrProbe>>& nex::GlobalIllumination::getProbes() const
{
	return mProbes;
}

nex::ProbeVob* nex::GlobalIllumination::addUninitProbeUnsafe(nex::Scene& scene, const glm::vec3& position, unsigned storeID)
{
	auto probe = std::make_unique<PbrProbe>(position, storeID);

	auto* meshRootNode = StaticMesh::createNodeHierarchy(mProbeScene.get(),
		{ std::pair<Mesh*, Material*>(PbrProbe::getSphere(), probe->getMaterial()) });

	auto vob = std::make_unique<ProbeVob>(meshRootNode, probe.get());
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

nex::CubeMapArray * nex::GlobalIllumination::getPrefilteredMaps()
{
	return mFactory.getPrefilteredMaps();
}

const nex::GlobalIllumination::ProbesData & nex::GlobalIllumination::getProbesData() const
{
	return mProbesData;
}

nex::ShaderStorageBuffer * nex::GlobalIllumination::getProbesShaderBuffer()
{
	return &mProbesBuffer;
}

nex::PbrProbeFactory* nex::GlobalIllumination::getFactory()
{
	return &mFactory;
}

void nex::GlobalIllumination::setActiveProbe(PbrProbe * probe)
{
	mActive = probe;
}

void nex::GlobalIllumination::setAmbientPower(float ambientPower)
{
	mAmbientLightPower = ambientPower;
}

void nex::GlobalIllumination::update(const nex::Scene::ProbeRange & activeProbes)
{
	mProbesData.resize(activeProbes.size());
	ProbeVob::ProbeData* data = mProbesData.data();

	unsigned counter = 0;

	for (auto* vob : activeProbes) {
		const auto& trafo = vob->getMeshRootNode()->getWorldTrafo();
		data[counter].positionWorld = glm::vec4(trafo[3][0], trafo[3][1], trafo[3][2], 0);
		data[counter].arrayIndex.x = vob->getProbe()->getArrayIndex();
		++counter;
	}

	const auto oldSize = mProbesBuffer.getSize();

	if (mProbesData.memSize() == oldSize) {
		mProbesBuffer.update(data, mProbesData.memSize());
	}
	else {
		mProbesBuffer.resize(data, mProbesData.memSize(), ShaderBuffer::UsageHint::DYNAMIC_COPY);
	}
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
	const DirectionalLight& light)
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
	PerspectiveCamera camera(size, size, glm::radians(90.0f), 0.1f, 100.0f);
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
