#include <nex/pbr/Cluster.hpp>
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/material/Material.hpp>
#include <nex/shader/Technique.hpp>
#include <nex/shader/SimpleColorPass.hpp>
#include <nex/Scene.hpp>


nex::ProbeCluster::ProbeCluster(Scene* scene) : mScene(scene), mLastGenerated(nullptr)
{
}

nex::PerspectiveCamera& nex::ProbeCluster::getCamera()
{
	return mCamera;
}

void nex::ProbeCluster::generate()
{
	mCamera.update();
	const auto& frustum = mCamera.getFrustumWorld();

	auto mesh = std::make_unique<FrustumMesh>(frustum);

	auto container = std::make_unique<StaticMeshContainer>();

	thread_local auto pass = std::make_unique<SimpleColorPass>();
		pass->bind();
		pass->setColor(glm::vec4(2.0f, 0.0f, 0.0f, 1.0f));
	thread_local auto technique = std::make_unique<Technique>(pass.get());

	auto material = std::make_unique<Material>(technique.get());
	auto& state = material->getRenderState();
	state.fillMode = FillMode::LINE;
	state.doCullFaces = false;
	state.isTool = true;
	state.doShadowCast = false;
	state.doShadowReceive = false;

	container->add(std::move(mesh), std::move(material));
	container->finalize();

	mScene->acquireLock();
	mLastGenerated = mScene->addVobUnsafe(std::make_unique<MeshOwningVob>(std::move(container)), true);
}

void nex::ProbeCluster::deleteLastGenerated()
{
	if (mLastGenerated == nullptr) return;

	mScene->acquireLock();
	mScene->removeActiveVobUnsafe(mLastGenerated);
	auto& vobs = mScene->getVobsUnsafe();
	vobs.erase(std::remove_if(vobs.begin(), vobs.end(), [&](auto& vob) {
		return vob.get() == mLastGenerated;
	}));

	mLastGenerated = nullptr;
}



nex::gui::ProbeClusterView::ProbeClusterView(std::string title, MainMenuBar* menuBar, Menu* menu, ProbeCluster* cluster, PerspectiveCamera* activeCamera) :
	MenuWindow(std::move(title), menuBar, menu), mCluster(cluster), mActiveCamera(activeCamera)
{

}

void nex::gui::ProbeClusterView::drawSelf()
{

	auto& camera = mCluster->getCamera();
	auto position = camera.getPosition();
	auto fovY = glm::degrees(camera.getFovY());
	auto aspect = camera.getAspectRatio();
	auto zNear = camera.getNearDistance();
	auto zFar = camera.getFarDistance();
	auto look = camera.getLook();
	auto up = camera.getUp();

	
	if (ImGui::DragFloat3("Start position", (float*)& position)) {
		camera.setPosition(position, true);
	}

	if (ImGui::DragFloat3("Look direction", (float*)& look, 0.1f, -1.0f, 1.0f)) {
		camera.setLook(look);
	}

	if (ImGui::DragFloat3("Up direction", (float*)& up, 0.1f, -1.0f, 1.0f)) {
		camera.setUp(up);
	}
	
	if (ImGui::DragFloat("Vertical FOV", &fovY)) {
		camera.setFovY(glm::radians(fovY));
	}

	if (ImGui::DragFloat("Aspect ratio", &aspect)) {
		camera.setAspectRatio(aspect);
	}

	if (ImGui::DragFloat("Near plane", &zNear, 0.1f, 0.0f, FLT_MAX)) {
		camera.setNearDistance(zNear);
	}

	if (ImGui::DragFloat("Far plane", &zFar, 0.1f, 0.0f, FLT_MAX)) {
		camera.setFarDistance(zFar);
	}

	if (ImGui::Button("Align to active camera")) {
		camera.setPosition(mActiveCamera->getPosition(), true);
		camera.setFovY(mActiveCamera->getFovY());
		camera.setAspectRatio(mActiveCamera->getAspectRatio());
		camera.setNearDistance(mActiveCamera->getNearDistance());
		camera.setFarDistance(mActiveCamera->getFarDistance());
		camera.setLook(mActiveCamera->getLook());
		camera.setUp(mActiveCamera->getUp());
	}



	if (ImGui::Button("Create frustum")) {
		mCluster->generate();
	}

	if (ImGui::Button("Delete last generated")) {
		mCluster->deleteLastGenerated();
	}
}