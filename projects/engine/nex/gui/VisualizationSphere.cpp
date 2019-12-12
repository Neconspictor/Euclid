#include <nex/gui/VisualizationSphere.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/renderer/RenderEngine.hpp>
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/effects/EffectLibrary.hpp>
#include <nex/scene/Scene.hpp>

nex::VisualizationSphere::VisualizationSphere(Scene* scene) : 
	mScene(scene),
	mVisualizationVob(nullptr, nullptr),
	mIsVisible(false)
{

	nex::ResourceLoader::get()->enqueue([=]()->nex::Resource* {
		nex::RenderEngine::getCommandQueue()->push([=]() {

			//mSimpleColorPass = std::make_unique<SimpleColorPass>();
			auto* simpleColorShader = nex::RenderBackend::get()->getEffectLibrary()->getSimpleColorShader();
			auto material = std::make_unique<SimpleColorMaterial>(simpleColorShader);
			auto& state = material->getRenderState();

			state.fillMode = FillMode::POINT;
			state.doCullFaces = false;
			state.doShadowCast = false;
			state.doShadowReceive = false;
			state.isTool = true;

			mVisualizationMG.add(std::make_unique<SphereMesh>(32, 32, false),
				std::move(material));

			mVisualizationMG.calcBatches();
			mVisualizationMG.finalize();
			mVisualizationVob.setBatches(mVisualizationMG.getBatches());
			bool test = false;
		});

		return &mVisualizationMG;
	});
}

nex::VisualizationSphere::~VisualizationSphere() = default;

void nex::VisualizationSphere::show(bool visible)
{
	// Skip if no state change
	if (mIsVisible == visible) return;

	mIsVisible = visible;
	auto lock = mScene->acquireLock();

	if (mIsVisible) {
		mScene->addActiveVobUnsafe(&mVisualizationVob);
	}
	else {
		mScene->removeActiveVobUnsafe(&mVisualizationVob);
	}
}

nex::Vob* nex::VisualizationSphere::getVob()
{
	return &mVisualizationVob;
}

const nex::Vob* nex::VisualizationSphere::getVob() const
{
	return &mVisualizationVob;
}