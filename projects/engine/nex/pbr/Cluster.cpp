#include <nex/pbr/Cluster.hpp>


nex::ProbeCluster::ProbeCluster()
{
}

nex::PerspectiveCamera& nex::ProbeCluster::getCamera()
{
	return mCamera;
}



nex::gui::ProbeClusterView::ProbeClusterView(std::string title, MainMenuBar* menuBar, Menu* menu, ProbeCluster* cluster) :
	MenuWindow(std::move(title), menuBar, menu), mCluster(cluster)
{

}

void nex::gui::ProbeClusterView::drawSelf()
{

	auto& camera = mCluster->getCamera();
	auto position = camera.getPosition();
	auto fovY = camera.getFovY();
	auto aspect = camera.getAspectRatio();
	auto near = camera.getNearDistance();
	auto far = camera.getFarDistance();

	
	if (ImGui::DragFloat3("Start position", (float*)& position)) {
		camera.setPosition(position, true);
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



	if (ImGui::Button("Create frustum")) {

	}
}