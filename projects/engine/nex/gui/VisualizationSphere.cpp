#include <nex/gui/VisualizationSphere.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/renderer/RenderEngine.hpp>
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/effects/EffectLibrary.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/shader/ShaderProvider.hpp>

nex::VisualizationSphere::VisualizationSphere(Scene* scene) : 
	mScene(scene),
	mVisualizationVob(),
	mIsVisible(false)
{

	nex::ResourceLoader::get()->enqueue<nex::Resource*>([=]()->nex::Resource* {
		nex::RenderEngine::getCommandQueue()->push([=]() {

			auto material = nex::RenderBackend::get()->getEffectLibrary()->createSimpleColorMaterial();
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
			mVisualizationVob.setMeshGroup(nex::make_not_owning(&mVisualizationMG));
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

bool nex::VisualizationSphere::isVisible() const
{
	return mIsVisible;
}
