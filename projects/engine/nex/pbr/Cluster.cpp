#include <nex/pbr/Cluster.hpp>
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/material/Material.hpp>
#include <nex/shader/Technique.hpp>
#include <nex/shader/SimpleColorPass.hpp>
#include <nex/Scene.hpp>
#include <nex/gui/Util.hpp>


nex::ProbeCluster::ProbeCluster(Scene* scene) : mScene(scene), 
mPass(std::make_unique<SimpleColorPass>()),
mTechnique(std::make_unique<Technique>(mPass.get())), 
mMaterial(std::make_unique<Material>(mTechnique.get()))
{
	mPass->bind();
	mPass->setColor(glm::vec4(2.0f, 0.0f, 0.0f, 1.0f));

	auto& state = mMaterial->getRenderState();
	state.fillMode = FillMode::LINE;
	state.doCullFaces = false;
	state.isTool = true;
	state.doShadowCast = false;
	state.doShadowReceive = false;
}

nex::ProbeCluster::~ProbeCluster() = default;

nex::PerspectiveCamera& nex::ProbeCluster::getCamera()
{
	return mCamera;
}

void nex::ProbeCluster::generate(const Frustum& frustum)
{
	auto mesh = std::make_unique<FrustumMesh>(frustum);

	auto container = std::make_unique<StaticMeshContainer>();

	container->addMapping(mesh.get(), mMaterial.get());
	container->add(std::move(mesh));
	container->finalize();

	mScene->acquireLock();
	mScene->addVobUnsafe(std::make_unique<MeshOwningVob>(std::move(container)), true);
}

void nex::ProbeCluster::generateClusterElement(const ClusterElement& elem)
{
	mCamera.update();
	auto frustumView = mCamera.calcClusterElementViewSpace(elem.xOffset, elem.yOffset, elem.zOffset, elem.depth, elem.width, elem.height);
	const auto viewInv = inverse(mCamera.getView());
	const auto frustumWorld = frustumView * viewInv;
	generate(frustumWorld);

}

void nex::ProbeCluster::generateCluster(const ClusterSize& clusterSize)
{
	mCamera.update();
	ClusterElement elem;
	auto container = std::make_unique<StaticMeshContainer>();

	elem.width = 1.0f / (float) clusterSize.xSize;
	elem.height = 1.0f / (float) clusterSize.ySize;
	elem.depth = 1.0f / (float) clusterSize.zSize;

	for (size_t x = 0; x < clusterSize.xSize; ++x ) {
		for (size_t y = 0; y < clusterSize.ySize; ++y) {
			for (size_t z = 0; z < clusterSize.zSize; ++z) {

				elem.xOffset = x * elem.width;
				elem.yOffset = y * elem.height;
				elem.zOffset = z * elem.depth;

				auto frustumView = mCamera.calcClusterElementViewSpace(elem.xOffset, elem.yOffset, elem.zOffset, elem.depth, elem.width, elem.height);
				const auto viewInv = inverse(mCamera.getView());
				const auto frustumWorld = frustumView * viewInv;
				auto mesh = std::make_unique<FrustumMesh>(frustumWorld);

				container->addMapping(mesh.get(), mMaterial.get());
				container->add(std::move(mesh));
			}
		}
	}

	container->finalize();
	container->merge();
	mScene->acquireLock();
	mScene->addVobUnsafe(std::make_unique<MeshOwningVob>(std::move(container)), true);

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
		camera.update();
		mCluster->generate(camera.getFrustumWorld());
	}

	ImGui::Dummy(ImVec2(0, 10));
	nex::gui::Separator(2.0f);
	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Text("Cluster element:");


	ImGui::DragFloat("X offset", &mClusterElement.xOffset, 0.1f, 0.0f, 1.0f, "%.5f");
	ImGui::DragFloat("Y offset", &mClusterElement.yOffset, 0.1f, 0.0f, 1.0f, "%.5f");
	ImGui::DragFloat("Z offset", &mClusterElement.zOffset, 0.1f, 0.0f, 1.0f, "%.5f");

	ImGui::DragFloat("width", &mClusterElement.width, 0.1f, 0.0f, 1.0f, "%.5f");
	ImGui::DragFloat("height", &mClusterElement.height, 0.1f, 0.0f, 1.0f, "%.5f");
	ImGui::DragFloat("depth", &mClusterElement.depth, 0.1f, 0.0f, 1.0f, "%.5f");

	if (ImGui::Button("Create cluster element")) {
		mCluster->generateClusterElement(mClusterElement);
	}

	ImGui::Dummy(ImVec2(0, 10));
	nex::gui::Separator(2.0f);
	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Text("Cluster:");


	ImGui::DragScalar("X size", ImGuiDataType_U64 , &mClusterSize.xSize, 1.0f);
	ImGui::DragScalar("Y size", ImGuiDataType_U64, &mClusterSize.ySize, 1.0f);
	ImGui::DragScalar("Z size", ImGuiDataType_U64, &mClusterSize.zSize, 1.0f);

	if (ImGui::Button("Create cluster")) {
		mCluster->generateCluster(mClusterSize);
	}
}