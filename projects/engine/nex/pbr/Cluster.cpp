#include <nex/pbr/Cluster.hpp>


nex::ProbeCluster::ProbeCluster()
{
}

nex::PerspectiveCamera& nex::ProbeCluster::getCamera()
{
	return mCamera;
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
	auto near = camera.getNearDistance();
	auto far = camera.getFarDistance();
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

	if (ImGui::DragFloat("Near plane", &near, 0.1f, 0.0f, FLT_MAX)) {
		camera.setNearDistance(near);
	}

	if (ImGui::DragFloat("Far plane", &far, 0.1f, 0.0f, FLT_MAX)) {
		camera.setFarDistance(far);
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

	}
}