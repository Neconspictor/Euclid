#pragma once

#include <nex/gui/vob/VobView.hpp>


namespace nex
{
	class Vob;
	class ProbeManager;
}

namespace nex::gui
{

	class Picker;
	class ProbeVobView;

	class VobViewMapper {
	public:

		static void init(ProbeManager* probeManager);
		static nex::gui::VobView* getViewByVob(Vob* vob);
	private:
		static std::unique_ptr<ProbeVobView> mProbeVobView;

	};
}