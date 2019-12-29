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
	}

	void OceanVobView::draw(Vob* vob, Scene* scene, Picker* picker, bool doOneTimeChanges)
	{
		auto* oceanVob = dynamic_cast<OceanVob*>(vob);
		
		if (!oceanVob || !oceanVob->getOcean()) {
			VobView::draw(vob, scene, picker, doOneTimeChanges);
			return;
		}

		auto* ocean = oceanVob->getOcean();

		auto tileCount = ocean->getTileCount();

		if (ImGui::InputScalarN("tile count (x-z)", ImGuiDataType_U32, &tileCount, 2)) {
			ocean->setTileCount(tileCount);
			oceanVob->recalculateLocalBoundingBox();
		}
		

		VobView::draw(vob, scene, picker, doOneTimeChanges);
	}
}