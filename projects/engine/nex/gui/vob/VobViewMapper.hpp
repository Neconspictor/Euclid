#pragma once

#include <nex/gui/vob/VobView.hpp>


namespace nex
{
	class Vob;
}

namespace nex::gui
{

	class Picker;

	class VobViewMapper {
	public:
		static nex::gui::VobView* getViewByVob(Vob* vob);
	};
}