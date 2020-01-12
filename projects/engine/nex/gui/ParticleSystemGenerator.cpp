#include <nex/gui/ParticleSystemGenerator.hpp>
#include <imgui/imgui.h>
#include "nex/gui/ImGUI_Extension.hpp"
#include <glm/gtc/matrix_transform.hpp>
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
#include <memory>
#include <nex/particle/Particle.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/shader/ShaderProvider.hpp>

#undef max


nex::gui::ParticleSystemGenerator::ParticleSystemGenerator(nex::Scene* scene, VisualizationSphere* sphere, nex::Camera* camera, 
	nex::Window* window,
	nex::ParticleShader* shader) :
	Drawable(false),
	mTextureViewer(glm::vec2(128), "Select Texture", window),
	mSphere(sphere),
	mCamera(camera),
	mScene(scene),
	mPlacementOffset(2.0f),
	mAverageLifeTime(4.0f),
	mAverageScale(1.0f),
	mAverageSpeed(0.4f),
	mBoundingBox({ {-0.3f, 0.0f, -0.3f}, {0.3f, 1.0f, 0.3f} }),
	mGravityInfluence(0.0f),
	mMaxParticles(1000),
	mPps(70.0f),
	mRotation(0.0f),
	mRandomizeRotation(false),
	mShowPlacementHelper(true),
	mSortParticles(false),
	mBlendingMode(Additive),
	mShader(shader)
{
	mTextureViewer.getTextureView().showAllOptions(false);
}

nex::gui::ParticleSystemGenerator::~ParticleSystemGenerator() = default;


void nex::gui::ParticleSystemGenerator::setScene(nex::Scene * scene)
{
	mScene = scene;
}

void nex::gui::ParticleSystemGenerator::setVisible(bool visible, bool recursive)
{
	bool oldVisibleState = isVisible();
	Drawable::setVisible(visible, recursive);

	auto stateChange = visible != oldVisibleState;

	if (stateChange && !oldVisibleState) {
		auto* vob = mSphere->getVob();
		vob->setPositionLocal(mCamera->getPosition() + mPlacementOffset * mCamera->getLook());
		vob->setScaleLocal(glm::vec3(0.5f));
		vob->updateTrafo(true);
	}

	if (stateChange) {

		if (visible) mSphere->show(mShowPlacementHelper);
		else mSphere->show(false);
	}
}


void nex::gui::ParticleSystemGenerator::setCamera(nex::Camera* camera)
{
	mCamera = camera;
}

void nex::gui::ParticleSystemGenerator::createParticleSystem(const glm::vec3& position, Texture* texture)
{
	// particle system
	AABB boundingBox = { glm::vec3(-0.3f, 0.0f, -0.3f), glm::vec3(0.3f, 1.0f, 0.3f) };
	auto shaderProvider = std::make_shared<ShaderProvider>(mShader);
	auto particleMaterial = std::make_unique<ParticleShader::Material>(shaderProvider);
	ParticleRenderer::createParticleMaterial(particleMaterial.get());
	particleMaterial->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	particleMaterial->texture = texture;

	if (mBlendingMode == Transparency)
		particleMaterial->getRenderState().blendDesc = BlendDesc::createAlphaTransparency();

	auto particleSystem = std::make_unique<VarianceParticleSystem>(
		mAverageLifeTime, 
		mAverageScale, 
		mAverageSpeed, 
		boundingBox, 
		mGravityInfluence, 
		std::move(particleMaterial), //material
		mMaxParticles,
		position,
		mPps,
		mRotation,
		mRandomizeRotation,
		mSortParticles
		);

	particleSystem->setDirection(glm::vec3(0, 1, 0), PI / 16.0f);

	//particleSystem.setScaleVariance(0.015f);
	//particleSystem.setSpeedVariance(0.025f);
	//particleSystem.setLifeVariance(0.0125f);

	auto lock = mScene->acquireLock();
	mScene->addVobUnsafe(std::move(particleSystem));
}

void nex::gui::ParticleSystemGenerator::drawSelf()
{
	auto* vob = mSphere->getVob();
	auto position = vob->getPositionLocal();

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


	static const char* blendingModes [2] = { "Additive","Transparency" };

	ImGui::Combo("Blending Mode", static_cast<int*>(&mBlendingMode), blendingModes, 2);
	ImGui::Checkbox("Sort particles", &mSortParticles);

	mTextureViewer.drawGUI();

	ImGui::Dummy(ImVec2(0, 10));
	nex::gui::Separator(2.0f);
	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Text("Placement related configuration:");
	ImGui::DragFloat("Camera look offset", (float*)&mPlacementOffset, 0.1f, 0.0f, 0.0f, "%.5f");

	if (ImGui::Checkbox("Show placement helper", &mShowPlacementHelper)) {
		mSphere->show(mShowPlacementHelper);
	}

	if (ImGui::Button("Place in front of camera")) {
		position = mCamera->getPosition() + mPlacementOffset * mCamera->getLook();
	}

	vob->setPositionLocal(position);
	vob->updateTrafo(true);

	nex::gui::Separator(2.0f);

	auto& future = mTextureViewer.getTextureFuture();
	

	if (ImGui::Button("Create") && future.is_ready()) {
		createParticleSystem(position, reinterpret_cast<Texture*>(future.get()));
	}
}

void nex::gui::ParticleSystemGenerator::onCanvasResizeSelf(unsigned width, unsigned height)
{
	//LOG(Logger("ParticleSystemGenerator::onCanvasResizeSelf"), Info) << "Called!";
	mCanvasSize.x = width;
	mCanvasSize.y = height;

	auto maxSide = std::max<unsigned>(width, height);
	maxSide = maxSide * 0.1;


	float size = (float)std::max<unsigned>(maxSide, 64);

	//mTextureView.setViewSize({ size ,size });
}