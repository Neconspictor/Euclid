#include <nex/gui/vob/PbrProbeVobView.hpp>
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
#include <nex/scene/Vob.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/gui/Picker.hpp>
#include <nex/pbr/PbrProbe.hpp>

namespace nex::gui
{
	PbrProbeVobView::PbrProbeVobView() : VobView(),
		mBrdfView({}, ImVec2(256, 256)),
		mConvolutedView({}, ImVec2(256, 256)),
		mPrefilteredView({}, ImVec2(256, 256))
	{
	}

	void PbrProbeVobView::draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges)
	{
		VobView::draw(vob, scene, picker, camera, doOneTimeChanges);

		auto* probeVob = dynamic_cast<ProbeVob*>(vob);
		if (!probeVob) {
			return;
		}

		auto* probe = probeVob->getProbe();

		ImGui::Text("pbr probe vob");

		if (doOneTimeChanges) {
			auto& irradiance = mConvolutedView.getTextureDesc();
			irradiance.level = probe->getArrayIndex();

			auto& probePrefiltered = mPrefilteredView.getTextureDesc();
			probePrefiltered.level = probe->getArrayIndex();
		}

		if (ImGui::TreeNode("Brdf Lookup map"))
		{
			auto* texture = probe->getBrdfLookupTexture();
			auto& probePrefiltered = mBrdfView.getTextureDesc();
			probePrefiltered.texture = texture;
			probePrefiltered.flipY = ImageFactory::isYFlipped();
			probePrefiltered.sampler = nullptr;


			mBrdfView.updateTexture(true);
			mBrdfView.drawGUI();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Convoluted map"))
		{
			auto* texture = probe->getIrradianceMaps();
			auto& irradiance = mConvolutedView.getTextureDesc();
			irradiance.texture = texture;
			irradiance.flipY = ImageFactory::isYFlipped();
			irradiance.sampler = nullptr;

			mConvolutedView.updateTexture(true);
			mConvolutedView.drawGUI();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Prefiltered map"))
		{
			auto* texture = probe->getPrefilteredMaps();
			auto& probePrefiltered = mPrefilteredView.getTextureDesc();
			probePrefiltered.texture = texture;
			probePrefiltered.flipY = ImageFactory::isYFlipped();
			probePrefiltered.sampler = nullptr;

			mPrefilteredView.updateTexture(true);
			mPrefilteredView.drawGUI();

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
	}
}