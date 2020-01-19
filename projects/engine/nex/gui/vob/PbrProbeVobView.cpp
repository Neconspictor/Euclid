#include <nex/gui/vob/PbrProbeVobView.hpp>
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
#include <nex/GI/PbrProbe.hpp>
#include <nex/GI/ProbeManager.hpp>

namespace nex::gui
{
	PbrProbeVobView::PbrProbeVobView(ProbeManager* probeManager) : VobView(),
		mBrdfView({}, ImVec2(256, 256)),
		mIrradianceView({}, ImVec2(256, 256)),
		mReflectionView({}, ImVec2(256, 256)),
		mProbeManager(probeManager)
	{
	}

	bool PbrProbeVobView::draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges)
	{
		if (!VobView::draw(vob, scene, picker, camera, doOneTimeChanges)) {
			return false;
		}

		auto* probeVob = dynamic_cast<ProbeVob*>(vob);
		if (!probeVob) {
			return true;
		}

		auto* probe = probeVob->getProbe();

		ImGui::Text("pbr probe vob");

		if (doOneTimeChanges) {
			auto& irradiance = mIrradianceView.getTextureDesc();
			irradiance.level = probe->getArrayIndex();

			auto& probePrefiltered = mReflectionView.getTextureDesc();
			probePrefiltered.level = probe->getArrayIndex();
		}

		if (ImGui::TreeNode("Brdf Lookup map"))
		{
			auto* texture = PbrProbeFactory::getBrdfLookupTexture();
			auto& probePrefiltered = mBrdfView.getTextureDesc();
			probePrefiltered.texture = texture;
			probePrefiltered.flipY = ImageFactory::isYFlipped();
			probePrefiltered.sampler = nullptr;


			mBrdfView.updateTexture(true);
			mBrdfView.drawGUI();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Irradiance map"))
		{
			auto* texture = mProbeManager->getIrradianceMaps();
			auto& irradiance = mIrradianceView.getTextureDesc();
			irradiance.texture = texture;
			irradiance.flipY = ImageFactory::isYFlipped();
			irradiance.sampler = nullptr;

			mIrradianceView.updateTexture(true);
			mIrradianceView.drawGUI();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Reflection map"))
		{
			auto* texture = mProbeManager->getReflectionMaps();
			auto& reflectionProbe = mReflectionView.getTextureDesc();
			reflectionProbe.texture = texture;
			reflectionProbe.flipY = ImageFactory::isYFlipped();
			reflectionProbe.sampler = nullptr;

			mReflectionView.updateTexture(true);
			mReflectionView.drawGUI();

			ImGui::TreePop();
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


		if (influenceType == PbrProbe::InfluenceType::SPHERE) {
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