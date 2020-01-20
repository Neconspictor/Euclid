#include <nex/gui/vob/ProbeVobView.hpp>
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
#include <nex/GI/Probe.hpp>
#include <nex/GI/ProbeManager.hpp>

namespace nex::gui
{
	ProbeVobView::ProbeVobView(ProbeManager* probeManager) : VobView(),
		mBrdfView({}, ImVec2(256, 256)),
		mIrradianceView({}, ImVec2(256, 256)),
		mReflectionView({}, ImVec2(256, 256)),
		mProbeManager(probeManager)
	{
	}

	bool ProbeVobView::draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges)
	{
		if (!VobView::draw(vob, scene, picker, camera, doOneTimeChanges)) {
			return false;
		}

		auto* probeVob = dynamic_cast<ProbeVob*>(vob);
		if (!probeVob) {
			return true;
		}

		auto* probe = probeVob->getProbe();
		const auto isIrrdadianceProbe = probe->getType() == Probe::Type::Irradiance;
		auto* factory = mProbeManager->getFactory();

		if (isIrrdadianceProbe) {
			ImGui::Text("Irradiance probe");
		}
		else {
			ImGui::Text("Reflection probe");
		}

		

		if (doOneTimeChanges) {
			auto& irradianceDesc = mIrradianceView.getTextureDesc();
			irradianceDesc.level = probe->getArrayIndex();

			auto& reflectionDesc = mReflectionView.getTextureDesc();
			reflectionDesc.level = probe->getArrayIndex();
		}

		if (ImGui::TreeNode("Brdf Lookup map"))
		{
			auto* texture = ProbeFactory::getBrdfLookupTexture();
			auto& lutDesc = mBrdfView.getTextureDesc();
			lutDesc.texture = texture;
			lutDesc.flipY = ImageFactory::isYFlipped();
			lutDesc.sampler = nullptr;


			mBrdfView.updateTexture(true);
			mBrdfView.drawGUI();

			ImGui::TreePop();
		}

		if (isIrrdadianceProbe) {
			if (ImGui::TreeNode("Irradiance map"))
			{
				auto* texture = factory->getIrradianceMaps();
				auto& irradianceDesc = mIrradianceView.getTextureDesc();
				irradianceDesc.texture = texture;
				irradianceDesc.flipY = ImageFactory::isYFlipped();
				irradianceDesc.sampler = nullptr;

				mIrradianceView.updateTexture(true);
				mIrradianceView.drawGUI();

				ImGui::TreePop();
			}
		}
		else {

			if (ImGui::TreeNode("Reflection map"))
			{
				auto* texture = factory->getReflectionMaps();
				auto& reflectionDesc = mReflectionView.getTextureDesc();
				reflectionDesc.texture = texture;
				reflectionDesc.flipY = ImageFactory::isYFlipped();
				reflectionDesc.sampler = nullptr;

				mReflectionView.updateTexture(true);
				mReflectionView.drawGUI();

				ImGui::TreePop();
			}
		}


		auto influenceType = probe->getInfluenceType();

		const char* items[2] = {
			"SPHERE",
			"BOX"
		};

		if (ImGui::Combo("Influence type", (int*)&influenceType, items, 2)) {
			probe->setInfluenceType(influenceType);
			picker->select(*scene, probeVob);
		}

		auto position = probe->getPosition();
		if (nex::gui::Vector3D(&position, "Influence center"))
			probe->setPosition(position);


		if (influenceType == Probe::InfluenceType::SPHERE) {
			auto radius = probe->getInfluenceRadius();
			if (ImGui::DragFloat("Influence radius", &radius, 0.1f, 0.0f, FLT_MAX)) {
				probe->setInfluenceRadius(radius);
			}
		}
		else {

			AABB2 box2(probe->getInfluenceBox());
			if (nex::gui::Vector3D(&box2.halfWidth, "Influence bounding box half width"))
				probe->setInfluenceBox(box2.halfWidth);
		}

		return true;
	}
}