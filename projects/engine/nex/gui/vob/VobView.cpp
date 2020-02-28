#include <nex/gui/vob/VobView.hpp>
#include <imgui/imgui.h>
#include "nex/gui/ImGUI_Extension.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nfd/nfd.h>
#include <nex/platform/Window.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/gui/FileDialog.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <boxer/boxer.h>
#include <nex/scene/Vob.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/gui/Picker.hpp>
#include <nex/camera/Camera.hpp>
#include <list>
#include <imgui/imgui_internal.h>
#include <glm/glm.hpp>
#include <nex/anim/AnimationManager.hpp>

namespace nex::gui
{
	VobView::VobView(nex::Window* window) : mWindow(window)
	{
		mIconDesc.texture = TextureManager::get()->getImage("textures/_intern/icon/icon_triangle_mesh.png");
	}
	bool VobView::draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges)
	{
		ImGui::Text("Type: "); ImGui::SameLine();
		ImGui::Text(vob->getTypeName().c_str());
		if (hasIcon()) {
			ImGui::SameLine();

			auto* window = ImGui::GetCurrentWindow();
			window->DC.CurrLineSize.y *= 1.5;

			drawIcon();
		}

		if (ImGui::Button("Jump to vob")) {
			camera->setPosition(glm::vec3(vob->getTrafoMeshToWorld()[3]), true);
		}

		if (vob->isDeletable() && ImGui::Button("Delete Vob")) {
			scene->acquireLock();
			
			auto managedVob = scene->deleteVobUnsafe(vob);
			picker->deselect(*scene);
			return false;
		}

		bool inheritParentScale = vob->isParentScaleInherited();
		if (ImGui::Checkbox("Inherit parent scale", &inheritParentScale)) {
			vob->inheritParentScale(inheritParentScale);
			//vob->updateTrafo();
		}


		glm::vec3 position = vob->getPositionLocalToWorld();
		if (nex::gui::Vector3D(&position, "Position")) {
			vob->setPositionLocalToWorld(position);
		}
		

		glm::quat rotation = vob->getRotationLocalToParent();
		nex::gui::Quat(&rotation, "Orientation (Quaternion) - Radians");
		rotation = normalize(rotation);
		glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation));

		if (euler.x == -180.0f) euler.x = 180.0f;
		if (euler.z == -180.0f) euler.z = 180.0f;

		if (nex::gui::Vector3D(&euler, "Orientation (Euler X-Y-Z) - Degrees")) {
			euler.y = std::clamp(euler.y, -89.0f, 89.0f);
			vob->setRotationLocalToParent(radians(euler));
			vob->updateTrafo();
		}

		

		vob->getName().reserve(256);
		if (ImGui::InputText("Name", vob->getName().data(), vob->getName().capacity())) {
		
		}

		euler = glm::vec3(0.0f);
		if (nex::gui::Vector3D(&euler, "Rotate (Euler X-Y-Z) - Local - Degrees")) {
			vob->rotateLocal(radians(euler));
			vob->updateTrafo();
		}

		euler = glm::vec3(0.0f);
		if (nex::gui::Vector3D(&euler, "Rotate (Euler X-Y-Z) - Global - Degrees")) {
			vob->rotateGlobal(radians(euler));
			vob->updateTrafo();
		}


		glm::vec3 scale = vob->getScaleLocalToParent();
		if (nex::gui::Vector3D(&scale, "Scale", 0.1f)) {
			scale = maxVec(scale, glm::vec3(0.0f));
			vob->setScaleLocalToParent(scale);
			vob->updateTrafo();
		}


		if (vob->usesPerObjectMaterialData()) {

			nex::gui::Separator(2.0f);

			ImGui::Text("Material Data:");

			auto& data = vob->getPerObjectMaterialData();
			
			if (ImGui::DragFloat("Emission strength", &data.emissionStrength, 0.1f, 0.0f, 10.0f)) {
				data.emissionStrength = std::max<float>(data.emissionStrength, 0.0f);
			}

			ImGui::Checkbox("Probe lighting", (bool*)&data.probesUsed);
			ImGui::DragFloat("Probe lighting influence", &data.probeInfluence, 0.0f, 0.0f, 1.0f, "%.4f", 0.0f);
			ImGui::Checkbox("Cone tracing", (bool*)&data.coneTracingUsed);
			ImGui::DragFloat("Cone tracing influence", &data.coneTracingInfluence, 0.0f, 0.0f, 1.0f, "%.4f", 0.0f);

			if (ImGui::TreeNode("Probe specular irradiance weights")) {


				std::stringstream ss;
				
				for (int i = 0; i < 4; ++i) {
					ss << "id: " << data.specularReflectionIds[i] << "\tweight: " << data.specularReflectionWeights[i] << "\n";
				}
				

				ImGui::Text(ss.str().c_str());

				ImGui::TreePop();
			}

			nex::gui::Separator(2.0f);
			drawKeyFrameAni(vob);
			nex::gui::Separator(2.0f);
		}

		return true;
	}
	void VobView::drawIcon()
	{
		auto& g = *GImGui;
		auto* window = ImGui::GetCurrentWindow();
		auto height = window->DC.CurrLineSize.y;

		//if (mCenterIconHeight)	window->DC.CursorPos.y += height / 2;
		ImGui::Image((void*)&mIconDesc, ImVec2(height, height), ImVec2(0, 0), ImVec2(1, 1), mIconTintColor);
		//if (mCenterIconHeight) window->DC.CursorPos.y -= height / 2;
	}

	bool VobView::hasIcon() const
	{
		return mIconDesc.texture != nullptr;
	}
	const ImGUI_TextureDesc& VobView::getIconDesc() const
	{
		return mIconDesc;
	}
	bool VobView::centerIconHeight() const
	{
		return mCenterIconHeight;
	}
	const ImVec4& VobView::getIconTintColor() const
	{
		return mIconTintColor;
	}


	void nex::gui::VobView::drawKeyFrameAni(nex::Vob* vob)
	{
		// We need a blue print
		if (!vob->getBluePrint()) return;


		int activeAnimatinIndex = -1;
		std::vector<const char*> animationNames;
		std::vector<KeyFrameAnimation> animations;

		if (ImGui::Combo("Active animation", &activeAnimatinIndex, animationNames.data(), animationNames.size())) {
			if (activeAnimatinIndex >= 0) {
				const auto* newAni = &animations[activeAnimatinIndex];
				vob->setActiveKeyFrameAnimation(SID(newAni->getName()));
			}
		}

		ImGui::SameLine();


		if (ImGui::Button("Load Keyframe Animation")) {
			mKeyFrameAniFuture = loadKeyFrameAnimation();
		}
	}

	nex::Future<nex::Resource*> nex::gui::VobView::loadKeyFrameAnimation()
	{
		return nex::ResourceLoader::get()->enqueue<nex::Resource*>([=]()->nex::Resource* {
			nex::gui::FileDialog fileDialog(mWindow);
			auto result = fileDialog.selectFile("gltf,glb,md5anim");

			if (result.state == FileDialog::State::Okay) {

				try {
					auto* manager = AnimationManager::get();
					auto anis = manager->loadKeyFrameAnimations(result.path.u8string());
					std::unordered_map<nex::Sid, std::unique_ptr<KeyFrameAnimation>> aniMap;
					for (auto& ani : anis) {

						ani->getTicksPerSecond();

						aniMap[SID(ani->getName())] = std::move(ani);
					}
				} 
				catch (const std::exception & e) {

					nex::ExceptionHandling::logExceptionWithStackTrace(Logger("VobView::loadKeyFrameAnimation"), e);

					void* nativeWindow = mWindow->getNativeWindow();
					boxer::show("Couldn't load keyframe animation!", "", boxer::Style::Error, boxer::Buttons::OK, nativeWindow);
				}

				catch (...) {
					void* nativeWindow = mWindow->getNativeWindow();
					boxer::show("Couldn't load keyframe animation!", "", boxer::Style::Error, boxer::Buttons::OK, nativeWindow);
				}
			}

			return nullptr;
			});
	}
}