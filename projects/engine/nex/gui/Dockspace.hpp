#pragma once

#include <nex/gui/Drawable.hpp>

namespace nex::gui
{

	class Dockspace : public Drawable {
	public:
		Dockspace();

		void drawGUI() override;

	protected:
		void drawSelf() override;

		std::string mParentWindowID;
		bool mInit = true;
	};
}