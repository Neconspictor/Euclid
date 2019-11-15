#include <nex/pbr/ProbeGenerator.hpp>
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/Scene.hpp>
#include <nex/effects/SimpleColorPass.hpp>
#include <nex/pbr/PbrProbe.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/pbr/GlobalIllumination.hpp>
#include <nex/renderer/Renderer.hpp>

nex::ProbeGenerator::ProbeGenerator(nex::Scene* scene, nex::GlobalIllumination* globalIllumination, nex::Renderer* renderer) :
	mScene(scene),
mSimpleColorPass(nullptr),
mProbeVisualizationVob(nullptr),
mIsVisible(false),
mInfluenceRadius(0.5f),
mGlobalIllumination(globalIllumination),
mRenderer(renderer)
{	
	ResourceLoader::get()->enqueue([=](nex::RenderEngine::CommandQueue* queue)->nex::Resource * {
		queue->push([=]() {

			mSimpleColorPass = std::make_unique<SimpleColorPass>();
			auto material = std::make_unique<Material>(mSimpleColorPass.get());
			auto& state = material->getRenderState();
			
			state.fillMode = FillMode::POINT;
			state.doCullFaces = false;
			state.doShadowCast = false;
			state.doShadowReceive = false;
			state.isTool = true;

			mProbeVisualizationMeshContainer.add(std::make_unique<SphereMesh>(32, 32, false),
				std::move(material));

			mProbeVisualizationMeshContainer.finalize();

			auto* root = mProbeVisualizationMeshContainer
									.createNodeHierarchyUnsafe();
			mProbeVisualizationVob.setMeshRootNode(root);
			bool test = false;
		});

		return &mProbeVisualizationMeshContainer;
	});
}

nex::ProbeGenerator::~ProbeGenerator() = default;

void nex::ProbeGenerator::setScene(nex::Scene* scene)
{
	mScene = scene;
}

void nex::ProbeGenerator::show(bool visible)
{
	// Skip if no state change
	if (mIsVisible == visible) return;

	mIsVisible = visible;
	mScene->acquireLock();

	if (mIsVisible) {

		//update(camera->getPosition() + 2.0f * camera->getLook(), 0.5f);

		mScene->addActiveVobUnsafe(&mProbeVisualizationVob);
	}
	else {
		mScene->removeActiveVobUnsafe(&mProbeVisualizationVob);
	}
}

const glm::vec3& nex::ProbeGenerator::getProbePosition() const
{
	return mProbeVisualizationVob.getPosition();
}

float nex::ProbeGenerator::getInfluenceRadius() const
{
	return mInfluenceRadius;
}

nex::ProbeVob* nex::ProbeGenerator::generate()
{
	auto* probe = mGlobalIllumination->addUninitProbeUnsafe(mProbeVisualizationVob.getPosition(),
		mGlobalIllumination->getNextStoreID());

	mGlobalIllumination->bakeProbe(probe, *mScene, mRenderer);


	probe->getProbe()->setInfluenceRadius(mInfluenceRadius);

	probe->getMeshRootNode()->updateWorldTrafoHierarchy();

	mScene->acquireLock();
	mScene->addActiveVobUnsafe(probe);

	return probe;
}

void nex::ProbeGenerator::update(const glm::vec3& position, float influenceRadius)
{
	mProbeVisualizationVob.setPosition(position);
	mInfluenceRadius = influenceRadius;
	mProbeVisualizationVob.setScale(glm::vec3(mInfluenceRadius));
	mProbeVisualizationVob.updateTrafo(true);
}