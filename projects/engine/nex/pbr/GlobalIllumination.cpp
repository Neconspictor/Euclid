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
mProbeBakePass(std::make_unique<ProbeBakePass>()), mAmbientLightPower(1.0f)
{
	auto deferredGeometryPass = std::make_unique<PbrDeferredGeometryPass>(Shader::create(
		"pbr/pbr_deferred_geometry_pass_vs.glsl",
		"pbr/pbr_deferred_geometry_pass_fs.glsl"));


	PbrDeferred::LightingPassFactory deferredFactory = [](CascadedShadow* c, GlobalIllumination* g) {
		return std::make_unique<PbrDeferredLightingPass>(
			"pbr/pbr_deferred_lighting_pass_vs.glsl",
			"pbr/pbr_deferred_lighting_pass_fs.glsl",
			g,
			c);
	};

	PbrForward::LightingPassFactory forwardFactory = [](CascadedShadow* c, GlobalIllumination* g) {
		return std::make_unique<PbrForwardPass>(
			"pbr/pbr_forward_vs.glsl",
			"pbr/pbr_forward_fs.glsl",
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
	PerspectiveCamera camera(PbrProbe::IRRADIANCE_SIZE, PbrProbe::IRRADIANCE_SIZE, glm::radians(90.0f), 0.1f, 100.0f);
	camera.update();
	RenderCommandQueue commandQueue;

	TextureData data;
	data.colorspace = ColorSpace::RGB;
	data.internalFormat = InternFormat::RGB32F;
	data.pixelDataType = PixelDataType::FLOAT;

	auto renderTarget = std::make_unique<nex::CubeRenderTarget>(PbrProbe::IRRADIANCE_SIZE, PbrProbe::IRRADIANCE_SIZE, data);

	RenderAttachment depth;
	depth.target = TextureTarget::TEXTURE2D;
	data = TextureData();
	data.colorspace = ColorSpace::DEPTH;
	data.internalFormat = InternFormat::DEPTH24;
	depth.texture = std::make_unique<RenderBuffer>(PbrProbe::IRRADIANCE_SIZE, PbrProbe::IRRADIANCE_SIZE, data);
	depth.type = RenderAttachmentType::DEPTH;

	renderTarget->useDepthAttachment(std::move(depth));
	renderTarget->updateDepthAttachment();

	DirectionalLight light;
	light.setColor(glm::vec3(1.0f));
	light.setPower(1.0f);
	light.setDirection(glm::vec3(1.0f));


	mDeferred->setDirLight(&light);
	mForward->setDirLight(&light);

	auto* pbrTechnique = renderer->getPbrTechnique();
	pbrTechnique->overrideForward(mForward.get());
	pbrTechnique->overrideDeferred(mDeferred.get());

	for (auto& probe : mProbes) { //const auto& spatial : mProbeSpatials

		//const auto position = glm::vec3(spatial);
		const auto position = glm::vec3(4.0f);
		commandQueue.useSphereCulling(position, camera.getFarDistance());
		collectBakeCommands(commandQueue, scene, true);
		auto commandBuffers = commandQueue.getCommands(RenderCommandQueue::Deferrable | RenderCommandQueue::Transparent);
		auto cubeMap = renderToCubeMap(commandBuffers, *mProbeBakePass,*renderTarget, position, camera.getProjectionMatrix(), light);
		auto readImage = StoreImage::create(cubeMap.get());
		StoreImage::fill(mFactory.getIrradianceMaps(), readImage, probe->getArrayIndex());
	}

	pbrTechnique->overrideForward(nullptr);
	pbrTechnique->overrideDeferred(nullptr);
}

const std::vector<std::unique_ptr<nex::PbrProbe>>& nex::GlobalIllumination::getProbes() const
{
	return mProbes;
}

nex::ProbeVob* nex::GlobalIllumination::createVobUnsafe(PbrProbe* probe, Scene& scene)
{
	auto* meshRootNode = StaticMesh::createNodeHierarchy(&scene,
		{ std::pair<Mesh*, Material*>(PbrProbe::getSphere(), probe->getMaterial()) });

	auto vob = std::make_unique<ProbeVob>(meshRootNode, probe);

	return (ProbeVob*)scene.addVobUnsafe(std::move(vob), true);
}

void nex::GlobalIllumination::addProbe(std::unique_ptr<PbrProbe> probe)
{
	mProbes.emplace_back(std::move(probe));
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
}

std::shared_ptr<nex::CubeMap> nex::GlobalIllumination::renderToCubeMap(const nex::RenderCommandQueue::BufferCollection & buffers, 
	ProbeBakePass& pass,
	CubeRenderTarget & renderTarget, 
	const glm::vec3 & worldPosition, 
	const glm::mat4 & projection,
	const DirectionalLight& light)
{
	thread_local auto* renderBackend = RenderBackend::get();

	//view matrices;
	const auto& views = CubeMap::getViewLookAts();
	const auto viewModel = glm::translate(glm::mat4(1.0f), worldPosition);

	renderTarget.bind();
	renderBackend->setViewPort(0, 0, renderTarget.getWidth(), renderTarget.getHeight());
	renderTarget.clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);

	pass.bind();

	const glm::vec3 ups[6]{
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f)
	};

	const glm::vec3 dir[6]{
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	};



	for (unsigned side = 0; side < views.size(); ++side) {
		
		const auto view = glm::lookAt(worldPosition, worldPosition + dir[side], ups[side]);
		pass.updateConstants(light, projection, view);
		renderTarget.useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X));

		for (const auto& buffer : buffers) {
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

		}
	}


	auto& resultAttachment = renderTarget.getColorAttachments()[0];
	auto result = std::dynamic_pointer_cast<CubeMap>(resultAttachment.texture);

	// now create mipmaps for the cubemap fighting render artificats in the prefilter map
	//result->generateMipMaps();
	return result;
}
