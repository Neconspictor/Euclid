#include <nex/gui/vob/OceanVobView.hpp>
#include <imgui/imgui.h>
#include <nex/scene/Vob.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/gui/Picker.hpp>
#include <nex/water/Ocean.hpp>

namespace nex::gui
{
	OceanVobView::OceanVobView() : VobView()
	{
		mIconDesc.texture = TextureManager::get()->getImage("_intern/icon/icon_ocean.png");
		mCenterIconHeight = true;
		mIconTintColor = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	}

	bool OceanVobView::draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges)
	{
		if (!VobView::draw(vob, scene, picker, camera, doOneTimeChanges))
			return false;

		auto* oceanVob = dynamic_cast<OceanVob*>(vob);
		
		if (!oceanVob || !oceanVob->getOcean()) {
			return true;
		}

		auto* ocean = oceanVob->getOcean();


		auto murk = ocean->getMurk();
		if (ImGui::InputFloat("Murk", &murk)) {
			ocean->setMurk(murk);
		}

		auto roughness = ocean->getRoughness();
		if (ImGui::InputFloat("Roughness", &roughness)) {
			ocean->setRoughness(roughness);
		}


		auto tileCount = ocean->getTileCount();

		if (ImGui::InputScalarN("tile count (x-z)", ImGuiDataType_U32, &tileCount, 2)) {
			ocean->setTileCount(tileCount);
			oceanVob->recalculateLocalBoundingBox();
		}

		ImGui::Checkbox("Draw wireframe", ocean->getWireframeState());

		bool usePSSR = ocean->isPSSRUsed();
		if (ImGui::Checkbox("Use PSSR", &usePSSR)) {
			ocean->usePSSR(usePSSR);
		}

		return true;
	
	}
}