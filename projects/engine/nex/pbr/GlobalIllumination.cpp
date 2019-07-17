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


nex::GlobalIllumination::GlobalIllumination(const std::string& compiledProbeDirectory, unsigned prefilteredSize, unsigned depth) :
mFactory(prefilteredSize, depth), mProbesBuffer(1, sizeof(ProbeData), ShaderBuffer::UsageHint::DYNAMIC_COPY)
{
}

nex::GlobalIllumination::~GlobalIllumination() = default;

void nex::GlobalIllumination::bakeProbes(const Scene & scene)
{
	PerspectiveCamera camera(PbrProbe::IRRADIANCE_SIZE, PbrProbe::IRRADIANCE_SIZE, glm::radians(90.0f), 0.1f, 100.0f);
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

	for (const auto& spatial : mProbeSpatials) {

		const auto position = glm::vec3(spatial);
		commandQueue.useSphereCulling(position, camera.getFarDistance());
		collectBakeCommands(commandQueue, scene, true);
		auto commandBuffers = commandQueue.getCommands(RenderCommandQueue::Deferrable | RenderCommandQueue::Forward | RenderCommandQueue::Transparent);
		//auto cubeMap = renderToCubeMap(commandBuffers, *renderTarget, position, camera.getProjectionMatrix());

		//TODO create probes
	}

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
	TransformPass& pass,
	CubeRenderTarget & renderTarget, 
	const glm::vec3 & worldPosition, 
	const glm::mat4 & projection)
{
	thread_local auto* renderBackend = RenderBackend::get();

	//view matrices;
	const auto& views = CubeMap::getViewLookAts();

	renderTarget.bind();
	renderBackend->setViewPort(0, 0, renderTarget.getWidth(), renderTarget.getHeight());

	pass.bind();

	for (unsigned side = 0; side < views.size(); ++side) {
		const auto& view = views[side];
		pass.setViewProjectionMatrices(projection, view, view);
		renderTarget.useSide(static_cast<CubeMapSide>(side + (unsigned)CubeMapSide::POSITIVE_X));

		for (const auto& buffer : buffers) {
			StaticMeshDrawer::draw(*buffer, &pass);
		}
	}


	auto& resultAttachment = renderTarget.getColorAttachments()[0];
	auto result = std::dynamic_pointer_cast<CubeMap>(resultAttachment.texture);

	// now create mipmaps for the cubemap fighting render artificats in the prefilter map
	//result->generateMipMaps();
	return result;
}
