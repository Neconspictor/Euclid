#include <gui/ParticleSystemGenerator.hpp>
#include <imgui/imgui.h>
#include "nex/gui/Util.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nfd/nfd.h>
#include <nex/platform/Window.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/gui/FileDialog.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <boxer/boxer.h>
#include <nex/particle/Particle.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/gui/VisualizationSphere.hpp>
#include <glm/glm.hpp>


nex::gui::ParticleSystemGenerator::ParticleSystemGenerator(nex::Scene* scene, VisualizationSphere* sphere, nex::Camera* camera) :
	mSphere(sphere),
	mCamera(camera),
	mScene(scene),
	mPlacementOffset(2.0f),
	mAverageLifeTime(1.0f),
	mAverageScale(1.0f),
	mAverageSpeed(1.0f),
	mBoundingBox({ {-0.3f, 0.0f, -0.3f}, {0.3f, 1.0f, 0.3f} }),
	mGravityInfluence(1.0f),
	mMaxParticles(1000),
	mPps(10.0f),
	mRotation(0.0f),
	mRandomizeRotation(false)
{
}

nex::gui::ParticleSystemGenerator::~ParticleSystemGenerator() = default;


void nex::gui::ParticleSystemGenerator::setScene(nex::Scene * scene)
{
	mScene = scene;
}

void nex::gui::ParticleSystemGenerator::setVisible(bool visible)
{
	bool oldVisibleState = isVisible();
	Drawable::setVisible(visible);
	mSphere->show(visible);

	if (visible && !oldVisibleState) {
		auto* vob = mSphere->getVob();
		vob->setPosition(mCamera->getPosition() + mPlacementOffset * mCamera->getLook());
		vob->setScale(glm::vec3(0.5f));
		vob->updateTrafo(true);
	}
}


void nex::gui::ParticleSystemGenerator::setCamera(nex::Camera* camera)
{
	mCamera = camera;
}

void nex::gui::ParticleSystemGenerator::drawSelf()
{
	auto* vob = mSphere->getVob();
	auto position = vob->getPosition();

	ImGui::DragFloat3("Position", (float*)&position, 0.0f, 0.0f, 0.0f, "%.5f");

	ImGui::DragFloat("Average lifetime", &mAverageLifeTime, 0.01f, 0.0f, FLT_MAX, "%.5f", 1.0f);
	ImGui::DragFloat("Average scale", &mAverageScale, 0.01f, 0.0f, FLT_MAX, "%.5f", 1.0f);
	ImGui::DragFloat("Average speed", &mAverageSpeed, 0.01f, 0.0f, FLT_MAX, "%.5f", 1.0f);
	//mBoundingBox
	ImGui::DragFloat("Gravity influence", (float*)&mGravityInfluence, 0.01f, 0.0f, 1.0f, "%.5f", 1.0f);
	ImGui::DragInt("Max. emitted particles", &mMaxParticles, 1.0f, 0, MAXINT);
	ImGui::DragFloat("Emission rate", &mPps, 1.0f, 0.0f, FLT_MAX, "%.5f", 1.0f);
	ImGui::DragFloat("rotation (degrees)", &mRotation, 0.01f, 0.0f, 0.0f, "%.5f", 1.0f);

	ImGui::Checkbox("randomize rotation", &mRandomizeRotation);

	ImGui::Dummy(ImVec2(0, 10));
	nex::gui::Separator(2.0f);
	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Text("Placement related configuration:");
	ImGui::DragFloat("Camera look offset", (float*)&mPlacementOffset, 0.1f, 0.0f, 0.0f, "%.5f");

	if (ImGui::Button("Place in front of camera")) {
		position = mCamera->getPosition() + mPlacementOffset * mCamera->getLook();
	}

	nex::gui::Separator(2.0f);

	vob->setPosition(position);
	vob->updateTrafo(true);
}