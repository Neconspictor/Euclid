#pragma once

#include <nex/gui/vob/VobView.hpp>


namespace nex
{
	class Vob;
	class ProbeManager;
	class Window;
}

namespace nex::gui
{

	class Picker;
	class ProbeVobView;

	class VobViewMapper {
	public:

		static void init(ProbeManager* probeManager, nex::Window* window);
		static nex::gui::VobView* getViewByVob(Vob* vob);
	private:
		static std::vector<std::unique_ptr<VobView>> mViews;

	};
}